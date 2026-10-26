#include "LaunchRoutes.h"

#include "CreatureData.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "SharedDefines.h"
#include "Trainer.h"
#include "UnitDefines.h"
#include <unordered_set>

namespace Playerbot::V2::Fleet {

LaunchRoutes& LaunchRoutes::Instance()
{
    static LaunchRoutes inst;
    return inst;
}

uint32 LaunchRoutes::ComboBit(uint8 race, uint8 cls)
{
    // Same layout as contrib/launch-sim/import_route.py: dwarf classes in bits 0-11, gnome classes in bits 12-23.
    if (cls >= 12)
        return 0;
    if (race == RACE_DWARF)
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

    // Class trainers (talent-type trainers) of the launch area: within 3000 y of Coldridge Valley on map 0.
    trainers_.clear();
    for (auto const& [spawn_id, data] : sObjectMgr->GetAllCreatureData())
    {
        if (data.mapId != 0)
            continue;
        float const x = data.spawnPoint.GetPositionX(), y = data.spawnPoint.GetPositionY();
        if ((x + 6230.f) * (x + 6230.f) + (y - 400.f) * (y - 400.f) > 3000.f * 3000.f)
            continue;
        CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(data.id);
        if (!ct || !(ct->npcflag & UNIT_NPC_FLAG_TRAINER))
            continue;
        Trainer::Trainer const* tr = sObjectMgr->GetTrainer(sObjectMgr->GetCreatureDefaultTrainer(data.id));
        if (!tr || tr->GetType() != Trainer::Type::Talent)
            continue;
        trainers_.push_back({ spawn_id, tr->GetClassMask(), x, y, data.spawnPoint.GetPositionZ() });
    }
    TC_LOG_INFO("playerbot.v2", "[launch] {} class trainers in the launch area", trainers_.size());

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

ClassTrainer const* LaunchRoutes::NearestTrainer(uint8 cls, float x, float y) const
{
    if (cls == 0 || cls > 32)
        return nullptr;
    uint32 const bit = 1u << (cls - 1);
    ClassTrainer const* best = nullptr;
    float best_d = 0.f;
    for (ClassTrainer const& t : trainers_)
    {
        if (!(t.class_mask & bit))
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

RouteStep const* LaunchRoutes::CurrentStep(Player const* p, uint8 route) const
{
    if (!p || route >= routes_.size())
        return nullptr;
    uint32 const me = ComboBit(p->GetRace(), p->GetClass());
    for (RouteStep const& s : routes_[route])
    {
        if (!(s.combos & me))
            continue;
        if (s.action != RouteAction::Accept && s.action != RouteAction::Complete && s.action != RouteAction::TurnIn)
            continue;   // deathskip / hearth: not driven yet
        Quest const* q = sObjectMgr->GetQuestTemplate(s.quest);
        if (!q)
            continue;
        bool const rewarded = p->GetQuestRewardStatus(s.quest);
        QuestStatus const st = p->GetQuestStatus(s.quest);
        bool const in_log = st == QUEST_STATUS_INCOMPLETE || st == QUEST_STATUS_COMPLETE || st == QUEST_STATUS_FAILED;
        switch (s.action)
        {
            case RouteAction::Accept:
                if (rewarded || in_log)
                    continue;
                if (!p->CanTakeQuest(q, false))
                    continue;   // not for this bot (yet): prerequisites, level, class; the guide moves on
                return &s;
            case RouteAction::Complete:
            {
                if (rewarded || !in_log || st == QUEST_STATUS_COMPLETE)
                    continue;
                QuestObjectives const& objs = q->GetObjectives();
                if (s.objective == 0 || s.objective > objs.size())
                    continue;
                uint16 const slot = p->FindQuestSlot(s.quest);
                if (slot >= MAX_QUEST_LOG_SIZE || p->IsQuestObjectiveComplete(slot, q, objs[s.objective - 1]))
                    continue;
                return &s;
            }
            case RouteAction::TurnIn:
                if (rewarded || !in_log)
                    continue;
                return &s;
            default:
                continue;
        }
    }
    return nullptr;
}

} // namespace Playerbot::V2::Fleet
