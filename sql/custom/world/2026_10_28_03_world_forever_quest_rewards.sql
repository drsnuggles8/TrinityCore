-- Quest rewards Forever added that our older quest rows lack (QuestieDB Forever lists the item as a reward; Wowhead
-- Forever, read 28 Oct 2026, says whether it is a fixed reward or a choice). Each item goes into the next free slot after
-- the rewards the row already has; quantity 1 (Wowhead shows no stack count for any of them). Existing rewards stay as
-- they are; quests where the new item only fits by moving an existing one are in rewards_restructure.sql. Each UPDATE
-- only touches a row whose target slot is still empty, so it is safe to run again.

-- 9 The Killing Fields: choice 276695 Harvester's Pestslayer
UPDATE `quest_template` SET `RewardChoiceItemID3` = 276695, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 9 AND `RewardChoiceItemID3` = 0;

-- 14 The People's Militia: choice 270006 Boots of the People's Militia
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270006, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 14 AND `RewardChoiceItemID4` = 0;

-- 31 Aquatic Form: fixed 263411 Idol of Shifting Tides
UPDATE `quest_template` SET `RewardItem2` = 263411, `RewardAmount2` = 1 WHERE `ID` = 31 AND `RewardItem2` = 0;

-- 46 Bounty on Murlocs: choice 276686 Junior's Hunting Bow
UPDATE `quest_template` SET `RewardChoiceItemID4` = 276686, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 46 AND `RewardChoiceItemID4` = 0;

-- 168 Collecting Memories: choice 270007 Worn Miner's Waistcord
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270007, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 168 AND `RewardChoiceItemID3` = 0;

-- 176 Wanted:  "Hogger": choice 270000 Apprentice Wizard's Gown
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270000, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 176 AND `RewardChoiceItemID4` = 0;

-- 214 Red Silk Bandanas: choice 270005 Monastic Hammer
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270005, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 214 AND `RewardChoiceItemID4` = 0;

-- 256 WANTED: Chok'sul: choice 270019 Loch Walkers
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270019, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 256 AND `RewardChoiceItemID3` = 0;

-- 377 Crime and Punishment: choice 270029 Town Clerk's Mittens
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270029, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 377 AND `RewardChoiceItemID3` = 0;

-- 386 What Comes Around...: choice 270027 Ursine Hammer
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270027, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 386 AND `RewardChoiceItemID3` = 0;

-- 442 Assault on Fenris Isle: choice 270020 Gnollhide Pants; choice 276722 Gnoll Shackles
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270020, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0, `RewardChoiceItemID4` = 276722, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 442 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0;

-- 451 A Recipe For Death: choice 276723 Wise Gauntlets
UPDATE `quest_template` SET `RewardChoiceItemID3` = 276723, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 451 AND `RewardChoiceItemID3` = 0;

-- 501 Elixir of Pain: choice 276724 Trophy Belt
UPDATE `quest_template` SET `RewardChoiceItemID4` = 276724, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 501 AND `RewardChoiceItemID4` = 0;

-- 531 Vyrin's Revenge: fixed 270017 Hunting Satchel
UPDATE `quest_template` SET `RewardItem1` = 270017, `RewardAmount1` = 1 WHERE `ID` = 531 AND `RewardItem1` = 0;

-- 549 WANTED: Syndicate Personnel: fixed 276720 Criminal's Pauldrons
UPDATE `quest_template` SET `RewardItem1` = 276720, `RewardAmount1` = 1 WHERE `ID` = 549 AND `RewardItem1` = 0;

-- 898 Free From the Hold: choice 270011 Embroidered Leggings
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270011, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 898 AND `RewardChoiceItemID3` = 0;

-- 914 Leaders of the Fang: choice 270018 Hammerbone
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270018, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 914 AND `RewardChoiceItemID3` = 0;

-- 940 Teldrassil: fixed 281250 Forest Oracle's Cloak
UPDATE `quest_template` SET `RewardItem1` = 281250, `RewardAmount1` = 1 WHERE `ID` = 940 AND `RewardItem1` = 0;

