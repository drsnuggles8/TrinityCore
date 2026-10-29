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

#include "DynamicRespawn.h"

using DynamicRespawn::CombatWaitUntil;
using DynamicRespawn::PlayersForMinimum;
using DynamicRespawn::ScaleDelay;

TEST_CASE("A spawn with nobody else near it keeps its own timer", "[DynamicRespawn]")
{
    // Olo alone in Jasperlode Mine (Kobold Tunneler, 270 s) and in the Coldridge troll cave (Frostmane Troll Whelp, 180 s)
    CHECK(ScaleDelay(270, 1, 3.0, 20) == 270);
    CHECK(ScaleDelay(180, 1, 3.0, 20) == 180);
    // nobody counted at all (the killer pulled the mob around a corner)
    CHECK(ScaleDelay(180, 0, 3.0, 20) == 180);
}

TEST_CASE("Up to the rate the timer is unchanged, above it the timer shrinks to rate / players", "[DynamicRespawn]")
{
    CHECK(ScaleDelay(180, 3, 3.0, 20) == 180);
    CHECK(ScaleDelay(180, 4, 3.0, 20) == 135);
    CHECK(ScaleDelay(180, 6, 3.0, 20) == 90);
    CHECK(ScaleDelay(270, 9, 3.0, 20) == 90);
    CHECK(ScaleDelay(180, 7, 3.0, 20) == 78); // 77.14 rounds up
}

TEST_CASE("A crowd never brings a timer below the minimum", "[DynamicRespawn]")
{
    CHECK(ScaleDelay(180, 30, 3.0, 20) == 20);
    CHECK(ScaleDelay(180, 300, 3.0, 20) == 20);
    CHECK(ScaleDelay(180, 300, 3.0, 60) == 60);
}

TEST_CASE("Timers at or below the minimum are left alone", "[DynamicRespawn]")
{
    // quest objects such as Felix's box have 10 s timers
    CHECK(ScaleDelay(10, 50, 3.0, 10) == 10);
    CHECK(ScaleDelay(5, 50, 3.0, 10) == 5);
    // Grik'nir the Cold (60 s) under a 60 s minimum
    CHECK(ScaleDelay(60, 50, 3.0, 60) == 60);
}

TEST_CASE("Counting stops exactly where the timer reaches its minimum", "[DynamicRespawn]")
{
    CHECK(PlayersForMinimum(180, 3.0, 20) == 27);
    CHECK(PlayersForMinimum(270, 3.0, 20) == 41);

    for (uint32 delay : { 15u, 60u, 180u, 270u, 300u, 1800u })
        for (double rate : { 0.0, 1.0, 2.5, 3.0, 10.0, 40.0 })
            for (uint32 minimum : { 10u, 20u, 60u })
            {
                if (delay <= minimum)
                    continue;
                CAPTURE(delay, rate, minimum);
                uint32 const enough = PlayersForMinimum(delay, rate, minimum);
                // at the cut-off the timer is at its minimum, so players past it cannot change the result
                CHECK(ScaleDelay(delay, enough, rate, minimum) == minimum);
                // one player fewer and the timer is still above it (or the count is still within the rate)
                if (enough > 1)
                    CHECK((ScaleDelay(delay, enough - 1, rate, minimum) > minimum || enough - 1 <= rate));
            }
}

TEST_CASE("A due respawn waits while a player fights at the spawn point, but not forever", "[DynamicRespawn]")
{
    time_t const t0 = 1000;
    // nobody fighting, or the wait switched off: respawn now
    CHECK(CombatWaitUntil(t0, 0, false, 5, 15) == 0);
    CHECK(CombatWaitUntil(t0, 0, true, 0, 15) == 0);

    // a fight: 5 s at a time, from the first wait
    CHECK(CombatWaitUntil(t0, 0, true, 5, 15) == t0 + 5);
    CHECK(CombatWaitUntil(t0 + 5, t0, true, 5, 15) == t0 + 10);
    CHECK(CombatWaitUntil(t0 + 10, t0, true, 5, 15) == t0 + 15);
    // a camp that is never quiet: the respawn comes 15 s late at most
    CHECK(CombatWaitUntil(t0 + 15, t0, true, 5, 15) == 0);
    CHECK(CombatWaitUntil(t0 + 40, t0, true, 5, 15) == 0);
    // the last step is cut to the cap
    CHECK(CombatWaitUntil(t0 + 12, t0, true, 5, 15) == t0 + 15);
    // the fight ends during the wait: respawn now
    CHECK(CombatWaitUntil(t0 + 5, t0, false, 5, 15) == 0);
}

TEST_CASE("A combat wait is always later than now while it lasts", "[DynamicRespawn]")
{
    // ProcessRespawns asserts that a postponed respawn lies in the future
    for (uint32 step : { 1u, 5u, 20u })
        for (uint32 maxWait : { 1u, 5u, 15u, 60u })
            for (time_t elapsed = 0; elapsed < 70; ++elapsed)
            {
                CAPTURE(step, maxWait, elapsed);
                time_t const until = CombatWaitUntil(1000 + elapsed, 1000, true, step, maxWait);
                if (until)
                    CHECK(until > 1000 + elapsed);
                CHECK(until <= time_t(1000 + maxWait));
            }
}
