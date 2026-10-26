// LaunchSim - launch-day simulation: a crowd of fresh level-1 characters that start together with a real player.
//
//   .playerbot launch prepare [bots] [late]   create the roster offline (dwarves and gnomes, starting in Coldridge Valley)
//   .playerbot launch arm                     the run starts (T0) when the next real player enters the world
//   .playerbot launch start                   start now
//   .playerbot launch stop                    log every launch bot out, keep the roster
//   .playerbot launch reset                   log out, delete and recreate the roster fresh at level 1 (new personas)
//   .playerbot launch status
//
// Each bot gets a persona (strategy x temperament x pace, drawn together: a racer is more likely greedy than a casual),
// an arrival time after T0 (most in the first hour, a trickle of late arrivals for hours) and a session length. The
// persona becomes the bot's LaunchProfile at login (Bot/LaunchProfile.h); the bots themselves are normal V2 bots with
// distribution_level 0, so the setup pipeline, the world spawner and the session rhythm leave them alone.
//
// World thread only: every entry point is a GM command, a world tick or a login hook.
#pragma once

#include "Define.h"
#include <string>
#include <unordered_map>
#include <vector>

class Player;

namespace Playerbot { class BotAI; }

namespace Playerbot::V2::Fleet {

class LaunchSim
{
public:
    static LaunchSim& Instance();

    std::string Prepare(uint32 bots, uint32 late);
    std::string Arm();
    std::string Start();
    std::string Stop();
    // bots > 0: a new roster of that size instead of the same characters again.
    std::string Reset(uint32 bots = 0, uint32 late = 0);
    std::string Status() const;
    // One line on the online launch bots: average level, quests turned in, dead, the rules they fired last.
    std::string Report() const;

    void OnWorldTick(uint32 now_ms);
    // A real (non-bot) player entered the world: T0 of an armed run.
    void OnRealPlayerLogin(Player* p);
    // A bot logged in: when it is a launch bot, give its BotAI the persona's LaunchProfile. True for a launch bot.
    bool OnBotLogin(Player* p, BotAI* ai);
    bool IsLaunchBot(uint64 guid_low) const;

private:
    enum class Phase : uint8 { Empty, Creating, Ready, Armed, Running, Stopping, Deleting };
    enum class BotState : uint8 { ToCreate, Offline, LoggingIn, Online, Done };

    struct Entry
    {
        uint64      guid_low = 0;
        std::string name;
        uint8       race = 0, cls = 0, gender = 0;
        uint8       persona = 0;
        uint32      arrival_s = 0;
        uint32      session_s = 0;
        uint32      seed = 0;
        BotState    state = BotState::ToCreate;
        uint32      online_since_ms = 0;
        uint32      login_sent_ms = 0;
    };

    LaunchSim() = default;
    void LoadRoster();
    void Roll(Entry& e, bool keep_identity, bool late);
    bool CreateOne(Entry& e);
    void DeleteOne(Entry const& e);
    void SaveRow(Entry const& e);
    std::vector<uint32> PersonaWeights() const;

    Phase              phase_ = Phase::Empty;
    bool               loaded_ = false;
    std::vector<Entry> roster_;
    std::unordered_map<uint64, size_t> by_guid_;
    uint32             t0_ms_ = 0;
    uint32             last_report_ms_ = 0;
    uint32             late_count_ = 0;   // the last late_count_ roster entries arrive after the first hour
    bool               recreate_after_delete_ = false;
    uint32             resize_bots_ = 0, resize_late_ = 0;
};

} // namespace Playerbot::V2::Fleet
