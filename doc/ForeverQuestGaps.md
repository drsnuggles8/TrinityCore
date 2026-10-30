# Forever quest gaps

What our world database lacks against live WoW Classic Forever, found 28 Oct 2026 by comparing QuestieDB's Forever data
(the items' `questRewards`) with `world.quest_template`, and checking each quest on Wowhead Forever.

## Full audit, 28 Oct 2026 (afternoon)

Every quest the world DB shares with the sources was compared (`contrib/launch-sim/questie/audit/`): 4614 with the
Forever QuestieDB, 3938 with Wowhead Forever's quest list rows (Olo's page cache of 23 Oct; fixed and choice items with
counts, levels, XP, money) and 1963 quest pages' XP multipliers, plus the XP Factoruide's collector saw on the beta.
Wowhead's rows are the better reward source: QuestieDB's item data lacks most of Forever's new reward items.

| Kind | Agree | Differ | Changed |
|---|---|---|---|
| Reward items (Wowhead) | 3904 | 34 | 25 (06) |
| Quest level (Wowhead) | | 17 | 14 (07) |
| XP at the quest's level | 2470 | 1001 | 70 (07), 912 need rounding (C++) |
| Money (Wowhead) | 2472 | 26 + 8 | 18 (08) |
| Givers / enders (Questie) | | 47 / 23 creature, 7 / 1 object | 20 / 12 / 5 / 1 (09) |
| Pre-quests (Questie) | | 161 missing, 141 differ | 5 (09) |

- `sql/custom/world/2026_10_28_06_world_forever_quest_rewards_wowhead.sql`: 24 quests get the Forever items Wowhead
  lists (mostly a third choice next to the old ones; 202, 613, 682, 684, 709, 1012 turn their single fixed item into a
  choice; 1203 moves Reedknot Ring into the choices next to Swamp Ring; 1101 gives the Quilboar Blaster instead of the
  "Mage-Eye" Blunderbuss, and 1102 gets it too; 1371 gets its two Engineering schematics); 97275 gives 2 potions.
- `2026_10_28_07_world_forever_quest_levels_xp.sql`: 14 quest levels (299, 472, 544, 556, 557, 614, 615, 620, 932, 952,
  1132, 3792, 7802, 7803). 2026_10_26_25 had put them into `quest_classic_level.QuestLevel`, which the server never reads
  (it takes the level from `quest_template_classic_level`). 66 dungeon quests get Forever's XP multiplier (1.75-2.675; the
  collector saw the multiplied XP on all 21 it has). 100, 606, 635 and 745 get the XP difficulty that gives Wowhead's XP.
- `2026_10_28_08_world_forever_quest_money.sql`: money for the six Forever quests added this morning (they paid nothing),
  ten vanilla quests whose Forever level or money changed, and 0 for 234 and 752 (the profession book quests).
- `2026_10_28_09_world_forever_quest_links_prequests.sql`: givers / enders for 26 quests that had none (Forever quests
  such as 95494, 95495, 95507 at Vrang Wildgore, 96895 at Deathguard Terrence, 98823 at Bashana Runetotem; the wanted
  posters for 549 and 567), and five pre-quests that Questie and a second source agree on (96393, 96897, 96898, 96899,
  98435).

**XP rounding (not data):** Forever rounds quest XP like retail (steps of 5 / 10 / 25 / 50 below 100 / 500 / 1000 /
above): the collector's in-game XP matches the rounded value for 95 of 108 quests where ours differs only by that
rounding (Kobold Camp Cleanup 335 -> 340, Princess Must Die! 780 -> 775). `Quest::XPValue` drops the rounding since the
vanilla formula was ported; 912 quests are affected.

**Left for review:**
- Wild Hearts (429): Wowhead Forever shows no item reward (ours: Discolored Healing Potion and its recipe); Stealing
  Supplies (1370): Wowhead's row gives 1 Silver Star instead of 200; Darkmoon Faire 7905 / 7926: 1 prize ticket against
  our 5. Not changed: a removal or count drop is likelier a Wowhead gap than a Forever change.
- 13 Forever quests whose in-game XP (collector) fits neither Wowhead nor us (91723, 91745, 96822, 96825, 96874, 96876,
  97253, 98319, 99052, 99080, 99127, 99131, 99143); 96875 and 97914, added this morning, keep their XP (the collector's
  figure contradicts Wowhead's row).
- Pre-quests: `contrib/launch-sim/questie/prequests-review-2026-10-28.txt` (Questie only, no second source).
- Giver differences where both sides have one (the Commendation Signet, Songflower and Windblossom quests are the same
  givers in a different quest-id order; Membership Card Renewal, Seal of Ascension, Spellfire Robes similar).
- Zul'Gurub's reputation quests: Questie says required level 60, ours (and Factoruide's Wowhead data) 58.

## Fixed (morning pass)

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

## Rewards that never arrived (30 Oct)

Olo's Jarreway turned in The Troll Cave (182) and The Stolen Journal (218) but holds neither the Anvilmar Hammer nor the
Smooth Walking Staff his guide picks (OloGuides `.turnin 182,2` / `.turnin 218,2`); the armour picks of 170, 179 and 183
arrived. The server data is right (822 bots hold 5761 from 182, 137 of 138 a 218 item), so the loss is on the turn-in.
Two things now leave a trace:

- A quest with reward choices turned in with no choice is refused and its reward window sent again (`[reward] ... no
  reward choice`); before, `Player::RewardQuest` gave no choice item and said nothing.
- Logger `player.items` (Server.log) records each real player's turn-in (the choice received and whether it landed) and
  every item sold or destroyed: `[reward]`, `[sell]`, `[destroy]`. The next choice turn-in shows where the item goes.

It showed (30 Oct, The Troll Cave, Princess Must Die!): the window had no choices to pick. 152 choice quests carry a retail
`QuestPackageID` (182: 561, 218: 562, 88: 791), and with one the client builds the choices from QuestPackageItem.db2,
which is empty in Forever 70685; quests without one (Webwood Venom, 916) were fine. `ObjectMgr::LoadQuests` now drops a
package with no items (`Quest N has QuestPackageID = P but the package has no items` in DBErrors.log), and closes up and
recounts the choice and item lists after its checks remove rewards, so a removed reward is no longer announced as an
empty slot.

The bots' starter-quest and junk-quest completion (`StarterQuestAutocomplete`, `JunkQuestResolver`) passed the choice's
index where `RewardQuest` takes an item id, so those quests gave bots no choice item; they pass the item now.

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
