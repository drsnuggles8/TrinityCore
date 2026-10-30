/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "tc_catch2.h"

#include "../../src/modules/PlayerbotV2/Threading/LazyBuildBudget.h"

#include <algorithm>
#include <cstdint>
#include <vector>

using Playerbot::LazyBuildBudget;

namespace
{
struct Outcome
{
    std::int64_t longest_wait_ms = 0;   // the longest any bot went without a build, the end of the run included
    std::uint32_t never_again = 0;      // bots that built at login and never after
    double builds_per_s = 0.0;
};

// `bots` idle bots (one build per `period_ms`) asked in the same order every frame of `frame_ms`, for `run_ms`; all due at
// once at login (a launch run resuming with its whole roster). `fair` false: a plain first-come cap of kPerFrame builds, the
// allowance as it was before 30 Oct.
Outcome Simulate(std::uint32_t bots, std::int64_t period_ms, std::int64_t frame_ms, std::int64_t run_ms, bool fair)
{
    std::vector<std::int64_t> next(bots, 0), last(bots, -1);
    std::vector<std::uint32_t> builds(bots, 0);
    LazyBuildBudget budget;
    Outcome out;
    std::uint64_t total = 0;
    for (std::int64_t now = 0; now < run_ms; now += frame_ms)
    {
        budget.Begin(now);
        std::uint32_t plain_left = LazyBuildBudget::kPerFrame;
        for (std::uint32_t i = 0; i < bots; ++i)
        {
            if (next[i] > now)
                continue;
            if (fair)
            {
                if (!budget.CanTake(now - next[i]))
                    continue;
                budget.Take();
            }
            else
            {
                if (plain_left == 0)
                    continue;
                --plain_left;
            }
            if (last[i] >= 0)
                out.longest_wait_ms = std::max(out.longest_wait_ms, now - last[i]);
            last[i] = now;
            ++builds[i];
            ++total;
            next[i] = now + period_ms;
        }
    }
    for (std::uint32_t i = 0; i < bots; ++i)
    {
        if (builds[i] <= 1)
            ++out.never_again;
        out.longest_wait_ms = std::max(out.longest_wait_ms, run_ms - last[i]);
    }
    out.builds_per_s = double(total) * 1000.0 / double(run_ms);
    return out;
}
}

TEST_CASE("More bots due than a frame allows: the most overdue build first, nobody freezes", "[PlayerbotLazyBuildBudget]")
{
    // live 30 Oct: 3281 bots, idle period 500 ms, ~40 ms frames (allowance 64 x 25 = 1600 builds a second)
    Outcome const now = Simulate(3281, 500, 40, 120000, true);
    CHECK(now.never_again == 0);
    CHECK(now.longest_wait_ms <= 3000);
    CHECK(now.builds_per_s >= 1300.0);   // the bar wastes little of the allowance

    // negative control: the first-come cap freezes the tail of the order
    Outcome const before = Simulate(3281, 500, 40, 120000, false);
    CHECK(before.never_again > 1000);
    CHECK(before.longest_wait_ms >= 110000);
}

TEST_CASE("Fewer bots than a frame allows: every bot keeps its period", "[PlayerbotLazyBuildBudget]")
{
    Outcome const out = Simulate(300, 500, 40, 30000, true);
    CHECK(out.never_again == 0);
    CHECK(out.longest_wait_ms <= 520);   // its period, rounded up to a frame
}

TEST_CASE("The bar rises while a frame runs out and falls while builds are left", "[PlayerbotLazyBuildBudget]")
{
    LazyBuildBudget b;
    b.Begin(1);
    for (std::uint32_t i = 0; i < LazyBuildBudget::kPerFrame; ++i)
    {
        REQUIRE(b.CanTake(0));
        b.Take();
    }
    CHECK_FALSE(b.CanTake(1000));   // out of builds: refused, whatever its lateness
    b.Begin(1);                     // the same frame: no refill
    CHECK(b.left == 0);
    b.Begin(2);
    CHECK(b.bar_ms == LazyBuildBudget::kBarStepMs / 2);
    CHECK_FALSE(b.CanTake(0));      // just due: below the bar
    CHECK(b.CanTake(LazyBuildBudget::kBarStepMs));
    b.Take();
    b.Begin(3);                     // 63 builds left over: the bar drops to 0
    CHECK(b.bar_ms == 0);
}
