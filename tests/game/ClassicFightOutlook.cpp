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

#include <vector>

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

TEST_CASE("The time a bot has left at the pace of its fight", "[ClassicFightOutlook]")
{
    using FightOutlook::MsToDie;
    using FightOutlook::kNoEnd;
    // a level 2 paladin under two cutpurses: 60 -> 33 hp in 3 s, 9 hp a second, 33 hp left is 3.7 s
    CHECK(MsToDie(Sample{1000, 60, 200}, Sample{4000, 33, 180}) == 3666);
    CHECK(MsToDie(Sample{1000, 60, 200}, Sample{4000, 33, 180}) < FightOutlook::kRunWhenDeadInMs);
    // one whelp on a level 6 bot: 150 -> 141 hp in 3 s leaves 47 s
    CHECK(MsToDie(Sample{1000, 150, 90}, Sample{4000, 141, 60}) == 47000);
    CHECK(MsToDie(Sample{1000, 60, 200}, Sample{2000, 33, 180}) == kNoEnd);   // no pace yet
    CHECK(MsToDie(Sample{1000, 33, 200}, Sample{4000, 40, 180}) == kNoEnd);   // healed
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

namespace
{
struct Guid
{
    int id = 0;
    bool IsEmpty() const { return id == 0; }
};

// The fields of NearbyUnit FleeTrail::Crowd reads (through PullSize::JoinsPull)
struct Mob
{
    float x = 0, y = 0, z = 0;
    int hp = 100;
    bool is_player = false;
    bool untargetable = false;
    bool is_pacified = false;
    bool aggressive = true;
    Guid victim;
    bool unit_in_combat = false;
};

Mob MobAt(float x, float y)
{
    Mob m;
    m.x = x;
    m.y = y;
    return m;
}
}

TEST_CASE("A flee counts the idle aggressive mobs along its run", "[ClassicFightOutlook]")
{
    // a run from (0, 0) to (40, 0): a wolf pack at its end, one beside the line, one behind the bot, one far off
    std::vector<Mob> mobs{MobAt(42.f, 3.f), MobAt(38.f, -5.f), MobAt(20.f, 11.f), MobAt(-11.f, 0.f), MobAt(20.f, 30.f)};
    CHECK(FleeTrail::Crowd(mobs, 0.f, 0.f, 40.f, 0.f) == 4);
    // the other way round only the one that was behind it: the others are behind it now, the nearest 23 y off
    CHECK(FleeTrail::Crowd(mobs, 0.f, 0.f, -40.f, 0.f) == 1);
    // the attacker on the bot's back, a mob in another fight, a neutral one and a dead one wake nothing
    Mob chaser = MobAt(-5.f, 0.f);
    chaser.victim = Guid{1};
    Mob busy = MobAt(-20.f, 0.f);
    busy.unit_in_combat = true;
    Mob neutral = MobAt(-30.f, 0.f);
    neutral.aggressive = false;
    Mob dead = MobAt(-35.f, 0.f);
    dead.hp = 0;
    CHECK(FleeTrail::Crowd(std::vector<Mob>{chaser, busy, neutral, dead}, 0.f, 0.f, -40.f, 0.f) == 0);
}
