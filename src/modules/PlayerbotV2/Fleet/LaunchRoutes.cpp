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
#include <algorithm>
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

    // Class trainers of the launch area (within 3000 y of Coldridge Valley on map 0): trainers that are neither profession nor
    // pet trainers and teach a known set of classes (GetClassMask is every class when the data does not say).
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
        if (!tr || tr->GetType() == Trainer::Type::Tradeskill || tr->GetType() == Trainer::Type::Pet)
            continue;
        uint32 const mask = tr->GetClassMask();
        if (mask == 0 || (mask & 0x7FF) == 0x7FF)
            continue;
        trainers_.push_back({ spawn_id, mask, x, y, data.spawnPoint.GetPositionZ() });
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
