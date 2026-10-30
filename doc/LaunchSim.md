# Launch simulation (PlayerbotV2)

<!-- SUMMARY: keep this block current. -->
## Summary

_State 28 Oct 2026, 10:00. `olo` = `launch-sim` = 0954124f (build48, running before build49 started). The side branch
`fork-picks-1028` holds 37 more commits (fork fixes, LatvjuAvs's bot fixes, Forever's kill XP for lower mobs, launch bags
and money, the guide's optional hand-ins), built and benched as build49. Measured on the home server (Rocky 10, rootless
Podman, 32 GB host; the user slice is capped at MemoryHigh 3 GB / MemoryMax 4 GB for every container of the user)._

- **28 Oct, the night's work:** racers on the seeded bench from 4.32 to ~5.0 average level at 45 minutes (Racer pace);
  A New Threat completes (kill objectives head for their own creature; Olo's opportunistic trogg steps are not route
  steps); loot clocks pause in fights, loot and quest walks no longer pull against each other, quest objects are walked to
  and used, criteria-tree cast objectives work (Reading the Ley Lines 0 -> 8-12 turned in); full bags drop junk instead of
  stalling; the 27 Oct upstream batch merged; the census crowd measured (crowd43); a roster seed and bench scripts
  (`racer45.sh`, `crowd.sh`, `zones.sh`, `pmp.sh`).

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

