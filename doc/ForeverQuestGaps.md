# Forever quest gaps

What our world database lacks against live WoW Classic Forever, found 28 Oct 2026 by comparing QuestieDB's Forever data
(the items' `questRewards`) with `world.quest_template`, and checking each quest on Wowhead Forever.

## Fixed

- `sql/custom/world/2026_10_28_02_world_profession_books.sql`: the three profession books (Mining for Dummies, Wild Harvest,
  Pelt Collecting for Beginners) as reward choices of each starting zone's quest (234, 458, 752, 4402, 6395, 91743; 93552
  had them).
- `sql/custom/world/2026_10_28_03_world_forever_quest_rewards.sql`: 43 quests get the reward item Forever added (fixed or
  choice as Wowhead shows it, quantity 1); 7 quests (167, 249, 253, 704, 962, 1098, 1199) move their old fixed reward into
  the choices next to the new items; 140 gives Captain Sander's Locker (270257) instead of the Booty Bag (3343).
- `sql/custom/world/2026_10_28_04_world_forever_missing_quests.sql`: 11 quests added: 86574 Stonetalon Supply Run, 87288
  Soft Saber Pelts, 98298 Arugal's Folly, 97275 Whuut's the Rush, 97914 Expanding Horizons -> 97926 Making Do, 98189 Return
  the Statuette -> 98190 / 98191 A Friend of the Family, 95217 The Quarry's Smith, 96875 Beasts of Thunder Ridge.

Uncertain in those quests: quest texts are empty (no sniff), they pay no money, Flags 0 (not shareable), no POIs; 98298's
prerequisite (99) comes from Wowhead's series list only; 98190 has no XP figure (difficulty 1 assumed); the new drops
(267416 from Scarred Crag Boar 1689, 275720 from 3130 and 3131) use a 40% chance, as other Forever quest items whose rate
was never seen. Quest objective ids use a private range: 990000000 + quest x 10 + index.

Not changed: 9324, 9325, 9330, 9331, 9386 (Midsummer foods Questie lists are not on Wowhead Forever; 9324 and 9331 also
carry 23247 twice). 79097 got its reward but has no giver or ender in our database.

## Still missing (105 quests with an item reward, 629 in all)

Questie has 629 quest ids our database lacks; the 116 with an item reward were checked one by one:

| Group | Count | Added |
|---|---|---|
| Forever zone quests (86574-99267) | 54 | 8 (+3 chain links) |
| Class quests (Shaman, Paladin) | 7 | 0 |
| Event, profession, repeatable | 8 | 0 |
| Season of Discovery ids Forever reuses | 9 | 0 |
| Library books | 38 | 0 |

Why the rest are not added:

- **Giver or ender missing:** 86585 (Ylva, summon script), 86614 (object 665289), 92457 and 96984 (Image of Archmage
  Modera 269127), 93128 (Nerlokh 253177), 95508 (Walton 261366), 95805 (Ritual Fire 660739), 95999 (wanted poster 649051),
  97237 (Roy Lewells 268568), 98095, 98297, 98545, 99267; 99053 and 99123 are escorts (scripts needed).
- **Giver exists, target missing:** mobs for 86758 (Daggerfang), 87760, 95646, 95697, 96130, 96986, 96988, 97005, 98025,
  98028, 98310, 98326, 98403, 98824; objects for 92456, 93797, 96139, 97222. City of Dalaran (zone 16544) and the
  Excavation Site (zone 16732) are mostly absent.
- **Giver without a spawn:** 95682 (Deathstalker Agent), 98093 (Sulhasa, an `[UNUSED]` template).
- **No known giver:** 93196, 95250, 95534, 95776, 96987, 95809.
- **Started by an item that drops nowhere here:** 95189 (chest 614679 drops the Horde starter 275521; the Alliance one would
  need a faction-conditioned row), 95195 (start item 268535 without loot), 95161 (prerequisite 92415 missing).
- **Scripted:** 95885 (use an item at a stone), 99108 (duels count).
- **Shaman class quests:** 94473 Fire Sapta and 94616 Water Sapta need NPCs 257808 and 258043, 96016 needs 259023; 79442
  (Horde) has giver and ender but its prerequisite 79366 "Heed My Call" is missing (Twisted Totems script).
- **Paladin class quests:** 95042 and 95140 givers missing; 95126 needs the scripted 95125.
- **Event and profession quests:** 91736, 92109, 92110 givers missing; the Craftsman's Writs (94217, 94248) start from
  items of unknown source and 94248's ender 257304 is missing; 97583 (slime pet) needs a dance interaction; 98977 and 98983
  are Lunar Festival quests without a known giver.
- **Library books:** the librarians are spawned, but only 6 of the 36 book objects exist and none has loot; Questie hides
  these quests and ATT says 78150 needs all 36 books. Left out until confirmed in game.

Most of these need a sniff of live Forever (spawns, scripts, texts).
