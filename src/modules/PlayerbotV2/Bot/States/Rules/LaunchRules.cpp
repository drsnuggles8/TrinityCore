// Launch-simulation idle rules (Fleet/LaunchSim, Bot/LaunchProfile.h).
//
// idle:launch_hold @995 - a launch bot that is setting up its UI after login, or away from the keyboard, does nothing at all:
// above the combat opener (993) and every errand, so it stands where it is. A mob that attacks it still starts a fight (the
// combat state is not an idle rule), like a real AFK player who gets pulled.
//
// The pauses come from the profile alone (login time, setup length, AFK rhythm, per-bot seed), so the rule keeps no state:
// AFK window k of length afk_every_ms holds one break at an offset and of a length hashed from (seed, k).

#include "Bot/IdleRule.h"
#include "Group/GroupSnapshot.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/LaunchProfile.h"

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

} // anonymous namespace

void RegisterLaunchRules(IdleRuleRegistry& r)
{
    IdleRule rule;
    rule.name     = "idle:launch_hold";
    rule.priority = 995;   // above the combat opener (993): a player at the keybind menu or AFK does nothing
    rule.gate     = &LaunchHoldGate;
    rule.fire     = &LaunchHoldFire;
    r.register_rule(std::move(rule));
}

} // namespace Playerbot
