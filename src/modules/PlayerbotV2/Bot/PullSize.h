#pragma once
// How many mobs a pull brings, and whether a bot is ready for it. A pulled mob brings the idle aggressive mobs standing next to it
// (its pack); neutral mobs, mobs already fighting and dead ones stay where they are. A bot pulls one mob as it is, but rests up
// before a pull of two or more (Olo, 29 Oct: melee bots walked into the Coldridge troll cave at half health, met three whelps at
// once, fled and started over, again and again). Pure: tested in tests/game/ClassicPullSize.cpp.

#include <cstdint>
#include <vector>

namespace Playerbot::PullSize {

constexpr float kPackRadius = 10.0f;            // mobs this close to the pulled one come with it (State_Idle kEngagedPackRadius)
constexpr std::int32_t kReadyHealthPct = 90;    // a pull of two or more waits for this much health ...
constexpr std::int32_t kReadyManaPct = 80;      // ... and this much mana (classes with mana)
constexpr std::uint32_t kMaxWaitMs = 120000;    // the wait gives up after this long without reaching the mark (no regeneration) ...
constexpr std::uint32_t kSkipAfterGiveUpMs = 60000; // ... and the bot pulls as it is for this long

// A mob that comes along when its neighbour is pulled: alive, a creature, targetable, idle (no victim, out of combat) and aggressive.
template <class Unit>
bool JoinsPull(Unit const& u)
{
    return u.hp > 0 && !u.is_player && !u.untargetable && !u.is_pacified && u.aggressive && u.victim.IsEmpty() && !u.unit_in_combat;
}

// The mobs a pull of `target` brings: the target plus every mob of `units` that joins (JoinsPull) within kPackRadius of it, in 3D.
template <class Unit>
std::uint32_t Estimate(Unit const& target, std::vector<Unit> const& units)
{
    std::uint32_t size = 1;
    for (Unit const& u : units)
    {
        if (u.guid == target.guid || !JoinsPull(u))
            continue;
        float const dx = u.x - target.x, dy = u.y - target.y, dz = u.z - target.z;
        if (dx * dx + dy * dy + dz * dz <= kPackRadius * kPackRadius)
            ++size;
    }
    return size;
}

// Ready for a pull of `size` mobs: any bot takes one mob (its usual recovery rules apply); two or more want kReadyHealthPct health
// and, for a class with mana, kReadyManaPct mana.
constexpr bool Ready(std::uint32_t size, std::int32_t health_pct, std::int32_t mana_pct, bool uses_mana)
{
    if (size < 2)
        return true;
    return health_pct >= kReadyHealthPct && (!uses_mana || mana_pct >= kReadyManaPct);
}

} // namespace Playerbot::PullSize
