#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/TrainingPlayer.h"

using namespace Playerbot::V2::Fleet;

TEST_CASE("A configured training player activates population with a headless session", "[ClassicTrainingPlayer]")
{
    REQUIRE(CountsAsPopulationPlayer(true, true, true));
    REQUIRE(CountsAsPopulationPlayer(true, false, false));
    REQUIRE_FALSE(CountsAsPopulationPlayer(true, true, false));
    REQUIRE_FALSE(CountsAsPopulationPlayer(false, true, true));
    REQUIRE_FALSE(CountsAsPopulationPlayer(false, false, false));
}

TEST_CASE("Training-player activation is explicit and ignores malformed GUIDs", "[ClassicTrainingPlayer]")
{
    auto guids = ParseTrainingPlayerGuids(" 42, 71;42,0,123junk,-1,18446744073709551616,, ");
    REQUIRE(guids.size() == 2);
    REQUIRE(guids.contains(42));
    REQUIRE(guids.contains(71));
    REQUIRE_FALSE(guids.contains(123));
    REQUIRE(ParseTrainingPlayerGuids("").empty());
}
