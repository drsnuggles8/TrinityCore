#include "LaunchSim.h"

#include "BotCharacterFactory.h"
#include "BotComposition.h"
#include "BotAccountMgr.h"
#include "BotIdentityRegistry.h"
#include "LaunchRoutes.h"
#include "Bot/BotAI.h"
#include "Bot/BotIntent.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/BotRegistry.h"
#include "Bot/LaunchProfile.h"
#include "Services.h"
#include "Session/BotSessionMgr.h"
#include "Threading/IntentQueue.h"
#include "Threading/SnapshotPublisher.h"
#include "Threading/TickScheduler.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Random.h"
#include "SharedDefines.h"
#include "StringConvert.h"
#include "Util.h"
#include "WorldSession.h"
#include <algorithm>
#include <array>
#include <map>
#include <sstream>

namespace Playerbot::V2::Fleet {

namespace {

// ---- Personas ----------------------------------------------------------------------------------------------------------
// Strategy x temperament x pace drawn together. Shares and paces from the launch research (2019 launch, Season of Mastery,
// Hardcore, Season of Discovery, the Forever beta and stress test); 1.0 pace = an uncrowded speedrun (Coldridge left at
// ~35 min). Weights can be overridden with Playerbot.Launch.PersonaWeights = "racer:5,rxp:12,...".
struct Persona
{
    char const*       key;
    uint16            weight;
    LaunchStrategy    strategy;
    float             pace_lo, pace_hi;
    std::array<uint8, 5> temper;          // Neutral, Friendly, Greedy, Loner, Griefer
    uint16            setup_lo_s, setup_hi_s;
    float             read_lo_s, read_hi_s;
    uint16            afk_every_min;      // 0 = never
    uint16            afk_lo_s, afk_hi_s;
    uint16            session_lo_min, session_hi_min;   // 0 = plays on
    uint8             class_pref;         // 0 = any
};

//                                                                     N   F   G   L   Gr
constexpr std::array<Persona, 15> kPersonas = { {
    { "racer",     5, LaunchStrategy::GuideRoute,   0.95f, 1.10f, { 25,  5, 60, 10,  0 },   0,  30, 0.0f,  0.5f,   0,   0,   0,   0,   0, 0 },
    { "rxp",      12, LaunchStrategy::GuideRoute,   1.00f, 1.30f, { 45, 10, 35, 10,  0 },  10, 120, 0.3f,  1.5f,  90,  60, 180,   0,   0,   0 },
    { "trolls",    3, LaunchStrategy::TrollGrinder, 1.00f, 1.20f, { 20,  0, 50, 30,  0 },   0,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
    { "skipper",   2, LaunchStrategy::Skipper,      1.00f, 1.20f, { 40,  0, 40, 20,  0 },   0,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
    { "questie",  25, LaunchStrategy::Quester,      1.60f, 2.40f, { 55, 30, 10,  5,  0 },  60, 300, 2.0f,  8.0f,  45,  60, 300, 120, 600,   0 },
    { "friends",  14, LaunchStrategy::Quester,      1.20f, 1.60f, { 50, 50,  0,  0,  0 },  30, 180, 1.0f,  5.0f,  60,  60, 240,   0,   0,   0 },
    { "grinder",   6, LaunchStrategy::CrowdGrinder, 1.20f, 1.60f, { 50,  0, 20, 30,  0 },  30, 120, 0.5f,  3.0f,  60,  60, 180,   0,   0,   0 },
    { "hunter",    5, LaunchStrategy::Quester,      1.20f, 1.50f, { 50,  0, 25, 25,  0 },  30, 150, 0.5f,  3.0f,  60,  60, 180,   0,   0,   CLASS_HUNTER },
    { "gatherer",  6, LaunchStrategy::Gatherer,     2.00f, 2.60f, { 50, 50,  0,  0,  0 },  60, 300, 2.0f,  8.0f,  45,  60, 300, 120, 480,   0 },
    { "casual",   15, LaunchStrategy::Casual,       3.00f, 4.00f, { 40, 50,  0, 10,  0 }, 120, 600, 8.0f, 40.0f,  20, 120, 600,  30, 180,   0 },
    { "partylead", 3, LaunchStrategy::PartyBuilder, 1.30f, 1.70f, { 40, 60,  0,  0,  0 },  30, 120, 1.0f,  4.0f,  60,  60, 180,   0,   0,   0 },
    { "idle",      6, LaunchStrategy::Idle,         4.00f, 4.00f, { 50, 20,  0, 30,  0 }, 300,1500,10.0f, 60.0f,  10, 180, 900,   5,  30,   0 },
    { "reroller",  4, LaunchStrategy::Reroller,     1.50f, 2.50f, { 50, 30, 10, 10,  0 },  60, 300, 1.0f,  6.0f,  30,  60, 240,  10,  40,   0 },
    { "ninja",     2, LaunchStrategy::KillStealer,  1.10f, 1.40f, {  0,  0, 30,  0, 70 },  10,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
    { "leaver",    2, LaunchStrategy::Leaver,       1.10f, 1.30f, { 50, 20, 20, 10,  0 },  10,  90, 0.3f,  2.0f,  90,  60, 180,   0,   0,   0 },
} };

// Classes of the Forever beta census (Alliance, 20 Sep, ~5000 characters, in percent x10), restricted below to what a
// dwarf or gnome can be. Dwarf 16% / Gnome 15% of the Alliance.
constexpr std::array<std::pair<uint8, uint16>, 8> kClassShares = { {
    { CLASS_HUNTER, 160 }, { CLASS_WARRIOR, 140 }, { CLASS_PALADIN, 135 }, { CLASS_PRIEST, 105 },
    { CLASS_ROGUE, 100 }, { CLASS_MAGE, 90 }, { CLASS_SHAMAN, 75 }, { CLASS_WARLOCK, 60 },
} };

uint32 Mix(uint32 a, uint32 b)
{
    uint32 h = a * 2654435761u ^ (b + 0x9E3779B9u + (a << 6) + (a >> 2));
    h ^= h >> 15;
    h *= 2246822519u;
    h ^= h >> 13;
    return h;
}

float Unit01(uint32 seed, uint32 k) { return float(Mix(seed, k) % 100000u) / 100000.0f; }

// Queues an intent for a bot from the world thread (as the whisper commands do, Bot/BotCommandParser.cpp Push).
template <class T>
bool PushIntent(Player const* bot, T body)
{
    BotId const id = bot->GetGUID().GetCounter();
    IntentQueue* q = Services::Registry().intents(id);
    IntentId* next = Services::Registry().next_intent_id(id);
    if (!q || !next)
        return false;
    BotIntentEmitter emit(q, id, /*source*/ 0, next);
    return emit.emit(WrapForIntentBody(std::move(body)));
}
float Lerp(float lo, float hi, float t) { return lo + (hi - lo) * t; }

template <typename Weights>
size_t PickWeighted(Weights const& w)
{
    uint32 total = 0;
    for (auto v : w) total += v;
    if (!total) return 0;
    uint32 roll = urand(0, total - 1);
    for (size_t i = 0; i < w.size(); ++i)
    {
        if (roll < w[i]) return i;
        roll -= w[i];
    }
    return 0;
}

char const* PhaseName(int p)
{
    static char const* const kNames[] = { "empty", "creating", "ready", "armed", "running", "stopping", "deleting" };
    return kNames[p];
}

// The guide a GuideRoute bot follows: racers their own fast route (Olo's, when imported), everyone else RestedXP mostly
// and the other imported guides (not Olo's) for the rest. Fixed by the bot's seed.
uint8 PickRoute(Persona const& p, uint32 seed)
{
    LaunchRoutes const& routes = LaunchRoutes::Instance();
    if (p.strategy != LaunchStrategy::GuideRoute || routes.Empty())
        return LaunchRoutes::kNoRoute;
    uint8 const olo = routes.Find("olo");
    uint8 const rxp = routes.Find("rxp");
    if (std::string_view(p.key) == "racer" && olo != LaunchRoutes::kNoRoute)
        return olo;
    std::vector<uint8> others;
    for (size_t i = 0; i < routes.Names().size(); ++i)
        if (uint8(i) != olo && uint8(i) != rxp)
            others.push_back(uint8(i));
    if (rxp != LaunchRoutes::kNoRoute && (others.empty() || Mix(seed, 11) % 100 < 75))
        return rxp;
    if (!others.empty())
        return others[Mix(seed, 12) % others.size()];
    return olo;   // only Olo's route is imported
}

} // anonymous namespace

LaunchSim& LaunchSim::Instance()
{
    static LaunchSim inst;
    return inst;
}

std::vector<uint32> LaunchSim::PersonaWeights() const
{
    std::vector<uint32> w;
    for (Persona const& p : kPersonas)
        w.push_back(p.weight);
    std::string const over = sConfigMgr->GetStringDefault("Playerbot.Launch.PersonaWeights", "", true);
    for (std::string_view item : Trinity::Tokenize(over, ',', false))
    {
        size_t const colon = item.find(':');
        if (colon == std::string_view::npos) continue;
        std::string_view const key = item.substr(0, colon);
        Optional<uint32> const val = Trinity::StringTo<uint32>(item.substr(colon + 1));
        if (!val) continue;
        for (size_t i = 0; i < kPersonas.size(); ++i)
            if (key == kPersonas[i].key)
                w[i] = *val;
    }
    return w;
}

void LaunchSim::Roll(Entry& e, bool keep_identity, bool late)
{
    e.persona = uint8(PickWeighted(PersonaWeights()));
    Persona const& p = kPersonas[e.persona];
    e.seed = urand(1, 0x7FFFFFFF);

    if (!keep_identity)
    {
        // Class first (the census is per class), then a race that can be it: dwarf 16 : gnome 15.
        for (int guard = 0; guard < 50; ++guard)
        {
            uint8 cls = p.class_pref;
            if (!cls)
            {
                std::array<uint16, kClassShares.size()> w{};
                for (size_t i = 0; i < kClassShares.size(); ++i) w[i] = kClassShares[i].second;
                cls = kClassShares[PickWeighted(w)].first;
            }
            bool const dwarf_ok = BotComposition::IsLegalRaceClass(RACE_DWARF, cls);
            bool const gnome_ok = BotComposition::IsLegalRaceClass(RACE_GNOME, cls);
            if (!dwarf_ok && !gnome_ok) continue;
            uint8 race = !gnome_ok ? RACE_DWARF : !dwarf_ok ? RACE_GNOME : (urand(0, 30) < 16 ? RACE_DWARF : RACE_GNOME);
            BotComposition::Pick const pick = BotComposition::Roll(race, cls, uint8(urand(0, 1)), "");
            if (!pick.race) continue;
            e.race = pick.race; e.cls = pick.cls; e.gender = pick.gender; e.name = pick.name;
            break;
        }
    }

    // Arrival after T0, the same for every persona (the login queue does not care who is a racer): 35% in the first 3 min,
    // 35% by 15 min, the rest by 60; late arrivals spread over hours one to six.
    if (late)
        e.arrival_s = urand(3600, 6 * 3600);
    else
    {
        uint32 const r = urand(0, 99);
        e.arrival_s = r < 35 ? urand(5, 180) : r < 70 ? urand(180, 900) : urand(900, 3600);
    }
    e.session_s = p.session_hi_min ? urand(p.session_lo_min, p.session_hi_min) * 60u : 0u;
}

bool LaunchSim::CreateOne(Entry& e)
{
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        BotCharacterFactory::Result const r = BotCharacterFactory::Create(nullptr, e.name, e.race, e.cls, e.gender);
        if (r.ok)
        {
            e.guid_low = r.guid.GetCounter();
            e.state = BotState::Offline;
            by_guid_[e.guid_low] = size_t(&e - roster_.data());
            SaveRow(e);
            return true;
        }
        TC_LOG_INFO("playerbot.v2", "[launch] create {} ({} {}) failed: {}", e.name, uint32(e.race), uint32(e.cls), r.reason);
        // Most failures are a name taken since it was rolled: roll another one.
        BotComposition::Pick const pick = BotComposition::Roll(e.race, e.cls, e.gender, "");
        if (pick.race)
            e.name = pick.name;
    }
    return false;
}

void LaunchSim::SaveRow(Entry const& e)
{
    CharacterDatabase.PExecute(
        "REPLACE INTO playerbot_launch_bot (character_guid_low, name, race, class, gender, persona, arrival_s, session_s, seed) "
        "VALUES ({}, '{}', {}, {}, {}, {}, {}, {}, {})",
        e.guid_low, e.name, uint32(e.race), uint32(e.cls), uint32(e.gender), uint32(e.persona), e.arrival_s, e.session_s, e.seed);
}

void LaunchSim::DeleteOne(Entry const& e)
{
    if (!e.guid_low)
        return;
    ObjectGuid const g = ObjectGuid::Create<HighGuid::Player>(e.guid_low);
    BotId const id = e.guid_low;
    if (Services::Registry().has(id))
    {
        Services::Scheduler().unregister_bot(id);
        Services::Snapshots().remove(id);
        Services::Registry().unregister_bot(id);
    }
    Services::Lifecycle().unmark_as_bot(id);
    uint32 const account = sCharacterCache->GetCharacterAccountIdByGuid(g);
    Player::DeleteFromDB(g, account, /*updateRealmChars*/ true, /*deleteFinally*/ true);
    Services::Accounts().note_character_removed(account);
    CharacterDatabase.PExecute("DELETE FROM playerbot_launch_bot WHERE character_guid_low = {}", e.guid_low);
}

void LaunchSim::LoadRoster()
{
    if (loaded_)
        return;
    loaded_ = true;
    LaunchRoutes::Instance().Load();
    QueryResult r = CharacterDatabase.Query(
        "SELECT character_guid_low, name, race, class, gender, persona, arrival_s, session_s, seed FROM playerbot_launch_bot");
    if (!r)
        return;
    do
    {
        Field* f = r->Fetch();
        Entry e;
        e.guid_low = f[0].GetUInt64();
        e.name = f[1].GetString();
        e.race = f[2].GetUInt8(); e.cls = f[3].GetUInt8(); e.gender = f[4].GetUInt8();
        e.persona = std::min<uint8>(f[5].GetUInt8(), uint8(kPersonas.size() - 1));
        e.arrival_s = f[6].GetUInt32(); e.session_s = f[7].GetUInt32(); e.seed = f[8].GetUInt32();
        e.state = BotState::Offline;
        roster_.push_back(std::move(e));
    } while (r->NextRow());
    for (size_t i = 0; i < roster_.size(); ++i)
    {
        by_guid_[roster_[i].guid_low] = i;
        if (roster_[i].arrival_s >= 3600) ++late_count_;
    }
    phase_ = Phase::Ready;
    TC_LOG_INFO("playerbot.v2", "[launch] roster loaded: {} bots", roster_.size());
}

std::string LaunchSim::Prepare(uint32 bots, uint32 late)
{
    LoadRoster();
    if (!roster_.empty())
        return "A roster exists already (" + std::to_string(roster_.size()) + " bots): use `.playerbot launch reset` for a fresh one.";
    if (bots == 0 || bots + late > 2000)
        return "Bots must be 1-2000 in total.";
    LaunchRoutes::Instance().Load();   // pick up routes imported since the server started
    roster_.resize(bots + late);
    for (size_t i = 0; i < roster_.size(); ++i)
        Roll(roster_[i], /*keep_identity*/ false, /*late*/ i >= bots);
    late_count_ = late;
    phase_ = Phase::Creating;
    return "Creating " + std::to_string(bots) + " launch bots + " + std::to_string(late) + " late arrivals (a few per tick). "
           "`.playerbot launch status` shows the progress; then `.playerbot launch arm` and log in.";
}

std::string LaunchSim::Arm()
{
    LoadRoster();
    if (phase_ != Phase::Ready)
        return std::string("Cannot arm: the run is ") + PhaseName(int(phase_)) + " (needs ready).";
    phase_ = Phase::Armed;
    return "Armed: the launch starts when the next real player enters the world.";
}

std::string LaunchSim::Start()
{
    LoadRoster();
    if (phase_ != Phase::Ready && phase_ != Phase::Armed)
        return std::string("Cannot start: the run is ") + PhaseName(int(phase_)) + " (needs ready or armed).";
    t0_ms_ = GameTime::GetGameTimeMS();
    asks_ = grants_ = rerolls_ = 0;
    replies_.clear();
    phase_ = Phase::Running;
    TC_LOG_INFO("playerbot.v2", "[launch] T0: {} bots arrive over the next hours", roster_.size());
    return "Launch started: " + std::to_string(roster_.size()) + " bots arrive from now on.";
}

std::string LaunchSim::Stop()
{
    LoadRoster();
    if (roster_.empty())
        return "No roster.";
    phase_ = Phase::Stopping;
    recreate_after_delete_ = false;
    return "Stopping: every launch bot logs out.";
}

std::string LaunchSim::Reset(uint32 bots, uint32 late)
{
    LoadRoster();
    if (roster_.empty())
        return "No roster: use `.playerbot launch prepare`.";
    if (bots + late > 2000)
        return "Bots must be 1-2000 in total.";
    phase_ = Phase::Stopping;
    recreate_after_delete_ = true;
    resize_bots_ = bots;
    resize_late_ = late;
    if (bots)
        return "Resetting: the launch bots log out and are deleted; then " + std::to_string(bots) + " + " + std::to_string(late) +
               " new ones are created fresh at level 1.";
    return "Resetting: the launch bots log out, are deleted and created again fresh at level 1 with new personas.";
}

std::string LaunchSim::Status()
{
    LoadRoster();
    std::array<uint32, 6> by_state{};
    std::vector<uint32> online_by_persona(kPersonas.size(), 0);
    std::map<uint32, uint32> levels;
    for (Entry const& e : roster_)
    {
        ++by_state[size_t(e.state)];
        if (e.state == BotState::Online)
        {
            ++online_by_persona[e.persona];
            if (Player* p = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(e.guid_low)))
                ++levels[p->GetLevel()];
        }
    }
    std::ostringstream o;
    o << "Launch " << PhaseName(int(phase_)) << ": " << roster_.size() << " bots (" << late_count_ << " late); to create "
      << by_state[0] << ", offline " << by_state[1] << ", logging in " << by_state[2] << ", online " << by_state[3]
      << ", done " << by_state[4] << ", rerolling " << by_state[5];
    if (phase_ == Phase::Running)
        o << "; T0 + " << (GameTime::GetGameTimeMS() - t0_ms_) / 60000u << " min";
    o << "\nOnline by persona:";
    for (size_t i = 0; i < kPersonas.size(); ++i)
        if (online_by_persona[i])
            o << ' ' << kPersonas[i].key << '=' << online_by_persona[i];
    o << "\nLevels:";
    for (auto const& [lvl, n] : levels)
        o << " L" << lvl << '=' << n;
    return o.str();
}

std::string LaunchSim::Report() const
{
    // What the online launch bots are doing: XP progress, quests turned in, dead ones, and the rule each fired last
    // (the busiest ten), so a run can be judged from the log without watching it.
    uint32 online = 0, dead = 0, quests = 0, grouped = 0;
    std::map<uint32, uint32> levels;
    uint64 xp_levels_x100 = 0;
    std::map<std::string, uint32> rules;
    for (Entry const& e : roster_)
    {
        if (e.state != BotState::Online)
            continue;
        Player* p = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(e.guid_low));
        if (!p)
            continue;
        ++online;
        dead += p->isDead() ? 1u : 0u;
        grouped += p->GetGroup() ? 1u : 0u;
        ++levels[p->GetLevel()];
        quests += uint32(p->GetRewardedQuestCount());
        uint32 const need = p->GetXPForNextLevel();
        xp_levels_x100 += uint64(p->GetLevel()) * 100u + (need ? uint64(p->GetXP()) * 100u / need : 0u);
        if (BotAI* ai = Services::Registry().ai(e.guid_low))
            if (char const* r = ai->last_rule_fired())
                ++rules[r];
    }
    std::ostringstream o;
    o << "T0+" << (phase_ == Phase::Running ? (GameTime::GetGameTimeMS() - t0_ms_) / 60000u : 0u) << "min online=" << online
      << " avg_level=" << (online ? double(xp_levels_x100) / 100.0 / online : 0.0) << " quests=" << quests << " dead=" << dead
      << " grouped=" << grouped << " inv_asked=" << asks_ << " inv_granted=" << grants_ << " rerolls=" << rerolls_
      << " levels:";
    for (auto const& [lvl, n] : levels)
        o << " L" << lvl << '=' << n;
    o << " rules:";
    std::vector<std::pair<uint32, std::string>> top;
    for (auto const& [r, n] : rules)
        top.emplace_back(n, r);
    std::sort(top.rbegin(), top.rend());
    for (size_t i = 0; i < top.size() && i < 10; ++i)
        o << ' ' << top[i].second << '=' << top[i].first;
    return o.str();
}

bool LaunchSim::OnAskInvite(Player* asker, ObjectGuid target)
{
    Player* t = ObjectAccessor::FindConnectedPlayer(target);
    if (!asker || !t || t == asker)
        return false;
    uint32 const now = GameTime::GetGameTimeMS();
    uint32 const h = Mix(uint32(asker->GetGUID().GetCounter()), uint32(target.GetCounter()) ^ (now / 1000u));
    static constexpr std::array<char const*, 8> kAsk = {"inv", "inv?", "inv pls", "invite?", "can i get an inv", "inv for the kill?",
                                                        "group?", "inv plz"};
    // A real player asking has whispered already; a bot's whisper goes out through its own intent queue.
    if (asker->GetSession() && asker->GetSession()->IsBot())
        PushIntent(asker, WhisperIntent{t->GetName(), kAsk[h % kAsk.size()]});
    ++asks_;
    BotAI* tai = IsLaunchBot(target.GetCounter()) ? Services::Registry().ai(target.GetCounter()) : nullptr;
    if (!tai || !tai->launch().active)
        return true;   // a real player (or a bot outside the run) answers for themselves

    // Odds per temperament (Neutral, Friendly, Greedy, Loner, Griefer) that the target invites, and, when it does not, that it
    // says no instead of ignoring the whisper.
    static constexpr std::array<uint8, 5> kGrant = {45, 95, 5, 0, 10};
    static constexpr std::array<uint8, 5> kSayNo = {40, 100, 25, 0, 50};
    static constexpr std::array<char const*, 6> kNo = {"sry", "no", "nah", "solo", "doing it solo sry", "no ty"};
    uint8 const tt = std::min<uint8>(uint8(tai->launch().temperament), 4);
    Reply r{target, asker->GetGUID(), now + 1500u + (h >> 16) % 4500u, false, {}};
    if ((h >> 8) % 100u < kGrant[tt])
    {
        r.invite = true;
        ++grants_;
    }
    else if ((h >> 4) % 100u < kSayNo[tt])
        r.text = kNo[(h >> 12) % kNo.size()];
    else
        return true;
    replies_.push_back(std::move(r));
    return true;
}

void LaunchSim::SendReplies(uint32 now_ms)
{
    for (auto it = replies_.begin(); it != replies_.end();)
    {
        if (int32(now_ms - it->due_ms) < 0)
        {
            ++it;
            continue;
        }
        Player* from = ObjectAccessor::FindConnectedPlayer(it->from);
        Player* to = ObjectAccessor::FindConnectedPlayer(it->to);
        if (from && to)
        {
            if (it->invite)
                PushIntent(from, InviteToGroupIntent{it->to});
            else
                PushIntent(from, WhisperIntent{to->GetName(), it->text});
        }
        it = replies_.erase(it);
    }
}

void LaunchSim::OnWorldTick(uint32 now_ms)
{
    if (!replies_.empty())
        SendReplies(now_ms);
    if (phase_ == Phase::Running && now_ms - last_report_ms_ >= 60000u)
    {
        last_report_ms_ = now_ms;
        TC_LOG_INFO("playerbot.v2", "[launch] report {}", Report());
    }
    switch (phase_)
    {
        case Phase::Creating:
        {
            uint32 made = 0;
            for (Entry& e : roster_)
            {
                if (e.state != BotState::ToCreate) continue;
                if (made++ >= 4) return;   // a few per tick: creation writes to the database on this thread
                if (!CreateOne(e))
                    e.state = BotState::Done;   // gave up on this one; the roster is one short
            }
            phase_ = Phase::Ready;
            TC_LOG_INFO("playerbot.v2", "[launch] roster ready: {} bots", roster_.size());
            return;
        }
        case Phase::Running:
        {
            uint32 const elapsed_s = (now_ms - t0_ms_) / 1000u;
            uint32 logins = 0;
            for (Entry& e : roster_)
            {
                ObjectGuid const g = ObjectGuid::Create<HighGuid::Player>(e.guid_low);
                switch (e.state)
                {
                    case BotState::Offline:
                        if (e.guid_low && e.arrival_s <= elapsed_s && logins < 6)
                        {
                            auto const r = Services::SessionMgr().LoginBot(g);
                            ++logins;
                            if (r.ok) { e.state = BotState::LoggingIn; e.login_sent_ms = now_ms; }
                            else TC_LOG_INFO("playerbot.v2", "[launch] login {} refused: {}", e.name, r.reason);
                        }
                        break;
                    case BotState::LoggingIn:
                        if (now_ms - e.login_sent_ms > 120000u)
                            e.state = BotState::Offline;   // never arrived: try again
                        break;
                    case BotState::Online:
                        if (!ObjectAccessor::FindConnectedPlayer(g))
                            e.state = BotState::Done;      // gone (kicked, crashed session)
                        else if (e.session_s && now_ms - e.online_since_ms >= e.session_s * 1000u)
                        {
                            Services::SessionMgr().LogoutBot(g);
                            bool const reroll = kPersonas[e.persona].strategy == LaunchStrategy::Reroller;
                            e.state = reroll ? BotState::Rerolling : BotState::Done;
                            e.reroll_at_ms = now_ms + 15000u;   // after the logout went through
                            TC_LOG_INFO("playerbot.v2", "[launch] {} ({}) ends its session after {} min{}", e.name,
                                kPersonas[e.persona].key, e.session_s / 60u, reroll ? ", rerolls" : "");
                        }
                        break;
                    case BotState::Rerolling:
                        if (int32(now_ms - e.reroll_at_ms) >= 0 && !ObjectAccessor::FindConnectedPlayer(g) && logins < 6)
                        {
                            ++logins;
                            std::string const old_name = e.name;
                            by_guid_.erase(e.guid_low);
                            DeleteOne(e);
                            e.guid_low = 0;
                            Roll(e, /*keep_identity*/ false, /*late*/ false);
                            e.arrival_s = elapsed_s + urand(60, 180);   // character creation, a new name, the intro
                            if (CreateOne(e))
                            {
                                ++rerolls_;
                                TC_LOG_INFO("playerbot.v2", "[launch] {} deleted, back as {} ({} {}) in {} s", old_name, e.name,
                                    kPersonas[e.persona].key, uint32(e.cls), e.arrival_s - elapsed_s);
                            }
                            else
                                e.state = BotState::Done;
                        }
                        break;
                    default:
                        break;
                }
            }
            return;
        }
        case Phase::Stopping:
        {
            bool any_online = false;
            for (Entry& e : roster_)
            {
                ObjectGuid const g = ObjectGuid::Create<HighGuid::Player>(e.guid_low);
                if (e.guid_low && ObjectAccessor::FindConnectedPlayer(g))
                {
                    any_online = true;
                    Services::SessionMgr().LogoutBot(g);
                }
                if (e.guid_low)
                    e.state = BotState::Offline;
            }
            if (!any_online)
                phase_ = recreate_after_delete_ ? Phase::Deleting : Phase::Ready;
            return;
        }
        case Phase::Deleting:
        {
            uint32 done = 0;
            for (Entry& e : roster_)
            {
                if (!e.guid_low) continue;
                if (done++ >= 5) return;
                by_guid_.erase(e.guid_low);
                DeleteOne(e);
                e.guid_low = 0;
            }
            recreate_after_delete_ = false;
            // A new size: a new roster altogether.
            if (resize_bots_)
            {
                roster_.clear();
                phase_ = Phase::Empty;
                TC_LOG_INFO("playerbot.v2", "[launch] roster deleted; {}", Prepare(resize_bots_, resize_late_));
                resize_bots_ = resize_late_ = 0;
                return;
            }
            // All gone: the same names, races and classes again, fresh, with new personas, arrivals and sessions.
            for (size_t i = 0; i < roster_.size(); ++i)
            {
                Roll(roster_[i], /*keep_identity*/ true, /*late*/ i + late_count_ >= roster_.size());
                roster_[i].state = BotState::ToCreate;
            }
            phase_ = Phase::Creating;
            TC_LOG_INFO("playerbot.v2", "[launch] roster deleted; recreating {} bots", roster_.size());
            return;
        }
        default:
            return;
    }
}

void LaunchSim::OnRealPlayerLogin(Player* p)
{
    if (phase_ != Phase::Armed || !p)
        return;
    TC_LOG_INFO("playerbot.v2", "[launch] {} entered the world: launch starts", p->GetName());
    Start();
}

bool LaunchSim::IsLaunchBot(uint64 guid_low) const
{
    return by_guid_.contains(guid_low);
}

bool LaunchSim::OnBotLogin(Player* p, BotAI* ai)
{
    if (!p || !ai)
        return false;
    LoadRoster();
    auto it = by_guid_.find(p->GetGUID().GetCounter());
    if (it == by_guid_.end())
        return false;
    Entry& e = roster_[it->second];
    e.state = BotState::Online;
    e.online_since_ms = GameTime::GetGameTimeMS();

    Persona const& pr = kPersonas[e.persona];
    LaunchProfile lp;
    lp.active = true;
    lp.persona = e.persona;
    lp.strategy = pr.strategy;
    // Temperament from the persona's odds, fixed by the bot's seed.
    auto const temperament = [&]() -> uint8
    {
        uint32 total = 0;
        for (uint8 w : pr.temper) total += w;
        uint32 roll = total ? Mix(e.seed, 7) % total : 0;
        for (uint8 i = 0; i < uint8(pr.temper.size()); ++i)
        {
            if (roll < pr.temper[i]) return i;
            roll -= pr.temper[i];
        }
        return 0;
    };
    lp.temperament = LaunchTemperament(temperament());
    lp.pace = Lerp(pr.pace_lo, pr.pace_hi, Unit01(e.seed, 1));
    lp.contest_tags = true;
    lp.login_ms = e.online_since_ms;
    lp.setup_ms = uint32(Lerp(pr.setup_lo_s, pr.setup_hi_s, Unit01(e.seed, 2)) * 1000.0f);
    lp.read_min_ms = uint32(pr.read_lo_s * 1000.0f);
    lp.read_max_ms = uint32(pr.read_hi_s * 1000.0f);
    if (pr.afk_every_min)
    {
        lp.afk_every_ms = uint32(float(pr.afk_every_min) * 60000.0f * Lerp(0.7f, 1.3f, Unit01(e.seed, 3)));
        lp.afk_min_ms = pr.afk_lo_s * 1000u;
        lp.afk_max_ms = pr.afk_hi_s * 1000u;
    }
    lp.session_ms = e.session_s * 1000u;
    lp.seed = e.seed;
    lp.route = PickRoute(pr, e.seed);
    ai->set_launch(lp);
    TC_LOG_INFO("playerbot.v2", "[launch] {} L{} {} {} arrives: persona {} pace {:.2f} temperament {} setup {}s session {} route {}",
        p->GetName(), uint32(p->GetLevel()), uint32(p->GetRace()), uint32(p->GetClass()), pr.key, lp.pace,
        uint32(lp.temperament), lp.setup_ms / 1000u, e.session_s ? std::to_string(e.session_s / 60u) + " min" : std::string("open"),
        lp.route == LaunchRoutes::kNoRoute ? std::string("-") : LaunchRoutes::Instance().Names()[lp.route]);
    return true;
}

} // namespace Playerbot::V2::Fleet