`Playerbot.Launch.AutoRun = 1` (Olo's always-on realm): when a real player enters the world and nobody else is online, a
ready roster starts (T0 = that login; one still being made starts once ready). The run persists: when the last real player
leaves it pauses (the bots log out, the clock stops), and a server restart keeps it too; the next login resumes it at its
clock, the bots that were playing log in again at once at their levels and the rest keep arriving on schedule. The clock is
saved every minute in `playerbot_launch_run`, the bots whose session ended in `playerbot_launch_done` (characters
database; both created at load). `.playerbot launch stop` keeps the clock as well; only `.playerbot launch reset` starts
over at level 1. A second player joining a running crowd changes nothing, unless it is a new level-1 character in a
run past its first half hour: then a fresh crowd of level-1 bots launches in its starting zone on the same arrival curve
(`Playerbot.Launch.CohortBots`, default 150 for Coldridge and the other zones by the census; at most `Playerbot.Launch.MaxBots` (5000) bots in all;
`playerbot_launch_cohort`; a reset removes them). (Until 28 Oct the last logout and every server
start reset the roster to level 1, so a restart left the players alone among level-1 bots.)

For comparing builds, `Playerbot.Launch.RosterSeed = N` (N > 0) makes `prepare` (and `reset` with a size) roll the same
roster again: zones, races, classes, genders, personas, the bots' seeds and arrivals (names still vary). The racer bench
(`~/forever/racer45.sh`) sets it; a random class mix moved the 45-minute level by more than a change did.

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

Arrival after T0 is the same for every persona (the login queue does not care who is a racer): 55% in the first 3 minutes,
30% more by 15 minutes, the rest by 60 (35/35/30 until 28 Oct); late arrivals (the `late` count) spread over hours one to six.

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
in the home server's worldserver.env) switches it on. Set it to 0 to practise against normal respawn timers. Without it
260 bots shared a few dozen wolves and boars and the average level after 45 minutes was 2.8.

**The count is local.** When a creature dies, the server counts the players (bots included) that are alive, not in GM mode,
within `Respawn.DynamicRadius` (60 y) of its spawn point and in line of sight of it. With up to `Respawn.DynamicRateCreature`
(3) of them it keeps its own timer; with N > 3 it takes 3/N of it, never less than `Respawn.DynamicMinimumCreature`.
A crowded camp therefore respawns fast, while a cave nobody else is in keeps retail timers: Olo alone in Jasperlode Mine
(kobolds 270 s) or the Coldridge troll cave (whelps 180 s) gets the full timer, and 30 bots round a wolf camp bring its
180 s down to the minimum. Timers already at or below the minimum (quest objects such as Felix's box, 10 s) never change.

**No respawn on top of a fight.** A creature on a continent whose respawn is due while a player (bot or not) fights within
`Respawn.CombatWaitRadius` (15 y) of its spawn point waits `Respawn.CombatWaitStep` (5 s) at a time, `Respawn.CombatWaitMax`
(15 s) in all, so a camp that is never quiet still respawns. `Respawn.CombatWaitStep = 0` switches it off.

`TC_LOGGER_MAPS_RESPAWN=2,Server` (debug, `Logger.maps.respawn`) logs each death with its timer and the players counted, and
each combat wait. Environment variables only override keys the config file has, which is why worldserver.conf carries
`Logger.maps.respawn` at the root's level; `TC_LOGGER_MAPS` alone is ignored (no `Logger.maps` line). Test-realm evidence on
29 Oct (200 bots in Coldridge, 15 min, 1520 kills): the spawn-area wolves at 20 s with 27+ players counted, troggs 42-108 s,
boars 135 s, every Frostmane troll at its full 180 s with a median of 2 players counted. `~/forever/respawn-check.sh` repeats
the run.

**History (zone-wide counting, before the local count):** the stock TrinityCore rule counted every player in the zone, so
120-300 bots in Dun Morogh or Elwynn scaled every spawn there, caves included. Rate 10 gave ~15 s timers: mobs came back while
players still fought at the spot and the named trolls and Felix's corner were impossible. Off was too few mobs for the
crowd; rate 40 with a 60 s and then a 20 s minimum still swarmed the caves the bots never go into.

## Racer pace

Six racers alone in Coldridge (`~/forever/racer45.sh`: `PERSONA_WEIGHTS racer:1`, the other zones at 0), 45 minutes, on
Olo's route. Real pace for comparison: Olo's measured medians on a quiet server are level 4 at 11.6 minutes, 5 at 18.6, 6
at 28.2 (Olo-Route-Findings, `validate`).

| Date | Build | Avg level 15 / 30 / 45 min | Quests turned in (6 racers) | What changed |
|---|---|---|---|---|
| 27 Oct | build38 (q170, 25 min) | 3.6 at 25 min | 17 | baseline: 170's Rockjaw objective 0-4 of 6, Burly 6/6 |
| 27 Oct | build40 | 3.15 / 3.83 / 4.32 | 21 | kill objectives in a shared POI polygon head for their own creature; Olo's opportunistic trogg steps no longer imported as route steps; loot walk clock paused in fights |
| 28 Oct | build41 | 3.51 / 4.47 / 5.11 | 31 (170 completed by 4 of 6) | in-range loot clock paused in fights; open-world loot walks do not wait for idle mobs; given-up mobs stay given up, stop_attack ends Auto Shot; guide followers train only near their route; no hub walks under level 6. Loot give-ups in range 10 -> 0 |
| 28 Oct | build43, roster seed 1 (2 warlocks, warrior, rogue, 2 shamans) | 3.43 / 4.36 / 5.14 | 33 | the 27 Oct upstream merge; a grey is dropped for a quest's starting item; host busy with CI builds (the worldserver's CPU 16-18% instead of 8-9%). Loot walk give-ups 8, opener give-ups 12, idle-watch strikes 5. The baseline for later builds (same seed) |
| 28 Oct | build44, seed 1 | 3.50 / 4.34 / 4.90 | 29 | quest objects: planar reach, walk lockout 15 s (the shared WanderToNode lockout: racers detoured to ore veins, idle-watch strikes 5 -> 21; split off in build45) |
| 28 Oct | build45, seed 1 | 3.56 / 4.36 / 4.93 | 27 | cast-at-focus objectives; QuestGoWalk lockout. Strikes 16, five of them racers walking back to Anvilmar's vendor with one free slot (build47: guide followers sell only within 40 y) |
| 28 Oct | build47, seed 1 | 3.48 / 4.41 / 4.97 | 25 | melee bots face their victim and chase again; quest-object walks hold; guide followers sell only within 40 y (still 4 strikes: the full-bag town walk took over, fixed on fork-picks-1028). 170 complete for all six, none turned in: the guide's optional hand-in was not imported (fixed on fork-picks-1028) |
| 28 Oct | build42 | 3.36 / 4.11 / 4.73 | 28 | the far quest walk no longer interrupts a loot walk (loot walk give-ups 49 -> 6, idle-watch strikes 36 -> 10); another class mix (three shamans and a hunter, build41 none): not comparable, hence the roster seed |

Brakes found in build40's run: Dwarven Outfitters (8 Tough Wolf Meat) took 10-16 minutes (42 wolf corpses given up: the
loot walk waited for idle mobs beside the corpse and the in-range loot clock counted fights); trainer walks of up to
350 y at every even level from 4; the idle watch walking level-3 racers toward Kharanos; a hunter re-engaging an
unreachable trogg every 10 s.

Run-to-run spread with the same seed and build is not measured yet; builds 43-45 (5.14, 4.90, 4.93) differ by about that
much, so changes under ~0.3 levels need repeated runs.

### The census crowd (crowd43)

