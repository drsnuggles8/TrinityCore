// LaunchPlaces - the spots of the dwarf/gnome launch (Coldridge Valley, Dun Morogh, Loch Modan) the launch strategies head
// for, from the world database's spawns (map 0). Bot/LaunchProfile.h, Fleet/LaunchSim.
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

inline float Dist2d(Spot const& s, float x, float y) { float const dx = s.x - x, dy = s.y - y; return dx * dx + dy * dy; }

} // namespace Playerbot::LaunchPlaces
