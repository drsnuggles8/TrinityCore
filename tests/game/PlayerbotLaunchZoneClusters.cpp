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

#include "../../src/modules/PlayerbotV2/Fleet/LaunchZoneClusters.h"

using Playerbot::LaunchZoneClusters::Of;

TEST_CASE("A player in Dun Morogh wakes Ironforge and Loch Modan too", "[PlayerbotLaunchZoneClusters]")
{
    CHECK(Of(1) == Of(1537));
    CHECK(Of(1) == Of(38));
    // but not Elwynn or Teldrassil
    CHECK(Of(1) != Of(12));
    CHECK(Of(1) != Of(141));
}

TEST_CASE("The human and night elf start areas are clusters of their own", "[PlayerbotLaunchZoneClusters]")
{
    CHECK(Of(12) == Of(1519));
    CHECK(Of(12) == Of(40));
    CHECK(Of(12) == Of(44));
    CHECK(Of(141) == Of(1657));
    CHECK(Of(141) == Of(148));
    CHECK(Of(12) != Of(141));
}

TEST_CASE("A zone in no cluster stands alone", "[PlayerbotLaunchZoneClusters]")
{
    CHECK(Of(16593) == 16593u);   // Zephras Isle
    CHECK(Of(10) == 10u);         // Duskwood
    CHECK(Of(10) != Of(12));
}
