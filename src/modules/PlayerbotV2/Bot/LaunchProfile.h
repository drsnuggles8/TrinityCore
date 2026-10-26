// LaunchProfile - what a launch-simulation bot is (Fleet/LaunchSim): its persona (strategy x temperament x pace) and the
// pace-driven pauses that make a crowd of fresh level 1s look like players on launch day rather than a speedrun fleet.
//
// Set once by LaunchSim at login, on the world thread, before the bot's first think tick (Module::OnPlayerLogin), and only
// read afterwards: the bot logic reads it from its worker without a lock. A bot that is not part of a launch run has
// active == false and every consumer behaves as before.
#pragma once

#include "Define.h"
#include <cstdint>

namespace Playerbot {

// How a bot levels (WHAT it does). Phase 1 plays Quester (the bots' own nearest-objective questing) for every strategy;
// the others get their own rules in later phases.
enum class LaunchStrategy : uint8
{
    Quester       = 0,   // quests without a guide: whatever is nearest (Questie-style)
    GuideRoute    = 1,   // follows a guide's step order (RestedXP and others)
    TrollGrinder  = 2,   // runs to the troll camp, grinds to 4, leaves Coldridge at 4
    Skipper       = 3,   // a quest or two, then out of the starter area early (death-skip)
    CrowdGrinder  = 4,   // drops contested quests, grinds what is free
    Gatherer      = 5,   // mining / herbalism / cooking detours
    Casual        = 6,   // reads everything, explores, slow
    PartyBuilder  = 7,   // "LF Grik'nir, inv": forms groups at named and contested spawns
    Idle          = 8,   // stands around, AFK, logs out early
    Reroller      = 9,   // plays a while, deletes, comes back at level 1
    KillStealer   = 10,  // tags what others are fighting, skips lines
    Leaver        = 11,  // quests like a Quester, moves on to Loch Modan at level 8
    Count
};

// How a bot treats other players (invites, requests, sharing).
enum class LaunchTemperament : uint8
{
    Neutral  = 0,   // sometimes yes, sometimes no
    Friendly = 1,   // accepts and grants invites, shares quests
    Greedy   = 2,   // takes invites and tags, gives nothing back
    Loner    = 3,   // ignores everyone
    Griefer  = 4,   // steals tags and loot on purpose
    Count
};

struct LaunchProfile
{
    bool   active = false;
    uint8  persona = 0;                                   // index into the persona table (LaunchSim)
    LaunchStrategy    strategy    = LaunchStrategy::Quester;
    LaunchTemperament temperament = LaunchTemperament::Neutral;

    // 1.0 = speedrunner, 3.5 = very casual. Scales the pauses below.
    float  pace = 1.0f;
    // Contest tags: go for any mob not yet tagged by another party (instead of skipping every mob that is already fighting).
    bool   contest_tags = true;
    // The guide a GuideRoute bot follows (index into Fleet::LaunchRoutes; 0xFF = none, the bot quests on its own).
    uint8  route = 0xFF;

    // Pauses, in the snapshot clock (BotSnapshotView::published_at_ms, the world's game-time ms).
    uint32 login_ms = 0;            // when the bot entered the world
    uint32 setup_ms = 0;            // UI / keybind setup at the start: stands still this long after login
    uint32 read_min_ms = 0;         // reading a quest's text before accepting / turning it in
    uint32 read_max_ms = 0;
    uint32 afk_every_ms = 0;        // mean time between AFK breaks (0 = never)
    uint32 afk_min_ms = 0;
    uint32 afk_max_ms = 0;
    uint32 session_ms = 0;          // logs out this long after login (0 = plays on); LaunchSim enforces it
    uint32 seed = 0;                // per-bot randomness for the pause schedule
};

// What a launch bot remembers about groups (Bot/States/Rules/LaunchRules.cpp). Worker thread only, unlike the profile.
struct LaunchSocial
{
    uint32 last_ask_ms = 0;         // the last "inv?" whisper (one every 40 s at most)
    uint64 asked_low = 0;           // the player it asked: an invite from them is taken without thinking twice
    uint32 group_since_ms = 0;      // first tick seen in the current group (0 = not in one)
    uint32 busy_until_ms = 0;       // browsing a trainer's list: idle:launch_hold keeps it still until then
    uint32 train_move_ms = 0;       // the last move order towards the trainer
};

} // namespace Playerbot
