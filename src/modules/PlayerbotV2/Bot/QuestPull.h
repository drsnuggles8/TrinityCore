#pragma once
// The quest target a solo quester would pull next, with the size of that pull (Bot/PullSize.h). Read by idle:prepare_pull
// (OocConsumableRules: rest up before a pull of two or more) and idle:launch_party_build (LaunchRules: ask for a group before a
// hard one).

#include "Define.h"

namespace Playerbot {

class BotSnapshotView;
struct NearbyUnit;

struct NextPull
{
    NearbyUnit const* target = nullptr;
    uint32 size = 0;
};

// The nearest idle creature in the scan that advances a kill or drop objective in the bot's log, and how many mobs pulling it brings.
NextPull NextQuestPull(BotSnapshotView const& s);

// A pull a lone quester of `bot_level` should not take on alone (PullSize::Hard): three or more mobs, an elite / rare target, or
// one two levels or more above the bot.
bool HardPull(NextPull const& next, uint8 bot_level);

} // namespace Playerbot
