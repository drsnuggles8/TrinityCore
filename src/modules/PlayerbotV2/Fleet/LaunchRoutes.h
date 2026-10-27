// LaunchRoutes - leveling routes for launch-simulation bots that follow a guide (LaunchStrategy::GuideRoute).
//
// A route is an ordered list of quest steps (accept / complete an objective / turn in) imported from a guide file with
// contrib/launch-sim/import_route.py into playerbot_launch_route_step. Nothing about a bot's progress is stored: the step a
// bot is on is the first step for its race and class it has not done yet, read from its quest log every snapshot build.
// So deaths, detours and resets need no bookkeeping, and a step that cannot be done (a class quest of another class, a
// quest the bot was never offered) is passed over.
//
// Loaded on the world thread (LaunchSim prepare / first use), read-only afterwards; the snapshot builders read it.
#pragma once

#include "Define.h"
#include <string>
#include <unordered_map>
#include <vector>

class Player;

namespace Playerbot::V2::Fleet {

enum class RouteAction : uint8 { None = 0, Accept = 1, Complete = 2, TurnIn = 3, Deathskip = 4, Hearth = 5 };

struct RouteStep
{
    RouteAction action = RouteAction::None;
    uint32      quest = 0;
    uint8       objective = 0;   // 1-based, complete steps only
    uint32      combos = 0;      // ComboBit(race, class) of the classes that do this step
};

struct RouteGiver { uint32 map = 0; float x = 0.f, y = 0.f, z = 0.f; };

// A class trainer of the launch area (Dun Morogh, Ironforge, Loch Modan) on map 0.
struct ClassTrainer
{
    uint64 spawn_id = 0;
    uint32 class_mask = 0;   // Trainer::GetClassMask
    float  x = 0.f, y = 0.f, z = 0.f;
};

class LaunchRoutes
{
public:
    static constexpr uint8 kNoRoute = 0xFF;

    static LaunchRoutes& Instance();

    // (Re)reads every route from the database. World thread, while no snapshot build runs.
    void Load();
    bool Empty() const { return routes_.empty(); }
    std::vector<std::string> const& Names() const { return names_; }
    // Index of a route by name, kNoRoute when there is none of that name.
    uint8 Find(std::string const& name) const;

    // The first step of `route` this player still has to do (nullptr: none left, or no such route).
    RouteStep const* CurrentStep(Player const* p, uint8 route) const;
    // The quests of the next `steps` steps this player still has to do, in route order (duplicates removed): what a player
    // with the guide open has picked up or is about to, and works on in parallel (wolves for one quest while the boars of
    // another come along), not one step at a time.
    void Window(Player const* p, uint8 route, uint32 steps, std::vector<uint32>& quests) const;
    // Where a quest's giver stands (first spawn of a creature that starts it), nullptr when unknown.
    RouteGiver const* Giver(uint32 quest) const;

    // The class trainer for `cls` nearest to (x, y) on map 0, nullptr when there is none in the launch area.
    ClassTrainer const* NearestTrainer(uint8 cls, float x, float y) const;

    static uint32 ComboBit(uint8 race, uint8 cls);

private:
    // The step is for this player's race and class and still to do (CurrentStep's test).
    bool Pending(Player const* p, RouteStep const& s, uint32 me) const;

    std::vector<std::string>             names_;
    std::vector<std::vector<RouteStep>>  routes_;
    std::unordered_map<uint32, RouteGiver> givers_;
    std::vector<ClassTrainer>            trainers_;
};

} // namespace Playerbot::V2::Fleet
