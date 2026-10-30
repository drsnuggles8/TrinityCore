#pragma once
// The per-frame allowance of snapshot builds for the slow tiers (Cruise, Idle, Parked), TickScheduler::should_build_snapshot.
// The bots are asked in the same registry order every frame, so a plain first-come cap let the first bots in that order take
// every build whenever more were due than it allows, and the last ones never built again: their snapshot, and with it the
// clock their rules read, stopped (live 30 Oct: 3281 launch bots online, 1274 standing in their login pause for good). So a
// due bot builds only when it is at least `bar_ms` past its deadline: a frame that ran out with bots refused raises the bar, a
// frame with builds left lowers it in proportion. The most overdue bots go first wherever they stand in the order; a crowd
// larger than the allowance stretches everyone's cadence evenly instead of freezing the tail (simulated: 3281 idle bots at
// 500 ms, 40 ms frames - every bot builds at least every 2.6 s, at 90 % of the allowance). Pure: tested in
// tests/game/PlayerbotLazyBuildBudget.cpp.

#include <algorithm>
#include <cstdint>

namespace Playerbot {

struct LazyBuildBudget
{
    static constexpr std::uint32_t kPerFrame = 64;
    static constexpr std::int64_t kBarStepMs = 20;

    std::int64_t frame = -1;
    std::uint32_t left = 0;
    std::int64_t bar_ms = 0;
    bool refused = false;   // this frame turned a bot away for want of builds

    // The first ask of a frame settles the last one's bar and refills the allowance.
    void Begin(std::int64_t frame_id)
    {
        if (frame_id == frame)
            return;
        if (frame >= 0)
        {
            if (left == 0 && refused)
                bar_ms += kBarStepMs / 2;
            else if (left > 0)
                bar_ms = std::max<std::int64_t>(0, bar_ms - kBarStepMs * std::int64_t(left) / 16);
        }
        frame = frame_id;
        left = kPerFrame;
        refused = false;
    }
    // A bot `late_ms` past its deadline (0 = just due) may build now.
    bool CanTake(std::int64_t late_ms)
    {
        if (left == 0)
        {
            refused = true;
            return false;
        }
        return late_ms >= bar_ms;
    }
    void Take() { --left; }
};

} // namespace Playerbot
