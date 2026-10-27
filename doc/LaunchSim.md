# Launch simulation (PlayerbotV2)

<!-- SUMMARY: keep this block current. -->
## Summary

_State 27 Oct 2026, 21:45. The server runs build38 (client 1.60.1.70634) with a fresh roster of 616 bots, prepared but
**not armed** (Olo is testing logins: `.playerbot launch arm` starts the crowd at the next login). Measured on the home
server (Rocky 10, rootless Podman, 32 GB host; the user slice is capped at MemoryHigh 3 GB / MemoryMax 4 GB for every
container of the user)._

- **What works:** a launch crowd in all four Alliance starting zones, sized from the Coldridge count by the end-of-beta
  census (200 in Coldridge gives 186 humans in Northshire, 112 night elves in Shadowglen, 118 Skyborne on Zephras Isle),
  race and class pairs from the same census; 12 active personas (the explorer is off, see the persona table), ~40% of
  them following a guide (RestedXP, Olo's route), each bot's surname is its persona; random arrival after T0, AFK /
  UI-setup / quest-reading pauses, first tag only, "inv?" requests and temperament-based answers, griefers that snipe
  tags, class trainer visits, grind camps per zone, leavers to the next zone (Coldridge through North Gate Pass to
  Thelsamar), a minute report in the log (`[launch] report`).
- **27 Oct, the day's fixes:** launch bots take full paths to far goals (path cap 512 polygons, A* pool 32768 nodes: the
  Coldridge Pass tunnel and the walks out of every starting zone are complete now); hunters close in or melee when Auto Shot
  cannot fire (opener give-ups 327 -> 16 per test); spell ranks resolve to the known rank (paladins had no seal); 17 fork
  fixes (crash on group kills, Zephras and Dun Morogh quests, creature spell scaling, resist penalty), cyman1988's class
  mechanics (Windfury, totems, poisons, talents), the Legacy perk Talented, full-name uniqueness (same first name, other
  surname), the friend list and names by "Name Surname"; quest givers, enders, spawns and pre-quests from the Forever
  QuestieDB (contrib/launch-sim/questie); the release client 70634 (realms moved, login through an SSH tunnel).
- **Racers (mix31, 45 min):** average level 3.4, the best 5 at 41 minutes; slower than real racers (5-6). Known brake:
  few quest turn-ins (2-6 in 45 minutes): kill quests in the trogg area progress slowly and loot walks time out
  (Open items).
- **Capacity:** 500 bots (370 online at once) at 1.85-1.89 GB RSS, 26-28% CPU and 3.6 ms of bot-module work per world
  tick; the 4 GB cap is far away. Idle server 1.7 GB (was 2.33 GB) with `Load.Locales = 0` and `CacheDataQueries = 0`;
  CPU at 300 bots went from ~30% to ~17% with the slower world loop and no update packets for bots.
- **Behaviour (last run, build18, 300 bots, dynamic respawn on):** median level 2 at ~15 minutes, 4 at 45-60 minutes,
  the first level 5 at ~36 minutes and 29 at 60 minutes; 667 quests turned in, 844 invite requests (440 granted),
  groups form and break up, rerollers come back at level 1, trainers get visited at even levels. CPU 20-32%, RSS 1.8-1.9 GB.
  Slower than the goal's "5 by ~90 minutes" median only if that is read strictly; the crowd is the brake, as at launch.
