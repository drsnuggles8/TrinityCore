// Launch-simulation idle rules (Fleet/LaunchSim, Bot/LaunchProfile.h).
//
// idle:launch_hold @995 - a launch bot that is setting up its UI after login, or away from the keyboard, does nothing at all:
// above the combat opener (993) and every errand, so it stands where it is. A mob that attacks it still starts a fight (the
// combat state is not an idle rule), like a real AFK player who gets pulled.
//
// The pauses come from the profile alone (login time, setup length, AFK rhythm, per-bot seed), so the rule keeps no state:
// AFK window k of length afk_every_ms holds one break at an offset and of a length hashed from (seed, k).
//
// Groups (temperaments, LaunchTemperament). Forever has no shared tagging: a mob tagged by someone else pays only the
// tagger's group, so on launch day people whisper the tagger for an invite and leave right after the kill.
// idle:launch_ask_invite @850 - a launch bot that wants a mob another player has tagged (its quest needs the mob) whispers
//   the tagger "inv?" (Fleet/LaunchSim::OnAskInvite answers for a launch bot). Greedy bots ask most, loners never.
// idle:launch_leave_group @852 - leaves the group out of combat once it has stayed long enough for its temperament:
//   greedy bots and loners right after the kill, friendly ones after a while.
// LaunchAcceptsInvite - the group_invite_accept rule asks it whether a launch bot takes an invite.
//
// idle:launch_party_build @849 - a party builder (LaunchStrategy::PartyBuilder) working a kill objective invites solo
//   players of about its level close by, up to a group of three, with a "LF group" line the first time.
// idle:launch_chatter @420 - now and then a line in General (or /say) of the kind a launch crowd writes: the camp is
//   crowded, where is the cave, LFG, first time in Classic. Casuals and friendly bots talk most, loners and racers least.
// idle:launch_snipe @860 - a kill stealer (LaunchStrategy::KillStealer) goes for untagged mobs another player stands next
//   to (about to pull, or walking up to), within 35 y and its level band: it tags first and the other player loses the mob.
// idle:launch_hearth @870 - an explorer whose route reached a hearth stop uses its hearthstone (and stands still for the
//   cast, idle:launch_hold).
// idle:launch_tram @871 - an explorer at a Deeprun Tram portal rides to the other city: it stands for the ride
//   (LaunchPlaces::kTramRideMs, idle:launch_hold) and comes out at the other portal. The car itself is not ridden (the
//   travel planner walked the explorers over Searing Gorge instead).
// idle:launch_train @740 - after each even level from 4 a launch bot walks to its class trainer (the builder names it in
//   the snapshot) and buys what it can afford, then reads the list a few seconds (idle:launch_hold). Above the quest and
//   vendor errands, below the social rules.

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/BotIntent.h"
#include "Bot/LaunchPlaces.h"
#include "Bot/LaunchProfile.h"
#include "QuestDef.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace Playerbot {

namespace {

uint32 Mix(uint32 a, uint32 b)
{
    uint32 h = a * 2654435761u ^ (b + 0x9E3779B9u + (a << 6) + (a >> 2));
    h ^= h >> 15;
    h *= 2246822519u;
    h ^= h >> 13;
    return h;
}

// Why the bot is holding right now ("" = it is not).
char const* HoldReason(LaunchProfile const& lp, uint32 now)
{
    if (!lp.active || now < lp.login_ms)
        return "";
    uint32 const since = now - lp.login_ms;
    if (since < lp.setup_ms)
        return "setup";
    if (lp.afk_every_ms == 0 || lp.afk_max_ms == 0)
        return "";
    uint32 const k = since / lp.afk_every_ms;
    if (k == 0)
        return "";   // no break in the first window: the setup pause already covers the start
    uint32 const h = Mix(lp.seed, k);
    uint32 const len = lp.afk_min_ms + (lp.afk_max_ms > lp.afk_min_ms ? h % (lp.afk_max_ms - lp.afk_min_ms) : 0u);
    uint32 const room = lp.afk_every_ms > len ? lp.afk_every_ms - len : 0u;
    uint32 const start = k * lp.afk_every_ms + (room ? (h >> 8) % room : 0u);
    return since >= start && since < start + len ? "afk" : "";
}

bool LaunchHoldGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    if (ai.launch().active && s.published_at_ms() < ai.launch_social().busy_until_ms && !s.in_combat())
        return true;
    return *HoldReason(ai.launch(), s.published_at_ms()) != '\0';
}

