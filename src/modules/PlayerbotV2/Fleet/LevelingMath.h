#pragma once
// Engine-independent inference and reward accounting, exercised by the same tests as the native caller.
#include <rapidjson/document.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace Playerbot::V2::Fleet::Leveling
{
constexpr std::array<std::string_view,20> Kinds{"baseline","attack","accept","turnin","loot","quest_goal","approach","local_move","rest","heal","retreat","hearth","jump","quest_item","interact","cast","train","repair","vendor","equip"};
constexpr std::array<std::string_view,84> Features{"health","mana","level","xp_fraction","in_combat","bag_space","quest_count","turnin_count","offer_count","visible_enemies","attackers","stationary_time","elapsed_time","deaths","durability","hearth_ready","x","y","z","goal_distance","has_goal","has_quest_tool","quest_potential","visible_friends","money","no_progress_time","episode_time","task_goal","usable_items","food_count","water_count","health_potions","trainer_offers","repair_cost","known_corpses","gossip_options","last_reward","recent_refusals","last_move_progress","last_goal_progress","previous_action_bit0","previous_action_bit1","previous_action_bit2","previous_action_bit3","previous_action_bit4","since_last_action","health_delta","mana_delta","action_baseline","action_attack","action_accept","action_turnin","action_loot","action_quest_goal","action_approach","action_local_move","action_rest","action_heal","action_retreat","action_hearth","action_jump","action_quest_item","action_interact","action_cast","action_train","action_repair","action_vendor","action_equip","distance","height","target_health","relative_health","level_difference","quest_relevant","quest_xp","line_of_sight","price_fraction","mana_cost_fraction","health_effect","mana_effect","operation","item_target_kind","goal_failures","objective_remaining"};
constexpr std::size_t StateWidth=48,ActionWidth=84,KindCount=20;
using State = std::array<float,StateWidth>;
using ActionFeatures = std::array<float,ActionWidth>;
enum class Kind : std::uint8_t { Baseline,Attack,Accept,Turnin,Loot,QuestGoal,Approach,LocalMove,Rest,Heal,Retreat,Hearth,Jump,QuestItem,Interact,Cast,Train,Repair,Vendor,Equip };

inline rapidjson::Value const& Field(rapidjson::Value const& value,char const* name)
{
    if (!value.IsObject() || !value.HasMember(name)) throw std::runtime_error(std::string("Missing leveling field: ")+name);
    return value[name];
}
inline float Number(rapidjson::Value const& value)
{
    if (!value.IsNumber() || !std::isfinite(value.GetDouble()) || std::fabs(value.GetDouble())>10000)
        throw std::runtime_error("Invalid leveling number");
    return static_cast<float>(value.GetDouble());
}
struct Dense
{
    struct Layer { std::size_t inputs=0,outputs=0;std::vector<float> weights,bias; };
    std::array<Layer,3> layers;
    float Score(std::span<float const> input) const
    {
        std::array<float,128> a{},b{};
        if (input.size()!=layers[0].inputs) throw std::runtime_error("Invalid leveling input width");
        for (std::size_t i=0;i<input.size();++i) { if (!std::isfinite(input[i])) throw std::runtime_error("Non-finite policy input");a[i]=input[i]; }
        for (std::size_t n=0;n<3;++n)
        {
            auto const& l=layers[n];
            for (std::size_t out=0;out<l.outputs;++out)
            {
                float sum=l.bias[out];for (std::size_t in=0;in<l.inputs;++in) sum+=l.weights[out*l.inputs+in]*a[in];
                b[out]=n<2?std::tanh(sum):sum;
            }
            a=b;
        }
        if (!std::isfinite(a[0])) throw std::runtime_error("Non-finite policy output");
        return a[0];
    }
    static Dense Parse(rapidjson::Value const& list,std::size_t inputs)
    {
        if (!list.IsArray() || list.Size()!=3) throw std::runtime_error("Invalid leveling layers");
        Dense out;
        for (std::size_t i=0;i<3;++i)
        {
            auto& l=out.layers[i];l.inputs=i==0?inputs:64;l.outputs=i==2?1:64;
            auto const& weights=Field(list[i],"weights");auto const& bias=Field(list[i],"bias");
            if (!weights.IsArray() || !bias.IsArray() || weights.Size()!=l.outputs || bias.Size()!=l.outputs)
                throw std::runtime_error("Invalid leveling layer shape");
            for (auto const& row:weights.GetArray())
            {
                if (!row.IsArray() || row.Size()!=l.inputs) throw std::runtime_error("Invalid leveling weight shape");
                for (auto const& x:row.GetArray()) l.weights.push_back(Number(x));
            }
            for (auto const& x:bias.GetArray()) l.bias.push_back(Number(x));
        }
        return out;
    }
};
struct Model
{
    Dense actor,critic;
    std::string checkpoint;
    std::uint64_t epoch=0,step=0;
    static Model Parse(std::string const& text,std::string const& expected)
    {
        rapidjson::Document d;d.Parse(text.c_str());
        if (d.HasParseError()) throw std::runtime_error("Invalid leveling JSON");
        auto const& schema=Field(d,"schema");
        if (!schema.IsString() || std::string_view(schema.GetString())!="leveling-options-v2") throw std::runtime_error("Wrong leveling schema");
        auto check_names=[](rapidjson::Value const& values,auto const& names,std::size_t size)
        {
            if (!values.IsArray() || values.Size()!=size) throw std::runtime_error("Invalid leveling feature names");
            for (std::size_t i=0;i<size;++i) if (!values[i].IsString() || std::string_view(values[i].GetString())!=names[i])
                throw std::runtime_error("Leveling feature order mismatch");
        };
        check_names(Field(d,"feature_names"),Features,ActionWidth);check_names(Field(d,"state_names"),Features,StateWidth);
        check_names(Field(d,"action_kinds"),Kinds,KindCount);
        Model m;auto const& hash=Field(d,"checkpoint_sha256");auto const& epoch=Field(d,"epoch");auto const& step=Field(d,"optimizer_step");
        if (!hash.IsString() || expected.size()!=64 || expected!=hash.GetString() ||
            !std::all_of(expected.begin(),expected.end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');}) ||
            !epoch.IsUint64() || !epoch.GetUint64() || !step.IsUint64()) throw std::runtime_error("Invalid leveling identity");
        m.checkpoint=expected;m.epoch=epoch.GetUint64();m.step=step.GetUint64();
        m.actor=Dense::Parse(Field(d,"actor"),ActionWidth);m.critic=Dense::Parse(Field(d,"critic"),StateWidth);
        for (auto [name,head]:{std::pair{"actor",&m.actor},std::pair{"critic",&m.critic}})
        {
            auto const& tests=Field(Field(d,"parity"),name);
            if (!tests.IsArray() || tests.Size()<2 || tests.Size()>64) throw std::runtime_error("Missing leveling parity");
            for (auto const& test:tests.GetArray())
            {
                auto const& inputs=Field(test,"features");std::vector<float> x;
                if (!inputs.IsArray()) throw std::runtime_error("Invalid parity vector");
                for (auto const& v:inputs.GetArray()) x.push_back(Number(v));
                if (std::fabs(head->Score(x)-Number(Field(test,"score")))>1e-4) throw std::runtime_error("Leveling inference parity failure");
            }
        }
        return m;
    }
};
struct Sample { std::size_t index=0;std::vector<double> probabilities; };
inline Sample Select(std::span<float const> logits,double uniform)
{
    if (logits.empty() || logits.size()>48 || !(uniform>=0&&uniform<1)) throw std::runtime_error("Invalid leveling action menu");
    float max=*std::max_element(logits.begin(),logits.end());Sample out;double total=0;
    for (float x:logits) { if (!std::isfinite(x)) throw std::runtime_error("Invalid policy logit");out.probabilities.push_back(std::exp(double(x-max)));total+=out.probabilities.back(); }
    double cumulative=0;out.index=logits.size()-1;
    for (std::size_t i=0;i<logits.size();++i) { out.probabilities[i]/=total;cumulative+=out.probabilities[i];if (uniform<cumulative) {out.index=i;break;} }
    // Normalize remaining entries too, including when sampling stopped at an early candidate.
    for (std::size_t i=out.index+1;i<logits.size();++i) out.probabilities[i]/=total;
    return out;
}
struct Progress
{
    std::uint64_t version=0,xp=0,deaths=0,quests=0;
    std::uint32_t ms=0;
    double potential=0,x=0,y=0,z=0,hp=1,mana=1;
    bool alive=true,combat=false;
};
struct RewardSpec
{
    double xp=1,quest=1,death=1,completion=4,failure=2,time=.0005,stall=.001,potential=.1,refusal=.01,gamma=.9998;
};
struct Reward { double xp=0,quest=0,death=0,time=0,stall=0,potential=0,total=0,seconds=0,discount=0; };
inline Reward Outcome(Progress const& before,Progress const& after,bool terminal,double xp_reference,RewardSpec const& spec={})
{
    Reward r;r.seconds=double(std::uint32_t(after.ms-before.ms))/1000;
    if (!(r.seconds>0&&r.seconds<=1200) || !(xp_reference>0) || after.xp<before.xp || after.quests<before.quests || after.deaths<before.deaths)
        throw std::runtime_error("Non-monotonic leveling progress");
    r.discount=std::pow(spec.gamma,r.seconds);r.xp=spec.xp*double(after.xp-before.xp)/xp_reference;r.quest=spec.quest*double(after.quests-before.quests);
    r.death=-spec.death*double(after.deaths-before.deaths);r.time=-spec.time*r.seconds;
    double distance=std::hypot(after.x-before.x,after.y-before.y);
    if (!after.combat && after.hp>=.9 && after.mana>=.9 && distance<1 && after.xp==before.xp && after.quests==before.quests) r.stall=-spec.stall*r.seconds;
    r.potential=spec.potential*(r.discount*(terminal?0:after.potential)-before.potential);
    r.total=r.xp+r.quest+r.death+r.time+r.stall+r.potential;return r;
}
}