-- 943 The Absent Minded Prospector: choice 270022 Archaeologist's Gown
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270022, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 943 AND `RewardChoiceItemID3` = 0;

-- 982 Deep Ocean, Vast Sea: choice 270004 Murloc-hide Gloves
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270004, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 982 AND `RewardChoiceItemID3` = 0;

-- 1013 The Book of Ur: choice 270030 Tattered Mittens
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270030, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 1013 AND `RewardChoiceItemID3` = 0;

-- 1046 Raene's Cleansing: fixed 276242 Replica Dartol's Rod of Transformation
UPDATE `quest_template` SET `RewardItem2` = 276242, `RewardAmount2` = 1 WHERE `ID` = 1046 AND `RewardItem2` = 0;

-- 1059 Reclaiming the Charred Vale: choice 270035 Reclaimer's Robes
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270035, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 1059 AND `RewardChoiceItemID3` = 0;

-- 1142 Mortality Wanes: choice 270047 Sentinel's Boots
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270047, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 1142 AND `RewardChoiceItemID3` = 0;

-- 1144 Willix the Importer: choice 274078 Boar Signet
UPDATE `quest_template` SET `RewardChoiceItemID4` = 274078, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 1144 AND `RewardChoiceItemID4` = 0;

-- 1200 Blackfathom Villainy: choice 270031 Dark Ritual Leggings; choice 270032 Cultist's Armguards
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270031, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0, `RewardChoiceItemID4` = 270032, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 1200 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0;

-- 1275 Researching the Corruption: choice 270021 Staghide Armguards
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270021, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 1275 AND `RewardChoiceItemID3` = 0;

-- 1527 Call of Fire: fixed 263412 Totem of Charged Flames
UPDATE `quest_template` SET `RewardItem2` = 263412, `RewardAmount2` = 1 WHERE `ID` = 1527 AND `RewardItem2` = 0;

-- 2040 Underground Assault: choice 270015 Bravo's Armbands; choice 270016 Dreamer's Leggings
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270015, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0, `RewardChoiceItemID4` = 270016, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 2040 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0;

-- 2418 Power Stones: choice 270055 Charged Leather Bracers
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270055, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 2418 AND `RewardChoiceItemID4` = 0;

-- 2904 A Fine Mess: choice 270042 Technician's Bracers
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270042, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 2904 AND `RewardChoiceItemID3` = 0;

-- 2928 Gyrodrillmatic Excavationators: choice 270045 Operator's Gloves
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270045, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 2928 AND `RewardChoiceItemID3` = 0;

-- 3514 Horde Presence: choice 270038 Runed Gauntlets
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270038, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 3514 AND `RewardChoiceItemID3` = 0;

-- 5061 Aquatic Form: fixed 263411 Idol of Shifting Tides
UPDATE `quest_template` SET `RewardItem2` = 263411, `RewardAmount2` = 1 WHERE `ID` = 5061 AND `RewardItem2` = 0;

-- 5321 The Sleeper Has Awakened: choice 270014 Deerhide Pants
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270014, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 5321 AND `RewardChoiceItemID4` = 0;

-- 5724 Returning the Lost Satchel: choice 270003 Garrison Cuffs
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270003, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 5724 AND `RewardChoiceItemID3` = 0;

-- 6283 Bloodfury Bloodline: choice 270028 Harpy Bangles
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270028, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 6283 AND `RewardChoiceItemID3` = 0;

-- 6481 Earthen Arise: choice 270010 Raptorcrest Bracers
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270010, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 6481 AND `RewardChoiceItemID3` = 0;

-- 6521 An Unholy Alliance: choice 270054 Cultist's Chestguard
UPDATE `quest_template` SET `RewardChoiceItemID4` = 270054, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 6521 AND `RewardChoiceItemID4` = 0;

-- 6561 Blackfathom Villainy: choice 270031 Dark Ritual Leggings; choice 270032 Cultist's Armguards
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270031, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0, `RewardChoiceItemID4` = 270032, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0 WHERE `ID` = 6561 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0;