bool LaunchHoldFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    if (s.is_moving())
        emit.stop_movement();
    char const* why = HoldReason(ai.launch(), s.published_at_ms());
    ai.set_last_rule_fired(*why == 's' ? "idle:launch_setup" : *why == 'a' ? "idle:launch_afk" : "idle:launch_browse");
    return true;
}

// A kill objective of an unfinished quest in the bot's log needs this creature.
bool QuestNeedsMob(BotSnapshotView const& s, uint32 entry)
{
    for (QuestEntry const& q : s.raw().quest_log.quests)
    {
        if (q.state != 0)
            continue;
        for (QuestObjectiveEntry const& o : q.objectives)
        {
            if (o.type != QUEST_OBJECTIVE_MONSTER || o.progress >= o.amount)
                continue;
            if (uint32(o.object_id) == entry ||
                std::find(o.credit_alias_entries.begin(), o.credit_alias_entries.end(), entry) != o.credit_alias_entries.end())
                return true;
        }
    }
    return false;
}

// Per temperament (Neutral, Friendly, Greedy, Loner, Griefer).
constexpr std::array<uint8, 5>  kAskPct    = {12, 8, 35, 0, 20};      // asks the tagger of a mob it wants
constexpr std::array<uint8, 5>  kAcceptPct = {65, 100, 70, 10, 50};   // takes an invite it did not ask for
constexpr std::array<uint16, 5> kStayLoS   = {60, 150, 20, 10, 30};   // stays in a group this long (s), up to twice as long (it follows the leader meanwhile)
constexpr uint32 kAskEveryMs = 90000;   // 300 bots at 40 s asked ~24 times a minute

uint8 Temper(LaunchProfile const& lp) { return std::min<uint8>(uint8(lp.temperament), 4); }

ObjectGuid AskTarget(BotSnapshotView const& s, BotAI& ai)
{
    float bx, by, bz;
    s.position(bx, by, bz);
    uint32 const pct = kAskPct[Temper(ai.launch())];
    for (NearbyUnit const& u : s.raw().combat.nearby_enemies)
    {
        if (!u.tapped || u.tapped_by_us || u.hp <= 0 || !u.victim.IsPlayer() || u.victim == s.raw().guid)
            continue;
        float const dx = u.x - bx, dy = u.y - by;
        if (dx * dx + dy * dy > 40.0f * 40.0f)
            continue;
        // Each tagger at most once in 5 minutes, and only when this bot is the asking kind for this mob.
        if (ai.action_recently_tried(BotAI::ActionKind::InviteOther, u.victim.GetCounter(), s.published_at_ms()))
            continue;
        if (Mix(ai.launch().seed, uint32(u.guid.GetCounter())) % 100u >= pct)
            continue;
        if (!QuestNeedsMob(s, u.entry))
            continue;
        return u.victim;
    }
    return ObjectGuid::Empty;
}

bool LaunchAskInviteGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    LaunchProfile const& lp = ai.launch();
    if (!lp.active || kAskPct[Temper(lp)] == 0 || s.in_group() || s.has_group_invite() || !s.is_alive())
        return false;
    uint32 const now = s.published_at_ms();
    if (ai.launch_social().last_ask_ms && now - ai.launch_social().last_ask_ms < kAskEveryMs)
        return false;
    return !AskTarget(s, ai).IsEmpty();
}

bool LaunchAskInviteFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    ObjectGuid const tagger = AskTarget(s, ai);
    if (tagger.IsEmpty())
        return false;
    uint32 const now = s.published_at_ms();
    ai.note_action_retry(BotAI::ActionKind::InviteOther, tagger.GetCounter(), now);
    ai.launch_social().last_ask_ms = now;
    ai.launch_social().asked_low = tagger.GetCounter();
    emit.ask_for_invite(tagger);
    ai.set_last_rule_fired("idle:launch_ask_invite");
    return true;
}

bool LaunchLeaveGroupGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    LaunchProfile const& lp = ai.launch();
    if (!lp.active)
        return false;
    LaunchSocial& soc = ai.launch_social();
    uint32 const now = s.published_at_ms();
    if (!s.in_group())
    {
        soc.group_since_ms = 0;
        return false;
    }
    if (soc.group_since_ms == 0)
        soc.group_since_ms = now;
    if (s.in_combat() || !s.is_alive())
        return false;
    uint32 const lo = kStayLoS[Temper(lp)] * 1000u;
    uint32 const stay = lo + Mix(lp.seed, soc.group_since_ms / 1000u) % (lo + 1u);
    return now - soc.group_since_ms >= stay;
}

