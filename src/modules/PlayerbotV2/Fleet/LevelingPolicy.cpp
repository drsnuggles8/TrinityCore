#include "LevelingPolicy.h"
#include "LevelingMath.h"
#include "LevelingEpisode.h"
#include "TrainingCapture.h"
#include "TrainingPlayer.h"
#include "Bot/BotAI.h"
#include "Bot/BotSnapshotView.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/BotRegistry.h"
#include "Services.h"
#include "Config.h"
#include "GameTime.h"
#include "Log.h"
#include "CryptoHash.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Map.h"
#include "GridDefines.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Trainer.h"
#include "ItemTemplate.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellHistory.h"
#include "UnitDefines.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <deque>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <random>
#include <unordered_map>

namespace Playerbot::V2::Fleet::LevelingPolicy
{
namespace
{
using namespace Leveling;
using Writer=rapidjson::Writer<rapidjson::StringBuffer>;
std::shared_ptr<Model const> model;
std::unordered_set<std::uint64_t> guids;
std::unordered_set<std::uint64_t> participants;
Task task;
rapidjson::Document episode_seeds;
bool evaluation=false;
struct Choice
{
    Kind kind=Kind::Baseline;ObjectGuid target;std::uint32_t quest=0,item=0,spell=0;
    float x=0,y=0,z=0;ActionFeatures features{};
    std::uint8_t operation=0,bag=0,slot=0,equip_slot=0;
    float price=0,mana_cost=0,health_effect=0,mana_effect=0;
};
struct KnownGoal { std::uint32_t quest=0;float x=0,y=0,z=0; };
struct Frame { Progress progress;State state{};bool allowed=true;std::uint8_t level=1;std::vector<KnownGoal> goals;std::vector<Choice> affordances; };
struct Pending
{
    bool active=false;std::uint64_t id=0,segment=1;Progress before;State state{};
    double value=0,logp=0;Choice action;std::uint32_t refusals=0;
};
struct Actor
{
    std::mutex mutex;std::deque<Frame> frames;Pending pending;std::uint64_t sequence=0,segment=1;
    std::uint32_t born=0,moved=0;float markx=0,marky=0;bool scope_finished=false;
    double xp_reference=400;std::mt19937_64 random;
    std::uint32_t goals_at=0;std::vector<KnownGoal> goals;
    Episode episode;bool ending_logged=false;int last_kind=-1;
    double last_reward=0,last_refusals=0,last_move=0,last_goal_progress=0,hp_delta=0,mana_delta=0;
    std::unordered_map<std::string,double> goal_failures;
    std::uint32_t affordances_at=0;std::vector<Choice> affordances;
    explicit Actor(std::uint64_t guid):random(guid^0x504c55544f504f4cull) {}
};
std::mutex actors_mutex;
std::unordered_map<std::uint64_t,std::shared_ptr<Actor>> actors;
std::shared_ptr<Actor> Get(std::uint64_t guid)
{
    std::lock_guard lock(actors_mutex);
    auto it=actors.find(guid);if (it!=actors.end())return it->second;
    if (actors.size()>=128)return {};
    auto p=std::make_shared<Actor>(guid);
    auto id=std::to_string(guid);
    if(episode_seeds.IsObject()&&episode_seeds.HasMember("participants")&&episode_seeds["participants"].HasMember(id.c_str()))
    {
        auto const& seed=episode_seeds["participants"][id.c_str()];
        if(seed.HasMember("sampling_seed")&&seed["sampling_seed"].IsUint64())p->random.seed(seed["sampling_seed"].GetUint64());
        if(seed.HasMember("last_xp"))p->episode.Seed(seed);
        if(seed.HasMember("memory"))
        {
            auto const& memory=seed["memory"];
            if(memory.HasMember("last_kind"))p->last_kind=memory["last_kind"].GetInt();
            if(memory.HasMember("last_reward"))p->last_reward=Number(memory["last_reward"]);
            if(memory.HasMember("refusals"))p->last_refusals=Number(memory["refusals"]);
            if(memory.HasMember("move_progress"))p->last_move=Number(memory["move_progress"]);
            if(memory.HasMember("goal_progress"))p->last_goal_progress=Number(memory["goal_progress"]);
            if(memory.HasMember("health_delta"))p->hp_delta=Number(memory["health_delta"]);
            if(memory.HasMember("mana_delta"))p->mana_delta=Number(memory["mana_delta"]);
            if(memory.HasMember("goal_failures"))for(auto it=memory["goal_failures"].MemberBegin();it!=memory["goal_failures"].MemberEnd();++it)p->goal_failures[it->name.GetString()]=Number(it->value);
        }
    }
    actors.emplace(guid,p);return p;
}
void Text(Writer& w,char const* key,std::string const& value) { w.Key(key);w.String(value.c_str()); }
template<class Values>void Array(Writer& w,char const* key,Values const& values)
{ w.Key(key);w.StartArray();for(auto x:values)w.Double(x);w.EndArray(); }
float Fraction(double n,double d) { return d>0?float(std::clamp(n/d,0.,1.)):0; }
std::vector<Choice> Affordances(Player const* player,BotSnapshot const& s)
{
    std::vector<Choice> out;if(!player)return out;BotSnapshotView view(s);
    auto add=[&](Kind kind,ObjectGuid target,float x,float y,float z)->Choice&
    { Choice c;c.kind=kind;c.target=target;c.x=x;c.y=y;c.z=z;out.push_back(c);return out.back(); };
    auto effects=[](SpellInfo const* spell,Choice& c)
    {
        for(auto const& e:spell->GetEffects())
        {
            if(e.IsEffect(SPELL_EFFECT_HEAL))c.health_effect+=float(std::max(0,e.CalcValueAsInt()));
            if(e.IsEffect(SPELL_EFFECT_ENERGIZE)&&e.MiscValue==POWER_MANA)c.mana_effect+=float(std::max(0,e.CalcValueAsInt()));
        }
    };
    // Only actual usable on-use items are offered. Delivery items and inert quest objects are not use actions.
    for(auto const& item:s.inventory.bag_items)
    {
        auto proto=sObjectMgr->GetItemTemplate(item.entry);
        if(!proto||item.entry==6948||player->CanUseItem(proto)!=EQUIP_ERR_OK)continue;
        for(auto const* effect:proto->Effects)
        {
            if(!effect||effect->TriggerType!=ITEM_SPELLTRIGGER_ON_USE||effect->SpellID<=0)continue;
            auto spell=sSpellMgr->GetSpellInfo(uint32(effect->SpellID),DIFFICULTY_NONE);
            if(!spell||player->GetSpellHistory()->HasCooldown(spell,item.entry))continue;
            auto offer=[&](ObjectGuid id,float x,float y,float z)
            {
                Choice& c=add(Kind::QuestItem,id,x,y,z);c.item=item.entry;c.spell=spell->Id;effects(spell,c);
                auto specific=spell->GetSpellSpecific();
                c.operation=specific==SPELL_SPECIFIC_FOOD?1:specific==SPELL_SPECIFIC_DRINK?2:specific==SPELL_SPECIFIC_FOOD_AND_DRINK?3:0;
                for(auto const& q:s.quest_log.quests)if(q.source_item_id==item.entry)c.quest=q.quest_id;
            };
            if(spell->CheckTarget(player,player)==SPELL_CAST_OK)offer(s.guid,s.position.x,s.position.y,s.position.z);
            std::size_t targets=0;
            for(auto const& target:s.quest_log.actionable_objectives)
            {
                if(targets>=3)break;auto unit=ObjectAccessor::GetUnit(*player,target.source_guid);
                if(!unit||!player->CanSeeOrDetect(unit)||spell->CheckTarget(player,unit)!=SPELL_CAST_OK)continue;
                offer(unit->GetGUID(),unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ());out.back().quest=target.quest_id;++targets;
            }
            break;
        }
        if(out.size()>=12)break;
    }
    if(!s.vitals.in_combat)
    {
        for(auto const& npc:s.combat.nearby_friends)
        {
            if(npc.is_player)continue;float distance=std::hypot(std::hypot(npc.x-s.position.x,npc.y-s.position.y),npc.z-s.position.z);
            if(npc.npc_flags&UNIT_NPC_FLAG_TRAINER)
            {
                auto trainer=sObjectMgr->GetTrainer(sObjectMgr->GetCreatureDefaultTrainer(npc.entry));
                if(trainer&&trainer->GetType()!=Trainer::Type::Tradeskill&&trainer->GetType()!=Trainer::Type::Pet&&(trainer->GetClassMask()&player->GetClassMask()))
                {
                    std::size_t count=0;
                    for(auto const& entry:trainer->GetSpells())
                    {
                        if(count>=4||entry.ReqLevel>player->GetLevel()||player->HasSpell(entry.SpellId)||entry.MoneyCost>player->GetMoney())continue;
                        if(entry.ReqSkillLine&&player->GetSkillValue(entry.ReqSkillLine)<entry.ReqSkillRank)continue;
                        if(std::any_of(entry.ReqAbility.begin(),entry.ReqAbility.end(),[&](auto id){return id&&!player->HasSpell(id);}))continue;
                        auto info=sSpellMgr->GetSpellInfo(entry.SpellId,DIFFICULTY_NONE);if(!info||!player->IsSpellFitByClassAndRace(entry.SpellId))continue;
                        bool teaches_known=false;
                        for(auto const& e:info->GetEffects())if(e.IsEffect(SPELL_EFFECT_LEARN_SPELL)&&player->HasSpell(e.TriggerSpell))teaches_known=true;
                        if(teaches_known)continue;
                        auto& c=add(distance<=4.5?Kind::Train:Kind::Approach,npc.guid,npc.x,npc.y,npc.z);c.spell=entry.SpellId;c.price=float(entry.MoneyCost);++count;
                    }
                }
            }
            if(npc.npc_flags&UNIT_NPC_FLAG_VENDOR)
            {
                if(s.vendor_visit.phases_pending&2)add(distance<=4.5?Kind::Vendor:Kind::Approach,npc.guid,npc.x,npc.y,npc.z);
                auto vendor=sObjectMgr->GetNpcVendorItemList(npc.entry);
                if(vendor&&player->GetMoney()>0&&s.consumables.food_drink_count<8)
                    for(uint32 i=0;i<vendor->GetItemCount();++i)
                    {
                        auto item=vendor->GetItem(i);if(!item||item->ExtendedCost||item->PlayerConditionId)continue;
                        auto proto=sObjectMgr->GetItemTemplate(item->item);
                        if(!proto||proto->GetClass()!=ITEM_CLASS_CONSUMABLE||proto->GetSubClass()!=ITEM_SUBCLASS_FOOD_DRINK||proto->GetBuyPrice()>player->GetMoney()||player->CanUseItem(proto)!=EQUIP_ERR_OK)continue;
                        auto& c=add(distance<=4.5?Kind::Vendor:Kind::Approach,npc.guid,npc.x,npc.y,npc.z);c.operation=1;c.item=item->item;c.price=float(proto->GetBuyPrice());break;
                    }
            }
            if((npc.npc_flags&UNIT_NPC_FLAG_REPAIR)&&s.vendor_visit.estimated_repair_cost&&s.vendor_visit.estimated_repair_cost<=player->GetMoney())
            {auto& c=add(distance<=4.5?Kind::Repair:Kind::Approach,npc.guid,npc.x,npc.y,npc.z);c.price=float(s.vendor_visit.estimated_repair_cost);}
        }
        for(auto const& item:s.inventory.bag_items)
            if(item.equip_slot<19&&item.item_level>s.inventory.equipped[item.equip_slot].item_level)
            {auto& c=add(Kind::Equip,s.guid,s.position.x,s.position.y,s.position.z);c.item=item.entry;c.bag=item.bag;c.slot=item.slot;c.equip_slot=item.equip_slot;c.health_effect=float(item.item_level)/20;break;}
        for(auto const& option:s.gossip.gossip_options)
        {auto& c=add(Kind::Interact,s.gossip.gossip_npc,s.position.x,s.position.y,s.position.z);c.operation=3;c.slot=option.order_index;}
    }
    // Rank-aware learned spell choices, with actual mana costs and target checks rather than hardcoded spell IDs.
    std::unordered_set<uint32> spell_roots;
    for(auto id:s.spellbook.known_spells)
    {
        auto spell=sSpellMgr->GetSpellInfo(id,DIFFICULTY_NONE);
        if(!spell||spell->IsPassive()||view.resolve_rank(id)!=id||!view.is_ready(id)||!view.can_cast())continue;
        auto root=spell->GetFirstRankSpell()->Id;if(!spell_roots.insert(root).second)continue;
        bool heal=false,damage=false;
        for(auto const& e:spell->GetEffects()){heal|=e.IsEffect(SPELL_EFFECT_HEAL);damage|=e.IsEffect(SPELL_EFFECT_SCHOOL_DAMAGE);}
        if(!heal&&!damage)continue;
        int mana=0;for(auto const& cost:spell->CalcPowerCost(player,spell->GetSchoolMask()))if(cost.Power==POWER_MANA)mana+=cost.Amount;
        if(mana>s.vitals.power[POWER_MANA])continue;
        if(heal&&s.vitals.hp<s.vitals.max_hp&&spell->CheckTarget(player,player)==SPELL_CAST_OK)
        {auto& c=add(Kind::Heal,s.guid,s.position.x,s.position.y,s.position.z);c.spell=id;c.mana_cost=float(mana);effects(spell,c);}
        if(damage)
            for(auto const& enemy:s.combat.nearby_enemies)
            {
                if(enemy.hp<=0||enemy.untargetable||!enemy.in_los||enemy.no_xp_kill||enemy.is_player||(enemy.tapped&&!enemy.tapped_by_us))continue;
                auto unit=ObjectAccessor::GetUnit(*player,enemy.guid);if(!unit||spell->CheckTarget(player,unit)!=SPELL_CAST_OK)continue;
                auto& c=add(Kind::Cast,enemy.guid,enemy.x,enemy.y,enemy.z);c.spell=id;c.mana_cost=float(mana);break;
            }
        if(out.size()>=32)break;
    }
    ObjectGuid corpse;Services::Registry().with_loot(s.bot_id,[&](auto const& list){if(!list.empty())corpse=list.front();});
    if(!corpse.IsEmpty())
        if(auto unit=ObjectAccessor::GetCreature(*player,corpse);unit&&player->CanSeeOrDetect(unit))
        {
            float d=player->GetDistance(unit);add(d<=4.5?Kind::Loot:Kind::Approach,corpse,unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ());
        }
    return out;
}
double Potential(BotSnapshot const& s)
{
    double total=0;
    for (auto const& q:s.quest_log.quests)
    {
        if(std::find(task.eligible.begin(),task.eligible.end(),q.quest_id)==task.eligible.end())continue;
        double progress=0;
        for (auto const& o:q.objectives) if(o.amount>0) progress+=std::clamp(double(o.progress)/o.amount,0.,1.);
        total+=q.objectives.empty()?(q.state==1?1.:0.):progress/q.objectives.size();
    }
    return std::clamp(total/25.,0.,1.);
}
void ApplyMemory(State& x,Actor const& actor)
{
    x[36]=float(std::clamp(actor.last_reward/4,-2.,2.));x[37]=float(std::min(1.,actor.last_refusals/10));x[38]=float(std::min(2.,actor.last_move/20));
    x[39]=float(std::clamp(actor.last_goal_progress/40,-2.,2.));
    for(std::size_t bit=0;bit<5;++bit)x[40+bit]=actor.last_kind>=0?float((actor.last_kind>>bit)&1):0;
    x[46]=float(actor.hp_delta);x[47]=float(actor.mana_delta);
}
void Memory(Writer& w,Actor const& actor)
{
    w.Key("memory");w.StartObject();w.Key("last_kind");w.Int(actor.last_kind);w.Key("last_reward");w.Double(actor.last_reward);
    w.Key("refusals");w.Double(actor.last_refusals);w.Key("move_progress");w.Double(actor.last_move);
    w.Key("goal_progress");w.Double(actor.last_goal_progress);w.Key("health_delta");w.Double(actor.hp_delta);w.Key("mana_delta");w.Double(actor.mana_delta);
    w.Key("goal_failures");w.StartObject();for(auto const& [key,value]:actor.goal_failures){w.Key(key.c_str());w.Double(value);}w.EndObject();w.EndObject();
}
std::string GoalKey(Choice const& c)
{ return std::to_string(std::lround(c.x/5))+":"+std::to_string(std::lround(c.y/5))+":"+std::to_string(std::lround(c.z/5)); }
State Context(BotSnapshot const& s,Actor const& actor,Progress const& p)
{
    BotSnapshotView v(s);State x{};
    x[0]=float(p.hp);x[1]=float(p.mana);x[2]=std::min(1.f,float(s.identity.level)/10);
    x[3]=Fraction(s.identity.xp,s.identity.xp_for_level);x[4]=s.vitals.in_combat;x[5]=std::min(1.f,float(s.bags.bag_free_slots)/16);
    x[6]=std::min(1.f,float(s.quest_log.quests.size())/25);x[7]=std::min(1.f,float(s.quest_discovery.quest_turnins.size())/10);
    x[8]=std::min(1.f,float(s.quest_discovery.quest_offers.size())/10);x[9]=std::min(1.f,float(s.combat.nearby_enemies.size())/8);
    x[10]=std::min(1.f,float(v.attackers_count())/8);x[11]=std::min(1.f,float(std::uint32_t(p.ms-actor.moved))/60000);
    x[12]=std::min(1.f,float(std::uint32_t(p.ms-actor.born))/900000);x[13]=std::min(1.f,float(actor.episode.deaths)/5);
    x[14]=float(v.lowest_equipped_durability_pct())/100;x[15]=v.can_hearth();
    x[16]=std::clamp((s.position.x+6200)/1000,-2.f,2.f);x[17]=std::clamp(s.position.y/1000,-2.f,2.f);x[18]=std::clamp(s.position.z/1000,-2.f,2.f);
    float goal_distance=2001;
    for(auto const& goal:actor.goals)
        for(auto const& q:s.quest_log.quests)if(q.quest_id==goal.quest&&q.state==0)
            goal_distance=std::min(goal_distance,std::hypot(goal.x-s.position.x,goal.y-s.position.y));
    x[19]=goal_distance<=2000?std::min(4.f,goal_distance/500):0;
    x[20]=goal_distance<=2000;x[21]=s.quest_log.current_objective_tool.valid;x[22]=float(p.potential);
    x[23]=std::min(1.f,float(s.combat.nearby_friends.size())/100);
    x[24]=std::min(2.f,float(std::log1p(std::max(0,s.inventory.gold))/std::log(10000.)));
    x[25]=float(std::min(2.,actor.episode.stagnant/task.stagnation));x[26]=float(std::min(2.,actor.episode.observed/task.maximum));
    x[27]=task.required.empty()?1:float(std::count_if(task.required.begin(),task.required.end(),[&](auto id){return actor.episode.rewarded.contains(id); }))/task.required.size();
    x[31]=std::min(1.f,float(s.consumables.health_potion_count)/10);
    x[33]=Fraction(s.vendor_visit.estimated_repair_cost,std::max(1,s.inventory.gold));x[35]=std::min(1.f,float(s.gossip.gossip_options.size())/8);
    std::unordered_set<uint32> food,water;
    for(auto const& c:actor.affordances)
    {
        if(c.kind==Kind::QuestItem)
        {
            x[28]+=1.f/12;
            if(c.operation&1&&food.insert(c.item).second)x[29]+=float(v.item_count(c.item))/20;
            if(c.operation&2&&water.insert(c.item).second)x[30]+=float(v.item_count(c.item))/20;
        }
        if(c.kind==Kind::Train)x[32]+=1.f/8;
        if(c.kind==Kind::Loot)x[34]+=1.f/8;
    }
    for(auto index:{28,29,30,32,34})x[index]=std::min(1.f,x[index]);
    x[45]=actor.pending.active?std::min(1.f,float(std::uint32_t(p.ms-actor.pending.before.ms))/60000):0;
    ApplyMemory(x,actor);return x;
}
void Finish(std::uint64_t guid,Actor& actor,Frame& frame,bool terminal,bool truncated)
{
    auto& p=actor.pending;if(!p.active)return;
    if (frame.progress.ms==p.before.ms) {p.active=false;return;}
    auto reward=Outcome(p.before,frame.progress,terminal,actor.xp_reference,task.reward);
    double refusal=-task.reward.refusal*std::min<std::uint32_t>(p.refusals,25);
    double completion=actor.episode.terminal=="completed_task"?task.reward.completion:0;
    double failure=actor.episode.terminal.starts_with("failed_")?-task.reward.failure:0;
    reward.total+=refusal+completion+failure;
    actor.last_kind=int(p.action.kind);actor.last_reward=reward.total;actor.last_refusals=p.refusals;
    actor.last_move=std::hypot(frame.progress.x-p.before.x,frame.progress.y-p.before.y);
    actor.hp_delta=frame.progress.hp-p.before.hp;actor.mana_delta=frame.progress.mana-p.before.mana;
    bool move=p.action.kind==Kind::QuestGoal||p.action.kind==Kind::Approach||p.action.kind==Kind::LocalMove||p.action.kind==Kind::Retreat;
    actor.last_goal_progress=move?std::hypot(p.action.x-p.before.x,p.action.y-p.before.y)-std::hypot(p.action.x-frame.progress.x,p.action.y-frame.progress.y):0;
    if(move&&p.refusals&&actor.last_move<1)
    {
        if(actor.goal_failures.size()>=64)actor.goal_failures.erase(actor.goal_failures.begin());
        actor.goal_failures[GoalKey(p.action)]=std::min(5.,actor.goal_failures[GoalKey(p.action)]+1);
    }
    ApplyMemory(frame.state,actor);
    rapidjson::StringBuffer buffer;Writer w(buffer);w.StartObject();
    Text(w,"checkpoint_sha256",model->checkpoint);w.Key("decision_id");w.Uint64(p.id);w.Key("segment");w.Uint64(p.segment);
    w.Key("previous_source_snapshot");w.Uint64(p.before.version);w.Key("seconds");w.Double(reward.seconds);
    w.Key("reward");w.Double(reward.total);w.Key("discount");w.Double(reward.discount);
    w.Key("terminated");w.Bool(terminal);w.Key("truncated");w.Bool(truncated);
    w.Key("next_value");w.Double(terminal?0:model->critic.Score(frame.state));Array(w,"next_state",frame.state);
    w.Key("reward_terms");w.StartObject();
    for (auto [key,value]:{std::pair{"xp",reward.xp},std::pair{"quest",reward.quest},std::pair{"death",reward.death},
         std::pair{"time",reward.time},std::pair{"stall",reward.stall},std::pair{"potential",reward.potential},std::pair{"refusal",refusal},std::pair{"completion",completion},std::pair{"failure",failure}})
    {w.Key(key);w.Double(value);}w.EndObject();
    w.Key("xp_before");w.Uint64(p.before.xp);w.Key("xp_after");w.Uint64(frame.progress.xp);
    w.Key("quests_before");w.Uint64(p.before.quests);w.Key("quests_after");w.Uint64(frame.progress.quests);
    w.Key("deaths_before");w.Uint64(p.before.deaths);w.Key("deaths_after");w.Uint64(frame.progress.deaths);
    Text(w,"episode_end_reason",actor.episode.terminal);Memory(w,actor);
    w.EndObject();TrainingCapture::PolicyEvent(guid,frame.progress.version,"leveling_transition",buffer.GetString());
    p.active=false;if(terminal||truncated)++actor.segment;
}
std::vector<Choice> Menu(BotSnapshot const& s,BotAI& ai,State const& state,std::span<Choice const> affordances,std::span<KnownGoal const> goals,Actor const& actor)
{
    BotSnapshotView v(s);std::vector<Choice> menu;
    auto add=[&](Kind kind,ObjectGuid target={},float x=0,float y=0,float z=0,std::uint32_t quest=0,std::uint32_t item=0,std::uint32_t spell=0)->Choice*
    {
        if(menu.size()>=48)return nullptr;
        Choice c;c.kind=kind;c.target=target;c.x=x;c.y=y;c.z=z;c.quest=quest;c.item=item;c.spell=spell;
        std::copy(state.begin(),state.end(),c.features.begin());c.features[StateWidth+std::size_t(kind)]=1;
        c.features[68]=std::log1p(std::hypot(x-s.position.x,y-s.position.y)/10)/std::log(5.f);c.features[69]=std::clamp((z-s.position.z)/10,-2.f,2.f);
        c.features[73]=quest!=0;
        if(std::find(task.required.begin(),task.required.end(),quest)!=task.required.end())c.features[73]=2;
        for(auto const& q:s.quest_log.quests)if(q.quest_id==quest)c.features[74]=std::min(5.f,float(q.xp_reward)/400);
        c.features[75]=1;
        for(auto const& q:s.quest_log.quests)if(q.quest_id==quest)
        {
            double remaining=0;for(auto const& o:q.objectives)if(o.amount>0)remaining+=std::clamp(double(o.amount-o.progress)/o.amount,0.,1.);
            c.features[83]=q.objectives.empty()?float(q.state==0):float(remaining/q.objectives.size());
        }
        menu.push_back(c);return &menu.back();
    };
    auto own=[&](Kind kind){return add(kind,s.guid,s.position.x,s.position.y,s.position.z);};
    own(Kind::Baseline);
    if(v.is_casting())return menu;
    for(auto const& option:affordances)
    {
        if((option.kind==Kind::Cast||option.kind==Kind::Heal)&&!v.is_ready(option.spell))continue;
        if(ai.move_refused_recently(option.x,option.y,option.z,s.published_at_ms)&&(option.kind==Kind::Approach))continue;
        auto c=add(option.kind,option.target,option.x,option.y,option.z,option.quest,option.item,option.spell);if(!c)break;
        c->operation=option.operation;c->bag=option.bag;c->slot=option.slot;c->equip_slot=option.equip_slot;
        c->features[76]=Fraction(option.price,std::max(1,s.inventory.gold));c->features[77]=Fraction(option.mana_cost,std::max(1,s.vitals.power[POWER_MANA]));
        c->features[78]=Fraction(option.health_effect,s.vitals.max_hp);c->features[79]=Fraction(option.mana_effect,s.vitals.max_power[POWER_MANA]);
        c->features[80]=float(option.operation)/4;c->features[81]=option.target==s.guid?0:option.target.IsGameObject()?2:1;
    }
    if(!s.vitals.in_combat)
    {
        add(Kind::Jump,{},s.position.x+std::cos(s.position.o)*3,s.position.y+std::sin(s.position.o)*3,s.position.z);
        if(state[0]<.95f||state[1]<.9f)own(Kind::Rest);
        if(v.can_hearth())own(Kind::Hearth);
    }
    auto position=[&](ObjectGuid id,float& x,float& y,float& z)
    {
        for(auto const& u:s.combat.nearby_friends)if(u.guid==id){x=u.x;y=u.y;z=u.z;return true;}
        for(auto const& o:s.world_objects.nearby_objects)if(o.guid==id){x=o.x;y=o.y;z=o.z;return true;}
        return false;
    };
    if(!s.vitals.in_combat)
    {
        for(auto const& tin:s.quest_discovery.quest_turnins)
        {
            float x,y,z;if(!position(tin.giver,x,y,z))continue;
            float d=std::hypot(std::hypot(x-s.position.x,y-s.position.y),z-s.position.z);
            if(!ai.move_refused_recently(x,y,z,s.published_at_ms))add(d<=4.5?Kind::Turnin:Kind::Approach,tin.giver,x,y,z,tin.quest_id);
        }
        std::size_t offers=0;
        for(auto const& off:s.quest_discovery.quest_offers)
        {
            float x,y,z;if(offers>=4||!position(off.giver,x,y,z))continue;
            float d=std::hypot(std::hypot(x-s.position.x,y-s.position.y),z-s.position.z);
            if(ai.move_refused_recently(x,y,z,s.published_at_ms))continue;
            add(d<=4.5?Kind::Accept:Kind::Approach,off.giver,x,y,z,off.quest_id);++offers;
        }
    }
    std::size_t enemies=0;
    for(auto const& u:s.combat.nearby_enemies)
    {
        if(enemies>=6||u.hp<=0||u.max_hp<=0||u.untargetable||u.is_player||u.no_xp_kill||!u.in_los||(u.tapped&&!u.tapped_by_us))continue;
        if(std::hypot(u.x-s.position.x,u.y-s.position.y)>40||ai.start_attack_recently_refused(u.guid.GetCounter(),s.published_at_ms))continue;
        Choice* c=nullptr;
        if(!s.vitals.in_combat)c=add(Kind::Attack,u.guid,u.x,u.y,u.z);
        if(c)
        {
            c->features[70]=Fraction(u.hp,u.max_hp);c->features[71]=std::clamp(float(u.max_hp)/std::max(1,s.vitals.max_hp),0.f,5.f);
            c->features[72]=std::clamp((float(u.level)-s.identity.level)/5,-2.f,2.f);
            for(auto const& a:s.quest_log.actionable_objectives)if(a.source_guid==u.guid)c->features[73]=1;
            ++enemies;
        }
    }
    if(s.vitals.in_combat)
    {
        float ax=0,ay=0;std::size_t n=0;
        for(auto const& u:s.combat.nearby_enemies)if(u.hp>0&&std::hypot(u.x-s.position.x,u.y-s.position.y)<20){ax+=u.x;ay+=u.y;++n;}
        if(n)
        {
            float dx=s.position.x-ax/n,dy=s.position.y-ay/n;float d=std::max(.1f,std::hypot(dx,dy));
            add(Kind::Retreat,{},s.position.x+dx/d*12,s.position.y+dy/d*12,s.position.z);
        }
    }
    else
    {
        for(auto const& goal:goals)
            for(auto const& q:s.quest_log.quests)
                if(q.quest_id==goal.quest&&q.state==0&&!ai.move_refused_recently(goal.x,goal.y,goal.z,s.published_at_ms))
                    add(Kind::QuestGoal,{},goal.x,goal.y,goal.z,goal.quest);
        std::size_t goals=0;
        for(auto const& q:s.quest_log.quests)
        {
            if(q.state==1&&q.ender_resolved&&q.ender_map_id==s.position.map_id&&goals<3&&
               !ai.move_refused_recently(q.ender_x,q.ender_y,q.ender_z,s.published_at_ms))
            {if(auto c=add(Kind::QuestGoal,{},q.ender_x,q.ender_y,q.ender_z,q.quest_id))c->features[74]=std::min(5.f,float(q.xp_reward)/400);++goals;}
        }
        for(auto const& a:s.quest_log.actionable_objectives)
        {
            if(a.type==2)
                for(auto const& o:s.world_objects.nearby_objects)if(o.guid==a.source_guid)
                    add(std::hypot(std::hypot(o.x-s.position.x,o.y-s.position.y),o.z-s.position.z)<=4.5f?Kind::Interact:Kind::Approach,o.guid,o.x,o.y,o.z,a.quest_id);
            if(a.type==3)
                for(auto const& u:s.combat.nearby_friends)if(u.guid==a.source_guid)
                    add(std::hypot(std::hypot(u.x-s.position.x,u.y-s.position.y),u.z-s.position.z)<=4.5f?Kind::Interact:Kind::Approach,u.guid,u.x,u.y,u.z,a.quest_id);
        }
        for(std::size_t i=0;i<8;++i)
        {
            float angle=float(i)*.7853981634f;float x=s.position.x+std::cos(angle)*8,y=s.position.y+std::sin(angle)*8;
            if(!ai.move_refused_recently(x,y,s.position.z,s.published_at_ms))add(Kind::LocalMove,{},x,y,s.position.z);
        }
    }
    for(auto& c:menu)
        if(auto it=actor.goal_failures.find(GoalKey(c));it!=actor.goal_failures.end())c.features[82]=float(std::min(1.,it->second/5));
    return menu;
}
bool Execute(Choice const& c,BotSnapshot const&,BotIntentEmitter& emit)
{
    if(c.kind!=Kind::Rest)emit.stand();
    switch(c.kind)
    {
        case Kind::Baseline:return true;
        case Kind::Attack:return emit.start_attack(c.target);
        case Kind::Accept:return emit.accept_quest(c.target,c.quest);
        case Kind::Turnin:return emit.complete_quest(c.target,c.quest,0xFF);
        case Kind::Loot:return emit.emit(LootIntent{c.target});
        case Kind::QuestGoal:case Kind::Approach:case Kind::LocalMove:return emit.move_to(c.x,c.y,c.z,true);
        case Kind::Retreat:emit.stop_attack();return emit.move_to(c.x,c.y,c.z,true);
        case Kind::Heal:case Kind::Cast:return emit.cast(c.spell,c.target);
        case Kind::Hearth:emit.stop_movement();return emit.hearth();
        case Kind::Rest:emit.stop_movement();return emit.sit();
        case Kind::Jump:return emit.jump(3.f);
        case Kind::QuestItem:return emit.use_item_by_entry(c.item,c.target);
        case Kind::Interact:
            if(c.operation==3)return emit.gossip_select(c.target,c.slot);
            if(c.target.IsGameObject())return emit.use_game_object(c.target);
            return emit.interact_with_npc(c.target);
        case Kind::Train:return emit.trainer_buy_spell(c.target,c.spell);
        case Kind::Repair:return emit.repair_all(c.target,false);
        case Kind::Vendor:return c.operation==1?emit.vendor_buy_by_entry(c.target,c.item,1):emit.vendor_sell_trash(c.target);
        case Kind::Equip:return emit.equip_item(c.bag,c.slot,c.equip_slot);
    }
    return false;
}
bool Hold(Choice const& c,BotSnapshot const& s)
{
    if(c.kind==Kind::Baseline)return false;
    if(s.vitals.in_combat&&(c.kind==Kind::Attack||c.kind==Kind::Cast||c.kind==Kind::Heal))return false;
    return true;
}
bool Completed(Choice const& c,BotSnapshot const& s,Frame const& frame,Actor const& actor)
{
    switch(c.kind)
    {
        case Kind::Attack:case Kind::Cast:
            for(auto const& enemy:s.combat.nearby_enemies)if(enemy.guid==c.target)return enemy.hp<=0;
            return !s.vitals.in_combat;
        case Kind::Loot:
            return std::none_of(frame.affordances.begin(),frame.affordances.end(),[&](auto const& v){return v.target==c.target&&(v.kind==Kind::Loot||v.kind==Kind::Approach);});
        case Kind::Accept:
            return std::any_of(s.quest_log.quests.begin(),s.quest_log.quests.end(),[&](auto const& q){return q.quest_id==c.quest;});
        case Kind::Turnin:return actor.episode.rewarded.contains(c.quest);
        case Kind::Equip:return c.equip_slot<19&&s.inventory.equipped[c.equip_slot].entry==c.item;
        default:return false;
    }
}
}

bool Controls(std::uint64_t guid) {return bool(model)&&guids.contains(guid)&&IsTrainingPlayerGuid(guid);}
bool Participant(std::uint64_t guid) {return bool(model)&&participants.contains(guid)&&IsTrainingPlayerGuid(guid);}
std::string Checkpoint(std::uint64_t guid) {return Controls(guid)?model->checkpoint:std::string{};}
std::uint64_t Epoch(std::uint64_t guid) {return Controls(guid)?model->epoch:0;}
std::uint64_t XPRewardCap()
{ std::uint64_t cap=0;for(std::uint32_t level=1;level<task.goal;++level)cap+=sObjectMgr->GetXPForLevel(level);return cap; }
void Initialize()
{
    if(!sConfigMgr->GetBoolDefault("Playerbot.Training.Leveling.Enable",false,true))return;
    try
    {
        guids=ParseTrainingPlayerGuids(sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.PlayerGuids","",true));
        evaluation=sConfigMgr->GetBoolDefault("Playerbot.Training.Leveling.Evaluation",false,true);
        if(guids.empty()||guids.size()>100)throw std::runtime_error("Leveling policy requires 1..100 configured training players");
        for(auto guid:guids)if(!IsTrainingPlayerGuid(guid))throw std::runtime_error("Leveling controller GUID is not a training player");
        auto path=sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.ModelFile","",true);
        auto expected=sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.CheckpointSHA256","",true);
        auto artifact=sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.ArtifactSHA256","",true);
        if(std::filesystem::file_size(path)>2*1024*1024)throw std::runtime_error("Leveling artifact exceeds 2 MiB");
        std::ifstream file(path,std::ios::binary);if(!file)throw std::runtime_error("Cannot open leveling policy");
        std::string text((std::istreambuf_iterator<char>(file)),{}),actual;constexpr char hex[]="0123456789abcdef";
        for(auto byte:Trinity::Crypto::SHA256::GetDigestOf(text)){actual+=hex[byte>>4];actual+=hex[byte&15];}
        if(artifact.size()!=64||actual!=artifact)throw std::runtime_error("Leveling artifact SHA256 mismatch");
        model=std::make_shared<Model const>(Model::Parse(text,expected));
        rapidjson::Document document;document.Parse(text.c_str());task=Task::Parse(Field(document,"task_spec"));
        auto seed_path=sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.EpisodeStateFile","",true);
        auto seed_hash=sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.EpisodeStateSHA256","",true);
        if(std::filesystem::file_size(seed_path)>4*1024*1024)throw std::runtime_error("Episode seed exceeds 4 MiB");
        std::ifstream seed_file(seed_path,std::ios::binary);if(!seed_file)throw std::runtime_error("Missing episode state file");
        std::string seed_text((std::istreambuf_iterator<char>(seed_file)),{}),actual_seed;
        for(auto byte:Trinity::Crypto::SHA256::GetDigestOf(seed_text)){actual_seed+=hex[byte>>4];actual_seed+=hex[byte&15];}
        if(actual_seed!=seed_hash)throw std::runtime_error("Episode seed checksum mismatch");
        episode_seeds.Parse(seed_text.c_str());
        if(episode_seeds.HasParseError()||std::string(Field(episode_seeds,"task_sha256").GetString())!=Field(document,"task_sha256").GetString())
            throw std::runtime_error("Episode state task mismatch");
        auto const& seeds=Field(episode_seeds,"participants");if(!seeds.IsObject()||seeds.MemberCount()>100)throw std::runtime_error("Invalid episode participants");
        participants.clear();for(auto it=seeds.MemberBegin();it!=seeds.MemberEnd();++it)
        {
            auto id=std::stoull(it->name.GetString());if(!IsTrainingPlayerGuid(id))throw std::runtime_error("Episode participant is not a training player");participants.insert(id);
        }
        for(auto id:guids)if(!participants.contains(id))throw std::runtime_error("Controller lacks an episode participant seed");
        TC_LOG_INFO("playerbot.v2","[leveling_policy] checkpoint={} epoch={} step={} actors={} parity=passed authority=leveling_options goal={} death_terminal={} reconsider_completed={}",
                    model->checkpoint,model->epoch,model->step,guids.size(),task.goal,task.death_terminates,task.reconsider_completed);
    }
    catch(std::exception const& e){model.reset();TC_LOG_ERROR("playerbot.v2","[leveling_policy] disabled: {}",e.what());}
}
void Observe(Player const* player,BotSnapshot const& s,std::uint64_t xp,std::uint64_t deaths,double xp_reference)
{
    if(!Participant(s.bot_id))return;
    auto a=Get(s.bot_id);if(!a)return;
    bool refresh=false;{std::lock_guard lock(a->mutex);refresh=!a->affordances_at||std::uint32_t(s.published_at_ms-a->affordances_at)>=1000;}
    // World API and registry reads happen on the world thread, outside the policy lock.
    // Worker threads consume only immutable choices published with their exact snapshot.
    auto options=refresh?Affordances(player,s):std::vector<Choice>{};
    std::lock_guard lock(a->mutex);
    try
    {
        if(refresh){a->affordances=std::move(options);a->affordances_at=s.published_at_ms;}
        if(!a->born){a->born=s.published_at_ms;a->moved=s.published_at_ms;a->markx=s.position.x;a->marky=s.position.y;}
        if(std::hypot(s.position.x-a->markx,s.position.y-a->marky)>=3){a->moved=s.published_at_ms;a->markx=s.position.x;a->marky=s.position.y;}
        BotSnapshotView v(s);Frame frame;auto& p=frame.progress;
        if(!a->goals_at||std::uint32_t(s.published_at_ms-a->goals_at)>=10000)
        {
            a->goals.clear();a->goals_at=s.published_at_ms;
            // Exactly the quest-map data returned to a client for quests already in this player's log.
            // Never query unseen creature spawn locations or future quests for the learned menu.
            for(auto const& q:s.quest_log.quests)
            {
                if(q.state!=0||q.unachievable||q.unturnable||a->goals.size()>=8)continue;
                auto data=sObjectMgr->GetQuestPOIData(int32(q.quest_id));if(!data)continue;
                KnownGoal best;float best_distance=2000;
                for(auto const& blob:data->Blobs)
                {
                    if(blob.MapID!=int32(s.position.map_id)||blob.ObjectiveIndex<0||blob.Points.empty()||blob.PlayerConditionID)continue;
                    bool relevant=q.objectives.empty();
                    for(auto const& o:q.objectives)
                        if(o.progress<o.amount&&((o.id&&blob.QuestObjectiveID==int32(o.id))||blob.QuestObjectID==o.object_id||blob.ObjectiveIndex==o.storage_index))relevant=true;
                    if(!relevant)continue;
                    double x=0,y=0,z=0;for(auto const& point:blob.Points){x+=point.X;y+=point.Y;z+=point.Z;}
                    x/=blob.Points.size();y/=blob.Points.size();z/=blob.Points.size();
                    float distance=std::hypot(float(x)-s.position.x,float(y)-s.position.y);
                    if(distance>=best_distance)continue;
                    if(z==0&&player&&player->GetMap())
                    {
                        float ground=player->GetMap()->GetHeight(player->GetPhaseShift(),float(x),float(y),MAX_HEIGHT,true);
                        z=ground>INVALID_HEIGHT?ground:s.position.z;
                    }
                    best={q.quest_id,float(x),float(y),float(z)};best_distance=distance;
                }
                if(best.quest)a->goals.push_back(best);
            }
        }
        frame.goals=a->goals;frame.affordances=a->affordances;
        p.version=s.version;p.ms=s.published_at_ms;p.xp=xp;p.xp_cap=XPRewardCap();p.deaths=deaths;p.quests=0;p.potential=Potential(s);
        std::vector<std::uint32_t> rewarded;
        if(player)for(auto q:player->getRewardedQuests())
        {
            rewarded.push_back(q);if(std::find(task.eligible.begin(),task.eligible.end(),q)!=task.eligible.end())++p.quests;
        }
        p.x=s.position.x;p.y=s.position.y;p.z=s.position.z;p.hp=Fraction(s.vitals.hp,s.vitals.max_hp);
        p.mana=s.vitals.max_power[POWER_MANA]>0?Fraction(s.vitals.power[POWER_MANA],s.vitals.max_power[POWER_MANA]):1;
        p.alive=s.vitals.is_alive;p.combat=s.vitals.in_combat;a->xp_reference=xp_reference;
        frame.level=s.identity.level;frame.allowed=!v.in_group()&&!v.has_owner_character()&&!v.on_transport()&&!v.in_battleground()&&
            !v.is_in_instance()&&s.identity.race==3&&s.identity.cls==7&&s.position.map_id==0&&s.area.zone_id==1;
        std::vector<Credit> credit;
        for(auto const& q:s.quest_log.quests)
            for(std::size_t index=0;index<q.objectives.size();++index)
            {
                auto const& o=q.objectives[index];credit.push_back({std::to_string(q.quest_id)+":"+std::to_string(o.id)+":"+
                    std::to_string(o.type)+":"+std::to_string(o.object_id)+":"+std::to_string(index),o.progress});
            }
        a->episode.Advance(p,frame.level,credit,rewarded,task);
        frame.state=Context(s,*a,p);a->frames.push_back(frame);if(a->frames.size()>64)a->frames.pop_front();
        bool death=a->pending.active&&p.deaths>a->pending.before.deaths;
        bool episode_end=!a->episode.terminal.empty();
        bool death_boundary=task.death_terminates&&(!p.alive||death);
        bool recovered=!task.death_terminates&&death&&p.alive;
        if(a->pending.active&&(episode_end||death_boundary||recovered||!frame.allowed))Finish(s.bot_id,*a,frame,episode_end||death_boundary,!frame.allowed);
        if(!a->episode.terminal.empty())
        {
            a->scope_finished=true;
            if(!a->ending_logged)
            {
                rapidjson::StringBuffer buffer;Writer w(buffer);w.StartObject();Text(w,"reason",a->episode.terminal);
                w.Key("observed_seconds");w.Double(a->episode.observed);w.Key("no_progress_seconds");w.Double(a->episode.stagnant);
                Memory(w,*a);w.EndObject();TrainingCapture::PolicyEvent(s.bot_id,s.version,"leveling_episode_end",buffer.GetString());a->ending_logged=true;
            }
        }
    }
    catch(std::exception const& e){a->pending.active=false;TC_LOG_ERROR("playerbot.v2","[leveling_policy] actor={} progress rejected: {}",s.bot_id,e.what());}
}
bool Tick(BotSnapshot const& s,BotAI& ai,BotIntentEmitter& emit)
{
    if(!Participant(s.bot_id))return false;
    auto a=Get(s.bot_id);if(!a)return false;std::lock_guard lock(a->mutex);
    try
    {
        Frame const* source=nullptr;for(auto it=a->frames.rbegin();it!=a->frames.rend();++it)if(it->progress.version==s.version){source=&*it;break;}
        if(!source||!source->allowed||!source->progress.alive||std::uint32_t(GameTime::GetGameTimeMS()-s.published_at_ms)>=2000)return false;
        Frame current=*source;Frame* frame=&current;
        if(a->scope_finished){if(!s.vitals.in_combat){emit.stop_attack();emit.stop_movement();return true;}return false;}
        if(!Controls(s.bot_id)||!TrainingCapture::PolicyRecordingAvailable(s.bot_id))return false;
        BotSnapshotView v(s);
        if(a->pending.active&&!ReadyForDecision(std::uint32_t(s.published_at_ms-a->pending.before.ms),v.is_casting(),
            Completed(a->pending.action,s,*frame,*a),task.reconsider_completed))return Hold(a->pending.action,s);
        if(a->pending.active)Finish(s.bot_id,*a,*frame,false,false);
        if(v.is_casting())return false;
        auto menu=Menu(s,ai,frame->state,frame->affordances,frame->goals,*a);
        auto started=std::chrono::steady_clock::now();
        std::vector<float> scores;for(auto const& c:menu)scores.push_back(model->actor.Score(c.features));
        auto sampled=Select(scores,std::generate_canonical<double,53>(a->random));
        if(evaluation)
        {
            sampled.index=std::size_t(std::max_element(scores.begin(),scores.end())-scores.begin());
            std::fill(sampled.probabilities.begin(),sampled.probabilities.end(),0);sampled.probabilities[sampled.index]=1;
        }
        auto const& chosen=menu[sampled.index];
        double value=model->critic.Score(frame->state),logp=std::log(std::max(1e-30,sampled.probabilities[sampled.index]));
        auto inference=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-started).count();
        std::uint64_t id=++a->sequence;
        rapidjson::StringBuffer buffer;Writer w(buffer);w.StartObject();Text(w,"checkpoint_sha256",model->checkpoint);
        Text(w,"mode",evaluation?"evaluation":"training");
        w.Key("inference_us");w.Double(inference);
        w.Key("epoch");w.Uint64(model->epoch);w.Key("decision_id");w.Uint64(id);w.Key("segment");w.Uint64(a->segment);
        Array(w,"state",frame->state);w.Key("selected");w.Uint64(sampled.index);w.Key("log_probability");w.Double(logp);w.Key("value");w.Double(value);
        w.Key("candidates");w.StartArray();
        for(std::size_t i=0;i<menu.size();++i)
        {
            auto const& c=menu[i];w.StartObject();Text(w,"kind",std::string(Kinds[std::size_t(c.kind)]));Text(w,"target",c.target.ToString());
            w.Key("quest_id");w.Uint(c.quest);w.Key("item");w.Uint(c.item);w.Key("spell");w.Uint(c.spell);
            w.Key("operation");w.Uint(c.operation);w.Key("bag");w.Uint(c.bag);w.Key("slot");w.Uint(c.slot);w.Key("equip_slot");w.Uint(c.equip_slot);
            w.Key("x");w.Double(c.x);w.Key("y");w.Double(c.y);w.Key("z");w.Double(c.z);Array(w,"features",c.features);
            w.Key("logit");w.Double(scores[i]);w.Key("probability");w.Double(sampled.probabilities[i]);w.EndObject();
        }
        w.EndArray();w.EndObject();
        if(!TrainingCapture::PolicyEvent(s.bot_id,s.version,"leveling_decision",buffer.GetString()))return false;
        a->pending={true,id,a->segment,frame->progress,frame->state,value,logp,chosen,0};
        if(!Execute(chosen,s,emit))++a->pending.refusals;
        ai.set_last_rule_fired("learned:leveling_option");return chosen.kind!=Kind::Baseline;
    }
    catch(std::exception const& e){a->pending.active=false;TC_LOG_ERROR("playerbot.v2","[leveling_policy] actor={} decision rejected: {}",s.bot_id,e.what());return false;}
}
void Executed(std::uint64_t guid,std::uint64_t source,std::uint8_t result)
{
    if(!Controls(guid)||result==0)return;auto a=Get(guid);if(!a)return;std::lock_guard lock(a->mutex);
    if(a->pending.active&&source>=a->pending.before.version)++a->pending.refusals;
}
void Logout(std::uint64_t guid)
{
    if(!Participant(guid))return;
    auto a=Get(guid);if(a)
    {
        std::lock_guard lock(a->mutex);if(!a->frames.empty())try
        {
            // A dead state's value is never used as an invented recovery bootstrap at a recording cut.
            if(!task.death_terminates&&!a->frames.back().progress.alive)a->pending.active=false;
            else Finish(guid,*a,a->frames.back(),false,true);
        }catch(...){a->pending.active=false;}
    }
    std::lock_guard lock(actors_mutex);actors.erase(guid);
}
}
