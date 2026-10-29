#pragma once

#include <charconv>
#include <cctype>
#include <cstdint>
#include <string_view>
#include <unordered_set>

class Player;

namespace Playerbot::V2::Fleet
{
// Explicit GUIDs, independent of the checkpoint surname. Empty configuration keeps the normal server behavior.
inline std::unordered_set<std::uint64_t> ParseTrainingPlayerGuids(std::string_view list)
{
    std::unordered_set<std::uint64_t> result;
    while (!list.empty())
    {
        auto const end = list.find_first_of(",;");
        std::string_view token = list.substr(0, end);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) token.remove_suffix(1);
        std::uint64_t guid = 0;
        auto const [ptr, error] = std::from_chars(token.data(), token.data() + token.size(), guid);
        if (error == std::errc{} && ptr == token.data() + token.size() && guid != 0)
            result.insert(guid);
        if (end == std::string_view::npos) break;
        list.remove_prefix(end + 1);
    }
    return result;
}

inline bool CountsAsPopulationPlayer(bool hasSession, bool botSession, bool configuredTrainingPlayer)
{
    return hasSession && (!botSession || configuredTrainingPlayer);
}

bool IsTrainingPlayer(Player const* player);
bool IsTrainingPlayerGuid(std::uint64_t guid);
bool IsPlayerForPopulation(Player const* player);
bool HasTrainingPlayers();
}
