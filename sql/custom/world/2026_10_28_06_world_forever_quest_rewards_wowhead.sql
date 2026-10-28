-- Quest rewards as Wowhead Forever lists them (quest list rows: fixed and choice items with counts; Olo's page cache of
-- 23 Oct 2026, checked on the live pages for 202, 1101 and 1203 on 28 Oct). QuestieDB's item data lacks most of these
-- Forever items, so the 28 Oct pass (2026_10_28_03) missed them. Where Wowhead prints an item both under "You will also
-- receive" and among the choices (682), it is a choice only (the same page quirk as 962 and 1199). Each statement sets
-- the whole reward block and is guarded on the row's current rewards: it runs once and is safe to run again.

-- 98 The Legend of Stalvan: fixed [] choice [2044 Crescent of Forlorn Spirits, 2043 Ring of Forlorn Spirits] -> fixed [] choice [2044 Crescent of Forlorn Spirits, 2043 Ring of Forlorn Spirits, 270053 Ring of Ruin]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 2044, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 2043, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270053, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 98 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 2044 AND `RewardChoiceItemID2` = 2043 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 202 Colonel Kurzen: fixed [4127 Shrapnel Blaster] choice [] -> fixed [] choice [4127 Shrapnel Blaster, 270070 Madman's Blade, 270071 Dark Hearted Hammer]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4127, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270070, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270071, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 202 AND `RewardItem1` = 4127 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 208 Big Game Hunter: fixed [] choice [17686 Master Hunter's Bow, 17687 Master Hunter's Rifle] -> fixed [] choice [17686 Master Hunter's Bow, 17687 Master Hunter's Rifle, 270084 Master Hunter's Wand, 270085 Master Hunter's Spellsword]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 17686, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 17687, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270084, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 270085, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 208 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 17686 AND `RewardChoiceItemID2` = 17687 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 231 A Daughter's Love: fixed [2000 Archeus] choice [] -> fixed [2000 Archeus, 270051 Ladimore Heirloom Ring] choice []
UPDATE `quest_template` SET
  `RewardItem1` = 2000, `RewardAmount1` = 1,
  `RewardItem2` = 270051, `RewardAmount2` = 1,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 0, `RewardChoiceItemQuantity1` = 0, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 0, `RewardChoiceItemQuantity2` = 0, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 0, `RewardChoiceItemQuantity3` = 0, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 231 AND `RewardItem1` = 2000 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 296 Ormer's Revenge: fixed [3682 Recipe: Curiously Tasty Omelet] choice [3493 Raptor's End, 3566 Raptorbane Armor, 5246 Excavation Rod] -> fixed [3682 Recipe: Curiously Tasty Omelet] choice [3493 Raptor's End, 3566 Raptorbane Armor, 5246 Excavation Rod, 270039 Raptorclaw Shoulders]
UPDATE `quest_template` SET
  `RewardItem1` = 3682, `RewardAmount1` = 1,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3493, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 3566, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 5246, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 270039, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 296 AND `RewardItem1` = 3682 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 3493 AND `RewardChoiceItemID2` = 3566 AND `RewardChoiceItemID3` = 5246 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 304 A Grim Task: fixed [] choice [2916 Gold Lion Shield, 2917 Tranquil Ring] -> fixed [] choice [2916 Gold Lion Shield, 2917 Tranquil Ring, 270050 Fury Ring]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 2916, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 2917, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270050, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 304 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 2916 AND `RewardChoiceItemID2` = 2917 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 378 The Fury Runs Deep: fixed [] choice [3562 Belt of Vindication, 1264 Headbasher] -> fixed [] choice [3562 Belt of Vindication, 1264 Headbasher, 270036 Magistrate's Pantaloons]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3562, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 1264, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270036, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 378 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 3562 AND `RewardChoiceItemID2` = 1264 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 504 Crushridge Warmongers: fixed [] choice [5249 Burning Sliver, 3763 Lunar Buckler] -> fixed [] choice [5249 Burning Sliver, 3763 Lunar Buckler, 270072 Alterac Assassin's Gloves]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 5249, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 3763, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270072, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 504 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 5249 AND `RewardChoiceItemID2` = 3763 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 515 Elixir of Agony: fixed [] choice [3749 High Apothecary Cloak, 3747 Meditative Sash] -> fixed [] choice [3749 High Apothecary Cloak, 3747 Meditative Sash, 276727 Glistening Stompers]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3749, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 3747, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 276727, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 515 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 3749 AND `RewardChoiceItemID2` = 3747 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 547 Humbert's Sword: fixed [] choice [3750 Ribbed Breastplate, 3751 Mercenary Leggings] -> fixed [] choice [3750 Ribbed Breastplate, 3751 Mercenary Leggings, 270046 Ethereal Mantle]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 3750, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 3751, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270046, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 547 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 3750 AND `RewardChoiceItemID2` = 3751 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 613 Cracking Maury's Foot: fixed [4129 Collection Plate] choice [] -> fixed [] choice [4129 Collection Plate, 270090 Beggar's Leggings, 270091 Seadog's Battered Cuffs]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4129, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270090, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270091, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 613 AND `RewardItem1` = 4129 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 679 Call to Arms: fixed [] choice [9520 Silent Hunter, 9521 Skullsplitter] -> fixed [] choice [9520 Silent Hunter, 9521 Skullsplitter, 270069 Mindslicer]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 9520, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 9521, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270069, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 679 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 9520 AND `RewardChoiceItemID2` = 9521 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 680 The Real Threat: fixed [] choice [4976 Mistspray Kilt, 4977 Sword of Hammerfall] -> fixed [] choice [4976 Mistspray Kilt, 4977 Sword of Hammerfall, 270079 Witchdoctor's Dress]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4976, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 4977, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270079, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 680 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 4976 AND `RewardChoiceItemID2` = 4977 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 682 Stromgarde Badges: fixed [4741 Stromgarde Cavalry Leggings] choice [] -> fixed [] choice [4741 Stromgarde Cavalry Leggings, 270056 Stromgarde Librarian Leggings, 270057 Stromgarde Operative Pants]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4741, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270056, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270057, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 682 AND `RewardItem1` = 4741 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 684 Wanted!  Marez Cowl: fixed [4744 Arcane Runed Bracers] choice [] -> fixed [] choice [4744 Arcane Runed Bracers, 270067 Wild Headdress, 270068 Stromgarde Bracers]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4744, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270067, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270068, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 684 AND `RewardItem1` = 4744 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 685 Wanted!  Otto and Falconcrest: fixed [] choice [5247 Rod of Sorrow, 4745 War Rider Bracers] -> fixed [] choice [5247 Rod of Sorrow, 4745 War Rider Bracers, 270080 Arathi Armbands]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 5247, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 4745, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270080, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 685 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 5247 AND `RewardChoiceItemID2` = 4745 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 709 Solution to Doom: fixed [4746 Doomsayer's Robe] choice [] -> fixed [] choice [4746 Doomsayer's Robe, 270074 Doomcaller's Pants, 270075 Fists of Impending Doom]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4746, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270074, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270075, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 709 AND `RewardItem1` = 4746 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 1012 Insane Druids: fixed [5813 Emil's Brand] choice [] -> fixed [] choice [5813 Emil's Brand, 270048 Uthil's Cudgel, 270049 Mavoris's Warstaff]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 5813, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270048, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 270049, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1012 AND `RewardItem1` = 5813 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 1101 The Crone of the Kraul: fixed [3041 "Mage-Eye" Blunderbuss] choice [4197 Berylline Pads, 6742 Stonefist Girdle, 6725 Marbled Buckler] -> fixed [274084 Quilboar Blaster] choice [4197 Berylline Pads, 6742 Stonefist Girdle, 6725 Marbled Buckler]
UPDATE `quest_template` SET
  `RewardItem1` = 274084, `RewardAmount1` = 1,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4197, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 6742, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 6725, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1101 AND `RewardItem1` = 3041 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 4197 AND `RewardChoiceItemID2` = 6742 AND `RewardChoiceItemID3` = 6725 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 1102 A Vengeful Fate: fixed [] choice [4197 Berylline Pads, 6742 Stonefist Girdle, 6725 Marbled Buckler] -> fixed [274084 Quilboar Blaster] choice [4197 Berylline Pads, 6742 Stonefist Girdle, 6725 Marbled Buckler]
UPDATE `quest_template` SET
  `RewardItem1` = 274084, `RewardAmount1` = 1,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 4197, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 6742, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 6725, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1102 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 4197 AND `RewardChoiceItemID2` = 6742 AND `RewardChoiceItemID3` = 6725 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 1203 Jarl Needs a Blade: fixed [9622 Reedknot Ring, 5016 Artisan's Trousers] choice [] -> fixed [5016 Artisan's Trousers] choice [9622 Reedknot Ring, 270052 Swamp Ring]
UPDATE `quest_template` SET
  `RewardItem1` = 5016, `RewardAmount1` = 1,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 9622, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 270052, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 0, `RewardChoiceItemQuantity3` = 0, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1203 AND `RewardItem1` = 9622 AND `RewardItem2` = 5016 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 1371 Gizmo for Warug: fixed [] choice [] -> fixed [] choice [269903 Schematic: Hoof-Shaped Foot Pedal, 269901 Schematic: Centaur Banner Deployment Device]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 269903, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 269901, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 0, `RewardChoiceItemQuantity3` = 0, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 0, `RewardChoiceItemQuantity4` = 0, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 1371 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 0 AND `RewardChoiceItemID2` = 0 AND `RewardChoiceItemID3` = 0 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 5088 Arikara: fixed [] choice [15464 Brute Hammer, 15465 Stingshot Wand, 15466 Clink Shield] -> fixed [] choice [15464 Brute Hammer, 15465 Stingshot Wand, 15466 Clink Shield, 270037 Smouldering Leggings]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 15464, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 15465, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 15466, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 270037, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 5088 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 15464 AND `RewardChoiceItemID2` = 15465 AND `RewardChoiceItemID3` = 15466 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 5943 Gizelton Caravan: fixed [] choice [15691 Sidegunner Shottie, 15692 Kodo Brander, 15695 Studded Ring Shield] -> fixed [] choice [15691 Sidegunner Shottie, 15692 Kodo Brander, 15695 Studded Ring Shield, 270065 Farseer's Forgotten Shield]
UPDATE `quest_template` SET
  `RewardItem1` = 0, `RewardAmount1` = 0,
  `RewardItem2` = 0, `RewardAmount2` = 0,
  `RewardItem3` = 0, `RewardAmount3` = 0,
  `RewardItem4` = 0, `RewardAmount4` = 0,
  `RewardChoiceItemID1` = 15691, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 15692, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 15695, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0,
  `RewardChoiceItemID4` = 270065, `RewardChoiceItemQuantity4` = 1, `RewardChoiceItemDisplayID4` = 0,
  `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0, `RewardChoiceItemDisplayID5` = 0,
  `RewardChoiceItemID6` = 0, `RewardChoiceItemQuantity6` = 0, `RewardChoiceItemDisplayID6` = 0
WHERE `ID` = 5943 AND `RewardItem1` = 0 AND `RewardItem2` = 0 AND `RewardItem3` = 0 AND `RewardItem4` = 0 AND `RewardChoiceItemID1` = 15691 AND `RewardChoiceItemID2` = 15692 AND `RewardChoiceItemID3` = 15695 AND `RewardChoiceItemID4` = 0 AND `RewardChoiceItemID5` = 0 AND `RewardChoiceItemID6` = 0;

-- 97275 Whuut's the Rush: fixed [] choice [3385 Lesser Mana Potion, 929 Healing Potion] -> fixed [] choice [3385 Lesser Mana Potion x2, 929 Healing Potion x2]
UPDATE `quest_template` SET `RewardChoiceItemQuantity1` = 2, `RewardChoiceItemQuantity2` = 2 WHERE `ID` = 97275;
