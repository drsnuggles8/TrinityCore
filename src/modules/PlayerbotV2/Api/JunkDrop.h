// JunkDrop - which item a bot with full bags destroys to make room for loot or a quest's starting item it cannot otherwise take.
// A group in a dungeon never leaves to vendor, so the bags only ever fill; a real player drops the worst grey for something
// better. A launch bot does the same out questing instead of walking to a vendor (a racer keeps to its route), and when it has
// no grey left it drops its cheapest white junk (a replaced weapon or armour piece, a trade good, a misc item) for a quest item
// or for something worth more. Never: quest items, the hearthstone, consumables, ammo, bags, reagents, keys (`keep`).
// Pure decision logic.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Playerbot
{

struct JunkCandidate
{
    std::uint64_t value = 0;   // vendor value of the whole stack
    bool poor = false;         // grey quality: always destroyable (unless `keep`)
    bool quest = false;        // quest items are never destroyed
    bool white_junk = false;   // common quality weapon / armour / trade good / misc: destroyable when the caller allows it
    bool keep = false;         // the hearthstone, consumables, ammo, bags, reagents, keys
};

inline constexpr std::size_t kNoJunk = static_cast<std::size_t>(-1);

// Returns the index of the item to destroy, or kNoJunk.
// 1. The cheapest grey worth less than the incoming item (a non-grey incoming item beats any grey; a grey one only a cheaper grey).
// 2. With `allow_white` and no such grey: the cheapest white junk, for a quest item or when it is worth less than the incoming one.
inline std::size_t PickJunkToDrop(std::vector<JunkCandidate> const& bags, bool incoming_poor, std::uint64_t incoming_value,
    bool incoming_quest = false, bool allow_white = false)
{
    std::size_t best = kNoJunk;
    for (std::size_t i = 0; i < bags.size(); ++i)
    {
        JunkCandidate const& c = bags[i];
        if (!c.poor || c.quest || c.keep) continue;
        if (incoming_poor && c.value >= incoming_value) continue;
        if (best == kNoJunk || c.value < bags[best].value) best = i;
    }
    if (best != kNoJunk || !allow_white || incoming_poor)
        return best;
    for (std::size_t i = 0; i < bags.size(); ++i)
    {
        JunkCandidate const& c = bags[i];
        if (!c.white_junk || c.quest || c.keep) continue;
        if (!incoming_quest && c.value >= incoming_value) continue;
        if (best == kNoJunk || c.value < bags[best].value) best = i;
    }
    return best;
}

} // namespace Playerbot