build43, 616 bots (200 in Coldridge, the other zones by the census), default personas, 45 minutes (`~/forever/crowd.sh`):
529 online at 45 minutes, average level 3.98 (levels: 29 L1, 34 L2, 136 L3, 277 L4, 53 L5), 2314 quests turned in, RSS
1.50-1.90 GB, the worldserver at about one core (the host's CI runners compiled at the same time). By zone: Coldridge
dwarves 3.59 / 4.0 quests per bot, gnomes 3.42 / 3.3, Northshire 3.63 / 4.7, Shadowglen 3.26 / 3.4, Zephras 2.99 / 4.7.
Stack profile at t=30 (`~/forever/pmp.sh`, 20 gdb samples): the busy thread is mostly `Map::Update` (70% of busy stacks:
creature updates, visibility, relocation notifies); the bot module's snapshot build and intent drain ~15%.

Quests many bots held and none finished: Reading the Ley Lines (Zephras, 86 holders: a criteria-tree objective, cast Read
Ley Line at a ley line; build45 handles such objectives: 12 turned in of 31 in the zones test), Harvesting Windstones (50),
Falling With Style (65, another criteria tree, not a cast: open), A Refugee's Quandary (55), The Stolen Journal (39: the
picker always found a nearer objective than Grik'nir in the cave), Iverron's Antidote (16). Quest-object uses: 3 in 45
minutes (build44-46 fix the reach, the walk lockout and the walk being overridden; still open in the build45 zones test).

**The Stolen Journal, 29 Oct:** the guided bots do go for Grik'nir (rxp step 50) but rarely get there: in guided60 16 accepted
it and 2 turned it in, in racer45-build52 4 and 0. The route was right; the fights were not. Leoniah (warrior, level 4) went
into the troll cave three times in 19 minutes, met two to four whelps at once each time, fled at 25 % health and started over
from the cave mouth; the priest Narinah ground through and finished. A solo quester never sat down to eat near its quest
targets, so it went in at whatever health it had. Now it sizes each pull first (`idle:prepare_pull`, Bot/PullSize.h): the
quest target it would take next plus the idle aggressive mobs within 10 y of it. One mob is pulled as before; for two or more,
or a hard pull, the bot eats, drinks or sits until 90 % health and 80 % mana (it gives up after 2 minutes without regeneration,
for 1 minute). Once it sits it finishes, even when the pack thins out for a moment: in build75's racer45 bots stood up after a
few seconds at 36-57 % health because a whelp wandered off and the pull counted as one again.
Log: `[prepare_pull] <bot> waits before pulling N mobs`. Before a pull too hard to take alone (three or more mobs, an
elite, or a mob two levels or more above the bot as it sees it after content tuning) a bot also asks for a group as players do ("LFG Grik'nir"): `idle:launch_party_build`, which only the partylead persona
ran before, now runs for every launch bot but loners and griefers, inviting solo players of about its level within 30 y up to a
group of three. A group's kill now has one loot on the corpse under the group's loot method, with each member's quest items
(Unit::Kill); retail gave every tapper a full loot of its own, so two grouped players looted twice as much.

**The Stolen Journal, 30 Oct:** resting and hard-pull groups did not move it (build75 1 of 17, build76 0 of 11). In build76's
guided60, 15 of the 19 deaths in the troll cave came mid-flee, and a third of all flees ended in death within 40 s
(`flee-outcome.py`). `combat:solo_flee` ran straight away from its attackers, which in a cave is deeper in, and it ran from
fights it was winning (one bot at 22 % from a single whelp at 59 %). Now a solo bot under its flee mark first reads the pace
of the last 1.5-4 s (Bot/FightOutlook.h): if at that pace its attackers die 1.5 times sooner than it does, it fights on
(`[solo_flee] <bot> fights on`). When it does run, it runs back along the way it came (Bot/FleeTrail.h: a point every 10 y
it walked out of combat), after a town NPC ahead and before a quest hub. Groups: any launch bot but loners and griefers
invites a solo player it sees within 30 y on one of its own unfinished quests ("same quest? lets group"), takes such an
invite, and the group stays together while a member near it is still on the quest, up to 15 minutes (Olo: "group up
whenever they see another player doing the quest"). Log: `[party_build] <bot> invites <guid> (same quest | hard pull)`.
The cave bench (`~/forever/cave.sh BOTS MINUTES LABEL`, test realm only) starts dwarf and gnome guide followers at level 4
with every guide quest before The Stolen Journal handed in, and counts 218 taken / Grik'nir kills / handed in, cave deaths and
flee outcomes.

## Capacity and performance

Measurements go here, each with the build, bot count and what changed.

| Date | Build | Bots | Idle RSS | RSS with bots | Update diff avg | CPU | Notes |
|---|---|---|---|---|---|---|---|
| 26 Oct | build9 (98147d0c) | 100 (72 online at 25 min) | 2329 MB | 2200-2300 MB | 1-2 ms | 26-31% | defaults; no bot took a quest (companion rule bug, fixed in f660dfc9) |
| 27 Oct | build10 (f660dfc9) | 100 (72 online at 29 min) | 1707 MB | 1745-1820 MB | 20 ms (the MinWorldUpdateTime floor) | 9.5-12% | locales/query cache off, tick 20/50 ms, no update blocks for bots; at 27 min L1=21 L2=29 L3=16 L4=5, 92 quests turned in, 46 "inv?" (24 granted) |
| 27 Oct | build14 (bf5ca456) | 300 (254 online at 39 min) | 1675 MB | 1720-1850 MB | 20 ms floor | 16-26% | module tick 2.3-2.9 ms (snapshot build 1.4-1.8 ms) at 262 bots; 495 quests turned in, 323 "inv?" (193 granted), 8 rerolls; levels at 39 min L1=41 L2=98 L3=113 L4=2 (too slow: death hotspots, fixed in 0df87616) |
| 27 Oct | build14 (bf5ca456) | 500 (370 online at 23 min) | | 1850-1890 MB | 20 ms floor | 26-28% | ~0.5 MB per online bot; 533 quests turned in at 23 min |
| 27 Oct | build18 (018aa1e1) | 300 (279 online at 60 min) | | 1790-1870 MB | 20 ms floor | 20-32% | dynamic respawn on, all behaviour fixes: levels at 15 min L1=88 L2=75 L3=43; 30 min L1=31 L2=83 L3=75 L4=39; 45 min L4=90 L5=7; 60 min L1=31 L2=57 L3=65 L4=97 L5=29; 667 quests, 844 "inv?" (440 granted), 7 rerolls |
| 28 Oct | build58 (24026563be) | 5000 (4980 online, fast arrival) | | 3.5 GB | ~53 ms (42-68) | | module 18 ms; snapshot build split over the workers 22 -> 9 ms; still in the profile: bot relocation (30%, every moving bot visited 533 y), the serial OnWorldUpdate loops |
| 28 Oct | build59 (6d2f744c83) | 5000 (4980 online, fast arrival) | | 3.6 GB | ~35 ms (33-41) | | a moving bot updates only the map's real players and has creatures within 50 y check it for aggro; moving creatures check bots only within 50 y; no session lookups per bot per tick. Profile now led by the snapshot build on the workers |
| 28 Oct | build60 (86592555f6) | 615 crowd (483 online at 12 min) | | 2.1 GB | 1-3 ms | 41-62% | no dynamic respawn: at 12 min L1=307 L2=176, 711 quests; standing at the spawn 6% / 5% / 2% at 5 / 10 / 12 min, standing still anywhere ~30% (mostly between quest-walk steps); troll whelp kills spread over 12 cells of 40 y across the camp |

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

- **Racer pace:** see [Racer pace](#racer-pace) for the runs. Real racers skip "A New Threat" (170) and "A Refugee's
  Quandary" (3361, Felix's box, chest and bucket) as noob traps (Olo); Olo's route takes 170 only opportunistically and
  the racers now do too. Quests per racer (~4-5 in 45 minutes against 8+ wanted): kill speed per class at levels 2-4
  (about 1.6 kills a minute against 3-4 for a player) is the next brake; run-to-run spread on the same seed is unmeasured.
- **Quest objects out of reach:** Harvesting Windstones (Zephras) stays at 0: most of its 29 windstones sit on rocks the
  bots cannot path onto (and lock 4088 needs skill 42; fork-picks-1028 drops the lock). A Refugee's Quandary (3361) and
  Iverron's Antidote (3521) also never finish. Counting objects as targets in idle:quest_path kept the bots at the
  windstones and starved Reading the Ley Lines (reverted 9056f360).
- **Hunters at range:** in Shadowglen and Zephras hunters still give up openers at 27-30 y with Auto Shot running
  (fork-picks-1028 retargets a running Auto Shot, LatvjuAvs 5610fd6b); Falling With Style (92474) is a criteria tree that
  is no spell cast: no rule for it.
- **Decisions taken 28 Oct (Olo):** LatvjuAvs's bot fixes taken (cherry-picks and ports, not the branch); Forever kill XP
  for lower mobs (mob base, forever-bugs#138) taken; Trixx34 master and Sakhwoow forever-bots-merge reviewed (two SQL
  fixes taken, the rest is infrastructure, localisation or already in).
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
