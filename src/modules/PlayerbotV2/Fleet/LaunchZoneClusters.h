#pragma once
// Which launch bots are logged in while the run goes on (Playerbot.Launch.PlayerZonesOnly, AutoRun): only those in the same
// start-area cluster as a real player - Dun Morogh, Ironforge and Loch Modan; Elwynn, Stormwind, Westfall and Redridge;
// Teldrassil, Darnassus and Darkshore. A zone in no cluster (Zephras Isle, Duskwood, ...) is a cluster of its own. When every
// player has left a cluster its bots log out where they stand and come back with the next player (Olo, 30 Oct: the whole
// roster of 3281 logged in at once for one player in Dun Morogh). Pure: tested in tests/game/PlayerbotLaunchZoneClusters.cpp.

#include <cstdint>

namespace Playerbot::LaunchZoneClusters {

// The cluster of an area-table zone id: the cluster's first zone, or the zone itself when it is in none.
constexpr std::uint32_t Of(std::uint32_t zone)
{
    switch (zone)
    {
        case 1: case 1537: case 38:            return 1;     // Dun Morogh, Ironforge, Loch Modan
        case 12: case 1519: case 40: case 44:  return 12;    // Elwynn Forest, Stormwind, Westfall, Redridge Mountains
        case 141: case 1657: case 148:         return 141;   // Teldrassil, Darnassus, Darkshore
        default:                               return zone;
    }
}

} // namespace Playerbot::LaunchZoneClusters
