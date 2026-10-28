#include "TrainingPlayer.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"

namespace Playerbot::V2::Fleet
{
bool IsTrainingPlayer(Player const* player)
{
    // Immutable during a server run, thread-safe initialization and reads. Checkpoint renames keep the GUID.
    static auto const guids = ParseTrainingPlayerGuids(
        sConfigMgr->GetStringDefault("Playerbot.Training.PlayerGuids", "", true));
    return player && player->GetSession() && player->GetSession()->IsBot() && guids.contains(player->GetGUID().GetCounter());
}

bool IsPlayerForPopulation(Player const* player)
{
    if (!player) return false;
    WorldSession const* session = player->GetSession();
    return CountsAsPopulationPlayer(session != nullptr, session && session->IsBot(), IsTrainingPlayer(player));
}
}
