#pragma once
// KillCycle: one [kill_cycle] line per looted kill (Api/PlayerbotAPI.cpp, loot_drain), how long each part of a kill took:
// the distance at the engage, engage -> death and death -> loot. The bots' kill rate is about half a player's (racer bench,
// 28 Oct: 16-29 s from the engage to the loot against ~10-14 s), and this tells the approach, the fight and the loot walk
// apart. The engage is the first start_attack on a target; world thread only, the mutex is for safety.
#include "Define.h"
#include <mutex>
#include <unordered_map>

namespace Playerbot::V2::KillCycle
{
struct Entry
{
    uint64 target = 0;
    uint32 entry = 0;
    uint32 engage_ms = 0;
    uint32 death_ms = 0;
    float  dist = 0.f;
};

inline std::mutex& Mutex() { static std::mutex m; return m; }
inline std::unordered_map<uint64, Entry>& Map() { static std::unordered_map<uint64, Entry> m; return m; }

inline void Engage(uint64 bot, uint64 target, uint32 entry, float dist, uint32 now_ms)
{
    std::scoped_lock lock(Mutex());
    Entry& e = Map()[bot];
    if (e.target == target && e.death_ms == 0)
        return;   // the same fight, re-engaged
    e = Entry{ target, entry, now_ms ? now_ms : 1u, 0, dist };
}

inline void Death(uint64 bot, uint64 target, uint32 now_ms)
{
    std::scoped_lock lock(Mutex());
    auto it = Map().find(bot);
    if (it != Map().end() && it->second.target == target && it->second.death_ms == 0)
        it->second.death_ms = now_ms ? now_ms : 1u;
}

inline bool Looted(uint64 bot, uint64 target, Entry& out)
{
    std::scoped_lock lock(Mutex());
    auto it = Map().find(bot);
    if (it == Map().end() || it->second.target != target || it->second.death_ms == 0)
        return false;
    out = it->second;
    Map().erase(it);
    return true;
}
} // namespace Playerbot::V2::KillCycle
