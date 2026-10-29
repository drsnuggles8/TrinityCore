#pragma once
#include "Bot/BotIntent.h"
#include <optional>
#include <utility>

namespace Playerbot::V2::Fleet
{
inline bool IsPrivilegedTrainingIntent(IntentBody const& body)
{
    return std::holds_alternative<UnstuckIntent>(body) || std::holds_alternative<TeleportToIntent>(body) ||
        std::holds_alternative<NearTeleportToIntent>(body) || std::holds_alternative<ResetCooldownsIntent>(body);
}

// Gate the actual executor call, not just individual planning/recovery paths. Ordinary spell/item travel stays legal.
template<class Operation>
auto ExecuteTrainingIntent(bool training_player, IntentBody const& body, Operation&& operation)
    -> std::optional<decltype(operation())>
{
    if (training_player && IsPrivilegedTrainingIntent(body)) return std::nullopt;
    return std::forward<Operation>(operation)();
}
}
