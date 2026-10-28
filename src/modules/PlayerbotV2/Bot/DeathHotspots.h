// DeathHotspots - where bots keep dying. Every bot death by a creature is noted with the killer's level; a 100 y cell
// that saw DeathsNeeded deaths within WindowSec is a hotspot, and it is dangerous for a bot that is more than
// LevelMargin levels below the strongest killer seen there. Quest pursuit uses it to give up objectives inside a
// hotspot, so a bot stops walking into the camp that has just killed a dozen of its peers (Booty Bay Bloodsail camp,
// 2026-10-07: 20 Fireball deaths). Written on the world thread (Module::OnDeath), read from the bot workers.

#pragma once

#include "Define.h"
#include <cmath>
#include <deque>
#include <mutex>
#include <unordered_map>

namespace Playerbot {

class DeathHotspots
{
public:
    static constexpr float  kCell         = 100.0f;
    static constexpr uint32 kWindowSec    = 20u * 60u;
    static constexpr uint32 kDeathsNeeded = 3;
    static constexpr int    kLevelMargin  = 2;
    // A launch bot: its starter camps kill bots all the time with mobs a few levels up (pulls stolen, adds, respawns into the
    // crowd), so only a place with killers far above it counts (Raven Hill's level-19 undead for a level-2 bot that wandered
    // south: one bot died there 42 times in 40 minutes, live 28 Oct).
    static constexpr int    kLaunchLevelMargin = 5;

    static DeathHotspots& Instance()
    {
        static DeathHotspots inst;
        return inst;
    }

    void NoteDeath(uint32 map, float x, float y, uint8 killer_level, uint64 now_s, uint64 bot = 0)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        std::deque<Death>& q = cells_[Key(map, x, y)];
        Prune(q, now_s);
        q.push_back({ now_s, killer_level });
        if (bot)
        {
            std::deque<OwnDeath>& o = own_[bot];
            while (!o.empty() && now_s > o.front().at_s + kLoopWindowSec) o.pop_front();
            o.push_back({ now_s, map, x, y });
        }
        if (q.size() > 64) q.pop_front();
    }

    // True when (x,y) lies in a hotspot whose strongest recent killer is more than kLevelMargin levels above the bot.
    bool Dangerous(uint32 map, float x, float y, uint8 bot_level, uint64 now_s, int level_margin = kLevelMargin)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = cells_.find(Key(map, x, y));
        if (it == cells_.end()) return false;
        Prune(it->second, now_s);
        if (it->second.size() < kDeathsNeeded) return false;
        uint8 top = 0;
        for (Death const& d : it->second) top = d.killer_level > top ? d.killer_level : top;
        return int(top) - int(bot_level) > level_margin;
    }

    // The bot itself died kLoopDeaths times within kLoopWindowSec within kLoopRadius of (x, y), whatever the killers' level: a death
    // loop (a ghost that rises at a graveyard among mobs it cannot survive, live 28 Oct: Raven Hill 42 deaths, Zephras Isle's
    // Galestriders 15). The bot leaves the place and gives up the objective there (idle:leave_death_hotspot, pursue_quest_goal).
    static constexpr uint32 kLoopDeaths = 3;
    static constexpr uint32 kLoopWindowSec = 15u * 60u;
    static constexpr float  kLoopRadius = 100.0f;
    bool Looping(uint64 bot, uint32 map, float x, float y, uint64 now_s)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = own_.find(bot);
        if (it == own_.end()) return false;
        uint32 n = 0;
        for (OwnDeath const& d : it->second)
            if (d.map == map && now_s <= d.at_s + kLoopWindowSec && (d.x - x) * (d.x - x) + (d.y - y) * (d.y - y) <= kLoopRadius * kLoopRadius)
                ++n;
        return n >= kLoopDeaths;
    }

private:
    struct Death { uint64 at_s; uint8 killer_level; };
    struct OwnDeath { uint64 at_s; uint32 map; float x, y; };

    static uint64 Key(uint32 map, float x, float y)
    {
        int32 const cx = int32(std::floor(x / kCell));
        int32 const cy = int32(std::floor(y / kCell));
        return (uint64(map) << 40) ^ (uint64(uint32(cx) & 0xFFFFF) << 20) ^ uint64(uint32(cy) & 0xFFFFF);
    }
    static void Prune(std::deque<Death>& q, uint64 now_s)
    {
        while (!q.empty() && now_s > q.front().at_s + kWindowSec) q.pop_front();
    }

    std::mutex mtx_;
    std::unordered_map<uint64, std::deque<Death>> cells_;
    std::unordered_map<uint64, std::deque<OwnDeath>> own_;   // per bot: its own recent deaths
};

} // namespace Playerbot
