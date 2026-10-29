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
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class Player;
class Quest;

namespace Playerbot::V2::Fleet {

// TurnInIfDone: a guide's optional hand-in (".isQuestComplete Q" + ".turnin Q" in an #optional step): only while the quest is
// complete, as Olo's Coldridge guide takes A New Threat (170) only if the troggs filled on the way.
// Level: a guide's grind step (".xp 4", ".xp 3+1130", ".xp 10-1720"): the bot takes no quest of the route until it has that level
// and XP, and grinds where it stands (idle:engage_nearby_mob); the steps after it wait (Olo, 29 Oct: Guided bots left Coldridge
// at level 3, the RestedXP guide there ends at about 5 1/3).
enum class RouteAction : uint8 { None = 0, Accept = 1, Complete = 2, TurnIn = 3, Deathskip = 4, Hearth = 5, TurnInIfDone = 6, Level = 7 };

struct RouteStep
{
    RouteAction action = RouteAction::None;
    uint32      quest = 0;       // Level steps: the level << 24 | the XP into that level
    uint8       objective = 0;   // 1-based, complete steps only; Level steps: 1 = the XP counts back from the next level (".xp 10-1720")
    uint32      combos = 0;      // ComboBit(race, class) of the classes that do this step
};

struct RouteGiver { uint32 map = 0; float x = 0.f, y = 0.f, z = 0.f; };

// A stop of an explorer route (LaunchStrategy::Explorer, routes from the launch-day exploration guides, doc/LaunchSim.md).
enum class ExploreAction : uint8
{
    Discover = 0,   // enter the subzone (exploration XP is awarded on entering it)
    Flight   = 1,   // talk to the flight master: learn the flight path
    Bind     = 2,   // set the hearthstone at the innkeeper
    Hearth   = 3,   // use the hearthstone (no place of its own)
    Tram     = 4,   // take the Deeprun Tram from this city's portal (name: "Ironforge" or "Stormwind City")
};

struct ExploreStop
{
    ExploreAction action = ExploreAction::Discover;
    uint32 area = 0;              // Discover: the subzone (AreaTable id)
    float  x = 0.f, y = 0.f, z = 0.f;
    char const* name = "";
};

// A class trainer of the launch area (Dun Morogh, Ironforge, Loch Modan) on map 0.
struct ClassTrainer
{
    uint64 spawn_id = 0;
    uint32 map = 0;
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
    // Where CurrentStep is in its route (Steps(route) when no step is left), the route's length and a step by index: the snapshot
    // builder logs each step a guide follower finishes or passes over ([route], checked by the guided bench).
    size_t CurrentIndex(Player const* p, uint8 route) const;
    size_t Steps(uint8 route) const { return route < routes_.size() ? routes_[route].size() : 0; }
    RouteStep const* StepAt(uint8 route, size_t i) const
    { return route < routes_.size() && i < routes_[route].size() ? &routes_[route][i] : nullptr; }
    // How a step the bot has moved past ended: "done", or why it was passed over ("skipped:cannot_take", ...); nullptr when
    // the step is not for this bot's race and class.
    char const* PassedAs(Player const* p, RouteStep const& s) const;
    // An accept step whose quest the bot misses only by a level or two: the route waits on it like a grind step (no quests
    // after it, the bot grinds) instead of passing it over and coming back.
    bool WaitsForLevel(Player const* p, RouteStep const& s) const;
    // The quests of the next `steps` steps this player still has to do, in route order (duplicates removed): what a player
    // with the guide open has picked up or is about to, and works on in parallel (wolves for one quest while the boars of
    // another come along), not one step at a time.
    void Window(Player const* p, uint8 route, uint32 steps, std::vector<uint32>& quests) const;
    // Where a quest's giver stands (first spawn of a creature that starts it), nullptr when unknown.
    RouteGiver const* Giver(uint32 quest) const;

    // The explorer routes of the starting zones (doc/LaunchSim.md "Explorer routes"), resolved against this server's area
    // data; a stop whose subzone, flight master or innkeeper is not found is left out (and logged at load). ExploreRoute picks
    // a bot's route by its starting zone (Bot/LaunchPlaces.h) and seed; kNoRoute when the zone has none.
    uint8 ExploreRoute(uint8 zone, uint32 seed) const;
    bool  HasExploreRoute(uint8 zone) const;
    size_t ExploreStops(uint8 route) const { return route < explore_.size() ? explore_[route].size() : 0; }
    ExploreStop const* ExploreStopAt(uint8 route, size_t i) const
    { return route < explore_.size() && i < explore_[route].size() ? &explore_[route][i] : nullptr; }

    // The class trainer for `cls` nearest to (x, y) on `map`, nullptr when there is none in the starting zones' areas.
    ClassTrainer const* NearestTrainer(uint32 map, uint8 cls, float x, float y) const;

    static uint32 ComboBit(uint8 race, uint8 cls);

private:
    // The step is for this player's race and class and still to do (CurrentStep's test).
    bool Pending(Player const* p, RouteStep const& s, uint32 me) const;
    static bool LevelShort(Player const* p, Quest const* q);

    std::vector<std::string>             names_;
    std::vector<std::vector<RouteStep>>  routes_;
    std::unordered_map<uint32, RouteGiver> givers_;
    std::vector<ClassTrainer>            trainers_;
    std::vector<std::vector<ExploreStop>> explore_;              // per route spec (LaunchRoutes.cpp kExploreSpecs)
    void LoadExploreRoutes();
};

} // namespace Playerbot::V2::Fleet
