# Launch simulation (PlayerbotV2)

<!-- SUMMARY: keep this block current. -->
## Summary

_Status: work in progress (26-27 Oct 2026). Numbers below are measured on the home server (Rocky 10, rootless Podman,
32 GB host, but the user slice is capped at MemoryHigh 3 GB / MemoryMax 4 GB for every container of the user)._

- **What works:** a roster of fresh level-1 dwarves and gnomes in Coldridge Valley with 15 personas (strategy x temperament x
  pace), random arrival after T0, guide routes (RestedXP and Olo's route, imported locally), troll grinders, skippers,
  leavers, crowd grinders, AFK and setup pauses, quest reading pauses, first-tag only (no shared tagging), "inv?" requests
  and temperament-based invites, class trainer visits instead of free spells, a minute report in the log.
- **Measured:** see [Capacity and performance](#capacity-and-performance).
- **Next:** see [Open items](#open-items).

## Using it

```
.playerbot launch prepare [bots] [late]   create the roster offline (a few characters per tick)
.playerbot launch arm                     the run starts (T0) when the next real player enters the world
.playerbot launch start                   start now (no real player needed)
.playerbot launch stop                    log every launch bot out, keep the roster
.playerbot launch reset [bots] [late]     delete and recreate the roster at level 1 (new personas); with a size, a new roster size
.playerbot launch status | report
```

Without a client the commands go through SOAP (`~/forever/tc-cmd.py <command>` on the home server). A typical practice run:
`.playerbot launch reset 300 0`, wait for "ready", `.playerbot launch arm`, log in; the crowd starts with you.

World bots should be off during a launch run (`Playerbot.World.Enable = 0`) so only the launch crowd is in Coldridge.

## How a launch bot is put together

| Piece | Where | What |
|---|---|---|
| Roster | `Fleet/LaunchSim.{h,cpp}`, table `playerbot_launch_bot` | characters, persona, arrival, session length, seed |
| Persona -> profile | `LaunchSim::OnBotLogin`, `Bot/LaunchProfile.h` | strategy, temperament, pace, pauses; read-only after login |
| Pauses | `Bot/States/Rules/LaunchRules.cpp` `idle:launch_hold` @995 | UI setup after login, AFK breaks, browsing a trainer |
| Quest reading | `Bot/States/Rules/QuestRules.cpp` | accept delay from the profile's read range |
| Guide routes | `Fleet/LaunchRoutes.{h,cpp}`, table `playerbot_launch_route_step`, `contrib/launch-sim/import_route.py` | the step a bot is on is derived from its quest log every build |
| Places | `Bot/LaunchPlaces.h`, builder block "Launch strategies that steer by place" | troll camp, Kharanos, Thelsamar |
| Tags | `Creature::SetTappedBy` (`Classic.SharedTagging = 0`), `NearbyUnit::tapped/tapped_by_us`, `combat:launch_tag_lost` | first tag only; a bot drops a mob someone else tagged |
| Groups | `idle:launch_ask_invite` @850, `idle:launch_leave_group` @852, `LaunchAcceptsInvite`, `LaunchSim::OnAskInvite` | "inv?" to the tagger, answers by temperament, leave after the kill |
| Trainers | `LaunchRoutes::NearestTrainer`, `idle:launch_train` @740, `Module::OnLevelUp` | no free spells; walk to the class trainer at even levels from 4 |

Launch bots are ordinary V2 bots with `distribution_level 0` (operator bots), so the setup pipeline, the world spawner and the
session rhythm leave them alone.

### Personas

Shares and paces come from the launch research (2019 Classic launch, Season of Mastery, Hardcore, Season of Discovery, the
Forever beta and stress test). Pace 1.0 = an uncrowded speedrun (Coldridge left at ~35 min). Weights can be overridden with
`Playerbot.Launch.PersonaWeights = "racer:5,rxp:12,..."`.

| Persona | Weight | Strategy | Pace | Mostly |
|---|---|---|---|---|
| racer | 5 | GuideRoute (Olo's route) | 0.95-1.10 | greedy |
| rxp | 12 | GuideRoute (RestedXP, other guides) | 1.0-1.3 | neutral / greedy |
| trolls | 3 | TrollGrinder: whelps to 4, leaves Coldridge at 4 | 1.0-1.2 | greedy / loner |
| skipper | 2 | Skipper: out of Coldridge at 3 | 1.0-1.2 | neutral / greedy |
| questie | 25 | Quester (nearest objective) | 1.6-2.4 | neutral / friendly |
| friends | 14 | Quester | 1.2-1.6 | neutral / friendly |
| grinder | 6 | CrowdGrinder: drops crowded quest spots | 1.2-1.6 | neutral / loner |
| hunter | 5 | Quester (hunters) | 1.2-1.5 | mixed |
| gatherer | 6 | Gatherer | 2.0-2.6 | neutral / friendly |
| casual | 15 | Casual: reads everything, slow | 3.0-4.0 | friendly |
| partylead | 3 | PartyBuilder | 1.3-1.7 | friendly |
| idle | 6 | Idle: AFK a lot, logs out early | 4.0 | neutral |
| reroller | 4 | Reroller | 1.5-2.5 | neutral |
| ninja | 2 | KillStealer | 1.1-1.4 | griefer |
| leaver | 2 | Leaver: Loch Modan at 8 | 1.1-1.3 | neutral |

Arrival after T0 is the same for every persona (the login queue does not care who is a racer): 35% in the first 3 minutes,
35% by 15 minutes, the rest by 60; late arrivals (the `late` count) spread over hours one to six.

Classes follow the Forever beta census (Alliance), restricted to what a dwarf or gnome can be.

### Temperaments

| | asks the tagger for an invite | grants an "inv?" | takes an invite it did not ask for | stays in a group |
|---|---|---|---|---|
| Neutral | 25% of the mobs it wants | 45% (else "sry" 40%) | 65% | 60-120 s |
| Friendly | 15% | 95% | always | 2.5-5 min |
| Greedy | 70% | 5% | 70% | 20-40 s |
| Loner | never | never | 10% | 10-20 s |
| Griefer | 40% | 10% | 50% | 30-60 s |

### Guide routes

Route data never goes into git (the guides are CC BY-NC-SA, CC BY-NC-ND or private). `contrib/launch-sim/import_route.py`
reads a guide file you have locally and writes SQL for `playerbot_launch_route_step`; load it into the characters database
after the worldserver created the table (migration 0901).

## Capacity and performance

Measurements go here, each with the build, bot count and what changed.

| Date | Build | Bots | Idle RSS | RSS with bots | Update diff avg | CPU | Notes |
|---|---|---|---|---|---|---|---|
| 26 Oct | build9 (98147d0c) | 100 (72 online at 25 min) | 2329 MB | 2200-2300 MB | 1-2 ms | 26-31% | defaults; no bot took a quest (companion rule bug, fixed in f660dfc9) |

The bots themselves are cheap; the worldserver's own data is what fills the memory. Startup profile (RSS sampled every
0.2 s against the timestamped Server.log, `startup-mem.sh` on the home server), biggest steps with the defaults:

| Step | RSS growth |
|---|---|
| DB2 data stores | ~560 MB |
| creature locale strings (222875) | ~550 MB |
| quest template locale strings | ~270 MB |
| query cache (creature / gameobject / quest query packets, every locale) | ~210 MB |
| waypoint path nodes | ~190 MB |

Configuration changes, each measured against the defaults (steady state about two minutes after startup, no bots):

| Change | Idle RSS | Idle CPU | Notes |
|---|---|---|---|
| defaults | 2329 MB | 37% | the world loop ticks every ~1 ms (55k ticks a minute) |
| `Load.Locales = 0`, `CacheDataQueries = 0` | | | locales: enUS only (the client is enUS); queries answered on demand, a handful of real players |
| + `MinWorldUpdateTime = 20`, `MapUpdateInterval = 50` | 1712 MB | ~36-42% (noisy) | the bot module's own `[mem]` line |

In `worldserver.env` (home server): `TC_LOAD_LOCALES=0`, `TC_CACHE_DATA_QUERIES=0`, `TC_MIN_WORLD_UPDATE_TIME=20`,
`TC_MAP_UPDATE_INTERVAL=50`.

Code changes for capacity:

- Bot sessions get no update blocks (`BaseEntity` create / values builders return early for a bot target,
  `Player::SendInitialVisiblePackets` too): a bot has no client and `BotSession::SendPacket` drops everything, yet every
  visible object was serialized for every bot. Visibility (`m_clientGUIDs`) is tracked as before.
- jemalloc options (`malloc_conf` in worldserver `Main.cpp`; the build ignores `MALLOC_CONF`): background purging, dirty
  pages back to the OS after 2 s, 8 arenas.

## Open items

- Strategies still playing as Quester: Gatherer, Casual (Ironforge visit), PartyBuilder, Reroller (delete and recreate),
  KillStealer; zone progression after Dun Morogh; deathskip.
- Chat lines beyond invites; reactions to a real player's whispers.
- Camping spawn points, instant/ranged openers per class.