bool LaunchLeaveGroupFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    LaunchProfile const& lp = ai.launch();
    uint32 const h = Mix(lp.seed, s.published_at_ms() / 1000u);
    // Friendly and neutral players often say thanks on the way out.
    static constexpr std::array<char const*, 5> kBye = {"ty", "thx", "ty!", "thanks", "gl"};
    if ((lp.temperament == LaunchTemperament::Friendly || lp.temperament == LaunchTemperament::Neutral) && h % 100u < 50u)
        emit.say(kBye[(h >> 8) % kBye.size()]);
    emit.group_leave();
    ai.launch_social().group_since_ms = 0;
    ai.launch_social().asked_low = 0;
    ai.set_last_rule_fired("idle:launch_leave_group");
    return true;
}

ObjectGuid PartyRecruit(BotSnapshotView const& s, BotAI& ai)
{
    float bx, by, bz;
    s.position(bx, by, bz);
    for (NearbyUnit const& u : s.raw().combat.nearby_friends)
    {
        if (!u.is_player || u.is_in_group || u.guid == s.raw().guid || !u.victim.IsEmpty())
            continue;
        int32 const dl = int32(u.level) - int32(s.level());
        if (dl > 2 || dl < -2)
            continue;
        float const dx = u.x - bx, dy = u.y - by;
        if (dx * dx + dy * dy > 30.0f * 30.0f)
            continue;
        if (ai.action_recently_tried(BotAI::ActionKind::InviteOther, u.guid.GetCounter(), s.published_at_ms()))
            continue;
        return u.guid;
    }
    return ObjectGuid::Empty;
}

bool LaunchPartyBuildGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const& g, uint32)
{
    LaunchProfile const& lp = ai.launch();
    if (!lp.active || lp.strategy != LaunchStrategy::PartyBuilder || !s.is_alive() || s.in_combat() || s.has_group_invite())
        return false;
    if (s.in_group() && g.member_count() >= 3)
        return false;
    auto const& obj = s.raw().quest_log.current_objective;
    if (s.raw().quest_log.current_quest_id == 0 || obj.type != QUEST_OBJECTIVE_MONSTER || obj.progress >= obj.amount)
        return false;
    return !PartyRecruit(s, ai).IsEmpty();
}

bool LaunchPartyBuildFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    ObjectGuid const who = PartyRecruit(s, ai);
    if (who.IsEmpty())
        return false;
    uint32 const now = s.published_at_ms();
    if (!s.in_group() && !ai.action_recently_tried(BotAI::ActionKind::InviteOther, 0, now))
    {
        static constexpr std::array<char const*, 4> kLf = {"LF group for this, inv up", "anyone want to group?", "grp?", "LFG, send inv"};
        emit.say_world(kLf[Mix(ai.launch().seed, now / 60000u) % kLf.size()]);
        ai.note_action_retry(BotAI::ActionKind::InviteOther, 0, now);   // key 0: the shout, once in 5 minutes
    }
    ai.note_action_retry(BotAI::ActionKind::InviteOther, who.GetCounter(), now);
    emit.invite_to_group(who);
    ai.set_last_rule_fired("idle:launch_party_build");
    return true;
}

// Minutes between two lines, by temperament (Neutral, Friendly, Greedy, Loner, Griefer); scaled by pace, so a casual talks
// more than a racer. 0 = never.
constexpr std::array<uint16, 5> kChatEveryMin = {25, 15, 40, 0, 20};

bool LaunchChatterGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    LaunchProfile const& lp = ai.launch();
    if (!lp.active || kChatEveryMin[Temper(lp)] == 0 || s.in_combat() || !s.is_alive())
        return false;
    uint32 const now = s.published_at_ms();
    LaunchSocial& soc = ai.launch_social();
    if (soc.next_chat_ms == 0)
    {
        // First line somewhere in the first interval after login, so the crowd does not all talk at once.
        uint32 const every = uint32(float(kChatEveryMin[Temper(lp)]) * 60000.0f / std::max(lp.pace, 1.0f) * 1.5f);
        soc.next_chat_ms = now + Mix(lp.seed, 11) % std::max(every, 60000u);
        return false;
    }
    return int32(now - soc.next_chat_ms) >= 0;
}

