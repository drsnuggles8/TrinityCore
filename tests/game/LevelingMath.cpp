#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/LevelingMath.h"
#include <cstdlib>
#include <fstream>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace Playerbot::V2::Fleet::Leveling;
static std::string SyntheticLevelingArtifact()
{
    rapidjson::StringBuffer buffer;rapidjson::Writer<rapidjson::StringBuffer> w(buffer);w.StartObject();
    w.Key("schema");w.String("leveling-options-v1");w.Key("checkpoint_sha256");w.String(std::string(64,'a').c_str());
    w.Key("epoch");w.Uint64(1);w.Key("optimizer_step");w.Uint64(0);
    for(auto [name,count]:{std::pair{"feature_names",48},std::pair{"state_names",24}})
    {w.Key(name);w.StartArray();for(int i=0;i<count;++i)w.String(Features[i].data());w.EndArray();}
    w.Key("action_kinds");w.StartArray();for(auto kind:Kinds)w.String(kind.data());w.EndArray();
    for(auto [name,width]:{std::pair{"actor",48},std::pair{"critic",24}})
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
    for(auto [name,width]:{std::pair{"actor",48},std::pair{"critic",24}})
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
    REQUIRE(r.xp==1);REQUIRE(r.quest==.25);REQUIRE(r.death==-.5);REQUIRE(r.time==-.0025);REQUIRE(r.stall==0);
    REQUIRE(r.total>0);REQUIRE(r.seconds==5);
    auto end=Outcome(before,after,true,400);REQUIRE(end.potential==Catch::Approx(-.02));
    after=before;after.ms+=5000;r=Outcome(before,after,false,400);REQUIRE(r.total<0);REQUIRE(r.stall<0);
    after.xp=before.xp-1;REQUIRE_THROWS(Outcome(before,after,false,400));
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
