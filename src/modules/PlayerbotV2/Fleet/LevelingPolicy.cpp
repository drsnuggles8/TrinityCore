#include "LevelingPolicy.h"
#include "LevelingMath.h"
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
#include "SHA256.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <deque>
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
struct Choice
{
    Kind kind=Kind::Baseline;ObjectGuid target;std::uint32_t quest=0,item=0,spell=0;
    float x=0,y=0,z=0;ActionFeatures features{};
};
struct Frame { Progress progress;State state{};bool allowed=true;std::uint8_t level=1; };
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
    explicit Actor(std::uint64_t guid):random(guid^0x504c55544f504f4cull) {}
};
std::mutex actors_mutex;
std::unordered_map<std::uint64_t,std::shared_ptr<Actor>> actors;
std::shared_ptr<Actor> Get(std::uint64_t guid)
{
    std::lock_guard lock(actors_mutex);
    auto it=actors.find(guid);if (it!=actors.end())return it->second;
    if (actors.size()>=128)return {};
    auto p=std::make_shared<Actor>(guid);actors.emplace(guid,p);return p;
}
void Text(Writer& w,char const* key,std::string const& value) { w.Key(key);w.String(value.c_str()); }
template<class Values>void Array(Writer& w,char const* key,Values const& values)
{ w.Key(key);w.StartArray();for(auto x:values)w.Double(x);w.EndArray(); }
float Fraction(double n,double d) { return d>0?float(std::clamp(n/d,0.,1.)):0; }
double Potential(BotSnapshot const& s)
{
    double total=0;
    for (auto const& q:s.quest_log.quests)
    {
        double progress=0;
        for (auto const& o:q.objectives) if(o.amount>0) progress+=std::clamp(double(o.progress)/o.amount,0.,1.);
        total+=q.objectives.empty()?(q.state==1?1.:0.):progress/q.objectives.size();
    }
    return std::clamp(total/25.,0.,1.);
}
State Context(BotSnapshot const& s,Actor const& actor,Progress const& p)
{
    BotSnapshotView v(s);State x{};
    x[0]=float(p.hp);x[1]=float(p.mana);x[2]=std::min(1.f,float(s.identity.level)/10);
    x[3]=Fraction(s.identity.xp,s.identity.xp_for_level);x[4]=s.vitals.in_combat;x[5]=std::min(1.f,float(s.bags.bag_free_slots)/16);
    x[6]=std::min(1.f,float(s.quest_log.quests.size())/25);x[7]=std::min(1.f,float(s.quest_discovery.quest_turnins.size())/10);
    x[8]=std::min(1.f,float(s.quest_discovery.quest_offers.size())/10);x[9]=std::min(1.f,float(s.combat.nearby_enemies.size())/8);
    x[10]=std::min(1.f,float(v.attackers_count())/8);x[11]=std::min(1.f,float(std::uint32_t(p.ms-actor.moved))/60000);
    x[12]=std::min(1.f,float(std::uint32_t(p.ms-actor.born))/900000);x[13]=std::min(1.f,float(p.deaths)/5);
    x[14]=float(v.lowest_equipped_durability_pct())/100;x[15]=v.can_hearth();
    x[16]=std::clamp((s.position.x+6200)/1000,-2.f,2.f);x[17]=std::clamp(s.position.y/1000,-2.f,2.f);x[18]=std::clamp(s.position.z/1000,-2.f,2.f);
    auto const& poi=s.quest_log.current_objective_poi;
    x[19]=poi.valid?std::min(2.f,std::hypot(poi.x-s.position.x,poi.y-s.position.y)/500):0;
    x[20]=poi.valid;x[21]=s.quest_log.current_objective_tool.valid;x[22]=float(p.potential);
    x[23]=std::min(1.f,float(s.combat.nearby_friends.size())/100);return x;
}
void Finish(std::uint64_t guid,Actor& actor,Frame const& frame,bool terminal,bool truncated)
{
    auto& p=actor.pending;if(!p.active)return;
    if (frame.progress.ms==p.before.ms) {p.active=false;return;}
    auto reward=Outcome(p.before,frame.progress,terminal,actor.xp_reference);
    double refusal=-.01*std::min<std::uint32_t>(p.refusals,25);reward.total+=refusal;
    rapidjson::StringBuffer buffer;Writer w(buffer);w.StartObject();
    Text(w,"checkpoint_sha256",model->checkpoint);w.Key("decision_id");w.Uint64(p.id);w.Key("segment");w.Uint64(p.segment);
    w.Key("previous_source_snapshot");w.Uint64(p.before.version);w.Key("seconds");w.Double(reward.seconds);
    w.Key("reward");w.Double(reward.total);w.Key("discount");w.Double(reward.discount);
    w.Key("terminated");w.Bool(terminal);w.Key("truncated");w.Bool(truncated);
    w.Key("next_value");w.Double(terminal?0:model->critic.Score(frame.state));Array(w,"next_state",frame.state);
    w.Key("reward_terms");w.StartObject();
    for (auto [key,value]:{std::pair{"xp",reward.xp},std::pair{"quest",reward.quest},std::pair{"death",reward.death},
         std::pair{"time",reward.time},std::pair{"stall",reward.stall},std::pair{"potential",reward.potential},std::pair{"refusal",refusal}})
    {w.Key(key);w.Double(value);}w.EndObject();
    w.Key("xp_before");w.Uint64(p.before.xp);w.Key("xp_after");w.Uint64(frame.progress.xp);
    w.Key("quests_before");w.Uint64(p.before.quests);w.Key("quests_after");w.Uint64(frame.progress.quests);
    w.Key("deaths_before");w.Uint64(p.before.deaths);w.Key("deaths_after");w.Uint64(frame.progress.deaths);
    w.EndObject();TrainingCapture::PolicyEvent(guid,frame.progress.version,"leveling_transition",buffer.GetString());
    p.active=false;if(terminal||truncated)++actor.segment;
}
std::vector<Choice> Menu(BotSnapshot const& s,BotAI& ai,State const& state,ObjectGuid corpse)
{
    BotSnapshotView v(s);std::vector<Choice> menu;
    auto add=[&](Kind kind,ObjectGuid target={},float x=0,float y=0,float z=0,std::uint32_t quest=0,std::uint32_t item=0,std::uint32_t spell=0)->Choice*
    {
        if(menu.size()>=32)return nullptr;
        Choice c;c.kind=kind;c.target=target;c.x=x;c.y=y;c.z=z;c.quest=quest;c.item=item;c.spell=spell;
        std::copy(state.begin(),state.end(),c.features.begin());c.features[24+std::size_t(kind)]=1;
        c.features[40]=std::min(2.f,std::hypot(x-s.position.x,y-s.position.y)/40);c.features[41]=std::clamp((z-s.position.z)/10,-2.f,2.f);
        c.features[47]=1;menu.push_back(c);return &menu.back();
    };
    auto own=[&](Kind kind){return add(kind,s.guid,s.position.x,s.position.y,s.position.z);};
    own(Kind::Baseline);
    if(v.is_casting())return menu;
    if(!s.vitals.in_combat)
    {
        own(Kind::Wait);
        if(state[0]<.95f||state[1]<.9f)own(Kind::Rest);
        if(v.can_hearth())own(Kind::Hearth);
    }
    if(state[0]<.85f&&state[1]>.15f&&v.knows_spell(331)&&v.is_ready(331))
        add(Kind::Heal,s.guid,s.position.x,s.position.y,s.position.z,0,0,331);
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
        else if(v.knows_spell(403)&&v.is_ready(403)&&state[1]>.1)c=add(Kind::Cast,u.guid,u.x,u.y,u.z,0,0,403);
        if(c)
        {
            c->features[42]=Fraction(u.hp,u.max_hp);c->features[43]=std::clamp(float(u.max_hp)/std::max(1,s.vitals.max_hp),0.f,5.f);
            c->features[44]=std::clamp((float(u.level)-s.identity.level)/5,-2.f,2.f);
            for(auto const& a:s.quest_log.actionable_objectives)if(a.source_guid==u.guid)c->features[45]=1;
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
        if(!corpse.IsEmpty())add(Kind::Loot,corpse,s.position.x,s.position.y,s.position.z);
        auto const& poi=s.quest_log.current_objective_poi;
        if(poi.valid&&poi.map_id==s.position.map_id&&!ai.move_refused_recently(poi.x,poi.y,poi.z,s.published_at_ms))
            add(Kind::QuestGoal,{},poi.x,poi.y,poi.z,s.quest_log.current_quest_id);
        std::size_t goals=0;
        for(auto const& q:s.quest_log.quests)
        {
            if(q.state==1&&q.ender_resolved&&q.ender_map_id==s.position.map_id&&goals<3&&
               !ai.move_refused_recently(q.ender_x,q.ender_y,q.ender_z,s.published_at_ms))
            {if(auto c=add(Kind::QuestGoal,{},q.ender_x,q.ender_y,q.ender_z,q.quest_id))c->features[46]=std::min(5.f,float(q.xp_reward)/400);++goals;}
            if(q.source_item_id&&v.item_count(q.source_item_id))
                add(Kind::QuestItem,s.guid,s.position.x,s.position.y,s.position.z,q.quest_id,q.source_item_id);
        }
        for(auto const& a:s.quest_log.actionable_objectives)
        {
            if(a.type==2)
                for(auto const& o:s.world_objects.nearby_objects)if(o.guid==a.source_guid)
                    add(std::hypot(o.x-s.position.x,o.y-s.position.y)<=4.5f?Kind::Interact:Kind::Approach,o.guid,o.x,o.y,o.z,a.quest_id);
            if(a.type==3)
                for(auto const& u:s.combat.nearby_friends)if(u.guid==a.source_guid)
                    add(std::hypot(u.x-s.position.x,u.y-s.position.y)<=4.5f?Kind::Interact:Kind::Approach,u.guid,u.x,u.y,u.z,a.quest_id);
        }
        for(std::size_t i=0;i<8;++i)
        {
            float angle=float(i)*.7853981634f;float x=s.position.x+std::cos(angle)*8,y=s.position.y+std::sin(angle)*8;
            if(!ai.move_refused_recently(x,y,s.position.z,s.published_at_ms))add(Kind::LocalMove,{},x,y,s.position.z);
        }
    }
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
        case Kind::Loot:return emit.loot(c.target);
        case Kind::QuestGoal:case Kind::Approach:case Kind::LocalMove:return emit.move_to(c.x,c.y,c.z,true);
        case Kind::Retreat:emit.stop_attack();return emit.move_to(c.x,c.y,c.z,true);
        case Kind::Heal:case Kind::Cast:return emit.cast(c.spell,c.target);
        case Kind::Hearth:emit.stop_movement();return emit.hearth();
        case Kind::Rest:emit.stop_movement();return emit.sit();
        case Kind::Wait:return emit.stop_movement();
        case Kind::QuestItem:return emit.use_item_by_entry(c.item,c.target);
        case Kind::Interact:
            if(c.target.IsGameObject())return emit.use_game_object(c.target);
            return emit.interact_with_npc(c.target);
    }
    return false;
}
bool Hold(Choice const& c,BotSnapshot const& s)
{
    if(c.kind==Kind::Baseline)return false;
    if(s.vitals.in_combat&&(c.kind==Kind::Attack||c.kind==Kind::Cast||c.kind==Kind::Heal))return false;
    return true;
}
}

