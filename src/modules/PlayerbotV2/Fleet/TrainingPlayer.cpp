#include "TrainingPlayer.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"

namespace Playerbot::V2::Fleet
{
namespace
{
auto const& TrainingGuids()
{
    // Immutable during a server run, thread-safe initialization and reads. Checkpoint renames keep the GUID.
    static auto const guids = ParseTrainingPlayerGuids(
        sConfigMgr->GetStringDefault("Playerbot.Training.PlayerGuids", "", true));
    return guids;
}
}

bool HasTrainingPlayers() { return !TrainingGuids().empty(); }

bool IsTrainingPlayer(Player const* player)
{
    auto const& guids = TrainingGuids();
    if (guids.empty()) return false;
    return player && player->GetSession() && player->GetSession()->IsBot() && guids.contains(player->GetGUID().GetCounter());
}

bool IsPlayerForPopulation(Player const* player)
{
    if (!player) return false;
    WorldSession const* session = player->GetSession();
    return CountsAsPopulationPlayer(session != nullptr, session && session->IsBot(), IsTrainingPlayer(player));
}
}
