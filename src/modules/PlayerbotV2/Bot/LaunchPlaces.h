// LaunchPlaces - the Alliance starting zones of a launch run and the spots the launch strategies head for, from the world
// database's spawns (key NPCs and innkeepers). Bot/LaunchProfile.h, Fleet/LaunchSim.
#pragma once

namespace Playerbot::LaunchPlaces {

struct Spot { float x, y, z; };

// Coldridge Valley: the starter area a bot "leaves" once it is farther than kColdridgeRadius from its middle.
inline constexpr Spot  kColdridge{ -6230.f, 400.f, 385.f };
inline constexpr float kColdridgeRadius = 450.f;
// Frostmane Troll Whelp camp (29 spawns of creature 706 around this point).
inline constexpr Spot  kTrollCamp{ -6447.f, 579.f, 387.f };
// Kharanos (Innkeeper Belm) and Thelsamar (Innkeeper Hearthstove).
inline constexpr Spot  kKharanos{ -5601.6f, -531.2f, 399.7f };
inline constexpr Spot  kThelsamar{ -5377.9f, -2973.9f, 323.3f };
// Ironforge, the Great Forge: where a casual goes sightseeing once.
inline constexpr Spot  kIronforge{ -4840.0f, -1033.0f, 502.0f };

// The Deeprun Tram's portals (areatrigger_teleport 2166 / 2171 exits, world_safe_locs 1102166 / 1102171): standing at one
// city's portal, a player comes out at the other's after the ride.
inline constexpr Spot  kTramIronforge{ -4839.5f, -1317.7f, 501.9f };
inline constexpr Spot  kTramStormwind{ -8354.2f, 524.1f, 91.8f };
inline constexpr unsigned kTramRideMs = 70000;   // walk down, wait for the car, ride, walk up

inline float Dist2d(Spot const& s, float x, float y) { float const dx = s.x - x, dy = s.y - y; return dx * dx + dy * dy; }

// A planar box on one map: where a starting zone's quest hubs may be while its bots are below level 10.
struct Box { float x0, x1, y0, y1; bool Has(float x, float y) const { return x >= x0 && x <= x1 && y >= y0 && y <= y1; } };

// An Alliance starting zone. town: the first town a skipper heads for; leave: where a leaver moves on to at 8; city: the
// capital a casual goes sightseeing in. A spot with x = y = 0 does not exist for the zone.
struct StartZone
{
    char const* key;
    unsigned    map;
    Spot        center;
    float       radius;     // the starting valley: a bot farther than this from `center` has left it
    Spot        town, leave, city;
    Spot        grind;      // the camp grinders (LaunchStrategy::TrollGrinder) farm before leaving the valley
    unsigned char grind_until; // ... until this level
    Box         hubs;       // the zone's quest hubs below level 10 (map `map`)
    bool        whole_map;  // hubs: the whole map (an island)
};

enum StartZoneId : unsigned char { kZoneColdridge = 0, kZoneNorthshire = 1, kZoneShadowglen = 2, kZoneZephras = 3, kZoneCount = 4 };

inline constexpr StartZone kZones[kZoneCount] = {
    // Dwarves and gnomes: Coldridge Valley -> Kharanos, Loch Modan, Ironforge.
    { "coldridge", 0, kColdridge, kColdridgeRadius, kKharanos, kThelsamar, kIronforge, kTrollCamp, 4,
      { -6800.f, -4500.f, -4800.f, 1000.f }, false },
    // Humans: Northshire Valley (Deputy Willem) -> Goldshire (Innkeeper Farley), Westfall (Sentinel Hill), Stormwind.
    { "northshire", 0, { -8933.5f, -136.5f, 83.4f }, 400.f, { -9462.7f, 16.2f, 57.0f }, { -10653.3f, 1166.4f, 34.9f },
      { -8867.8f, 673.7f, 98.0f }, {}, 0, { -10050.f, -8400.f, -1600.f, 800.f }, false },
    // Night elves: Shadowglen (Conservator Ilthalaine) -> Dolanaar (Innkeeper Keldamyr), Auberdine, Darnassus.
    { "shadowglen", 1, { 10328.9f, 826.1f, 1326.5f }, 350.f, { 9802.2f, 982.6f, 1314.0f }, { 6406.5f, 515.4f, 8.7f },
      { 10127.9f, 2224.8f, 1328.8f }, {}, 0, { 8500.f, 11200.f, -500.f, 2600.f }, false },
    // High Order Skyborne (Classic 1.60): Zephras Isle, map 2991, an island with no way off before level 10.
    { "zephras", 2991, { 4098.1f, 1849.7f, 975.6f }, 400.f, {}, {}, {}, {}, 0, {}, true },
};

// The starting zone of a race (Alliance: human, dwarf, night elf, gnome, High Order Skyborne).
inline StartZoneId ZoneForRace(unsigned race)
{
    switch (race)
    {
        case 1:  return kZoneNorthshire;
        case 4:  return kZoneShadowglen;
        case 95: return kZoneZephras;
        default: return kZoneColdridge;   // 3 dwarf, 7 gnome
    }
}

inline bool Exists(Spot const& s) { return s.x != 0.f || s.y != 0.f; }

// Is (map, x, y) in a zone's area for quest hubs below level 10?
inline bool InHubArea(StartZone const& z, unsigned map, float x, float y)
{
    return map == z.map && (z.whole_map || z.hubs.Has(x, y));
}

} // namespace Playerbot::LaunchPlaces
