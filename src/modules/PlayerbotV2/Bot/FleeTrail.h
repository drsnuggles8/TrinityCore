#pragma once
// The way a bot came: points it walked through out of combat, kSpacing apart, newest last. A solo bot that runs from a fight
// runs back along them (combat:solo_flee, State_InCombat) - ground it has just crossed, so no fresh mobs - instead of straight
// away from its attackers, which in a cave led deeper in (Olo, 30 Oct: in build76 guided60, 15 of the 19 deaths in the
// Coldridge troll cave came mid-flee). Pure: tested in tests/game/ClassicFightOutlook.cpp.

#include <array>
#include <cstdint>

namespace Playerbot {

struct FleeTrail
{
    static constexpr float kSpacing = 10.0f;     // a point every 10 y walked
    static constexpr float kJump = 60.0f;        // a step longer than this is a teleport, a flight or a ghost run: a new trail
    static constexpr float kMinRun = 15.0f;      // a flee heads for a point at least this far from the bot

    struct Point { float x = 0.f, y = 0.f, z = 0.f; };

    std::array<Point, 12> points{};
    std::uint8_t count = 0, head = 0;   // head: the next slot written
    std::uint32_t map = ~0u;

    // The bot is out of combat at (x, y, z) on `map`.
    void Note(std::uint32_t map_id, float x, float y, float z)
    {
        if (map_id != map)
        {
            map = map_id;
            count = 0;
        }
        if (count)
        {
            Point const& n = At(0);
            float const d2 = (x - n.x) * (x - n.x) + (y - n.y) * (y - n.y);
            if (d2 < kSpacing * kSpacing)
                return;
            if (d2 > kJump * kJump)
                count = 0;
        }
        points[head] = Point{x, y, z};
        head = std::uint8_t((head + 1) % points.size());
        if (count < points.size())
            ++count;
    }

    // The point `back` steps behind the newest (0 = the newest).
    Point const& At(std::uint8_t back) const { return points[(head + points.size() - 1 - back) % points.size()]; }

    // Where a flee from (x, y) runs to first: the newest point at least kMinRun away; -1 when the trail has none.
    int First(float x, float y) const
    {
        for (std::uint8_t i = 0; i < count; ++i)
        {
            Point const& p = At(i);
            if ((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y) >= kMinRun * kMinRun)
                return i;
        }
        return -1;
    }
};

} // namespace Playerbot
