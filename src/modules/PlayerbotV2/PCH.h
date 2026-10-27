// Playerbot V2 - Precompiled header
// Per BUILD.md: TrinityCore PCH essentials + V2 heavy STL.
// Keep this list short; PCH bloat hurts compile times more than it helps.

#pragma once

// STL — used everywhere in V2
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// TrinityCore foundation
#include "Define.h"
#include "ObjectGuid.h"

// TrinityCore's heavy game headers, each included by 20-60 of the module's 366 files (counted 27 Oct): parsed once per
// sub-library instead of once per file. Only core headers the bot work does not edit; a module header here would rebuild
// every sub-library's PCH on each change to it.
#include "Creature.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTime.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldSession.h"
