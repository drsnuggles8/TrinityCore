#include "LaunchRoutes.h"
#include "Bot/LaunchPlaces.h"

#include "CreatureData.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "SharedDefines.h"
#include "DB2Stores.h"
#include "PhasingHandler.h"
#include "Map.h"
#include "TerrainMgr.h"
#include "Trainer.h"
#include "UnitDefines.h"
#include <algorithm>
#include <span>
#include <unordered_set>

namespace Playerbot::V2::Fleet {

LaunchRoutes& LaunchRoutes::Instance()
{
    static LaunchRoutes inst;
    return inst;
}

uint32 LaunchRoutes::ComboBit(uint8 race, uint8 cls)
{
    // Same layout as contrib/launch-sim/import_route.py: the route's first race in bits 0-11, its second in bits 12-23. A route
    // is for one starting zone (dwarf/gnome, human or night elf: the name says, LaunchSim's PickRoute), so humans and night
    // elves use the first slot.
    if (cls >= 12)
        return 0;
    if (race == RACE_DWARF || race == RACE_HUMAN || race == RACE_NIGHTELF)
        return 1u << cls;
    if (race == RACE_GNOME)
        return 1u << (12 + cls);
    return 0;
}

void LaunchRoutes::Load()
{
    names_.clear();
    routes_.clear();
    givers_.clear();

    // Class trainers of the starting zones' areas (Bot/LaunchPlaces.h: Dun Morogh and Ironforge, Elwynn and Stormwind,
    // Teldrassil and Darnassus, Zephras Isle): trainers that are neither profession nor pet trainers and teach a known set of
    // classes (GetClassMask is every class when the data does not say).
    trainers_.clear();
    for (auto const& [spawn_id, data] : sObjectMgr->GetAllCreatureData())
    {
        float const x = data.spawnPoint.GetPositionX(), y = data.spawnPoint.GetPositionY();
        bool in_area = false;
        for (::Playerbot::LaunchPlaces::StartZone const& z : ::Playerbot::LaunchPlaces::kZones)
            in_area = in_area || ::Playerbot::LaunchPlaces::InHubArea(z, data.mapId, x, y);
        if (!in_area)
            continue;
        CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(data.id);
        if (!ct || !(ct->npcflag & UNIT_NPC_FLAG_TRAINER))
            continue;
        Trainer::Trainer const* tr = sObjectMgr->GetTrainer(sObjectMgr->GetCreatureDefaultTrainer(data.id));
        if (!tr || tr->GetType() == Trainer::Type::Tradeskill || tr->GetType() == Trainer::Type::Pet)
            continue;
        uint32 const mask = tr->GetClassMask();
        if (mask == 0 || (mask & 0x7FF) == 0x7FF)
            continue;
        trainers_.push_back({ spawn_id, data.mapId, mask, x, y, data.spawnPoint.GetPositionZ() });
    }
    TC_LOG_INFO("playerbot.v2", "[launch] {} class trainers in the launch area", trainers_.size());

    LoadExploreRoutes();

    QueryResult r = CharacterDatabase.Query("SELECT route, combos, action, quest, objective FROM playerbot_launch_route_step ORDER BY route, step");
    if (!r)
    {
        TC_LOG_INFO("playerbot.v2", "[launch] no routes in playerbot_launch_route_step (contrib/launch-sim/import_route.py)");
        return;
    }
    std::unordered_set<uint32> accept_quests;
    do
    {
        Field* f = r->Fetch();
        std::string const name = f[0].GetString();
        if (names_.empty() || names_.back() != name)
        {
            names_.push_back(name);
            routes_.emplace_back();
        }
        RouteStep s;
        s.combos = f[1].GetUInt32();
        s.action = RouteAction(f[2].GetUInt8());
        s.quest = f[3].GetUInt32();
        s.objective = f[4].GetUInt8();
        routes_.back().push_back(s);
        if (s.action == RouteAction::Accept)
            accept_quests.insert(s.quest);
    } while (r->NextRow());

    // Givers: the creatures that start the routes' quests, at their first spawn.
    std::unordered_map<uint32, uint32> giver_entry;   // quest -> creature entry
    for (auto const& [entry, quest] : *sObjectMgr->GetCreatureQuestRelationMapHACK())
        if (accept_quests.contains(quest) && !giver_entry.contains(quest))
            giver_entry[quest] = entry;
    std::unordered_set<uint32> wanted;
    for (auto const& [quest, entry] : giver_entry)
        wanted.insert(entry);
    std::unordered_map<uint32, RouteGiver> by_entry;   // creature entry -> its first spawn
    for (auto const& [spawn_id, data] : sObjectMgr->GetAllCreatureData())
        if (wanted.contains(data.id) && !by_entry.contains(data.id))
            by_entry[data.id] = { data.mapId, data.spawnPoint.GetPositionX(), data.spawnPoint.GetPositionY(), data.spawnPoint.GetPositionZ() };
    for (auto const& [quest, entry] : giver_entry)
        if (auto it = by_entry.find(entry); it != by_entry.end())
            givers_[quest] = it->second;
    size_t steps = 0;
    for (auto const& r2 : routes_) steps += r2.size();
    TC_LOG_INFO("playerbot.v2", "[launch] {} routes loaded ({} steps), {} quest givers placed", names_.size(), steps, givers_.size());
}

namespace {

struct RouteDef { ExploreAction action; char const* name; };

// From the launch-day exploration routes (Jay Ell's dwarf/gnome sheet and video, the Elysium and Blizzard forum
// write-ups; doc/LaunchSim.md "Explorer"). Discover stops name a subzone; Flight and Bind name the zone or town whose flight
// master / innkeeper is meant.
constexpr RouteDef kSouthLoop[] = {
    { ExploreAction::Discover, "Coldridge Pass" }, { ExploreAction::Discover, "Kharanos" }, { ExploreAction::Bind, "Kharanos" },
    { ExploreAction::Flight, "Ironforge" }, { ExploreAction::Tram, "Ironforge" }, { ExploreAction::Flight, "Stormwind City" },
    { ExploreAction::Discover, "Goldshire" }, { ExploreAction::Discover, "Crystal Lake" },
    { ExploreAction::Discover, "Brackwell Pumpkin Patch" }, { ExploreAction::Discover, "Eastvale Logging Camp" },
    { ExploreAction::Discover, "Ridgepoint Tower" }, { ExploreAction::Discover, "Maclure Vineyards" },
    { ExploreAction::Discover, "Fargodeep Mine" }, { ExploreAction::Discover, "Stonefield Farm" },
    { ExploreAction::Discover, "Forest's Edge" }, { ExploreAction::Discover, "Westbrook Garrison" },
    { ExploreAction::Discover, "Jansen Stead" }, { ExploreAction::Discover, "Saldean's Farm" },
    { ExploreAction::Flight, "Sentinel Hill" }, { ExploreAction::Discover, "The Dead Acre" },
    { ExploreAction::Hearth, "" },
    { ExploreAction::Discover, "Shimmer Ridge" }, { ExploreAction::Discover, "Chill Breeze Valley" },
    { ExploreAction::Discover, "Iceflow Lake" }, { ExploreAction::Discover, "Brewnall Village" },
    { ExploreAction::Discover, "Frostmane Hold" }, { ExploreAction::Discover, "Gnomeregan" },
    { ExploreAction::Discover, "Misty Pine Refuge" }, { ExploreAction::Discover, "Tundrid Hills" },
    { ExploreAction::Discover, "Amberstill Ranch" }, { ExploreAction::Discover, "Gol'Bolar Quarry" },
};
constexpr RouteDef kNorthRun[] = {
    { ExploreAction::Discover, "Coldridge Pass" }, { ExploreAction::Discover, "Kharanos" }, { ExploreAction::Bind, "Kharanos" },
    { ExploreAction::Flight, "Ironforge" },
    { ExploreAction::Discover, "Misty Pine Refuge" }, { ExploreAction::Discover, "Tundrid Hills" },
    { ExploreAction::Discover, "Amberstill Ranch" }, { ExploreAction::Discover, "Gol'Bolar Quarry" },
    { ExploreAction::Discover, "Shimmer Ridge" }, { ExploreAction::Discover, "Chill Breeze Valley" },
    { ExploreAction::Discover, "Brewnall Village" }, { ExploreAction::Discover, "Frostmane Hold" },
    { ExploreAction::Discover, "Gnomeregan" },
    { ExploreAction::Discover, "North Gate Pass" }, { ExploreAction::Discover, "Algaz Station" },
    { ExploreAction::Flight, "Thelsamar" }, { ExploreAction::Discover, "Dun Algaz" },
    { ExploreAction::Discover, "Dun Modr" }, { ExploreAction::Flight, "Menethil Harbor" },
};

// Jay Ell's Human tab (Wayback capture of his deleted sheet): Northshire to Goldshire (hearth), east Elwynn, Redridge
// (Lakeshire flight path), hearth back, west Elwynn, Westfall (Sentinel Hill), Stormwind, the tram, Ironforge, east Dun
// Morogh and the Kharanos loop. His deathskips (Hogger, Duskwood) are walked.
constexpr RouteDef kHumanSheet[] = {
    { ExploreAction::Discover, "Crystal Lake" }, { ExploreAction::Bind, "Goldshire" }, { ExploreAction::Discover, "Goldshire" },
    { ExploreAction::Discover, "Brackwell Pumpkin Patch" }, { ExploreAction::Discover, "Eastvale Logging Camp" },
    { ExploreAction::Discover, "Ridgepoint Tower" }, { ExploreAction::Discover, "Three Corners" },
    { ExploreAction::Discover, "Lakeridge Highway" }, { ExploreAction::Flight, "Lakeshire" },
    { ExploreAction::Discover, "Lake Everstill" }, { ExploreAction::Discover, "Lakeshire" },
    { ExploreAction::Discover, "Redridge Canyons" }, { ExploreAction::Hearth, "" },
    { ExploreAction::Discover, "Maclure Vineyards" }, { ExploreAction::Discover, "Fargodeep Mine" },
    { ExploreAction::Discover, "Stonefield Farm" }, { ExploreAction::Discover, "Forest's Edge" },
    { ExploreAction::Discover, "Westbrook Garrison" }, { ExploreAction::Discover, "Jansen Stead" },
    { ExploreAction::Discover, "Saldean's Farm" }, { ExploreAction::Flight, "Sentinel Hill" },
    { ExploreAction::Discover, "The Dead Acre" }, { ExploreAction::Discover, "Stendel's Pond" },
    { ExploreAction::Discover, "Stormwind City" }, { ExploreAction::Flight, "Stormwind City" },
    { ExploreAction::Tram, "Stormwind City" }, { ExploreAction::Flight, "Ironforge" },
    { ExploreAction::Discover, "Misty Pine Refuge" }, { ExploreAction::Discover, "Tundrid Hills" },
    { ExploreAction::Discover, "Amberstill Ranch" }, { ExploreAction::Discover, "Gol'Bolar Quarry" },
    { ExploreAction::Discover, "Kharanos" }, { ExploreAction::Discover, "Shimmer Ridge" },
    { ExploreAction::Discover, "Chill Breeze Valley" }, { ExploreAction::Discover, "Iceflow Lake" },
    { ExploreAction::Discover, "Brewnall Village" }, { ExploreAction::Discover, "Frostmane Hold" },
    { ExploreAction::Discover, "Coldridge Pass" },
};

// The routes of each starting zone (Bot/LaunchPlaces.h StartZoneId) and the share of its explorers on each.
struct ExploreSpec { uint8 zone; uint8 share; char const* name; std::span<RouteDef const> defs; };
ExploreSpec const kExploreSpecs[] = {
    { ::Playerbot::LaunchPlaces::kZoneColdridge,  60, "south loop",  kSouthLoop },
    { ::Playerbot::LaunchPlaces::kZoneColdridge,  40, "north run",   kNorthRun },
    { ::Playerbot::LaunchPlaces::kZoneNorthshire, 100, "human sheet", kHumanSheet },
};

// The regions the routes cross, where the ground is sampled for subzone points: Dun Morogh and Ironforge, Elwynn and
// Stormwind, Westfall, Loch Modan, the Wetlands, Redridge (map 0); Teldrassil and Darkshore (map 1).
struct Box { uint32 map; float x0, x1, y0, y1; };
constexpr Box kRegions[] = {
    { 0, -6800.f, -4400.f, -1600.f, 1100.f }, { 0, -9900.f, -8000.f, -1500.f, 1600.f }, { 0, -11500.f, -9800.f, 300.f, 2300.f },
    { 0, -6100.f, -4500.f, -4700.f, -1600.f }, { 0, -4500.f, -2300.f, -3600.f, -400.f }, { 0, -9900.f, -8800.f, -3300.f, -1500.f },
    { 1, 8500.f, 11200.f, -500.f, 2600.f }, { 1, 4000.f, 8000.f, -1500.f, 1500.f },
};

bool InRegions(uint32 map, float x, float y)
{
    for (Box const& b : kRegions)
        if (map == b.map && x >= b.x0 && x <= b.x1 && y >= b.y0 && y <= b.y1)
            return true;
    return false;
}

} // anonymous namespace

void LaunchRoutes::LoadExploreRoutes()
{
    explore_.assign(std::size(kExploreSpecs), {});
    // Subzone names of the routes -> AreaTable ids (Eastern Kingdoms and Kalimdor).
    std::unordered_map<std::string, std::vector<uint32>> ids_by_name;
    for (AreaTableEntry const* at : sAreaTableStore)
        if (at && (at->ContinentID == 0 || at->ContinentID == 1) && at->AreaName[LOCALE_enUS] && *at->AreaName[LOCALE_enUS])
            ids_by_name[at->AreaName[LOCALE_enUS]].push_back(at->ID);
    // Spawns of the regions by subzone (creatures, for a point on the ground), with the flight masters and innkeepers.
    struct Spot { float x, y, z; };
    std::unordered_map<uint32, std::vector<Spot>> by_area;
    std::vector<std::pair<Spot, uint32>> flight_masters, innkeepers;   // spot, subzone
    for (auto const& [spawn_id, data] : sObjectMgr->GetAllCreatureData())
    {
        if (!InRegions(data.mapId, data.spawnPoint.GetPositionX(), data.spawnPoint.GetPositionY()))
            continue;
        uint32 const area = sTerrainMgr.GetAreaId(PhasingHandler::GetEmptyPhaseShift(), data.mapId, data.spawnPoint);
        Spot const sp{ data.spawnPoint.GetPositionX(), data.spawnPoint.GetPositionY(), data.spawnPoint.GetPositionZ() };
        by_area[area].push_back(sp);
        if (CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(data.id))
        {
            if (ct->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER)
                flight_masters.push_back({ sp, area });
            if (ct->npcflag & UNIT_NPC_FLAG_INNKEEPER)
                innkeepers.push_back({ sp, area });
        }
    }
    // The ground itself, every 30 y over the regions: a subzone without creatures (Tundrid Hills) or whose creatures all stand
    // in buildings of their own subzone (Kharanos: Thunderbrew Distillery) still gets points. Terrain height only (no vmaps),
    // so a point is outdoors, where exploration counts.
    for (Box const& b : kRegions)
        if (std::shared_ptr<TerrainInfo> terrain = sTerrainMgr.LoadTerrain(b.map))
            for (float x = b.x0; x <= b.x1; x += 30.f)
                for (float y = b.y0; y <= b.y1; y += 30.f)
                {
                    float const z = terrain->GetGridHeight(PhasingHandler::GetEmptyPhaseShift(), b.map, x, y);
                    if (z <= INVALID_HEIGHT)
                        continue;
                    uint32 const area = sTerrainMgr.GetAreaId(PhasingHandler::GetEmptyPhaseShift(), b.map, x, y, z);
                    if (area)
                        by_area[area].push_back({ x, y, z });
                }
    // Is `area` the subzone `name`, or inside it (a town's buildings are subzones of their own)?
    auto const in_named = [&](uint32 area, std::vector<uint32> const& named)
    {
        for (int guard = 0; area && guard < 4; ++guard)
        {
            if (std::find(named.begin(), named.end(), area) != named.end())
                return true;
            AreaTableEntry const* at = sAreaTableStore.LookupEntry(area);
            area = at ? at->ParentAreaID : 0;
        }
        return false;
    };
    std::vector<std::string> missing;
    for (uint8 r = 0; r < std::size(kExploreSpecs); ++r)
    {
        std::span<RouteDef const> const defs = kExploreSpecs[r].defs;
        for (RouteDef const& d : defs)
        {
            ExploreStop st;
            st.action = d.action;
            st.name = d.name;
            if (d.action == ExploreAction::Hearth)
            {
                explore_[r].push_back(st);
                continue;
            }
            if (d.action == ExploreAction::Tram)
            {
                // The departure city's portal (the ride itself: idle:launch_tram).
                ::Playerbot::LaunchPlaces::Spot const& port = std::string_view(d.name) == "Ironforge"
                    ? ::Playerbot::LaunchPlaces::kTramIronforge : ::Playerbot::LaunchPlaces::kTramStormwind;
                st.x = port.x; st.y = port.y; st.z = port.z;
                explore_[r].push_back(st);
                continue;
            }
            auto const it = ids_by_name.find(d.name);
            if (it == ids_by_name.end())
            {
                missing.push_back(d.name);
                continue;
            }
            bool found = false;
            // The points of the named subzone (and of areas inside it), and the one nearest their middle: a point on the
            // ground, inside the subzone.
            std::vector<Spot> pts;
            for (auto const& [area, list] : by_area)
                if (in_named(area, it->second))
                    pts.insert(pts.end(), list.begin(), list.end());
            Spot const* mid = nullptr;
            if (!pts.empty())
            {
                double cx = 0, cy = 0;
                for (Spot const& p : pts) { cx += p.x; cy += p.y; }
                cx /= pts.size(); cy /= pts.size();
                mid = &pts.front();
                for (Spot const& p : pts)
                    if ((p.x - cx) * (p.x - cx) + (p.y - cy) * (p.y - cy) < (mid->x - cx) * (mid->x - cx) + (mid->y - cy) * (mid->y - cy))
                        mid = &p;
            }
            if (d.action == ExploreAction::Discover)
            {
                if (mid)
                {
                    st.area = it->second.front();
                    st.x = mid->x; st.y = mid->y; st.z = mid->z;
                    found = true;
                }
            }
            else
            {
                // The flight master / innkeeper of the place: one standing in it, else the one nearest its middle (within
                // 400 y; a town's inn is a building subzone of its own, Kharanos' Thunderbrew Distillery).
                auto const& list = d.action == ExploreAction::Flight ? flight_masters : innkeepers;
                Spot const* best = nullptr;
                float best_d = 400.f * 400.f;
                for (auto const& [sp, area] : list)
                {
                    if (in_named(area, it->second))
                    {
                        best = &sp;
                        break;
                    }
                    if (mid)
                        if (float const dd = (sp.x - mid->x) * (sp.x - mid->x) + (sp.y - mid->y) * (sp.y - mid->y); dd < best_d)
                            best = &sp, best_d = dd;
                }
                if (best)
                {
                    st.x = best->x; st.y = best->y; st.z = best->z;
                    found = true;
                }
            }
            if (found)
                explore_[r].push_back(st);
            else
                missing.push_back(d.name);
        }
    }
    std::string miss;
    for (std::string const& m : missing)
        miss += (miss.empty() ? "" : ", ") + m;
    std::string counts;
    for (size_t r = 0; r < explore_.size(); ++r)
        counts += (r ? ", " : "") + std::string(kExploreSpecs[r].name) + " " + std::to_string(explore_[r].size());
    TC_LOG_INFO("playerbot.v2", "[launch] explorer routes: {} stops{}{}", counts, missing.empty() ? "" : "; not found: ", miss);
}

uint8 LaunchRoutes::ExploreRoute(uint8 zone, uint32 seed) const
{
    uint32 total = 0;
    for (size_t r = 0; r < explore_.size(); ++r)
        if (kExploreSpecs[r].zone == zone && !explore_[r].empty())
            total += kExploreSpecs[r].share;
    if (!total)
        return kNoRoute;
    uint32 roll = (seed * 2654435761u >> 8) % total;
    for (size_t r = 0; r < explore_.size(); ++r)
        if (kExploreSpecs[r].zone == zone && !explore_[r].empty())
        {
            if (roll < kExploreSpecs[r].share)
                return uint8(r);
            roll -= kExploreSpecs[r].share;
        }
    return kNoRoute;
}

bool LaunchRoutes::HasExploreRoute(uint8 zone) const
{
    for (size_t r = 0; r < explore_.size(); ++r)
        if (kExploreSpecs[r].zone == zone && !explore_[r].empty())
            return true;
    return false;
}

ClassTrainer const* LaunchRoutes::NearestTrainer(uint32 map, uint8 cls, float x, float y) const
{
    if (cls == 0 || cls > 32)
        return nullptr;
    uint32 const bit = 1u << (cls - 1);
    ClassTrainer const* best = nullptr;
    float best_d = 0.f;
    for (ClassTrainer const& t : trainers_)
    {
        if (t.map != map || !(t.class_mask & bit))
            continue;
        float const d = (t.x - x) * (t.x - x) + (t.y - y) * (t.y - y);
        if (!best || d < best_d)
            best = &t, best_d = d;
    }
    return best;
}

uint8 LaunchRoutes::Find(std::string const& name) const
{
    for (size_t i = 0; i < names_.size(); ++i)
        if (names_[i] == name)
            return uint8(i);
    return kNoRoute;
}

RouteGiver const* LaunchRoutes::Giver(uint32 quest) const
{
    auto it = givers_.find(quest);
    return it == givers_.end() ? nullptr : &it->second;
}

bool LaunchRoutes::Pending(Player const* p, RouteStep const& s, uint32 me) const
{
    if (!(s.combos & me))
        return false;
    if (s.action != RouteAction::Accept && s.action != RouteAction::Complete && s.action != RouteAction::TurnIn)
        return false;   // deathskip / hearth: not driven yet
    Quest const* q = sObjectMgr->GetQuestTemplate(s.quest);
    if (!q)
        return false;
    bool const rewarded = p->GetQuestRewardStatus(s.quest);
    QuestStatus const st = p->GetQuestStatus(s.quest);
    bool const in_log = st == QUEST_STATUS_INCOMPLETE || st == QUEST_STATUS_COMPLETE || st == QUEST_STATUS_FAILED;
    switch (s.action)
    {
        case RouteAction::Accept:
            if (rewarded || in_log)
                return false;
            if (!p->CanTakeQuest(q, false))
                return false;   // not for this bot (yet): prerequisites, level, class; the guide moves on
            if (!Giver(s.quest))
                return false;   // started by an item or an object, or a giver without a spawn: the bot could not walk there
            return true;
        case RouteAction::Complete:
        {
            if (rewarded || !in_log || st == QUEST_STATUS_COMPLETE)
                return false;
            QuestObjectives const& objs = q->GetObjectives();
            if (s.objective == 0 || s.objective > objs.size())
                return false;
            uint16 const slot = p->FindQuestSlot(s.quest);
            return slot < MAX_QUEST_LOG_SIZE && !p->IsQuestObjectiveComplete(slot, q, objs[s.objective - 1]);
        }
        case RouteAction::TurnIn:
            return !rewarded && in_log;
        default:
            return false;
    }
}

RouteStep const* LaunchRoutes::CurrentStep(Player const* p, uint8 route) const
{
    if (!p || route >= routes_.size())
        return nullptr;
    uint32 const me = ComboBit(p->GetRace(), p->GetClass());
    for (RouteStep const& s : routes_[route])
        if (Pending(p, s, me))
            return &s;
    return nullptr;
}

void LaunchRoutes::Window(Player const* p, uint8 route, uint32 steps, std::vector<uint32>& quests) const
{
    quests.clear();
    if (!p || route >= routes_.size())
        return;
    uint32 const me = ComboBit(p->GetRace(), p->GetClass());
    for (RouteStep const& s : routes_[route])
    {
        if (!Pending(p, s, me))
            continue;
        if (std::find(quests.begin(), quests.end(), s.quest) == quests.end())
            quests.push_back(s.quest);
        if (--steps == 0)
            break;
    }
}

} // namespace Playerbot::V2::Fleet
