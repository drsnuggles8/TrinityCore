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

#ifndef TRINITY_DYNAMIC_RESPAWN_H
#define TRINITY_DYNAMIC_RESPAWN_H

#include "Define.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>

namespace DynamicRespawn
{
    // A spawn's own respawn timer is meant for up to `rate` players around it. With more players near it the timer
    // shrinks to rate / players of itself, never below `minimum`; a timer already at or below `minimum` stays as it is
    // (quest objects with 10 s timers).
    inline uint32 ScaleDelay(uint32 respawnDelay, uint32 nearbyPlayers, double rate, uint32 minimum)
    {
        if (respawnDelay <= minimum || nearbyPlayers <= rate)
            return respawnDelay;

        return std::max<uint32>(uint32(std::ceil(respawnDelay * rate / nearbyPlayers)), minimum);
    }

    // The fewest nearby players for which ScaleDelay returns `minimum`; counting further cannot change the result.
    inline uint32 PlayersForMinimum(uint32 respawnDelay, double rate, uint32 minimum)
    {
        if (!minimum)
            return std::numeric_limits<uint32>::max();

        return std::max<uint32>(uint32(std::ceil(respawnDelay * rate / minimum)), 1);
    }

    // A due respawn while a player fights at the spawn point: it waits `step` seconds at a time, `maxWait` seconds in all
    // counted from `waitingSince` (0: not waited yet), so a camp that is never quiet still respawns. Returns the time to
    // check again, or 0 to respawn now.
    inline time_t CombatWaitUntil(time_t now, time_t waitingSince, bool fightAtSpawn, uint32 step, uint32 maxWait)
    {
        if (!fightAtSpawn || !step)
            return 0;

        time_t const since = waitingSince ? waitingSince : now;
        if (now - since >= time_t(maxWait))
            return 0;

        return std::min<time_t>(now + step, since + maxWait);
    }
}

#endif