-- 6922 Baron Aquanis: choice 270043 Dreamer's Chestguard
UPDATE `quest_template` SET `RewardChoiceItemID3` = 270043, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0 WHERE `ID` = 6922 AND `RewardChoiceItemID3` = 0;

-- 79097 Baxtan: On Destructive Magics: fixed 211779 Comprehension Charm
UPDATE `quest_template` SET `RewardItem1` = 211779, `RewardAmount1` = 1 WHERE `ID` = 79097 AND `RewardItem1` = 0;

-- 94411 Meddlesome Mages: choice 263411 Idol of Shifting Tides; choice 263412 Totem of Charged Flames
UPDATE `quest_template` SET `RewardChoiceItemID1` = 263411, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0, `RewardChoiceItemID2` = 263412, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0 WHERE `ID` = 94411 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0;

-- ---- Rewards that move an old fixed reward into the choices (Wowhead Forever) ----
-- 167 Oh Brother. . .: fixed [1893] -> fixed [], choice [1893 Miner's Revenge, 270012 Miner's Workgloves, 270013 Miner's Workboots]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 1893, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270012, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270013, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 167 AND `RewardItem1` = 1893 AND `RewardChoiceItemID1` = 0;

-- 249 Morganth: fixed [5274] -> fixed [], choice [5274 Rose Mantle, 270033 Demonhide Bracers, 270034 Shadow Gauntlets]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 5274, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270033, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270034, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 249 AND `RewardItem1` = 5274 AND `RewardChoiceItemID1` = 0;

-- 253 Bride of the Embalmer: fixed [3560, 6223] -> fixed [6223 Crest of Darkshire], choice [3560 Mantle of Honor, 270044 Darkshire Shoulderguards]
UPDATE `quest_template` SET
  `RewardItem1` = 6223, `RewardAmount1` = 1,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3560, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270044, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 0, `RewardChoiceItemQuantity3` = 0, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 253 AND `RewardItem1` = 3560 AND `RewardItem2` = 6223 AND `RewardChoiceItemID1` = 0;

-- 704 Agmond's Fate: fixed [4980] -> fixed [], choice [4980 Prospector Gloves, 270059 Restorer's Fine Gloves, 270060 Excavator Gauntlets]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4980, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270059, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270060, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 704 AND `RewardItem1` = 4980 AND `RewardChoiceItemID1` = 0;

-- 962 Serpentbloom: fixed [10919] -> fixed [], choice [10919 Apothecary Gloves, 270008 Heat Resistant Mitts, 270009 Safety Boots]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 10919, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270008, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270009, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 962 AND `RewardItem1` = 10919 AND `RewardChoiceItemID1` = 0;

-- 1098 Deathstalkers in Shadowfang: fixed [3324] -> fixed [], choice [3324 Ghostly Mantle, 270023 Tanned Shoulderpads, 270024 Bronzed Shoulderguards]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3324, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270023, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270024, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1098 AND `RewardItem1` = 3324 AND `RewardChoiceItemID1` = 0;

-- 1199 Twilight Falls: fixed [6998, 7000] -> fixed [], choice [6998 Nimbus Boots, 7000 Heartwood Girdle, 270025 Silvered Gauntlets]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 6998, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 7000, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270025, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1199 AND `RewardItem1` = 6998 AND `RewardItem2` = 7000 AND `RewardChoiceItemID1` = 0;

-- 140 Captain Sander's Hidden Treasure: Wowhead Forever lists fixed 2842, 3342, 3344 and 270257 Captain Sander's Locker;
-- our row has 3343 Captain Sander's Booty Bag in slot 4 (all four fixed slots used). Forever seems to have swapped the
-- bag for the locker. Uncomment to swap:
UPDATE `quest_template` SET `RewardItem4` = 270257, `RewardAmount4` = 1 WHERE `ID` = 140 AND `RewardItem4` = 3343;
