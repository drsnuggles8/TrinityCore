#pragma once
#include <cstdint>
#include <span>
#include <string>
class Player;
class Creature;
namespace Playerbot { struct BotSnapshot; struct Intent; struct NearbyUnit; class BotIntentEmitter; }

namespace Playerbot::V2::Fleet::TrainingCapture
{
void Initialize();
void Login(Player const* player);
void Logout(Player const* player);
void Death(Player const* player);
void MonsterKill(Player const* player, Creature const* creature);
bool ControlsQuestTarget(std::uint64_t guid);
bool PolicyRecordingAvailable(std::uint64_t guid);
bool PolicyEvent(std::uint64_t guid,std::uint64_t source,char const* kind,std::string const& payload);
NearbyUnit const* TryQuestTarget(BotSnapshot const& snapshot, std::span<NearbyUnit const* const> menu, BotIntentEmitter& emitter);
void Observation(Player const* player, BotSnapshot const& snapshot);
void Executed(Player const* player, Intent const& intent, std::uint8_t result, bool exception = false);
void Shutdown();
}
