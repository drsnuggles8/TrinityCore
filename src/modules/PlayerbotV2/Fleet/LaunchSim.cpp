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
#include "Bot/LaunchPlaces.h"
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
#include <optional>
#include <random>
#include <shared_mutex>
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
constexpr std::array<Persona, 13> kPersonas = { {
    { "racer",     6, LaunchStrategy::GuideRoute,   0.95f, 1.10f, { 25,  5, 60, 10,  0 },   0,  30, 0.0f,  0.5f,   0,   0,   0,   0,   0, 0 },
    { "rxp",      45, LaunchStrategy::GuideRoute,   1.00f, 1.30f, { 45, 10, 35, 10,  0 },   5,  60, 0.3f,  1.5f,  90,  60, 180,   0,   0,   0 },
    { "trolls",    3, LaunchStrategy::TrollGrinder, 1.00f, 1.20f, { 20,  0, 50, 30,  0 },   0,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
    { "skipper",   2, LaunchStrategy::Skipper,      1.00f, 1.20f, { 40,  0, 40, 20,  0 },   0,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
    { "questie",  20, LaunchStrategy::Quester,      1.60f, 2.40f, { 53, 30, 10,  4,  3 },  15,  60, 2.0f,  8.0f,  45,  60, 300, 120, 600,   0 },
    { "friends",  12, LaunchStrategy::Quester,      1.20f, 1.60f, { 50, 50,  0,  0,  0 },  10,  45, 1.0f,  5.0f,  60,  60, 240,   0,   0,   0 },
    { "grinder",   6, LaunchStrategy::CrowdGrinder, 1.20f, 1.60f, { 45,  0, 20, 25, 10 },  15,  60, 0.5f,  3.0f,  60,  60, 180,   0,   0,   0 },
    { "gatherer",  6, LaunchStrategy::Gatherer,     2.00f, 2.60f, { 50, 50,  0,  0,  0 },  15,  60, 2.0f,  8.0f,  45,  60, 300, 120, 480,   0 },
    { "casual",   12, LaunchStrategy::Casual,       3.00f, 4.00f, { 40, 50,  0, 10,  0 },  30,  90, 8.0f, 40.0f,  20, 120, 600,  30, 180,   0 },
    { "partylead", 3, LaunchStrategy::PartyBuilder, 1.30f, 1.70f, { 40, 60,  0,  0,  0 },  15,  60, 1.0f,  4.0f,  60,  60, 180,   0,   0,   0 },
    { "idle",      6, LaunchStrategy::Idle,         4.00f, 4.00f, { 50, 20,  0, 30,  0 },  60, 180,10.0f, 60.0f,  10, 180, 900,   5,  30,   0 },
    { "leaver",    2, LaunchStrategy::Leaver,       1.10f, 1.30f, { 50, 20, 20, 10,  0 },   5,  45, 0.3f,  2.0f,  90,  60, 180,   0,   0,   0 },
    // explorer: weight 0 since 27 Oct - without the real routes' steps (the Rothie / mafsen exploration routes live in the
    // LevelForge guide service) the hand-made subzone tours led level-1 bots through mobs far above them. The code stays;
    // Playerbot.Launch.PersonaWeights = "explorer:2" brings them back once real routes are imported.
    { "explorer",  0, LaunchStrategy::Explorer,     1.00f, 1.20f, { 40, 20,  0, 40,  0 },   0,  60, 0.2f,  1.0f,   0,   0,   0,   0,   0,   0 },
} };

// Classic 1.60 surnames ("Name Surname" over the head and in the target frame): each launch bot carries its persona, so a
// player can tell the racer from the explorer at a glance. Same order as kPersonas.
constexpr std::array<char const*, 13> kPersonaSurnames = { {
    "Racer", "Guided", "Trollgrinder", "Skipper", "Questie", "Friendly", "Grinder",
    "Gatherer", "Casual", "Partylead", "Idler", "Leaver", "Explorer",
} };
static_assert(kPersonaSurnames.size() == kPersonas.size());

// Race and class pairs of the Forever beta census at the end of the beta (ClassicPlus.io addon census, data of 24-26 Oct
// 2026, 1,148,135 characters, as tabled by classicplus.world), Alliance, in hundreds of characters. Each starting zone draws
// from its own races' pairs (a dwarf shaman is 54% of the dwarves, a human paladin 28% of the humans), and the zones' sizes
// follow the same numbers (CensusShareOf).
struct RaceClassShare { uint8 zone; uint8 race; uint8 cls; uint16 hundreds; };
constexpr RaceClassShare kRaceClassShares[] = {
    // Coldridge Valley: dwarves (98.7k) and gnomes (88.2k)
    { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_SHAMAN, 533 }, { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_PALADIN, 168 },
    { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_HUNTER, 146 }, { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_WARRIOR, 77 },
    { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_PRIEST, 42 }, { LaunchPlaces::kZoneColdridge, RACE_DWARF, CLASS_ROGUE, 21 },
    { LaunchPlaces::kZoneColdridge, RACE_GNOME, CLASS_WARLOCK, 236 }, { LaunchPlaces::kZoneColdridge, RACE_GNOME, CLASS_MAGE, 225 },
    { LaunchPlaces::kZoneColdridge, RACE_GNOME, CLASS_PRIEST, 206 }, { LaunchPlaces::kZoneColdridge, RACE_GNOME, CLASS_ROGUE, 127 },
    { LaunchPlaces::kZoneColdridge, RACE_GNOME, CLASS_WARRIOR, 88 },
    // Northshire Valley: humans (173.8k)
    { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_PALADIN, 484 }, { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_WARRIOR, 295 },
    { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_HUNTER, 242 }, { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_WARLOCK, 242 },
    { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_PRIEST, 169 }, { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_ROGUE, 167 },
    { LaunchPlaces::kZoneNorthshire, RACE_HUMAN, CLASS_MAGE, 140 },
    // Shadowglen: night elves (104.3k)
    { LaunchPlaces::kZoneShadowglen, RACE_NIGHTELF, CLASS_DRUID, 311 }, { LaunchPlaces::kZoneShadowglen, RACE_NIGHTELF, CLASS_HUNTER, 307 },
    { LaunchPlaces::kZoneShadowglen, RACE_NIGHTELF, CLASS_WARRIOR, 150 }, { LaunchPlaces::kZoneShadowglen, RACE_NIGHTELF, CLASS_ROGUE, 146 },
    { LaunchPlaces::kZoneShadowglen, RACE_NIGHTELF, CLASS_PRIEST, 128 },
    // Zephras Isle: High Order Skyborne (race 95, 110.1k)
    { LaunchPlaces::kZoneZephras, 95, CLASS_DRUID, 361 }, { LaunchPlaces::kZoneZephras, 95, CLASS_MAGE, 281 },
    { LaunchPlaces::kZoneZephras, 95, CLASS_HUNTER, 226 }, { LaunchPlaces::kZoneZephras, 95, CLASS_WARRIOR, 123 },
    { LaunchPlaces::kZoneZephras, 95, CLASS_ROGUE, 109 },
};

// Census characters (hundreds) of a starting zone's races.
uint32 CensusShareOf(uint8 zone)
{
    uint32 n = 0;
    for (RaceClassShare const& rc : kRaceClassShares)
        if (rc.zone == zone)
            n += rc.hundreds;
    return n;
}

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

// Playerbot.Launch.RosterSeed > 0: Prepare rolls the same roster again (zones, races, classes, genders, personas, the bots'
// seeds and arrivals; the names still vary), so two builds can be benched on the same class mix (racer45 build41 had no
// hunter or shaman, build42 three shamans and a hunter, and their pace differed more than the change between them).
// Only Prepare's rolls use it, on the world thread.
std::optional<std::mt19937> gRollRng;
uint32 RollRand(uint32 lo, uint32 hi)
{
    return gRollRng ? std::uniform_int_distribution<uint32>(lo, hi)(*gRollRng) : urand(lo, hi);
}

template <typename Weights>
size_t PickWeighted(Weights const& w)
{
    uint32 total = 0;
    for (auto v : w) total += v;
    if (!total) return 0;
    uint32 roll = RollRand(0, total - 1);
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
// The suffix of the route names for a race's starting zone: "rxp", "olo" (dwarves and gnomes), "rxp-human", "rxp-nightelf".
std::string_view RouteSuffix(uint8 race)
{
    switch (race)
    {
        case RACE_HUMAN:    return "-human";
        case RACE_NIGHTELF: return "-nightelf";
        case RACE_DWARF:
        case RACE_GNOME:    return "";
        default:            return "-none";   // no guide for this race (Skyborne): it quests on its own
    }
}

uint8 PickRoute(Persona const& p, uint32 seed, uint8 race)
{
    LaunchRoutes const& routes = LaunchRoutes::Instance();
    if (p.strategy != LaunchStrategy::GuideRoute || routes.Empty())
        return LaunchRoutes::kNoRoute;
    std::string_view const sfx = RouteSuffix(race);
    auto const fits = [&](std::string const& name)
    {
        if (sfx.empty())
            return name.find('-') == std::string::npos;   // the dwarf/gnome routes carry no race suffix
        return name.size() > sfx.size() && name.compare(name.size() - sfx.size(), sfx.size(), sfx) == 0;
    };
    uint8 const olo = sfx.empty() ? routes.Find("olo") : LaunchRoutes::kNoRoute;
    uint8 const rxp = routes.Find("rxp" + std::string(sfx));
    if (std::string_view(p.key) == "racer" && olo != LaunchRoutes::kNoRoute)
        return olo;
    std::vector<uint8> others;
    for (size_t i = 0; i < routes.Names().size(); ++i)
        if (uint8(i) != olo && uint8(i) != rxp && fits(routes.Names()[i]))
            others.push_back(uint8(i));
    if (rxp != LaunchRoutes::kNoRoute && (others.empty() || Mix(seed, 11) % 100 < 75))
        return rxp;
    if (!others.empty())
        return others[Mix(seed, 12) % others.size()];
    return olo;   // only Olo's route is imported (dwarves and gnomes), or none for this race
}


// Playerbot.Launch.AutoRun = 1 (Olo's always-on server): the crowd starts when a real player logs in and nobody else is online.
// When the last real player logs out the run pauses (the bots log out, the clock stops), and a server restart keeps it too: the
// next login resumes it with the bots at their levels (playtest 28 Oct: a restart that reset the bots to level 1 left the
// players alone). `.playerbot launch reset` starts over at level 1.
bool AutoRun()
{
    return sConfigMgr->GetBoolDefault("Playerbot.Launch.AutoRun", false, true);
}

// Real (non-bot) players in the world, `except` left out.
uint32 RealPlayersOnline(Player const* except)
{
    uint32 n = 0;
    std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    for (auto const& [guid, pl] : ObjectAccessor::GetPlayers())
        if (pl && pl != except && pl->IsInWorld() && pl->GetSession() && !pl->GetSession()->IsBot())
            ++n;
    return n;
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

void LaunchSim::Roll(Entry& e, bool keep_identity, bool late, uint8 zone)
{
    // Personas tied to a zone's own places: the troll grinder needs a grind camp (LaunchPlaces StartZone::grind), the explorer
    // an exploration route of the zone (LaunchRoutes::HasExploreRoute). Elsewhere the roll is drawn again.
    for (int guard = 0; guard < 20; ++guard)
    {
        e.persona = uint8(PickWeighted(PersonaWeights()));
        LaunchStrategy const st = kPersonas[e.persona].strategy;
        if (st == LaunchStrategy::TrollGrinder && !LaunchPlaces::Exists(LaunchPlaces::kZones[zone].grind))
            continue;
        if (st == LaunchStrategy::Explorer && !LaunchRoutes::Instance().HasExploreRoute(zone))
            continue;
        // A guide follower needs a guide of the zone's race (none for the Skyborne): without one it would quest on its own
        // under a "Guided" or "Racer" surname.
        if (st == LaunchStrategy::GuideRoute && !LaunchRoutes::Instance().Empty())
        {
            uint8 const race = zone == LaunchPlaces::kZoneNorthshire ? uint8(RACE_HUMAN)
                             : zone == LaunchPlaces::kZoneShadowglen ? uint8(RACE_NIGHTELF)
                             : zone == LaunchPlaces::kZoneZephras    ? uint8(95) : uint8(RACE_DWARF);
            if (PickRoute(kPersonas[e.persona], 0, race) == LaunchRoutes::kNoRoute)
                continue;
        }
        break;
    }
    Persona const& p = kPersonas[e.persona];
    e.seed = RollRand(1, 0x7FFFFFFF);

    if (!keep_identity)
    {
        // A race and class pair of the zone, by the census (kRaceClassShares; a persona's class preference narrows it).
        std::vector<RaceClassShare const*> pairs;
        std::vector<uint32> weights;
        for (RaceClassShare const& rc : kRaceClassShares)
            if (rc.zone == zone && (!p.class_pref || rc.cls == p.class_pref) && BotComposition::IsLegalRaceClass(rc.race, rc.cls))
            {
                pairs.push_back(&rc);
                weights.push_back(rc.hundreds);
            }
        for (int guard = 0; guard < 50 && !pairs.empty(); ++guard)
        {
            RaceClassShare const& rc = *pairs[PickWeighted(weights)];
            uint8 const race = rc.race, cls = rc.cls;
            BotComposition::Pick const pick = BotComposition::Roll(race, cls, uint8(RollRand(0, 1)), "");
            if (!pick.race) continue;
            e.race = pick.race; e.cls = pick.cls; e.gender = pick.gender; e.name = pick.name;
            break;
        }
    }

    // Arrival after T0, the same for every persona (the login queue does not care who is a racer): 55% in the first 3 min,
    // 30% by 15 min, the rest by 60 (a launch-time queue; the old 35/35/30 left a start zone a third full after 3 minutes and
    // the level-3 areas empty at 15, playtest 28 Oct); late arrivals spread over hours one to six.
    // Playerbot.Launch.ArrivalMaxS > 0: everyone arrives within that many seconds (tests: "5 racers starting together").
    uint32 const arrival_max = uint32(std::max<int32>(0, sConfigMgr->GetIntDefault("Playerbot.Launch.ArrivalMaxS", 0, true)));
    if (late)
        e.arrival_s = RollRand(3600, 6 * 3600);
    else if (arrival_max)
        e.arrival_s = RollRand(1, std::max<uint32>(arrival_max, 1));
    else
    {
        uint32 const r = RollRand(0, 99);
        e.arrival_s = r < 55 ? RollRand(5, 180) : r < 85 ? RollRand(180, 900) : RollRand(900, 3600);
    }
    e.session_s = p.session_hi_min ? RollRand(p.session_lo_min, p.session_hi_min) * 60u : 0u;
}

bool LaunchSim::CreateOne(Entry& e)
{
    // Each attempt rolls a new appearance (and, after a failure, a new name). Skyborne have no client-made characters on the
    // server to copy appearances from, and about 60% of their random ones fail Player::Create's validation: 3 attempts left
    // 25 of 118 unmade (build24); 12 leave about one in 500.
    for (int attempt = 0; attempt < 12; ++attempt)
    {
        BotCharacterFactory::Result const r = BotCharacterFactory::Create(nullptr, e.name, e.race, e.cls, e.gender);
        if (r.ok)
        {
            e.guid_low = r.guid.GetCounter();
            e.state = BotState::Offline;
            by_guid_[e.guid_low] = size_t(&e - roster_.data());
            SaveRow(e);
            // The persona as surname. The character row's INSERT is queued before this UPDATE on the async worker; the cache
            // (what the PlayerData update field reads) gets it at once.
            std::string const surname = kPersonaSurnames[e.persona];
            sCharacterCache->UpdateCharacterSurname(r.guid, surname);
            CharacterDatabase.PExecute("UPDATE characters SET surname = '{}' WHERE guid = {}", surname, e.guid_low);
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

// The run's clock and the bots that ended their session, kept across restarts (created here: no SQL update needed).
static void EnsureRunTables()
{
    static bool done = false;
    if (done)
        return;
    done = true;
    CharacterDatabase.DirectExecute("CREATE TABLE IF NOT EXISTS playerbot_launch_run (id TINYINT UNSIGNED NOT NULL PRIMARY KEY, "
                                    "elapsed_s INT UNSIGNED NOT NULL DEFAULT 0)");
    CharacterDatabase.DirectExecute("CREATE TABLE IF NOT EXISTS playerbot_launch_done (character_guid_low BIGINT UNSIGNED NOT NULL PRIMARY KEY)");
}

void LaunchSim::SaveClock(uint32 elapsed_s)
{
    EnsureRunTables();
    resume_elapsed_s_ = elapsed_s;
    CharacterDatabase.PExecute("REPLACE INTO playerbot_launch_run (id, elapsed_s) VALUES (1, {})", elapsed_s);
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
        // A roster made before the persona surnames (or a row whose UPDATE lost the race with its INSERT) gets them here.
        sCharacterCache->UpdateCharacterSurname(ObjectGuid::Create<HighGuid::Player>(roster_[i].guid_low), kPersonaSurnames[roster_[i].persona]);
    }
    {
        std::string elt;
        for (char const* s : kPersonaSurnames)
            elt += std::string(", '") + s + "'";
        CharacterDatabase.PExecute("UPDATE characters c JOIN playerbot_launch_bot b ON b.character_guid_low = c.guid "
            "SET c.surname = ELT(b.persona + 1{}) WHERE b.persona < {}", elt, uint32(kPersonaSurnames.size()));
    }
    EnsureRunTables();
    if (QueryResult run = CharacterDatabase.Query("SELECT elapsed_s FROM playerbot_launch_run WHERE id = 1"))
        resume_elapsed_s_ = run->Fetch()[0].GetUInt32();
    if (QueryResult done = CharacterDatabase.Query("SELECT character_guid_low FROM playerbot_launch_done"))
        do
        {
            auto const it = by_guid_.find(done->Fetch()[0].GetUInt64());
            if (it != by_guid_.end())
                roster_[it->second].state = BotState::Done;
        } while (done->NextRow());
    phase_ = Phase::Ready;
    TC_LOG_INFO("playerbot.v2", "[launch] roster loaded: {} bots (clock T0+{} min)", roster_.size(), resume_elapsed_s_ / 60u);
}

std::string LaunchSim::Prepare(uint32 bots, uint32 late)
{
    LoadRoster();
    if (!roster_.empty())
        return "A roster exists already (" + std::to_string(roster_.size()) + " bots): use `.playerbot launch reset` for a fresh one.";
    if (bots == 0 || bots + late > 2000)
        return "Bots must be 1-2000 in total.";
    LaunchRoutes::Instance().Load();   // pick up routes imported since the server started
    // `bots` start in Coldridge Valley; the other Alliance starting zones are sized from it by the census (Coldridge's dwarves
    // and gnomes against the zone's race: 100 in Coldridge -> 93 humans, 56 night elves, 59 Skyborne). The config overrides a
    // zone (Playerbot.Launch.NorthshireBots, .ShadowglenBots, .ZephrasBots; -1 = by the census, 0 = none).
    // Order: Coldridge, Northshire, Shadowglen, Zephras, then the late arrivals (Coldridge).
    auto const zone_count = [bots](char const* key, uint8 zone)
    {
        int32 const set = sConfigMgr->GetIntDefault(key, -1, true);
        if (set >= 0)
            return uint32(std::min(set, 1000));
        return uint32((uint64(bots) * CensusShareOf(zone) + CensusShareOf(LaunchPlaces::kZoneColdridge) / 2) /
                      CensusShareOf(LaunchPlaces::kZoneColdridge));
    };
    std::array<uint32, LaunchPlaces::kZoneCount> per_zone = { bots,
        zone_count("Playerbot.Launch.NorthshireBots", LaunchPlaces::kZoneNorthshire),
        zone_count("Playerbot.Launch.ShadowglenBots", LaunchPlaces::kZoneShadowglen),
        zone_count("Playerbot.Launch.ZephrasBots", LaunchPlaces::kZoneZephras) };
    uint32 total = late;
    for (uint32 n : per_zone) total += n;
    if (total > 2000)
        return "Bots must be 1-2000 in total (all zones).";
    roster_.resize(total);
    if (int32 const roster_seed = sConfigMgr->GetIntDefault("Playerbot.Launch.RosterSeed", 0, true); roster_seed > 0)
        gRollRng.emplace(uint32(roster_seed));
    size_t i = 0;
    for (uint8 z = 0; z < LaunchPlaces::kZoneCount; ++z)
        for (uint32 k = 0; k < per_zone[z]; ++k, ++i)
            Roll(roster_[i], /*keep_identity*/ false, /*late*/ false, z);
    for (; i < roster_.size(); ++i)
        Roll(roster_[i], /*keep_identity*/ false, /*late*/ true, LaunchPlaces::kZoneColdridge);
    gRollRng.reset();
    late_count_ = late;
    phase_ = Phase::Creating;
    return "Creating " + std::to_string(bots) + " launch bots in Coldridge + " + std::to_string(per_zone[1]) + " Northshire + " +
           std::to_string(per_zone[2]) + " Shadowglen + " + std::to_string(per_zone[3]) + " Zephras + " + std::to_string(late) +
           " late arrivals (a few per tick). `.playerbot launch status` shows the progress; then `.playerbot launch arm` and log in.";
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
    // Resume a stopped or paused run at its clock (the bots that were playing log in again at once: their arrival has passed);
    // a fresh run brings back the bots that ended their session.
    t0_ms_ = GameTime::GetGameTimeMS() - resume_elapsed_s_ * 1000u;
    asks_ = grants_ = 0;
    replies_.clear();
    phase_ = Phase::Running;
    if (resume_elapsed_s_)
    {
        TC_LOG_INFO("playerbot.v2", "[launch] the run resumes at T0+{} min", resume_elapsed_s_ / 60u);
        return "Launch resumed at T0 + " + std::to_string(resume_elapsed_s_ / 60u) + " min.";
    }
    EnsureRunTables();
    CharacterDatabase.Execute("DELETE FROM playerbot_launch_done");
    for (Entry& e : roster_)
        if (e.guid_low && e.state == BotState::Done)
            e.state = BotState::Offline;
    SaveClock(0);
    TC_LOG_INFO("playerbot.v2", "[launch] T0: {} bots arrive over the next hours", roster_.size());
    return "Launch started: " + std::to_string(roster_.size()) + " bots arrive from now on.";
}

std::string LaunchSim::Stop()
{
    LoadRoster();
    if (roster_.empty())
        return "No roster.";
    if (phase_ == Phase::Running)
        SaveClock((GameTime::GetGameTimeMS() - t0_ms_) / 1000u);
    phase_ = Phase::Stopping;
    recreate_after_delete_ = false;
    return "Stopping: every launch bot logs out; start resumes at T0 + " + std::to_string(resume_elapsed_s_ / 60u) + " min.";
}

std::string LaunchSim::Reset(uint32 bots, uint32 late)
{
    LoadRoster();
    if (roster_.empty())
        return "No roster: use `.playerbot launch prepare`.";
    if (bots + late > 2000)
        return "Bots must be 1-2000 in total.";
    SaveClock(0);
    CharacterDatabase.Execute("DELETE FROM playerbot_launch_done");
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
    std::array<uint32, 5> by_state{};
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
      << ", done " << by_state[4];
    if (phase_ == Phase::Running)
        o << "; T0 + " << (GameTime::GetGameTimeMS() - t0_ms_) / 60000u << " min";
    else if (resume_elapsed_s_)
        o << "; resumes at T0 + " << resume_elapsed_s_ / 60u << " min";
    o << "\nOnline by zone:";
    {
        std::array<uint32, LaunchPlaces::kZoneCount> by_zone{};
        for (Entry const& e : roster_)
            if (e.state == BotState::Online)
                ++by_zone[LaunchPlaces::ZoneForRace(e.race)];
        for (uint8 z = 0; z < LaunchPlaces::kZoneCount; ++z)
            if (by_zone[z])
                o << ' ' << LaunchPlaces::kZones[z].key << '=' << by_zone[z];
    }
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
      << " grouped=" << grouped << " inv_asked=" << asks_ << " inv_granted=" << grants_
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
    if (!auto_init_done_ && AutoRun())
    {
        auto_init_done_ = true;
        LoadRoster();
        if (!roster_.empty())
            TC_LOG_INFO("playerbot.v2", "[launch] AutoRun: the run resumes at T0+{} min when a real player logs in",
                resume_elapsed_s_ / 60u);
    }
    if (!replies_.empty())
        SendReplies(now_ms);
    if (phase_ == Phase::Running && now_ms - last_report_ms_ >= 60000u)
    {
        last_report_ms_ = now_ms;
        TC_LOG_INFO("playerbot.v2", "[launch] report {}", Report());
        SaveClock((now_ms - t0_ms_) / 1000u);   // a crash or a restart resumes within a minute of where it was
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
            if (auto_start_pending_ && AutoRun() && RealPlayersOnline(nullptr) > 0)
            {
                auto_start_pending_ = false;
                TC_LOG_INFO("playerbot.v2", "[launch] a real player is waiting: the launch starts (AutoRun)");
                Start();
            }
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
                            e.state = BotState::Done;
                            CharacterDatabase.PExecute("REPLACE INTO playerbot_launch_done (character_guid_low) VALUES ({})", e.guid_low);
                            TC_LOG_INFO("playerbot.v2", "[launch] {} ({}) ends its session after {} min", e.name,
                                kPersonas[e.persona].key, e.session_s / 60u);
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
                if (e.guid_low && e.state != BotState::Done)
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
                Roll(roster_[i], /*keep_identity*/ true, /*late*/ i + late_count_ >= roster_.size(),
                     LaunchPlaces::ZoneForRace(roster_[i].race));
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
    if (!p)
        return;
    if (AutoRun() && RealPlayersOnline(p) == 0)
    {
        LoadRoster();
        if (phase_ == Phase::Ready || phase_ == Phase::Armed)
        {
            TC_LOG_INFO("playerbot.v2", "[launch] {} entered the world alone: the launch starts (AutoRun)", p->GetName());
            Start();
        }
        else if (phase_ != Phase::Running)
            auto_start_pending_ = true;   // the roster is being made: it starts once ready
        return;
    }
    if (phase_ != Phase::Armed)
        return;
    TC_LOG_INFO("playerbot.v2", "[launch] {} entered the world: launch starts", p->GetName());
    Start();
}

void LaunchSim::OnRealPlayerLogout(Player* p)
{
    if (!p || !AutoRun() || RealPlayersOnline(p) != 0)
        return;
    auto_start_pending_ = false;
    if (phase_ != Phase::Running && phase_ != Phase::Armed)
        return;
    TC_LOG_INFO("playerbot.v2", "[launch] {} left, nobody is online: the run pauses until the next login (AutoRun)", p->GetName());
    if (phase_ == Phase::Running)
        Stop();
    else
        phase_ = Phase::Ready;   // armed, never started
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
    lp.route = PickRoute(pr, e.seed, p->GetRace());
    ai->set_launch(lp);
    // A gatherer picks up its gathering profession right away (a shortcut for the trip to the trainer in Kharanos or
    // Ironforge): dwarves mostly Mining, with a pick, gnomes mostly Herbalism. Its gather rule (idle:gather) then detours to
    // the veins and herbs it passes. Only on the first login of a fresh character (no skill yet).
    if (pr.strategy == LaunchStrategy::Gatherer && !p->HasSkill(SKILL_MINING) && !p->HasSkill(SKILL_HERBALISM))
    {
        bool const mining = p->GetRace() == RACE_DWARF ? Unit01(e.seed, 21) < 0.75f : Unit01(e.seed, 21) < 0.35f;
        if (mining)
        {
            p->LearnSpell(2575, false);   // Mining (Apprentice)
            p->LearnSpell(2580, false);   // Find Minerals
            p->StoreNewItemInBestSlots(2901, 1, ItemContext::NONE);   // Mining Pick
        }
        else
        {
            p->LearnSpell(2366, false);   // Herbalism (Apprentice)
            p->LearnSpell(2383, false);   // Find Herbs
        }
    }
    TC_LOG_INFO("playerbot.v2", "[launch] {} L{} {} {} arrives: persona {} pace {:.2f} temperament {} setup {}s session {} route {}",
        p->GetName(), uint32(p->GetLevel()), uint32(p->GetRace()), uint32(p->GetClass()), pr.key, lp.pace,
        uint32(lp.temperament), lp.setup_ms / 1000u, e.session_s ? std::to_string(e.session_s / 60u) + " min" : std::string("open"),
        lp.route == LaunchRoutes::kNoRoute ? std::string("-") : LaunchRoutes::Instance().Names()[lp.route]);
    return true;
}

} // namespace Playerbot::V2::Fleet
