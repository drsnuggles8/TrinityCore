#pragma once
#include <cstdint>

namespace Playerbot::V2
{
// Spend the real Hearthstone only after six minutes of failed ordinary walking.
// Eligibility comes from native inventory/cooldown checks; this grants no relocation or cooldown reset.
inline bool IdleHearthReady(std::uint32_t strikes, bool available)
{
    return available && strikes >= 8;
}
}
