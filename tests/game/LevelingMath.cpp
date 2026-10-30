#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/LevelingMath.h"
#include <cstdlib>
#include <fstream>

using namespace Playerbot::V2::Fleet::Leveling;
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
    auto path=std::getenv("PLUTO_LEVELING_TEST_MODEL");REQUIRE(path!=nullptr);
    std::ifstream f(path);REQUIRE(f.good());std::string text((std::istreambuf_iterator<char>(f)),{});
    rapidjson::Document d;d.Parse(text.c_str());auto expected=d["checkpoint_sha256"].GetString();
    auto model=Model::Parse(text,expected);ActionFeatures a{};State s{};
    REQUIRE(std::isfinite(model.actor.Score(a)));REQUIRE(std::isfinite(model.critic.Score(s)));
    REQUIRE_THROWS(Model::Parse(text,std::string(64,'f')));
}