- **Upstream bot-module bugs** fixed on the way (no bot ever took a quest; quest POIs pointing to another zone; the idle
  watch breaking holds): [Module bugs](#module-bugs-found-on-the-way-affect-world-bots-too).
- **Next:** [Open items](#open-items).

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
`Playerbot.Launch.PersonaWeights = "racer:6,rxp:45,..."`. About 40% follow a guide (racer + rxp: RestedXP and the
other imported guides; Olo's route for the racers in Coldridge); where no guide exists for the race (Skyborne) those two
personas are drawn again. Each bot's surname is its persona ("Name Racer", "Name
Explorer"; Racer, Guided, Trollgrinder, Skipper, Questie, Friendly, Grinder, Gatherer, Casual, Partylead, Idler, Leaver,
Explorer). The hunter, reroller and ninja personas were dropped on 27 Oct: the class shares already give 16% hunters, rerolls
barely touch a launch's first hours, and kill stealing is the griefer temperament (some grinders and questers), which goes
for untagged mobs other players stand next to.

| Persona | Weight | Strategy | Pace | Mostly |
|---|---|---|---|---|
| racer | 6 | GuideRoute (Olo's route) | 0.95-1.10 | greedy |
| rxp | 45 | GuideRoute (RestedXP, other guides) | 1.0-1.3 | neutral / greedy |
| trolls | 3 | TrollGrinder: whelps to 4, leaves Coldridge at 4 | 1.0-1.2 | greedy / loner |
| skipper | 2 | Skipper: out of Coldridge at 3 | 1.0-1.2 | neutral / greedy |
| questie | 20 | Quester (nearest objective) | 1.6-2.4 | neutral / friendly, 3% griefer |
| friends | 12 | Quester | 1.2-1.6 | neutral / friendly |
| grinder | 6 | CrowdGrinder: drops crowded quest spots | 1.2-1.6 | neutral / loner, 10% griefer |
| gatherer | 6 | Gatherer: Mining (with a pick) or Herbalism from login, detours to nodes | 2.0-2.6 | neutral / friendly |
| casual | 12 | Casual: reads everything, slow, one trip to Ironforge from level 6 | 3.0-4.0 | friendly |
| partylead | 3 | PartyBuilder | 1.3-1.7 | friendly |
| idle | 6 | Idle: AFK a lot, logs out early | 4.0 | neutral |
| leaver | 2 | Leaver: Loch Modan at 8 | 1.1-1.3 | neutral |
| explorer | 0 (off since 27 Oct) | Explorer: a subzone tour (below), no kills until level 6 or the end of the route, then quests. Off: the real exploration routes (Rothie, mafsen) are only in the LevelForge guide service, and the hand-made tours took level-1 bots through mobs far above them; `explorer:2` in PersonaWeights brings them back with imported routes | 1.0-1.2 | loner / neutral |

Arrival after T0 is the same for every persona (the login queue does not care who is a racer): 35% in the first 3 minutes,
35% by 15 minutes, the rest by 60; late arrivals (the `late` count) spread over hours one to six.

Race and class come in pairs from the Forever census at the end of the beta (ClassicPlus.io addon census, data of 24-26 Oct
2026, 1,148,135 characters, as tabled by classicplus.world; it replaced the 7 Oct snapshot of 644k and the 20 Sep Reddit
census of ~5000), each starting zone from its own races. The zones' sizes come from the same numbers: `bots` in Coldridge,
and per 100 there 93 humans in Northshire, 56 night elves in Shadowglen and 59 Skyborne on Zephras Isle.

| Zone | Census characters | Pairs |
|---|---|---|
| Coldridge | dwarf 98.7k, gnome 88.2k | dwarf shaman 29%, gnome warlock 13%, gnome mage 12%, gnome priest 11%, dwarf paladin 9%, dwarf hunter 8%, gnome rogue 7%, gnome warrior 5%, dwarf warrior 4%, dwarf priest 2%, dwarf rogue 1% |
| Northshire | human 173.8k | paladin 28%, warrior 17%, hunter 14%, warlock 14%, priest 10%, rogue 10%, mage 8% |
| Shadowglen | night elf 104.3k | druid 30%, hunter 29%, warrior 14%, rogue 14%, priest 12% |
| Zephras Isle | High Order Skyborne 110.1k | druid 33%, mage 26%, hunter 21%, warrior 11%, rogue 10% |

### Temperaments

| | asks the tagger for an invite | grants an "inv?" | takes an invite it did not ask for | stays in a group |
|---|---|---|---|---|
| Neutral | 12% of the mobs it wants | 45% (else "sry" 40%) | 65% | 60-120 s |
| Friendly | 8% | 95% | always | 2.5-5 min |
| Greedy | 35% | 5% | 70% | 20-40 s |
| Loner | never | never | 10% | 10-20 s |
| Griefer | 20% | 10% | 50% | 30-60 s |

### Guide routes

Route data never goes into git (the guides are CC BY-NC-SA, CC BY-NC-ND or private). `contrib/launch-sim/import_route.py`
reads a guide file you have locally and writes SQL for `playerbot_launch_route_step`; load it into the characters database
after the worldserver created the table (migration 0901).

### Explorer routes

From the launch-day exploration guides (Jay Ell's dwarf/gnome sheet and video, 2018-2019; the Elysium and Blizzard forum
write-ups; summary of the research in the session notes). Real explorers mixed in talk-to quests and reached level 5 in about
an hour; exploration XP is 55-85 per subzone at levels 1-5. `LaunchRoutes::LoadExploreRoutes` resolves the stops against this
server's area table by name (the stop is a creature spawn near the subzone's middle; flight masters and innkeepers by zone)
and logs any name it cannot find.

- **South loop (60% of explorers):** Coldridge Pass, Kharanos (hearth), Ironforge flight path, tram, Stormwind flight path,
  the Elwynn circle (Goldshire, Crystal Lake, Brackwell, Eastvale, Ridgepoint, Maclure, Fargodeep, Stonefield, Forest's
  Edge, Westbrook), Westfall (Jansen Stead, Saldean's, Sentinel Hill flight path, Dead Acre), hearth to Kharanos, the
  Kharanos loop (Shimmer Ridge, Chill Breeze, Iceflow Lake, Brewnall, Frostmane Hold, Gnomeregan), east Dun Morogh (Misty
  Pine, Tundrid Hills, Amberstill, Gol'Bolar).
- **North run (40%):** Kharanos (hearth), Ironforge flight path, east Dun Morogh, the Kharanos loop, North Gate Pass, Algaz,
  Thelsamar flight path, Dun Algaz, Dun Modr, Menethil flight path. The guides' deliberate death into the Wetlands (and the
  boat to Auberdine) is left out: the bot walks the pass.

### Navigation

The Coldridge navmesh reaches 217 of 235 quest-mob spawn points from Anvilmar directly (the other 18 are long paths cut
at the 74-polygon probe limit, walked in chunks). A 15-yard grid probe (`mmap_probe --batch` from Anvilmar) found a few
islands on the valley floor east of Anvilmar: patches below a ledge higher than the 2.7 y walkable climb, which a bot can
step or slide onto but never path off (two racers stuck there). `contrib/launch-sim/offmesh.txt` links them to the main
mesh; the home server's `0000_43_31.mmtile` is rebuilt with it (the shipped tile is kept in `~/forever/mmtest`). The
generator's default slope limit (55 degrees for walkable and steep alike) was tested at 70 and 85: more steep ground, no
islands connected, so it stays at 55 (bots would climb what players cannot). Note the tile naming: Coldridge is tile
`43,31` (43 = 32 - pos.x / 533.33, 31 = 32 - pos.y / 533.33).

### Starting zones

Besides Coldridge Valley (the `prepare` / `reset` count, dwarves and gnomes, where Olo practises), the other Alliance starting
zones get their own crowds from the config: `Playerbot.Launch.NorthshireBots` (humans, Northshire), `.ShadowglenBots` (night
elves, Shadowglen), `.ZephrasBots` (High Order Skyborne, Zephras Isle): -1 (the default) sizes the zone from Coldridge by the census, 0 = none,
any other number is that many. The home server leaves them at the census: 200 in Coldridge gives 186 / 111 / 118, about
615 bots, what it was measured at (2.1 GB, 75% CPU). Each zone (Bot/LaunchPlaces.h `kZones`) has its first town (Kharanos, Goldshire, Dolanaar), the
place leavers move on to (Thelsamar, Sentinel Hill, Auberdine), the capital casuals visit (Ironforge, Stormwind, Darnassus) and
the area its quest hubs stay in below level 10; Zephras Isle has none of these (an island). The troll grinder and the explorer
are Coldridge personas; elsewhere those rolls are drawn again. Guide routes by zone: `rxp` / `olo` (dwarves and gnomes),
`rxp-human`, `rxp-nightelf` (`import_route.py --races human|nightelf`). Dynamic respawn covers Elwynn, Teldrassil and Zephras
too (`2026_10_27_01_world_launch_dynamic_respawn_zones.sql`).

### Respawns

The 2019 Classic launch used dynamic respawns in the starter zones (more players, faster respawns); whether Forever does is
not known. `sql/custom/world/2026_10_27_00_world_launch_dynamic_respawn.sql` puts the hostile creatures of the launch area
(1345 spawns) into spawn group 2 "Dynamic Scaling (Quest objectives)"; `Respawn.DynamicMode = 1` (`TC_RESPAWN_DYNAMIC_MODE=1`
in the home server's worldserver.env) switches it on: with N players in Dun Morogh a respawn takes 10/N of its time, at
least 10 s. Set it to 0 to practise against normal respawn timers. Without it 260 bots shared a few dozen wolves and boars
and the average level after 45 minutes was 2.8.

## Capacity and performance

Measurements go here, each with the build, bot count and what changed.

| Date | Build | Bots | Idle RSS | RSS with bots | Update diff avg | CPU | Notes |
|---|---|---|---|---|---|---|---|
| 26 Oct | build9 (98147d0c) | 100 (72 online at 25 min) | 2329 MB | 2200-2300 MB | 1-2 ms | 26-31% | defaults; no bot took a quest (companion rule bug, fixed in f660dfc9) |
| 27 Oct | build10 (f660dfc9) | 100 (72 online at 29 min) | 1707 MB | 1745-1820 MB | 20 ms (the MinWorldUpdateTime floor) | 9.5-12% | locales/query cache off, tick 20/50 ms, no update blocks for bots; at 27 min L1=21 L2=29 L3=16 L4=5, 92 quests turned in, 46 "inv?" (24 granted) |
| 27 Oct | build14 (bf5ca456) | 300 (254 online at 39 min) | 1675 MB | 1720-1850 MB | 20 ms floor | 16-26% | module tick 2.3-2.9 ms (snapshot build 1.4-1.8 ms) at 262 bots; 495 quests turned in, 323 "inv?" (193 granted), 8 rerolls; levels at 39 min L1=41 L2=98 L3=113 L4=2 (too slow: death hotspots, fixed in 0df87616) |
| 27 Oct | build14 (bf5ca456) | 500 (370 online at 23 min) | | 1850-1890 MB | 20 ms floor | 26-28% | ~0.5 MB per online bot; 533 quests turned in at 23 min |
| 27 Oct | build18 (018aa1e1) | 300 (279 online at 60 min) | | 1790-1870 MB | 20 ms floor | 20-32% | dynamic respawn on, all behaviour fixes: levels at 15 min L1=88 L2=75 L3=43; 30 min L1=31 L2=83 L3=75 L4=39; 45 min L4=90 L5=7; 60 min L1=31 L2=57 L3=65 L4=97 L5=29; 667 quests, 844 "inv?" (440 granted), 7 rerolls |

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

In `worldserver.env` (home server): `TC_LOAD_LOCALES=0`, `TC_CACHE_DATA_QUERIES=0`. The slower world loop
(`MinWorldUpdateTime = 20`, `MapUpdateInterval = 50`) was undone on 27 Oct at Olo's request: it adds up to 50 ms to server
reactions and its own share of the CPU saving was never isolated; runs before that date had it.

Code changes for capacity:

- Bot sessions get no update blocks (`BaseEntity` create / values builders return early for a bot target,
  `Player::SendInitialVisiblePackets` too): a bot has no client and `BotSession::SendPacket` drops everything, yet every
  visible object was serialized for every bot. Visibility (`m_clientGUIDs`) is tracked as before.
- jemalloc options (`malloc_conf` in worldserver `Main.cpp`; the build ignores `MALLOC_CONF`): background purging, dirty
  pages back to the OS after 2 s, 8 arenas.

## Module bugs found on the way (affect world bots too)

Fixed in our fork; candidates for upstream (forever-bots) once Olo says so.

- **No bot ever took or handed in a quest** (f660dfc9): `idle:companion_quest_accept` / `_turnin` cleared the pause slot
  they share with `idle:quest_accept` / `_turnin` on every tick without a companion quest, so the reading pause restarted
  each tick.
- **Item objectives sent bots across the continent** (bf5ca456): the surface-over-cave correction adopted the DB-first
  dropper of an ITEM objective unconditionally; for Q179 that is a Timber Wolf in Elwynn.
- **Of several QuestPOI blobs, the first won** (614320cc): now the nearest to the bot.
- **The idle watch broke deliberate holds** (8ed5a465): bots standing on purpose (launch setup / AFK) were given a forced
  walk after 45 s.
- **Bot sessions got full update packets built** (b7160216, core): see Capacity.

## Open items

- **Racer pace** (mix31): 2-6 quests turned in per racer in 45 minutes. A racer on "A New Threat" (170) attacked 42
  troggs in 30 minutes without finishing it, and 30 loot walks timed out ("unreachable" at 32-57 y: the walk deadline
  runs out while new pulls keep it fighting on the trogg slope). Next: why the kills do not complete 170 (tags lost? no
  kill credit?) and whether loot walks should pause the deadline during combat.
- **Explorer persona off** until the real routes (Rothie's, mafsen's LevelForge guides) can be imported; the stop,
  give-up, spirit-healer and route-check machinery stays (`~/forever/routecheck.sh` checks every leg in 15 s).
- **Questie review list** (contrib/launch-sim/questie/prequests-review-2026-10-27.txt): pre-quests not taken over,
  among them Coldridge Valley Mail Delivery (233) after Dwarven Outfitters (179), which would change Olo's Coldridge route
  if the live game has it. 64 Forever quests of the starting zones and their neighbours are missing from the world DB
  altogether (no quest_template row).
- **Builds:** a one-file change builds in 30-60 s; a change to BotAI.h (included by ~70 module files) takes ~12 minutes
  with 4 jobs. The user slice's 4 GB cap is what limits the jobs (16 cores); raising it is Olo's call.
- Strategies still playing as Quester: Gatherer, Casual (Ironforge visit), PartyBuilder; zone progression after Dun
  Morogh; deathskip.
- Chat lines beyond invites; reactions to a real player's whispers.
- Camping spawn points, instant/ranged openers per class.
