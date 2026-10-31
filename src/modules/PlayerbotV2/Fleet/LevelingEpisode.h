#pragma once
#include "LevelingMath.h"
#include <unordered_map>
#include <unordered_set>

namespace Playerbot::V2::Fleet::Leveling
{
inline double EpisodeNumber(rapidjson::Value const& v)
{ if(!v.IsNumber()||!std::isfinite(v.GetDouble())||std::abs(v.GetDouble())>10000000)throw std::runtime_error("Invalid episode number");return v.GetDouble(); }
struct Task
{
    std::string id;std::uint32_t goal=6;double maximum=5400,stagnation=900;
    bool death_terminates=true,reconsider_completed=false;
    std::vector<std::uint32_t> required,eligible;RewardSpec reward;
    static Task Parse(rapidjson::Value const& value)
    {
        Task t;auto const& id=Field(value,"id");if(!id.IsString())throw std::runtime_error("Missing leveling task identity");t.id=id.GetString();
        t.goal=std::uint32_t(EpisodeNumber(Field(value,"goal_level")));t.maximum=EpisodeNumber(Field(value,"max_seconds"));t.stagnation=EpisodeNumber(Field(value,"no_progress_seconds"));
        if(t.goal<1||t.goal>10||t.maximum<=0||t.stagnation<=0)throw std::runtime_error("Invalid leveling task bounds");
        for(auto [name,destination]:{std::pair{"death_terminates",&t.death_terminates},std::pair{"reconsider_completed_actions",&t.reconsider_completed}})
            if(value.HasMember(name)){if(!value[name].IsBool())throw std::runtime_error("Invalid task option");*destination=value[name].GetBool();}
        for(auto [name,destination]:{std::pair{"required_quests",&t.required},std::pair{"eligible_quests",&t.eligible}})
        {
            auto const& list=Field(value,name);if(!list.IsArray()||list.Size()>100)throw std::runtime_error("Invalid leveling task quests");
            for(auto const& q:list.GetArray()){if(!q.IsUint())throw std::runtime_error("Invalid quest id");destination->push_back(q.GetUint());}
        }
        auto const& r=Field(value,"reward");
        t.reward={EpisodeNumber(Field(r,"xp")),EpisodeNumber(Field(r,"quest")),EpisodeNumber(Field(r,"death")),EpisodeNumber(Field(r,"completion")),EpisodeNumber(Field(r,"failure")),
            EpisodeNumber(Field(r,"time")),EpisodeNumber(Field(r,"stall")),EpisodeNumber(Field(r,"potential")),EpisodeNumber(Field(r,"refusal")),EpisodeNumber(Field(r,"gamma_per_second"))};
        if(!(t.reward.gamma>0&&t.reward.gamma<=1)||t.reward.failure<=0)throw std::runtime_error("Invalid leveling reward specification");
        return t;
    }
};
struct Credit { std::string key;std::int32_t value=0; };
struct Episode
{
    double observed=0,stagnant=0;std::uint64_t last_xp=0,deaths=0;std::uint32_t previous_ms=0;
    bool initialized=false,has_frame=false;std::string terminal;
    std::unordered_map<std::string,std::int32_t> peaks;
    std::unordered_set<std::uint32_t> rewarded;
    std::uint64_t last_deaths=0;
    void Seed(rapidjson::Value const& value)
    {
        observed=EpisodeNumber(Field(value,"observed_seconds"));stagnant=EpisodeNumber(Field(value,"no_progress_seconds"));
        auto const& xp=Field(value,"last_xp");if(!xp.IsUint64())throw std::runtime_error("Invalid episode seed XP");last_xp=xp.GetUint64();
        auto const& credits=Field(value,"objective_peaks");if(!credits.IsObject())throw std::runtime_error("Invalid episode credit seed");
        for(auto it=credits.MemberBegin();it!=credits.MemberEnd();++it)peaks[it->name.GetString()]=std::int32_t(EpisodeNumber(it->value));
        for(auto const& q:Field(value,"rewarded_quests").GetArray())rewarded.insert(q.GetUint());
        if(value.HasMember("terminal_reason"))terminal=value["terminal_reason"].GetString();
        if(value.HasMember("deaths")){if(!value["deaths"].IsUint64())throw std::runtime_error("Invalid episode death count");deaths=value["deaths"].GetUint64();}
        initialized=true;
    }
    bool Advance(Progress const& p,std::uint32_t level,std::span<Credit const> credit,std::span<std::uint32_t const> rewards,Task const& task)
    {
        if(!terminal.empty())return false;
        double elapsed=has_frame?double(std::uint32_t(p.ms-previous_ms))/1000:0;
        if(elapsed>60)throw std::runtime_error("Unobserved episode clock gap");
        bool advanced=initialized&&p.xp>last_xp;
        if(has_frame&&p.xp<last_xp)throw std::runtime_error("Episode seed XP rewound");
        if(has_frame){if(p.deaths<last_deaths)throw std::runtime_error("Episode death count rewound");deaths+=p.deaths-last_deaths;}
        for(auto q:rewards)if(initialized&&!rewarded.contains(q))advanced=true;
        for(auto const& c:credit)
        {
            auto it=peaks.find(c.key);
            if(it==peaks.end()){if(has_frame&&c.value>0)advanced=true;peaks[c.key]=std::max(0,c.value);}
            else if(c.value>it->second){advanced=true;it->second=c.value;}
        }
        observed+=elapsed;stagnant=advanced?0:stagnant+elapsed;
        last_xp=p.xp;rewarded={rewards.begin(),rewards.end()};previous_ms=p.ms;last_deaths=p.deaths;has_frame=true;initialized=true;
        bool complete=level>=task.goal&&std::all_of(task.required.begin(),task.required.end(),[&](auto q){return rewarded.contains(q);});
        if(complete)terminal="completed_task";
        else if(stagnant>=task.stagnation)terminal="failed_no_progress";
        else if(observed>=task.maximum)terminal="failed_time_limit";
        return !terminal.empty();
    }
};
}

