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

#include "../../src/modules/PlayerbotV2/Bot/FightOutlook.h"
#include "../../src/modules/PlayerbotV2/Bot/FleeTrail.h"

using namespace Playerbot;
using FightOutlook::Sample;
using FightOutlook::Winning;

TEST_CASE("A bot that kills its mob well before it dies fights on", "[ClassicFightOutlook]")
{
    // 22 % of 80 hp left against a whelp at 59 %: in 3 s the bot lost 9 hp and dealt 30
    CHECK(Winning(Sample{1000, 27, 64}, Sample{4000, 18, 34}));
    // the same bot against a second whelp that just joined: the mobs' health went up
    CHECK_FALSE(Winning(Sample{1000, 27, 64}, Sample{4000, 18, 98}));
    // even trade: it would die first
    CHECK_FALSE(Winning(Sample{1000, 30, 60}, Sample{4000, 20, 50}));
    // it wins (dead in 6 s, the mob in 4.5 s), but not by the margin
    CHECK_FALSE(Winning(Sample{1000, 30, 25}, Sample{4000, 20, 15}));
}

TEST_CASE("No pace, no fight on; a bot not losing health always fights on", "[ClassicFightOutlook]")
{
    CHECK_FALSE(Winning(Sample{1000, 27, 64}, Sample{2000, 18, 34}));   // 1 s is too little to read a pace
    CHECK(Winning(Sample{1000, 18, 64}, Sample{4000, 18, 60}));        // healing or not hit
    CHECK_FALSE(Winning(Sample{1000, 27, 64}, Sample{4000, 18, 64}));  // dealing nothing
}

TEST_CASE("The pace is read over the last 4 s of the same fight", "[ClassicFightOutlook]")
{
    FightOutlook::History h;
    CHECK(h.Reference(1000) == nullptr);
    for (uint32_t t = 1000; t <= 8000; t += 250)
        h.Add(Sample{t, int32_t(100 - t / 100), 50});
    // samples every 500 ms, so 1000, 1500 ... 8000; the oldest within 4 s of 8000 is 4000
    REQUIRE(h.Reference(8000) != nullptr);
    CHECK(h.Reference(8000)->at_ms == 4000);
    CHECK(h.Newest().at_ms == 8000);
    // a new fight after a pause: the old samples are gone
    h.Add(Sample{20000, 80, 40});
    CHECK(h.count == 1);
    CHECK(h.Reference(20000)->at_ms == 20000);
}

TEST_CASE("A flee runs back along the way the bot came", "[ClassicFightOutlook]")
{
    FleeTrail trail;
    // walking east into a cave, 5 y a step: a point every 10 y
    for (int i = 0; i <= 12; ++i)
        trail.Note(0, float(i * 5), 0.f, 0.f);
    REQUIRE(trail.count == 7);   // x = 0, 10, ... 60
    CHECK(trail.At(0).x == 60.f);
    // fighting at x = 62: the first point 15 y back is x = 40, then 30, ...
    int const first = trail.First(62.f, 0.f);
    REQUIRE(first == 2);
    CHECK(trail.At(uint8_t(first)).x == 40.f);
    CHECK(trail.At(uint8_t(first + 1)).x == 30.f);
    // nothing far enough: no trail flee
    FleeTrail near;
    near.Note(0, 0.f, 0.f, 0.f);
    CHECK(near.First(5.f, 0.f) == -1);
}

TEST_CASE("A teleport or a new map starts a new trail", "[ClassicFightOutlook]")
{
    FleeTrail trail;
    trail.Note(0, 0.f, 0.f, 0.f);
    trail.Note(0, 10.f, 0.f, 0.f);
    trail.Note(0, 500.f, 0.f, 0.f);   // a jump: the hearthstone, a flight, a ghost run
    CHECK(trail.count == 1);
    trail.Note(1, 500.f, 0.f, 0.f);   // same spot on another map
    CHECK(trail.count == 1);
    CHECK(trail.map == 1);
    // the ring keeps the newest 12 points
    for (int i = 1; i <= 20; ++i)
        trail.Note(1, 500.f + i * 10.f, 0.f, 0.f);
    CHECK(trail.count == 12);
    CHECK(trail.At(0).x == 700.f);
    CHECK(trail.At(11).x == 590.f);
}
