#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/LevelingMath.h"
#include "../../src/modules/PlayerbotV2/Fleet/LevelingEpisode.h"
#include "../../src/server/game/Combat/DamageResolution.h"
#include <cstdlib>
#include <fstream>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace Playerbot::V2::Fleet::Leveling;
TEST_CASE("Damage measurement distinguishes mitigation output, overkill and net health changes", "[TrainingTelemetry]")
{
    auto hit=MeasureDamageResolution(30,100,70);
    REQUIRE(hit.effective==30);REQUIRE(hit.health_removed==30);REQUIRE(hit.overkill==0);
    hit=MeasureDamageResolution(50,20,0);
    REQUIRE(hit.effective==20);REQUIRE(hit.overkill==30);
    hit=MeasureDamageResolution(20,100,95); // A reactive heal can hide part of the damage in net health.
    REQUIRE(hit.effective==20);REQUIRE(hit.health_removed==5);
    hit=MeasureDamageResolution(20,100,120);
    REQUIRE(hit.health_removed==0);
}
static std::string SyntheticLevelingArtifact()
{
    rapidjson::StringBuffer buffer;rapidjson::Writer<rapidjson::StringBuffer> w(buffer);w.StartObject();
    w.Key("schema");w.String("leveling-options-v2");w.Key("checkpoint_sha256");w.String(std::string(64,'a').c_str());
    w.Key("epoch");w.Uint64(1);w.Key("optimizer_step");w.Uint64(0);
    for(auto [name,count]:{std::pair{"feature_names",int(ActionWidth)},std::pair{"state_names",int(StateWidth)}})
    {w.Key(name);w.StartArray();for(int i=0;i<count;++i)w.String(Features[i].data());w.EndArray();}
    w.Key("action_kinds");w.StartArray();for(auto kind:Kinds)w.String(kind.data());w.EndArray();
    for(auto [name,width]:{std::pair{"actor",int(ActionWidth)},std::pair{"critic",int(StateWidth)}})
    {
        w.Key(name);w.StartArray();
        for(int layer=0;layer<3;++layer)
        {
            int inputs=layer==0?width:64,outputs=layer==2?1:64;w.StartObject();w.Key("weights");w.StartArray();
            for(int i=0;i<outputs;++i){w.StartArray();for(int j=0;j<inputs;++j)w.Double(i==0&&j==0?1:0);w.EndArray();}
            w.EndArray();w.Key("bias");w.StartArray();for(int i=0;i<outputs;++i)w.Double(0);w.EndArray();w.EndObject();
        }
        w.EndArray();
    }
    w.Key("parity");w.StartObject();
    for(auto [name,width]:{std::pair{"actor",int(ActionWidth)},std::pair{"critic",int(StateWidth)}})
    {
        w.Key(name);w.StartArray();
        for(int test=0;test<2;++test)
        {
            w.StartObject();w.Key("features");w.StartArray();for(int i=0;i<width;++i)w.Double(i==0?test:0);w.EndArray();
            w.Key("score");w.Double(test?std::tanh(std::tanh(1.)):0);w.EndObject();
        }
        w.EndArray();
    }
    w.EndObject();w.EndObject();return buffer.GetString();
}
TEST_CASE("Leveling rewards count only native progress and charge time, death and stalls", "[LevelingMath]")
{
    Progress before;before.ms=1000;before.xp=900;before.quests=3;before.potential=.2;
    auto after=before;after.ms+=5000;after.xp+=400;after.quests+=1;after.deaths=1;after.potential=.4;
    auto r=Outcome(before,after,false,400);
    REQUIRE(r.xp==1);REQUIRE(r.quest==1);REQUIRE(r.death==-1);REQUIRE(r.time==-.0025);REQUIRE(r.stall==0);
    REQUIRE(r.total>0);REQUIRE(r.seconds==5);
    auto end=Outcome(before,after,true,400);REQUIRE(end.potential==Catch::Approx(-.02));
    after=before;after.ms+=5000;r=Outcome(before,after,false,400);REQUIRE(r.total<0);REQUIRE(r.stall<0);
    after.xp=before.xp-1;REQUIRE_THROWS(Outcome(before,after,false,400));
}
TEST_CASE("Episode failure survives recorder restarts and completion requires quest milestones", "[LevelingMath]")
{
    Task task;task.required={233};Progress p;p.ms=1000;p.xp=100;
    Episode e;std::vector<Credit> credits;std::vector<uint32_t> rewards;
    REQUIRE_FALSE(e.Advance(p,6,credits,rewards,task));
    for(int i=0;i<15;++i){p.ms+=60000;e.Advance(p,6,credits,rewards,task);}
    REQUIRE(e.terminal=="failed_no_progress");
    rapidjson::Document seed;seed.Parse(R"({"observed_seconds":850,"no_progress_seconds":850,"last_xp":100,"objective_peaks":{},"rewarded_quests":[]})");
    Episode restarted;restarted.Seed(seed);p.ms=10;restarted.Advance(p,6,credits,rewards,task);
    p.ms+=50000;REQUIRE(restarted.Advance(p,6,credits,rewards,task));REQUIRE(restarted.terminal=="failed_no_progress");
    Episode success;rewards={233};REQUIRE(success.Advance(p,6,credits,rewards,task));REQUIRE(success.terminal=="completed_task");
}
TEST_CASE("Task reward parsing retains time discount precision", "[LevelingMath]")
{
    rapidjson::Document d;d.Parse(R"({"id":"test","goal_level":6,"max_seconds":5400,"no_progress_seconds":900,"required_quests":[94375],"eligible_quests":[94375],"reward":{"xp":1,"quest":1,"death":1,"completion":4,"failure":2,"time":0.0005,"stall":0.001,"potential":0.1,"refusal":0.01,"gamma_per_second":0.9998}})");
    auto task=Task::Parse(d);REQUIRE(task.reward.gamma==.9998);REQUIRE(task.required.front()==94375);
    Progress a,b;a.ms=1000;b.ms=6000;auto r=Outcome(a,b,false,400,task.reward);
    REQUIRE(r.discount==std::pow(.9998,5));
}
TEST_CASE("Completed actions can be reconsidered promptly without interrupting casts", "[LevelingMath]")
{
    REQUIRE_FALSE(ReadyForDecision(499,false,true,true));
    REQUIRE(ReadyForDecision(500,false,true,true));
    REQUIRE_FALSE(ReadyForDecision(1500,false,false,true));
    REQUIRE_FALSE(ReadyForDecision(1500,false,true,false));
    REQUIRE_FALSE(ReadyForDecision(8000,true,true,true));
    REQUIRE(ReadyForDecision(5000,false,false,false));
}
TEST_CASE("Leveling masked sampling preserves probabilities for every selected index", "[LevelingMath]")
{
    std::array<float,3> scores{0,1,2};
    for(double u:{0.,.2,.99})
    {
        auto sample=Select(scores,u);double total=0;
        for(double p:sample.probabilities)total+=p;
        REQUIRE(total==Catch::Approx(1));REQUIRE(sample.probabilities[sample.index]>0);
    }
    REQUIRE_THROWS(Select(scores,1));
}
TEST_CASE("Grinding beyond the task level cannot replace finishing quest milestones", "[LevelingMath]")
{
    Progress a,b;a.ms=1000;b.ms=6000;a.xp=7000;b.xp=7800;a.xp_cap=b.xp_cap=7600;
    REQUIRE(Outcome(a,b,false,400).xp==1.5);
    a.xp=7800;b.xp=8200;REQUIRE(Outcome(a,b,false,400).xp==0);
    b.xp_cap=8000;REQUIRE_THROWS(Outcome(a,b,false,400));
}
TEST_CASE("Leveling actor and critic validate real exported weights and parity", "[LevelingMath]")
{
    auto path=std::getenv("PLUTO_LEVELING_TEST_MODEL");std::string text;
    if(path){std::ifstream f(path);REQUIRE(f.good());text.assign(std::istreambuf_iterator<char>(f),{});}
    else text=SyntheticLevelingArtifact();
    rapidjson::Document d;d.Parse(text.c_str());auto expected=d["checkpoint_sha256"].GetString();
    auto model=Model::Parse(text,expected);ActionFeatures a{};State s{};
    REQUIRE(std::isfinite(model.actor.Score(a)));REQUIRE(std::isfinite(model.critic.Score(s)));
    REQUIRE_THROWS(Model::Parse(text,std::string(64,'f')));
}