bool Controls(std::uint64_t guid) {return bool(model)&&guids.contains(guid)&&IsTrainingPlayerGuid(guid);}
void Initialize()
{
    if(!sConfigMgr->GetBoolDefault("Playerbot.Training.Leveling.Enable",false,true))return;
    try
    {
        guids=ParseTrainingPlayerGuids(sConfigMgr->GetStringDefault("Playerbot.Training.Leveling.PlayerGuids","",true));
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
        TC_LOG_INFO("playerbot.v2","[leveling_policy] checkpoint={} epoch={} step={} actors={} parity=passed authority=leveling_options",
                    model->checkpoint,model->epoch,model->step,guids.size());
    }
    catch(std::exception const& e){model.reset();TC_LOG_ERROR("playerbot.v2","[leveling_policy] disabled: {}",e.what());}
}
void Observe(BotSnapshot const& s,std::uint64_t xp,std::uint64_t deaths,double xp_reference)
{
    if(!Controls(s.bot_id))return;
    auto a=Get(s.bot_id);if(!a)return;std::lock_guard lock(a->mutex);
    try
    {
        if(!a->born){a->born=s.published_at_ms;a->moved=s.published_at_ms;a->markx=s.position.x;a->marky=s.position.y;}
        if(std::hypot(s.position.x-a->markx,s.position.y-a->marky)>=3){a->moved=s.published_at_ms;a->markx=s.position.x;a->marky=s.position.y;}
        BotSnapshotView v(s);Frame frame;auto& p=frame.progress;
        p.version=s.version;p.ms=s.published_at_ms;p.xp=xp;p.deaths=deaths;p.quests=s.quest_log.completed_quest_count;p.potential=Potential(s);
        p.x=s.position.x;p.y=s.position.y;p.z=s.position.z;p.hp=Fraction(s.vitals.hp,s.vitals.max_hp);
        p.mana=s.vitals.max_power[POWER_MANA]>0?Fraction(s.vitals.power[POWER_MANA],s.vitals.max_power[POWER_MANA]):1;
        p.alive=s.vitals.is_alive;p.combat=s.vitals.in_combat;a->xp_reference=xp_reference;
        frame.level=s.identity.level;frame.allowed=!v.in_group()&&!v.has_owner_character()&&!v.on_transport()&&!v.in_battleground()&&
            !v.is_in_instance()&&s.identity.race==3&&s.identity.cls==7&&s.position.map_id==0&&s.area.zone_id==1;
        frame.state=Context(s,*a,p);a->frames.push_back(frame);if(a->frames.size()>64)a->frames.pop_front();
        bool death=a->pending.active&&p.deaths>a->pending.before.deaths;
        if(a->pending.active&&(!p.alive||death||frame.level>=6||!frame.allowed))Finish(s.bot_id,*a,frame,!p.alive||death||frame.level>=6,!frame.allowed);
        if(frame.level>=6)a->scope_finished=true;
    }
    catch(std::exception const& e){a->pending.active=false;TC_LOG_ERROR("playerbot.v2","[leveling_policy] actor={} progress rejected: {}",s.bot_id,e.what());}
}
bool Tick(BotSnapshot const& s,BotAI& ai,BotIntentEmitter& emit)
{
    if(!Controls(s.bot_id)||!TrainingCapture::PolicyRecordingAvailable(s.bot_id))return false;
    ObjectGuid corpse;Services::Registry().with_loot(s.bot_id,[&](auto const& pending){if(!pending.empty())corpse=pending.front();});
    auto a=Get(s.bot_id);if(!a)return false;std::lock_guard lock(a->mutex);
    try
    {
        Frame const* frame=nullptr;for(auto it=a->frames.rbegin();it!=a->frames.rend();++it)if(it->progress.version==s.version){frame=&*it;break;}
        if(!frame||!frame->allowed||!frame->progress.alive||std::uint32_t(GameTime::GetGameTimeMS()-s.published_at_ms)>=2000)return false;
        if(a->scope_finished){if(!s.vitals.in_combat){emit.stop_movement();return true;}return false;}
        BotSnapshotView v(s);
        if(a->pending.active&&(std::uint32_t(s.published_at_ms-a->pending.before.ms)<5000||v.is_casting()))return Hold(a->pending.action,s);
        if(a->pending.active)Finish(s.bot_id,*a,*frame,false,false);
        if(v.is_casting())return false;
        auto menu=Menu(s,ai,frame->state,corpse);std::vector<float> scores;for(auto const& c:menu)scores.push_back(model->actor.Score(c.features));
        auto sampled=Select(scores,std::generate_canonical<double,53>(a->random));auto const& choice=menu[sampled.index];
        double value=model->critic.Score(frame->state),logp=std::log(std::max(1e-30,sampled.probabilities[sampled.index]));
        std::uint64_t id=++a->sequence;
        rapidjson::StringBuffer buffer;Writer w(buffer);w.StartObject();Text(w,"checkpoint_sha256",model->checkpoint);
        w.Key("epoch");w.Uint64(model->epoch);w.Key("decision_id");w.Uint64(id);w.Key("segment");w.Uint64(a->segment);
        Array(w,"state",frame->state);w.Key("selected");w.Uint64(sampled.index);w.Key("log_probability");w.Double(logp);w.Key("value");w.Double(value);
        w.Key("candidates");w.StartArray();
        for(std::size_t i=0;i<menu.size();++i)
        {
            auto const& c=menu[i];w.StartObject();Text(w,"kind",std::string(Kinds[std::size_t(c.kind)]));Text(w,"target",c.target.ToString());
            w.Key("quest_id");w.Uint(c.quest);w.Key("item");w.Uint(c.item);w.Key("spell");w.Uint(c.spell);
            w.Key("x");w.Double(c.x);w.Key("y");w.Double(c.y);w.Key("z");w.Double(c.z);Array(w,"features",c.features);
            w.Key("logit");w.Double(scores[i]);w.Key("probability");w.Double(sampled.probabilities[i]);w.EndObject();
        }
        w.EndArray();w.EndObject();
        if(!TrainingCapture::PolicyEvent(s.bot_id,s.version,"leveling_decision",buffer.GetString()))return false;
        a->pending={true,id,a->segment,frame->progress,frame->state,value,logp,choice,0};
        if(!Execute(choice,s,emit))++a->pending.refusals;
        ai.set_last_rule_fired("learned:leveling_option");return choice.kind!=Kind::Baseline;
    }
    catch(std::exception const& e){a->pending.active=false;TC_LOG_ERROR("playerbot.v2","[leveling_policy] actor={} decision rejected: {}",s.bot_id,e.what());return false;}
}
void Executed(std::uint64_t guid,std::uint8_t result)
{
    if(!Controls(guid)||result==0)return;auto a=Get(guid);if(!a)return;std::lock_guard lock(a->mutex);if(a->pending.active)++a->pending.refusals;
}
void Logout(std::uint64_t guid)
{
    if(!Controls(guid))return;
    auto a=Get(guid);if(a)
    {
        std::lock_guard lock(a->mutex);if(!a->frames.empty())try{Finish(guid,*a,a->frames.back(),false,true);}catch(...){a->pending.active=false;}
    }
    std::lock_guard lock(actors_mutex);actors.erase(guid);
}
}
