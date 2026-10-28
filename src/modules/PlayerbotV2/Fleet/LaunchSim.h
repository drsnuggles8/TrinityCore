// LaunchSim - launch-day simulation: a crowd of fresh level-1 characters that start together with a real player.
//
//   .playerbot launch prepare [bots] [late]   create the roster offline (dwarves and gnomes, starting in Coldridge Valley)
//   .playerbot launch arm                     the run starts (T0) when the next real player enters the world
//   .playerbot launch start                   start now
//   .playerbot launch stop                    log every launch bot out, keep the roster (and the run's clock: start resumes)
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
#include "ObjectGuid.h"
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
    std::string Status();   // loads the roster first (after a restart it is read lazily)
    // One line on the online launch bots: average level, quests turned in, dead, the rules they fired last.
    std::string Report() const;

    void OnWorldTick(uint32 now_ms);
    // A real (non-bot) player entered the world: T0 of an armed run, or with Playerbot.Launch.AutoRun of a ready one when
    // nobody else is online.
    void OnRealPlayerLogin(Player* p);
    // A real player left the world. With Playerbot.Launch.AutoRun, when nobody is left the run pauses (the bots log out, the
    // clock stops) and the next login resumes it.
    void OnRealPlayerLogout(Player* p);
    // A bot logged in: when it is a launch bot, give its BotAI the persona's LaunchProfile. True for a launch bot.
    bool OnBotLogin(Player* p, BotAI* ai);
    bool IsLaunchBot(uint64 guid_low) const;
    // `asker` (a launch bot) whispers `target` for an invite (idle:launch_ask_invite). A launch bot that is asked answers a
    // few seconds later by its temperament: friendly ones invite, greedy ones and loners do not, the others sometimes, with a
    // short whisper now and then. A real player only gets the whisper. False when the target is gone.
    bool OnAskInvite(Player* asker, ObjectGuid target);

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

    // An answer to an "inv?" whisper, sent by `from` (a launch bot) once due: an invite, or a whisper back.
    struct Reply
    {
        ObjectGuid  from, to;
        uint32      due_ms = 0;
        bool        invite = false;
        std::string text;
    };

    LaunchSim() = default;
    void SendReplies(uint32 now_ms);
    void LoadRoster();
    // zone: the starting zone (Bot/LaunchPlaces.h StartZoneId) the character is made for.
    void Roll(Entry& e, bool keep_identity, bool late, uint8 zone);
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
    std::vector<Reply> replies_;
    bool               auto_init_done_ = false;   // AutoRun: the roster was reset once after the server started
    bool               auto_start_pending_ = false;   // AutoRun: a real player logged in while the roster was being made
    uint32             asks_ = 0, grants_ = 0;   // this run, for the report
    // The run's clock when it was stopped or paused (playerbot_launch_run, kept across restarts): Start resumes at it, 0 = a
    // fresh run. The bots that ended their session are kept in playerbot_launch_done.
    uint32             resume_elapsed_s_ = 0;
    void SaveClock(uint32 elapsed_s);
};

} // namespace Playerbot::V2::Fleet
