-- Quest levels and quest XP as on Wowhead Forever (quest list rows and each quest page's XP multiplier, Olo's page cache of
-- 23 Oct 2026; the game XP Factoruide's collector saw on the beta agrees wherever it has one).
--
-- 1. Quest levels. The server reads the quest level from quest_template_classic_level only; 2026_10_26_25 wrote its
--    Forever levels into quest_classic_level.QuestLevel, which nothing reads, so these quests kept the vanilla level
--    (and its XP). Both tables get the Forever level.
INSERT INTO `quest_template_classic_level` (`ID`, `LevelType`, `QuestLevel`) VALUES
(299, 2, 27), -- Uncovering the Past, was 28 (XP 2300 -> 2200)
(472, 2, 30), -- Fall of Dun Modr, was 25 (XP 1000 -> 1200)
(544, 2, 33), -- Prison Break In, was 34 (XP 3350 -> 3300)
(556, 2, 33), -- Stone Tokens, was 32 (XP 2550 -> 2650)
(557, 2, 33), -- Bracers of Binding, was 34 (XP 2700 -> 2650)
(614, 2, 47), -- The Captain's Chest, was 45 (XP 2900 -> 3150)
(615, 2, 50), -- The Captain's Cutlass, was 45 (XP 390 -> 470)
(620, 2, 50), -- The Monogrammed Sash, was 45 (XP 1950 -> 2350)
(932, 2, 8), -- Twisted Hatred, was 7 (XP 630 -> 700)
(952, 2, 17), -- Grove of the Ancients, was 11 (XP 880 -> 1250)
(1132, 2, 30), -- Fiora Longears, was 20 (XP 780 -> 1200)
(3792, 2, 55), -- Morrowgrain to Feathermoon Stronghold, was 50 (XP 470 -> 560)
(7802, 2, 60), -- A Donation of Wool, was 15 (XP 105 -> 660)
(7803, 2, 60) -- A Donation of Silk, was 30 (XP 610 -> 1650)
ON DUPLICATE KEY UPDATE `QuestLevel` = VALUES(`QuestLevel`);
UPDATE `quest_classic_level` SET `QuestLevel` = CASE `ID` WHEN 299 THEN 27 WHEN 472 THEN 30 WHEN 544 THEN 33 WHEN 556 THEN 33 WHEN 557 THEN 33 WHEN 614 THEN 47 WHEN 615 THEN 50 WHEN 620 THEN 50 WHEN 932 THEN 8 WHEN 952 THEN 17 WHEN 1132 THEN 30 WHEN 3792 THEN 55 WHEN 7802 THEN 60 WHEN 7803 THEN 60 END WHERE `ID` IN (299, 472, 544, 556, 557, 614, 615, 620, 932, 952, 1132, 3792, 7802, 7803);

-- 2. Dungeon quests: Forever multiplies their XP (Wowhead's per-quest XP multiplier, 1.75-2.675; the collector saw the
--    multiplied XP on all 21 of these it has). Our rows had multiplier 1.
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 166 AND `RewardXPMultiplier` = 1; -- The Defias Brotherhood: 2600 -> 6200
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 214 AND `RewardXPMultiplier` = 1; -- Red Silk Bandanas: 1250 -> 2950
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 373 AND `RewardXPMultiplier` = 1; -- The Unsent Letter: 870 -> 2050
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 377 AND `RewardXPMultiplier` = 1; -- Crime and Punishment: 2100 -> 4400
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 378 AND `RewardXPMultiplier` = 1; -- The Fury Runs Deep: 2750 -> 5800
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 386 AND `RewardXPMultiplier` = 1; -- What Comes Around...: 2000 -> 4200
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 387 AND `RewardXPMultiplier` = 1; -- Quell The Uprising: 2650 -> 5550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 388 AND `RewardXPMultiplier` = 1; -- The Color of Blood: 2650 -> 5550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 391 AND `RewardXPMultiplier` = 1; -- The Stockade Riots: 2350 -> 4950
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.75 WHERE `ID` = 704 AND `RewardXPMultiplier` = 1; -- Agmond's Fate: 2850 -> 5000
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 709 AND `RewardXPMultiplier` = 1; -- Solution to Doom: 3150 -> 6150
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 914 AND `RewardXPMultiplier` = 1; -- Leaders of the Fang: 2200 -> 4300
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 959 AND `RewardXPMultiplier` = 1; -- Trouble at the Docks: 1350 -> 2650
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 962 AND `RewardXPMultiplier` = 1; -- Serpentbloom: 1700 -> 3300
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 971 AND `RewardXPMultiplier` = 1; -- Knowledge in the Deeps: 2750 -> 6550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.675 WHERE `ID` = 1013 AND `RewardXPMultiplier` = 1; -- The Book of Ur: 2100 -> 5600
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.675 WHERE `ID` = 1014 AND `RewardXPMultiplier` = 1; -- Arugal Must Die: 3300 -> 8850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1049 AND `RewardXPMultiplier` = 1; -- Compendium of the Fallen: 3550 -> 8450
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.5 WHERE `ID` = 1050 AND `RewardXPMultiplier` = 1; -- Mythology of the Titans: 3550 -> 8900
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.675 WHERE `ID` = 1098 AND `RewardXPMultiplier` = 1; -- Deathstalkers in Shadowfang: 2000 -> 5350
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1101 AND `RewardXPMultiplier` = 1; -- The Crone of the Kraul: 3350 -> 7950
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 1102 AND `RewardXPMultiplier` = 1; -- A Vengeful Fate: 4050 -> 8500
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 1109 AND `RewardXPMultiplier` = 1; -- Going, Going, Guano!: 3300 -> 6950
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1113 AND `RewardXPMultiplier` = 1; -- Hearts of Zeal: 3300 -> 7850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1142 AND `RewardXPMultiplier` = 1; -- Mortality Wanes: 3050 -> 7250
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1144 AND `RewardXPMultiplier` = 1; -- Willix the Importer: 3050 -> 7250
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1160 AND `RewardXPMultiplier` = 1; -- Test of Lore: 2100 -> 5000
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1198 AND `RewardXPMultiplier` = 1; -- In Search of Thaelrid: 2400 -> 5700
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1199 AND `RewardXPMultiplier` = 1; -- Twilight Falls: 2550 -> 6050
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1200 AND `RewardXPMultiplier` = 1; -- Blackfathom Villainy: 3300 -> 7850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 1221 AND `RewardXPMultiplier` = 1; -- Blueleaf Tubers: 2100 -> 5000
UPDATE `quest_template` SET `RewardXPMultiplier` = 2 WHERE `ID` = 1442 AND `RewardXPMultiplier` = 1; -- Seeking the Kor Gem: 435 -> 875
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 1486 AND `RewardXPMultiplier` = 1; -- Deviate Hides: 1600 -> 3100
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 1487 AND `RewardXPMultiplier` = 1; -- Deviate Eradication: 2050 -> 4000
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 1491 AND `RewardXPMultiplier` = 1; -- Smart Drinks: 1350 -> 2650
UPDATE `quest_template` SET `RewardXPMultiplier` = 2 WHERE `ID` = 1740 AND `RewardXPMultiplier` = 1; -- The Orb of Soran'ruk: 2550 -> 5100
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.75 WHERE `ID` = 1806 AND `RewardXPMultiplier` = 1; -- The Test of Righteousness: 2200 -> 3850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2 WHERE `ID` = 1951 AND `RewardXPMultiplier` = 1; -- Rituals of Power: 3150 -> 6300
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 2040 AND `RewardXPMultiplier` = 1; -- Underground Assault: 1550 -> 3700
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 2841 AND `RewardXPMultiplier` = 1; -- Rig Wars: 2750 -> 6550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 2904 AND `RewardXPMultiplier` = 1; -- A Fine Mess: 2450 -> 5800
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2922 AND `RewardXPMultiplier` = 1; -- Save Techbot's Brain!: 2650 -> 4750
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2924 AND `RewardXPMultiplier` = 1; -- Essential Artificials: 3050 -> 5500
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2926 AND `RewardXPMultiplier` = 1; -- Gnogaine: 2200 -> 3950
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2928 AND `RewardXPMultiplier` = 1; -- Gyrodrillmatic Excavationators: 2450 -> 4400
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2929 AND `RewardXPMultiplier` = 1; -- The Grand Betrayal: 2750 -> 4950
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2930 AND `RewardXPMultiplier` = 1; -- Data Rescue: 3650 -> 6550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 2945 AND `RewardXPMultiplier` = 1; -- Grime-Encrusted Ring: 2700 -> 6400
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2947 AND `RewardXPMultiplier` = 1; -- Return of the Ring: 2700 -> 4850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 2949 AND `RewardXPMultiplier` = 1; -- Return of the Ring: 2700 -> 6400
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.8 WHERE `ID` = 2962 AND `RewardXPMultiplier` = 1; -- The Only Cure is More Green Glow: 2450 -> 4400
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 3523 AND `RewardXPMultiplier` = 1; -- Scourge of the Downs: 285 -> 675
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5722 AND `RewardXPMultiplier` = 1; -- Searching for the Lost Satchel: 880 -> 1800
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5723 AND `RewardXPMultiplier` = 1; -- Testing an Enemy's Strength: 1050 -> 2150
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5724 AND `RewardXPMultiplier` = 1; -- Returning the Lost Satchel: 1450 -> 2950
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5725 AND `RewardXPMultiplier` = 1; -- The Power to Destroy...: 1450 -> 2950
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5728 AND `RewardXPMultiplier` = 1; -- Hidden Enemies: 1150 -> 2350
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.025 WHERE `ID` = 5761 AND `RewardXPMultiplier` = 1; -- Slaying the Beast: 1150 -> 2350
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 6521 AND `RewardXPMultiplier` = 1; -- An Unholy Alliance: 3500 -> 7350
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.1 WHERE `ID` = 6522 AND `RewardXPMultiplier` = 1; -- An Unholy Alliance: 2800 -> 5900
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 6561 AND `RewardXPMultiplier` = 1; -- Blackfathom Villainy: 3300 -> 7850
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 6565 AND `RewardXPMultiplier` = 1; -- Allegiance to the Old Gods: 2650 -> 6300
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 6626 AND `RewardXPMultiplier` = 1; -- A Host of Evil: 3450 -> 8200
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 6921 AND `RewardXPMultiplier` = 1; -- Amongst the Ruins: 2750 -> 6550
UPDATE `quest_template` SET `RewardXPMultiplier` = 2.375 WHERE `ID` = 6922 AND `RewardXPMultiplier` = 1; -- Baron Aquanis: 3050 -> 7250
UPDATE `quest_template` SET `RewardXPMultiplier` = 1.95 WHERE `ID` = 6981 AND `RewardXPMultiplier` = 1; -- The Glowing Shard: 2650 -> 5150

-- 3. XP difficulty that gives Wowhead Forever's XP (no collector figure contradicts these).
UPDATE `quest_template` SET `RewardXPDifficulty` = 1 WHERE `ID` = 100 AND `RewardXPDifficulty` = 0; -- Call of Water: 0 -> 185
UPDATE `quest_template` SET `RewardXPDifficulty` = 5 WHERE `ID` = 606 AND `RewardXPDifficulty` = 2; -- Scaring Shaky: 820 -> 3300
UPDATE `quest_template` SET `RewardXPDifficulty` = 3 WHERE `ID` = 635 AND `RewardXPDifficulty` = 1; -- Crystal in the Mountains: 275 -> 1350
UPDATE `quest_template` SET `RewardXPDifficulty` = 5 WHERE `ID` = 745 AND `RewardXPDifficulty` = 4; -- Sharing the Land: 530 -> 700
