// TaxiFare - a bot that pays its own way (from LatvjuAvs's f960d4f966 / db464f2fcd, adapted to the launch sim).
// Upstream topped a bot up to 5 gold before every flight and covered any repair it could not afford, so a level 3 bot that took
// one flight carried 4 g 99 s. A launch bot (and every bot under Playerbot.World.FreshStart) pays instead: the planners pick
// only flights it can pay for, otherwise it walks; a poor bot repairs what it can afford, and one too poor to repair even a
// quarter of its gear keeps questing on broken gear, as a player does.
// The caller says whether the bot is a launch bot: the snapshot builder and the rules read their own BotAI, the API (world
// thread) asks LaunchSim.

#pragma once

#include "Config.h"
#include "DB2Stores.h"
#include "ObjectMgr.h"
#include "Player.h"
#include <vector>

namespace Playerbot::V2::TaxiFare {

inline bool FreshStart()
{
    static bool const fresh = sConfigMgr->GetBoolDefault("Playerbot.World.FreshStart", false, true);
    return fresh;
}

inline bool PayOwnWay(bool launch_bot) { return launch_bot || FreshStart(); }

// The fare of a flight along `nodes` (TaxiNodes ids, start first), as Player::ActivateTaxiPathTo charges it before the
// reputation discount (so never less than the real fare). 0 when a hop has no path.
inline uint64 RouteFare(Player const* p, std::vector<uint32> const& nodes)
{
    uint64 total = 0;
    for (size_t i = 1; i < nodes.size(); ++i)
    {
        uint32 path = 0, cost = 0;
        sObjectMgr->GetTaxiPath(nodes[i - 1], nodes[i], path, cost);
        if (!path)
            return 0;
        total += cost;
    }
    return p ? p->ApplyClassicLegacyPriceMod(total, 4) : total;
}

// Too poor to repair even a quarter of its gear: the critical repair walk and the flee to repair stand down for it (LatvjuAvs:
// a level 2 priest with 10 copper and all gear at 0% stood at idle:critical_repair_approach and gave up on every mob).
inline bool TooPoorToRepair(bool launch_bot, int64 money, uint32 repair_cost)
{
    return PayOwnWay(launch_bot) && repair_cost > 0 && money * 4 < int64(repair_cost);
}

} // namespace Playerbot::V2::TaxiFare
