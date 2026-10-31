#pragma once
#include <algorithm>
#include <cstdint>

class SpellInfo;

// Read-only result of the health-changing damage path, after mitigation and death prevention.
// Health removed excludes overkill; resolved damage preserves the hit's post-mitigation amount.
struct DamageResolution
{
    SpellInfo const* spell = nullptr;
    std::uint32_t resolved = 0, effective = 0, health_removed = 0, overkill = 0, health_before = 0, health_after = 0, school_mask = 0;
    std::uint32_t source_owner_level = 0, target_owner_level = 0, target_level = 0;
    std::uint8_t damage_type = 0, attack_type = 0, melee_outcome = 255;
    bool auto_attack = false;
};

inline DamageResolution MeasureDamageResolution(std::uint32_t resolved, std::uint32_t before, std::uint32_t after)
{
    DamageResolution result;
    result.resolved=resolved;result.health_before=before;result.health_after=after;
    result.effective=std::min(resolved,before);
    result.health_removed=std::min(resolved,before>after?before-after:0);
    result.overkill=resolved>before?resolved-before:0;
    return result;
}
