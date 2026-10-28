-- DRAFT. Forever quests the world DB lacks (QuestieDB Forever, read 28 Oct 2026), only those whose quest givers and
-- objective sources already exist and are spawned here. Rewards fixed/choice from Wowhead Forever; levels and XP
-- from QuestieDB / Factoruide's forever.sqlite (Wowhead XP, matched to a QuestXP difficulty). Not sourced, left at the
-- TrinityCore defaults: QuestDescription, AreaDescription, QuestCompletionLog, quest_request_items /
-- quest_offer_reward texts, money (RewardMoneyDifficulty 0, no quest_classic_money row), Flags 0 (not sharable),
-- POIs. quest_objectives IDs use the private range 990000000 + quest * 10 + index. Safe to run again.

DELETE FROM `quest_template` WHERE `ID` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `quest_template_addon` WHERE `ID` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `quest_objectives` WHERE `QuestID` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `quest_template_classic_level` WHERE `ID` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `quest_classic_level` WHERE `ID` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `creature_queststarter` WHERE `quest` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);
DELETE FROM `creature_questender` WHERE `quest` IN (86574,87288,98298,97275,97914,97926,98189,98190,98191,95217,96875);

-- 86574 Stonetalon Supply Run (Alliance, level 24/20): Night Elf Courier -> Innkeeper Faralia; reward fixed 281251 Courier's Wristguards. StartItem 277915 Stonetalon Supply Satchel (Wowhead: provided). Loot for 277917 already in creature_loot_template (4012-4015, 40%).
-- 87288 Soft Saber Pelts (Alliance, level 7/4): Aldia -> Aldia; reward fixed 280088 Soft Saber Sack. Loot for 280087 already on Nightsaber (2042, 40%). Aldia (3608) gets the quest-giver flag.
-- 98298 Arugal's Folly (Horde, level 16/9): Dalar Dawnweaver -> Dalar Dawnweaver; reward fixed 281315 Foulfur Gloves. PrevQuestID 99 from the Wowhead series only (entry 6 of 6 after 99); QuestieDB has no prerequisite. Loot for 280375 already on 1924 (40%).
-- 97275 Whuut's the Rush (Horde, level 15/9): Yelmak -> Whuut; reward choice 3385 Lesser Mana Potion, choice 929 Healing Potion. Delivery; Yelmak (3347) and Whuut (11046) get the quest-giver flag. No prerequisite known.
-- 97914 Expanding Horizons (Alliance, level 17/13): Thundris Windweaver -> Baros Alexston; reward no item. Not in the 116 (no item reward) but the prerequisite of 97926. XP: Wowhead 625, QuestXP level 17 difficulty 3 = 630.
-- 97926 Making Do (Alliance, level 17/13): Baros Alexston -> Thundris Windweaver; reward fixed 282286 Shipwright's Satchel. Delivery.
-- 98189 Return the Statuette (Alliance, level 25/20): Karl Boran -> Captain Stoutfist; reward no item. Not in the 116 (no item reward); link in the Wowhead series 286 -> 98189 -> 98190 -> 98191.
-- 98190 A Friend of the Family (Alliance, level 25/20): Captain Stoutfist -> Highlord Bolvar Fordragon; reward no item. Not in the 116 (no item reward). XP unknown (no source); difficulty 1 assumed like 98189/98191 (200 XP at 25).
-- 98191 A Friend of the Family (Alliance, level 25/20): Highlord Bolvar Fordragon -> Lord Grayson Shadowbreaker; reward choice 281296 Knight's Lance, choice 281297 Royal Dagger. Delivery.
-- 95217 The Quarry's Smith (Alliance, level 10/7): Frast Dokner -> Frast Dokner; reward fixed 277673 Plans: Rough Copper Chain Boots. Needs a new drop: 267416 Toughened Boar Hide from Scarred Crag Boar (1689), chance UNKNOWN, 40% like the other Forever quest items here. RequiredSkill Blacksmithing 1 from QuestieDB only (Wowhead lists none). Frast Dokner (1698) gets the quest-giver flag.
-- 96875 Beasts of Thunder Ridge (Horde, level 10/6): Kamari -> Kamari; reward fixed 277716 Pattern: Light Leather Reagent Bag. Needs a new drop: 275720 Rough Lizard Hide from Thunder Lizard (3130) and Lightning Hide (3131), chance UNKNOWN, 40% like the other Forever quest items here. RequiredSkill Leatherworking 1 from QuestieDB only. XP: Wowhead 850, QuestXP level 10 difficulty 5 = 840.
INSERT INTO `quest_template` (`ID`,`QuestType`,`QuestSortID`,`RewardXPDifficulty`,`RewardXPMultiplier`,`RewardMoneyDifficulty`,`RewardMoneyMultiplier`,`StartItem`,`Flags`,`RewardItem1`,`RewardAmount1`,`RewardItem2`,`RewardAmount2`,`RewardItem3`,`RewardAmount3`,`RewardItem4`,`RewardAmount4`,`RewardChoiceItemID1`,`RewardChoiceItemQuantity1`,`RewardChoiceItemDisplayID1`,`RewardChoiceItemID2`,`RewardChoiceItemQuantity2`,`RewardChoiceItemDisplayID2`,`RewardChoiceItemID3`,`RewardChoiceItemQuantity3`,`RewardChoiceItemDisplayID3`,`RewardChoiceItemID4`,`RewardChoiceItemQuantity4`,`RewardChoiceItemDisplayID4`,`RewardChoiceItemID5`,`RewardChoiceItemQuantity5`,`RewardChoiceItemDisplayID5`,`RewardChoiceItemID6`,`RewardChoiceItemQuantity6`,`RewardChoiceItemDisplayID6`,`RewardFactionID1`,`RewardFactionValue1`,`RewardFactionOverride1`,`AcceptedSoundKitID`,`CompleteSoundKitID`,`AllowableRaces`,`LogTitle`,`LogDescription`,`QuestDescription`,`AreaDescription`,`PortraitGiverText`,`PortraitGiverName`,`PortraitTurnInText`,`PortraitTurnInName`,`QuestCompletionLog`,`VerifiedBuild`) VALUES
(86574,2,406,5,1,0,1,277915,0,281251,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'Stonetalon Supply Run','Collect 6 Stonetalon Supplies from Pridewing Wyverns and deliver them to Innkeeper Faralia.','','','','','','','',0),
(87288,2,141,5,1,0,1,0,0,280088,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,69,0,10000,890,878,6130900294268439629,'Soft Saber Pelts','Bring 6 Soft Nightsaber Pelts to Aldia.','','','','','','','',0),
(98298,2,130,5,1,0,1,0,0,281315,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,0,10000,890,878,12261800583900083122,'Arugal\'s Folly','Bring 6 Worgen Bits to Dalar Dawnweaver in The Sepulcher.','','','','','','','',0),
(97275,2,1637,4,1,0,1,277566,0,0,0,0,0,0,0,0,0,3385,1,0,929,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,12261800583900083122,'Whuut\'s the Rush','Give Yelmak\'s Potion to Whuut.','','','','','','','',0),
(97914,2,1519,3,1,0,1,279045,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'Expanding Horizons','Bring Thundris\' Letter to Baros Alexston in Stormwind.$B$BThundris mentioned a boat in Auberdine that can take you there.','','','','','','','',0),
(97926,2,148,1,1,0,1,279048,0,282286,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'Making Do','Bring Alexston\'s Reply to Thundris Windweaver in Auberdine in Darkshore.','','','','','','','',0),
(98189,2,11,1,1,0,1,279589,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'Return the Statuette','Bring the Menethil Statuette to Captain Stoutfist in Menethil Keep.','','','','','','','',0),
(98190,2,1519,1,1,0,1,279595,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'A Friend of the Family','Bring the Menethil Statuette to Highlord Bolvar Fordragon in Stormwind Keep.','','','','','','','',0),
(98191,2,1519,1,1,0,1,279598,0,0,0,0,0,0,0,0,0,281296,1,0,281297,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,878,6130900294268439629,'A Friend of the Family','Bring the Menethil Statuette to Lord Grayson Shadowbreaker in the Cathedral of Light.','','','','','','','',0),
(95217,2,1,4,1,0,1,0,0,277673,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,0,7500,890,878,6130900294268439629,'The Quarry\'s Smith','Collect 12 Copper Bars and 4 Toughened Boar Hides and deliver them to Frast Dokner.','','','','','','','',0),
(96875,2,14,5,1,0,1,0,0,277716,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,0,7500,890,878,12261800583900083122,'Beasts of Thunder Ridge','Kamari in Orgrimmar wants you to collect 5 Rough Lizard Hide from beasts in Thunder Ridge, along with 2 Cured Light Hides.','','','','','','','',0);

INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`,`NextQuestID`,`RequiredSkillID`,`RequiredSkillPoints`,`ProvidedItemCount`) VALUES
(86574,0,0,0,0,1),
(87288,0,0,0,0,0),
(98298,99,0,0,0,0),
(97275,0,0,0,0,1),
(97914,0,97926,0,0,1),
(97926,97914,0,0,0,1),
(98189,286,0,0,0,1),
(98190,98189,0,0,0,1),
(98191,98190,0,0,0,1),
(95217,0,0,164,1,0),
(96875,0,0,165,1,0);

INSERT INTO `quest_objectives` (`ID`,`QuestID`,`Type`,`Order`,`StorageIndex`,`ObjectID`,`Amount`,`ConditionalAmount`,`Flags`,`Flags2`,`Visible`,`Description`,`VerifiedBuild`) VALUES
(990865740,86574,1,0,0,277917,6,0,0,0,1,'',0),  -- Stonetalon Supply Bundle x6
(990872880,87288,1,0,0,280087,6,0,0,0,1,'',0),  -- Soft Nightsaber Pelt x6
(990982980,98298,1,0,0,280375,6,0,0,0,1,'',0),  -- Worgen Bits x6
(990972750,97275,1,0,0,277566,1,0,0,0,1,'',0),  -- Yelmak's Potion x1
(990979140,97914,1,0,0,279045,1,0,0,0,1,'',0),  -- Thundris' Letter x1
(990979260,97926,1,0,0,279048,1,0,0,0,1,'',0),  -- Alexston's Reply x1
(990981890,98189,1,0,0,279589,1,0,0,0,1,'',0),  -- Menethil Statuette x1
(990981900,98190,1,0,0,279595,1,0,0,0,1,'',0),  -- Menethil Statuette x1
(990981910,98191,1,0,0,279598,1,0,0,0,1,'',0),  -- Menethil Statuette x1
(990952170,95217,1,0,0,2840,12,0,0,0,1,'',0),  -- Copper Bar x12
(990952171,95217,1,1,1,267416,4,0,0,0,1,'',0),  -- Toughened Boar Hide x4
(990968750,96875,1,0,0,275720,5,0,0,0,1,'',0),  -- Rough Lizard Hide x5
(990968751,96875,1,1,1,4231,2,0,0,0,1,'',0);  -- Cured Light Hide x2

INSERT INTO `quest_template_classic_level` (`ID`,`LevelType`,`QuestLevel`) VALUES
(86574,2,24),
(87288,2,7),
(98298,2,16),
(97275,2,15),
(97914,2,17),
(97926,2,17),
(98189,2,25),
(98190,2,25),
(98191,2,25),
(95217,2,10),
(96875,2,10);
INSERT INTO `quest_classic_level` (`ID`,`QuestLevel`,`MinLevel`,`MaxLevel`) VALUES
(86574,24,20,0),
(87288,7,4,0),
(98298,16,9,0),
(97275,15,9,0),
(97914,17,13,0),
(97926,17,13,0),
(98189,25,20,0),
(98190,25,20,0),
(98191,25,20,0),
(95217,10,7,0),
(96875,10,6,0);

INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES
(268925,86574),
(3608,87288),
(1938,98298),
(3347,97275),
(3649,97914),
(1646,97926),
(1242,98189),
(2104,98190),
(1748,98191),
(1698,95217),
(5811,96875);
INSERT INTO `creature_questender` (`id`,`quest`) VALUES
(16458,86574),
(3608,87288),
(1938,98298),
(11046,97275),
(1646,97914),
(3649,97926),
(2104,98189),
(1748,98190),
(928,98191),
(1698,95217),
(5811,96875);

-- quest-giver flag (npcflag 2) on NPCs that lack it
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 3608;  -- Aldia (quest 87288)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 3347;  -- Yelmak (quest 97275)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 11046;  -- Whuut (quest 97275)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 1698;  -- Frast Dokner (quest 95217)

-- New quest-item drops. CHANCE UNKNOWN: 40% follows the existing rows for Forever quest items whose drop rate was not
-- seen (creature_loot_template comment "quest item of the creature query"). Loot entry = creature entry (LootID here).
DELETE FROM `creature_loot_template` WHERE `Entry` = 1689 AND `Item` = 267416;
DELETE FROM `creature_loot_template` WHERE `Entry` = 3130 AND `Item` = 275720;
DELETE FROM `creature_loot_template` WHERE `Entry` = 3131 AND `Item` = 275720;
INSERT INTO `creature_loot_template` (`Entry`,`ItemType`,`Item`,`Chance`,`QuestRequired`,`LootMode`,`GroupId`,`MinCount`,`MaxCount`,`Comment`) VALUES
(1689,0,267416,40,1,1,0,1,1,'Forever quest item, chance unknown (questfix draft)'),
(3130,0,275720,40,1,1,0,1,1,'Forever quest item, chance unknown (questfix draft)'),
(3131,0,275720,40,1,1,0,1,1,'Forever quest item, chance unknown (questfix draft)');
-- quest items the creature query lists (Idx after the existing rows)
DELETE FROM `creature_questitem` WHERE (`CreatureEntry`,`DifficultyID`,`Idx`) IN ((1689,0,0),(3130,0,1),(3131,0,1));
INSERT INTO `creature_questitem` (`CreatureEntry`,`DifficultyID`,`Idx`,`ItemId`,`VerifiedBuild`) VALUES
(1689,0,0,267416,0),
(3130,0,1,275720,0),
(3131,0,1,275720,0);