bool LaunchChatterFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    LaunchProfile const& lp = ai.launch();
    uint32 const now = s.published_at_ms();
    uint32 const h = Mix(lp.seed, now / 1000u);
    static constexpr std::array<char const*, 28> kLines = {
        "so many people lol", "wolves are camped to death", "this is insane", "anyone know where the troll cave is?",
        "LFG Grik'nir", "where do i find the wolf meat", "first time playing classic!", "layer pls",
        "can't find a single boar", "lol 20 people on one wolf", "inv for wolves?", "who is the frost mage stealing tags",
        "this server is packed", "rip mobs", "is the cave to the west?", "where is Talin?", "need a group for the trolls",
        "tag stealers everywhere", "how do you even quest here", "omg the respawns", "brb food", "back",
        "anyone selling linen? jk", "Grik'nir group forming, /w me", "go go go", "nice layer", "gl everyone", "hi all"};
    std::string const line = kLines[h % kLines.size()];
    // Mostly General (channel 1), sometimes /say to whoever is around.
    if ((h >> 8) % 100u < 70u)
        emit.emit(ChatIntent{ ChannelChatIntent{ uint8(1), line } });
    else
        emit.say_world(line);
    uint32 const every = uint32(float(kChatEveryMin[Temper(lp)]) * 60000.0f / std::max(lp.pace, 1.0f));
    // Next line in half to one and a half intervals.
    ai.launch_social().next_chat_ms = now + every / 2u + (h >> 4) % std::max(every, 1u);
    ai.set_last_rule_fired("idle:launch_chatter");
    return true;
}

ObjectGuid SnipeTarget(BotSnapshotView const& s, BotAI& ai)
{
    float bx, by, bz;
    s.position(bx, by, bz);
    for (NearbyUnit const& u : s.raw().combat.nearby_enemies)
    {
        if (u.tapped || u.hp <= 0 || !u.victim.IsEmpty() || int32(u.level) > int32(s.level()) + 2)
            continue;
        float const dx = u.x - bx, dy = u.y - by;
        if (dx * dx + dy * dy > 35.0f * 35.0f)
            continue;
        if (ai.action_recently_tried(BotAI::ActionKind::PveTargetSwitch, u.guid.GetCounter(), s.published_at_ms()))
            continue;
        for (NearbyUnit const& f : s.raw().combat.nearby_friends)
        {
            if (!f.is_player || f.guid == s.raw().guid)
                continue;
            float const fx = f.x - u.x, fy = f.y - u.y;
            if (fx * fx + fy * fy <= 12.0f * 12.0f)
                return u.guid;
        }
    }
    return ObjectGuid::Empty;
}

bool LaunchSnipeGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    LaunchProfile const& lp = ai.launch();
    if (!lp.active || lp.strategy != LaunchStrategy::KillStealer || s.in_combat() || !s.is_alive() || s.hp_pct() < 60)
        return false;
    return !SnipeTarget(s, ai).IsEmpty();
}

bool LaunchSnipeFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    ObjectGuid const mob = SnipeTarget(s, ai);
    if (mob.IsEmpty())
        return false;
    ai.note_action_retry(BotAI::ActionKind::PveTargetSwitch, mob.GetCounter(), s.published_at_ms());
    emit.start_attack(mob);
    ai.set_last_rule_fired("idle:launch_snipe");
    return true;
}

bool LaunchHearthGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    return ai.launch().active && ai.launch_hearth_wanted() && s.is_alive() && !s.in_combat();
}

bool LaunchHearthFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    if (s.is_moving())
        emit.stop_movement();
    emit.hearth();
    ai.set_launch_hearth_wanted(false);
    ai.launch_social().busy_until_ms = s.published_at_ms() + 11000u;   // the 10 s cast
    ai.set_last_rule_fired("idle:launch_hearth");
    return true;
}

bool LaunchTramGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    return ai.launch().active && ai.launch_tram_to() != 0 && s.is_alive() && !s.in_combat();
}

