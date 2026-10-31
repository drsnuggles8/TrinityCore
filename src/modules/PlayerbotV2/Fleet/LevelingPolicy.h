#pragma once
#include <cstdint>
#include <string>
class Player;
namespace Playerbot { struct BotSnapshot;class BotAI;class BotIntentEmitter; }
namespace Playerbot::V2::Fleet::LevelingPolicy
{
void Initialize();
bool Controls(std::uint64_t guid);
bool Participant(std::uint64_t guid);
std::uint64_t XPRewardCap();
std::string Checkpoint(std::uint64_t guid);
std::uint64_t Epoch(std::uint64_t guid);
void Observe(Player const* player,BotSnapshot const& snapshot,std::uint64_t xp,std::uint64_t deaths,double xp_reference);
bool Tick(BotSnapshot const& snapshot,BotAI& ai,BotIntentEmitter& emitter);
void Executed(std::uint64_t guid,std::uint64_t source,std::uint8_t result);
void Logout(std::uint64_t guid);
}
