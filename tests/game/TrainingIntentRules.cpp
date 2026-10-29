#include "tc_catch2.h"
#include "../../src/modules/PlayerbotV2/Fleet/TrainingIntentRules.h"

using namespace Playerbot;
using namespace Playerbot::V2::Fleet;

TEST_CASE("Training actors cannot execute bot-only spatial rescue or cooldown resets", "[TrainingIntentRules]")
{
    for (IntentBody const& body : {IntentBody{TeleportToIntent{}}, IntentBody{NearTeleportToIntent{}},
                                  IntentBody{UnstuckIntent{}}, IntentBody{ResetCooldownsIntent{}}})
    {
        bool executed = false;
        auto result = ExecuteTrainingIntent(true, body, [&] { executed = true; return 42; });
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(executed);
        REQUIRE(IsPrivilegedTrainingIntent(body));
        result = ExecuteTrainingIntent(false, body, [&] { executed = true; return 42; });
        REQUIRE(result == 42);
        REQUIRE(executed);
    }
}

TEST_CASE("Ordinary training-player actions still reach the engine", "[TrainingIntentRules]")
{
    for (IntentBody const& body : {IntentBody{MoveToIntent{}}, IntentBody{CastSpellIntent{}},
                                  IntentBody{UseItemByEntryIntent{}}, IntentBody{JumpIntent{}}})
    {
        bool executed = false;
        auto result = ExecuteTrainingIntent(true, body, [&] { executed = true; return 7; });
        REQUIRE(result == 7);
        REQUIRE(executed);
        REQUIRE_FALSE(IsPrivilegedTrainingIntent(body));
    }
}