bool LaunchTramFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    LaunchSocial& soc = ai.launch_social();
    uint32 const now = s.published_at_ms();
    if (soc.tram_since_ms == 0)
    {
        // Down to the platform and onto the car: idle:launch_hold keeps the bot still for the ride.
        soc.tram_since_ms = now;
        soc.busy_until_ms = now + LaunchPlaces::kTramRideMs;
        if (s.is_moving())
            emit.stop_movement();
        ai.set_last_rule_fired("idle:launch_tram");
        return true;
    }
    LaunchPlaces::Spot const& to = ai.launch_tram_to() == 1 ? LaunchPlaces::kTramStormwind : LaunchPlaces::kTramIronforge;
    emit.near_teleport_to(to.x, to.y, to.z);
    ai.set_launch_tram_to(0);
    soc.tram_since_ms = 0;
    ai.set_last_rule_fired("idle:launch_tram_arrive");
    return true;
}

bool LaunchTrainGate(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, uint32)
{
    return ai.launch().active && !s.raw().launch_trainer.IsEmpty() && s.is_alive() && !s.in_combat();
}

bool LaunchTrainFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    auto const& r = s.raw();
    float bx, by, bz;
    s.position(bx, by, bz);
    float const dx = r.launch_trainer_x - bx, dy = r.launch_trainer_y - by;
    float const d = std::sqrt(dx * dx + dy * dy);
    uint32 const now = s.published_at_ms();
    LaunchSocial& soc = ai.launch_social();
    if (d > 4.5f)
    {
        // One order every 2.5 s while it walks: the move generator keeps the path between orders.
        if (soc.train_move_ms && now - soc.train_move_ms < 2500u && s.is_moving())
            return false;
        soc.train_move_ms = now;
        float const k = (d - 3.0f) / d;
        emit.move_to(bx + dx * k, by + dy * k, r.launch_trainer_z, /*run*/ true);
        ai.set_last_rule_fired("idle:launch_train_walk");
        return true;
    }
    emit.trainer_buy_all(r.launch_trainer);
    ai.set_launch_trained_level(uint8(s.level() & ~1u));
    LaunchProfile const& lp = ai.launch();
    soc.busy_until_ms = now + uint32(float(4000u + Mix(lp.seed, now / 1000u) % 8000u) * std::min(lp.pace, 3.0f));
    ai.set_last_rule_fired("idle:launch_train");
    return true;
}

} // anonymous namespace

bool LaunchAcceptsInvite(BotSnapshotView const& s, BotAI& ai)
{
    LaunchProfile const& lp = ai.launch();
    uint64 const leader = s.group_invite_leader().GetCounter();
    if (leader != 0 && leader == ai.launch_social().asked_low)
        return true;
    return Mix(lp.seed, uint32(leader)) % 100u < kAcceptPct[Temper(lp)];
}

void RegisterLaunchRules(IdleRuleRegistry& r)
{
    {
        IdleRule rule;
        rule.name     = "idle:launch_hold";
        rule.priority = 995;   // above the combat opener (993): a player at the keybind menu or AFK does nothing
        rule.gate     = &LaunchHoldGate;
        rule.fire     = &LaunchHoldFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_leave_group";
        rule.priority = 852;   // below group_invite_accept (855)
        rule.gate     = &LaunchLeaveGroupGate;
        rule.fire     = &LaunchLeaveGroupFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_train";
        rule.priority = 740;   // above the vendor (735) and quest errands, below the social rules
        rule.gate     = &LaunchTrainGate;
        rule.fire     = &LaunchTrainFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_tram";
        rule.priority = 871;
        rule.gate     = &LaunchTramGate;
        rule.fire     = &LaunchTramFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_hearth";
        rule.priority = 870;
        rule.gate     = &LaunchHearthGate;
        rule.fire     = &LaunchHearthFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_snipe";
        rule.priority = 860;   // above the quest and social rules: the point of a kill stealer
        rule.gate     = &LaunchSnipeGate;
        rule.fire     = &LaunchSnipeFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_chatter";
        rule.priority = 420;   // low: only when nothing more pressing fires
        rule.gate     = &LaunchChatterGate;
        rule.fire     = &LaunchChatterFire;
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_party_build";
        rule.priority = 849;
        rule.gate     = &LaunchPartyBuildGate;
        rule.fire     = &LaunchPartyBuildFire;
        rule.min_interval_ms = 8000;   // per bot
        r.register_rule(std::move(rule));
    }
    {
        IdleRule rule;
        rule.name     = "idle:launch_ask_invite";
        rule.priority = 850;
        rule.gate     = &LaunchAskInviteGate;
        rule.fire     = &LaunchAskInviteFire;
        r.register_rule(std::move(rule));
    }
}

} // namespace Playerbot
