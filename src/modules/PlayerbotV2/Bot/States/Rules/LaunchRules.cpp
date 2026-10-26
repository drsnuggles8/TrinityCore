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

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/LaunchProfile.h"
#include "QuestDef.h"
#include <algorithm>
#include <array>

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
    return *HoldReason(ai.launch(), s.published_at_ms()) != '\0';
}

bool LaunchHoldFire(BotSnapshotView const& s, BotAI& ai, GroupSnapshotView const&, BotIntentEmitter& emit, uint32)
{
    if (s.is_moving())
        emit.stop_movement();
    ai.set_last_rule_fired(*HoldReason(ai.launch(), s.published_at_ms()) == 's' ? "idle:launch_setup" : "idle:launch_afk");
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
constexpr std::array<uint8, 5>  kAskPct    = {25, 15, 70, 0, 40};     // asks the tagger of a mob it wants
constexpr std::array<uint8, 5>  kAcceptPct = {65, 100, 70, 10, 50};   // takes an invite it did not ask for
constexpr std::array<uint16, 5> kStayLoS   = {90, 300, 20, 10, 30};   // stays in a group this long (s), up to twice as long
constexpr uint32 kAskEveryMs = 40000;

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
        rule.name     = "idle:launch_ask_invite";
        rule.priority = 850;
        rule.gate     = &LaunchAskInviteGate;
        rule.fire     = &LaunchAskInviteFire;
        r.register_rule(std::move(rule));
    }
}

} // namespace Playerbot
