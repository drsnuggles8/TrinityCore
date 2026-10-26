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
| Neutral | 25% of the mobs it wants | 45% (else "sry" 40%) | 65% | 90-180 s |
| Friendly | 15% | 95% | always | 5-10 min |
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

## Open items

- Strategies still playing as Quester: Gatherer, Casual (Ironforge visit), PartyBuilder, Reroller (delete and recreate),
  KillStealer; zone progression after Dun Morogh; deathskip.
- Chat lines beyond invites; reactions to a real player's whispers.
- Camping spawn points, instant/ranged openers per class.
