#pragma once
#include <cstdint>
class Player;
namespace Playerbot { struct BotSnapshot; struct Intent; }

namespace Playerbot::V2::Fleet::TrainingCapture
{
void Initialize();
void Login(Player const* player);
void Logout(Player const* player);
void Death(Player const* player);
void Observation(Player const* player, BotSnapshot const& snapshot);
void Executed(Player const* player, Intent const& intent, std::uint8_t result, bool exception = false);
void Shutdown();
}
