#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/TrainingTargetModel.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <cstdlib>
#include <fstream>
#include <limits>

using namespace Playerbot::V2::Fleet::TrainingTarget;
namespace
{
std::string ConstantModel()
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> w(buffer);
    w.StartObject(); w.Key("schema_version"); w.Uint(1);
    w.Key("feature_names"); w.StartArray(); for (auto name : FeatureNames) w.String(name.data()); w.EndArray();
    w.Key("checkpoint_sha256"); w.String(std::string(64, 'a').c_str());
    w.Key("epoch"); w.Uint(7); w.Key("optimizer_step"); w.Uint(21);
    w.Key("layers"); w.StartArray();
    for (auto [inputs, outputs] : {std::pair{13,64}, std::pair{64,64}, std::pair{64,1}})
    {
        w.StartObject(); w.Key("weights"); w.StartArray();
        for (int out = 0; out < outputs; ++out) { w.StartArray(); for (int in = 0; in < inputs; ++in) w.Int(0); w.EndArray(); }
        w.EndArray(); w.Key("bias"); w.StartArray(); for (int out = 0; out < outputs; ++out) w.Int(outputs == 1 ? 2 : 0);
        w.EndArray(); w.EndObject();
    }
    w.EndArray(); w.Key("parity"); w.StartArray();
    for (int value : {0, 1})
    {
        w.StartObject(); w.Key("features"); w.StartArray(); for (int i = 0; i < 13; ++i) w.Int(value);
        w.EndArray(); w.Key("score"); w.Int(2); w.EndObject();
    }
    w.EndArray(); w.EndObject(); return buffer.GetString();
}
}

TEST_CASE("Frozen target models enforce identity, tensor shape and reference output", "[TrainingTargetModel]")
{
    auto json = ConstantModel();
    auto model = Model::Parse(json, std::string(64, 'a'));
    REQUIRE(model.epoch == 7);
    REQUIRE(model.optimizer_step == 21);
    REQUIRE(model.Score(Features{}) == 2);
    REQUIRE_THROWS(Model::Parse(json, std::string(64, 'b')));
    REQUIRE_THROWS(Model::Parse("{}", std::string(64, 'a')));
    REQUIRE_THROWS(Model::Parse("null", std::string(64, 'a')));
    REQUIRE_THROWS(Model::Parse("broken", std::string(64, 'a')));
    auto bad = json; bad.replace(bad.find("self_health_fraction"), 20, "wrong_feature");
    REQUIRE_THROWS(Model::Parse(bad, std::string(64, 'a')));
    bad = json; bad.replace(bad.find("\"score\":2"), 9, "\"score\":3");
    REQUIRE_THROWS(Model::Parse(bad, std::string(64, 'a')));
    bad = json;
    auto bias_start = bad.find("\"bias\":[")+8;
    bad.erase(bias_start, bad.find(']', bias_start)-bias_start);
    REQUIRE_THROWS(Model::Parse(bad, std::string(64, 'a')));
    auto features = Features{}; features[0] = std::numeric_limits<float>::quiet_NaN();
    REQUIRE_THROWS(model.Score(features));
    REQUIRE_THROWS(model.Score(std::span<float const>{}));
}

TEST_CASE("Target features preserve normalization and signed level differences", "[TrainingTargetModel]")
{
    auto features = Encode({50, 100, 20, 40, 3, 10, false}, {20, 60, 1, 3, 4, 0, true, false, true});
    REQUIRE(features[0] == 0.5f);
    REQUIRE(features[1] == 0.5f);
    REQUIRE(features[2] == 0.5f);
    REQUIRE(features[4] == 0.5f);
    REQUIRE(std::abs(features[5]-1.f/3) < 0.000001f);
    REQUIRE(std::abs(features[6]-0.2f) < 0.000001f);
    REQUIRE(std::abs(features[7]+1.f/3) < 0.000001f);
    REQUIRE(features[8] == 0.125f);
    REQUIRE(features[10] == 1);
    REQUIRE(features[11] == 0);
    REQUIRE(features[12] == 1);
}

TEST_CASE("Shadow history rejects missing, future, expired and evicted source frames", "[TrainingTargetModel]")
{
    History history;
    history.Push({1, 100, {}});
    REQUIRE(history.Find(1,100));
    REQUIRE_FALSE(history.Find(2,100));
    REQUIRE_FALSE(history.Find(1,99));
    REQUIRE(history.Find(1,2099));
    REQUIRE_FALSE(history.Find(1,2100));
    for (std::uint64_t version = 2; version <= 65; ++version) history.Push({version, 100, {}});
    REQUIRE_FALSE(history.Find(1,100));
    REQUIRE(history.Find(2,100));
    History next_episode;
    REQUIRE_FALSE(next_episode.Find(65,100));
    next_episode.Push({1,0xfffffff0u,{}});
    REQUIRE(next_episode.Find(1,20));
}

TEST_CASE("Private exported checkpoint matches PyTorch golden vectors", "[TrainingTargetModel]")
{
    char const* path = std::getenv("PLUTO_TEST_MODEL");
    if (!path) SKIP("Private checkpoint is supplied by the focused validation runner");
    std::ifstream input(path);
    REQUIRE(input.good());
    std::string json((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    auto model = Model::Parse(json, "6ebe769de813e12f3309b974bb60fe7b196de0088d9cd1c0cc05b5d82dcbbe10");
    REQUIRE(model.epoch == 80);
    REQUIRE(model.optimizer_step == 240);
    REQUIRE(std::isfinite(model.Score(Features{})));
}
