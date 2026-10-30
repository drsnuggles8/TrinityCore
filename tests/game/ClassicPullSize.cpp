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

#include "../../src/modules/PlayerbotV2/Bot/PullSize.h"

#include <vector>

using namespace Playerbot::PullSize;

namespace
{
struct Guid
{
    int id = 0;
    bool IsEmpty() const { return id == 0; }
    bool operator==(Guid const& o) const { return id == o.id; }
};

// The fields of NearbyUnit the pull estimate reads
struct Mob
{
    Guid guid;
    float x = 0, y = 0, z = 0;
    int hp = 100;
    bool is_player = false;
    bool untargetable = false;
    bool is_pacified = false;
    bool aggressive = true;
    Guid victim;
    bool unit_in_combat = false;
};

Mob At(int id, float x, float y, float z = 0.0f)
{
    Mob m;
    m.guid.id = id;
    m.x = x; m.y = y; m.z = z;
    return m;
}
}

TEST_CASE("A lone mob is a pull of one", "[ClassicPullSize]")
{
    std::vector<Mob> mobs = { At(1, 0, 0), At(2, 30, 0), At(3, 0, 25) };
    CHECK(Estimate(mobs[0], mobs) == 1);
}

TEST_CASE("Idle aggressive mobs within 10 yards of the target come with it", "[ClassicPullSize]")
{
    // the Coldridge troll cave: a whelp with two more beside it, a third farther in
    std::vector<Mob> mobs = { At(1, 0, 0), At(2, 6, 0), At(3, 0, -8), At(4, 18, 0) };
    CHECK(Estimate(mobs[0], mobs) == 3);
    // 3D: a mob on the floor above does not count
    std::vector<Mob> stacked = { At(1, 0, 0, 0), At(2, 3, 0, 12) };
    CHECK(Estimate(stacked[0], stacked) == 1);
}

TEST_CASE("Neutral, fighting, dead and player units stay out of the pull", "[ClassicPullSize]")
{
    std::vector<Mob> mobs = { At(1, 0, 0), At(2, 2, 0), At(3, 0, 2), At(4, -2, 0), At(5, 0, -2), At(6, 1, 1) };
    mobs[1].aggressive = false;          // a neutral boar
    mobs[2].victim.id = 99;              // already fighting someone
    mobs[3].hp = 0;                      // a corpse
    mobs[4].is_player = true;            // another player
    mobs[5].unit_in_combat = true;       // in combat, no victim yet
    CHECK(Estimate(mobs[0], mobs) == 1);
}

TEST_CASE("One mob never waits; two or more wait for 90 % health and 80 % mana", "[ClassicPullSize]")
{
    // a pull of one: as before, whatever the bot has
    CHECK(Ready(1, 40, 10, true));
    // a warrior (no mana) before three whelps
    CHECK_FALSE(Ready(3, 55, 0, false));
    CHECK(Ready(3, 90, 0, false));
    // a priest: health and mana both
    CHECK_FALSE(Ready(2, 100, 60, true));
    CHECK_FALSE(Ready(2, 85, 100, true));
    CHECK(Ready(2, 90, 80, true));
}

TEST_CASE("Three mobs, an elite or a mob two levels up is too hard alone", "[ClassicPullSize]")
{
    CHECK_FALSE(Hard(2, false, 3, 3));
    CHECK(Hard(3, false, 3, 3));
    CHECK(Hard(1, true, 3, 3));
    // a level-5 mob: hard for a level-3 bot, not for a level-4 one
    CHECK(Hard(1, false, 5, 3));
    CHECK_FALSE(Hard(1, false, 5, 4));
}

TEST_CASE("A hard pull, or a bot already resting, rests as for two mobs", "[ClassicPullSize]")
{
    CHECK(RestSize(1, false, false) == 1);
    CHECK(RestSize(1, true, false) == 2);
    // the pack thinned out to one while the bot sat: it keeps resting
    CHECK(RestSize(1, false, true) == 2);
    CHECK_FALSE(Ready(RestSize(1, false, true), 57, 42, true));
    CHECK(RestSize(3, false, true) == 3);
}
