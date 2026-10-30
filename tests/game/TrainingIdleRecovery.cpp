#include "tc_catch2.h"
#include <initializer_list>
#include "../../src/modules/PlayerbotV2/Bot/TrainingIdleRecovery.h"
#include "../../src/modules/PlayerbotV2/Bot/IdleWatch.h"

TEST_CASE("Training Hearthstone recovery waits for sustained failed walks and respects cooldown", "[TrainingIdleRecovery]")
{
    using namespace Playerbot::V2;
    IdleWatch state;
    REQUIRE_FALSE(IdleStuck(state, 10, 20, 1, false));
    for (std::uint32_t strike = 1; strike <= 8; ++strike)
    {
        REQUIRE(IdleStuck(state, 10, 20, 1 + strike*kIdleStuckMs, false));
        REQUIRE_FALSE(IdleHearthReady(state.strikes, false));
        REQUIRE(IdleHearthReady(state.strikes, true) == (strike == 8));
    }
    REQUIRE_FALSE(IdleStuck(state, 40, 20, 2 + 8*kIdleStuckMs, false));
    REQUIRE_FALSE(IdleHearthReady(state.strikes, true));
}
