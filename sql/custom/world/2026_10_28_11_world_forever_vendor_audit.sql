-- WoW Forever vendor audit (28 Oct): items Forever vendors sell that our world DB lacks. Adds only, no removals.
-- Sources: Wowhead Forever. Its Sells tab is script data WebFetch cannot read, so per-NPC lists come from
-- wow-professions.com/forever/npc/<name> (Forever-checked lists whose items link to wowhead.com/forever/item=ID), and
-- for single-vendor pets/novelties from the Wowhead Forever item page ("New in Forever", sold only in that NPC's zone).
-- Prices, reputation and skill requirements come from item data. Mote of Magic (2026_10_28_10) at the city suppliers is not repeated.

-- == Empty vendors (Dalaran/city tailoring recipe vendors) ==
-- 251973 Dominique Stefano (wowhead.com/forever/npc=251973; wow-professions.com/forever/npc/dominique-stefano): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag, 2325 Black Dye, 2324 Bleach, 6260 Blue Dye, 2320 Coarse Thread, 2321 Fine Thread, 4340 Gray Dye, 2605 Green Dye, 8343 Heavy Silken Thread, 6261 Orange Dye, 10290 Pink Dye, 4342 Purple Dye, 2604 Red Dye, 14341 Rune Thread, 4291 Silken Thread, 4341 Yellow Dye
DELETE FROM `npc_vendor` WHERE `entry`=251973 AND `item` IN (253665,253668,2325,2324,6260,2320,2321,4340,2605,8343,6261,10290,4342,2604,14341,4291,4341);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(251973,0,253665,0,0,0,1,0,0,0),(251973,0,253668,0,0,0,1,0,0,0),(251973,0,2325,0,0,0,1,0,0,0),(251973,0,2324,0,0,0,1,0,0,0),
(251973,0,6260,0,0,0,1,0,0,0),(251973,0,2320,0,0,0,1,0,0,0),(251973,0,2321,0,0,0,1,0,0,0),(251973,0,4340,0,0,0,1,0,0,0),
(251973,0,2605,0,0,0,1,0,0,0),(251973,0,8343,0,0,0,1,0,0,0),(251973,0,6261,0,0,0,1,0,0,0),(251973,0,10290,0,0,0,1,0,0,0),
(251973,0,4342,0,0,0,1,0,0,0),(251973,0,2604,0,0,0,1,0,0,0),(251973,0,14341,0,0,0,1,0,0,0),(251973,0,4291,0,0,0,1,0,0,0),
(251973,0,4341,0,0,0,1,0,0,0);
-- 258415 Angela Ward (wowhead.com/forever/npc=258415; wow-professions.com/forever/npc/angela-ward): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=258415 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(258415,0,253665,0,0,0,1,0,0,0),(258415,0,253668,0,0,0,1,0,0,0);
-- 258463 Darah (wowhead.com/forever/npc=258463; wow-professions.com/forever/npc/darah): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=258463 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(258463,0,253665,0,0,0,1,0,0,0),(258463,0,253668,0,0,0,1,0,0,0);
-- 258568 Antonio Bolero (wowhead.com/forever/npc=258568; wow-professions.com/forever/npc/antonio-bolero): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=258568 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(258568,0,253665,0,0,0,1,0,0,0),(258568,0,253668,0,0,0,1,0,0,0);

-- == Tailoring supplies: Forever reagent bag patterns ==
-- 1347 Alexandra Bolero (wowhead.com/forever/npc=1347; wow-professions.com/forever/npc/alexandra-bolero): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=1347 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1347,0,253665,0,0,0,1,0,0,0),(1347,0,253668,0,0,0,1,0,0,0);
-- 1454 Jennabink Powerseam (wowhead.com/forever/npc=1454; wow-professions.com/forever/npc/jennabink-powerseam): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=1454 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1454,0,253665,0,0,0,1,0,0,0),(1454,0,253668,0,0,0,1,0,0,0);
-- 1474 Rann Flamespinner (wowhead.com/forever/npc=1474; wow-professions.com/forever/npc/rann-flamespinner): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=1474 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1474,0,253665,0,0,0,1,0,0,0),(1474,0,253668,0,0,0,1,0,0,0);
-- 1672 Lohgan Eva (wowhead.com/forever/npc=1672; wow-professions.com/forever/npc/lohgan-eva): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=1672 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1672,0,253665,0,0,0,1,0,0,0),(1672,0,253668,0,0,0,1,0,0,0);
-- 2668 Danielle Zipstitch (wowhead.com/forever/npc=2668; wow-professions.com/forever/npc/danielle-zipstitch): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=2668 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2668,0,253665,0,0,0,1,0,0,0),(2668,0,253668,0,0,0,1,0,0,0);
-- 2669 Sheri Zipstitch (wowhead.com/forever/npc=2669; wow-professions.com/forever/npc/sheri-zipstitch): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=2669 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2669,0,253665,0,0,0,1,0,0,0),(2669,0,253668,0,0,0,1,0,0,0);
-- 2670 Xizk Goodstitch (wowhead.com/forever/npc=2670; wow-professions.com/forever/npc/xizk-goodstitch): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=2670 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2670,0,253665,0,0,0,1,0,0,0),(2670,0,253668,0,0,0,1,0,0,0);
-- 3091 Franklin Hamar (wowhead.com/forever/npc=3091; wow-professions.com/forever/npc/franklin-hamar): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=3091 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3091,0,253665,0,0,0,1,0,0,0),(3091,0,253668,0,0,0,1,0,0,0);
-- 3364 Borya (wowhead.com/forever/npc=3364; wow-professions.com/forever/npc/borya): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=3364 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3364,0,253665,0,0,0,1,0,0,0),(3364,0,253668,0,0,0,1,0,0,0);
-- 4168 Elynna (wowhead.com/forever/npc=4168; wow-professions.com/forever/npc/elynna): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=4168 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4168,0,253665,0,0,0,1,0,0,0),(4168,0,253668,0,0,0,1,0,0,0);
-- 4189 Valdaron (wowhead.com/forever/npc=4189; wow-professions.com/forever/npc/valdaron): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=4189 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4189,0,253665,0,0,0,1,0,0,0),(4189,0,253668,0,0,0,1,0,0,0);
-- 4577 Millie Gregorian (wowhead.com/forever/npc=4577; wow-professions.com/forever/npc/millie-gregorian): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=4577 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4577,0,253665,0,0,0,1,0,0,0),(4577,0,253668,0,0,0,1,0,0,0);
-- 5154 Poranna Snowbraid (wowhead.com/forever/npc=5154; wow-professions.com/forever/npc/poranna-snowbraid): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=5154 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5154,0,253665,0,0,0,1,0,0,0),(5154,0,253668,0,0,0,1,0,0,0);
-- 6567 Ghok'kah (wowhead.com/forever/npc=6567; wow-professions.com/forever/npc/ghokkah): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=6567 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(6567,0,253665,0,0,0,1,0,0,0),(6567,0,253668,0,0,0,1,0,0,0);
-- 6568 Vizzklick (wowhead.com/forever/npc=6568; wow-professions.com/forever/npc/vizzklick): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=6568 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(6568,0,253665,0,0,0,1,0,0,0),(6568,0,253668,0,0,0,1,0,0,0);
-- 8681 Outfitter Eric (wowhead.com/forever/npc=8681; wow-professions.com/forever/npc/outfitter-eric): 253665 Pattern: Linen Reagent Bag, 253668 Pattern: Woolen Reagent Bag
DELETE FROM `npc_vendor` WHERE `entry`=8681 AND `item` IN (253665,253668);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8681,0,253665,0,0,0,1,0,0,0),(8681,0,253668,0,0,0,1,0,0,0);

-- == Leatherworking supplies: Forever armor kit / reagent bag patterns ==
-- 3366 Tamar (wowhead.com/forever/npc=3366; wow-professions.com/forever/npc/tamar): 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=3366 AND `item` IN (252784,252783);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3366,0,252784,0,0,0,1,0,0,0),(3366,0,252783,0,0,0,1,0,0,0);
-- 4589 Joseph Moore (wowhead.com/forever/npc=4589; wow-professions.com/forever/npc/joseph-moore): 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=4589 AND `item` IN (252784,252783);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4589,0,252784,0,0,0,1,0,0,0),(4589,0,252783,0,0,0,1,0,0,0);
-- 5128 Bombus Finespindle (wowhead.com/forever/npc=5128; wow-professions.com/forever/npc/bombus-finespindle): 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=5128 AND `item` IN (252784,252783);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5128,0,252784,0,0,0,1,0,0,0),(5128,0,252783,0,0,0,1,0,0,0);
-- 5565 Jillian Tanner (wowhead.com/forever/npc=5565; wow-professions.com/forever/npc/jillian-tanner): 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=5565 AND `item` IN (252784,252783);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5565,0,252784,0,0,0,1,0,0,0),(5565,0,252783,0,0,0,1,0,0,0);
-- 4225 Saenorion (wowhead.com/forever/npc=4225; wow-professions.com/forever/npc/saenorion): 277716 Pattern: Light Leather Reagent Bag, 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit, 252812 Pattern: Forceful Heavy Armor Kit, 252811 Pattern: Mystic Heavy Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=4225 AND `item` IN (277716,252784,252783,252812,252811);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4225,0,277716,0,0,0,1,0,0,0),(4225,0,252784,0,0,0,1,0,0,0),(4225,0,252783,0,0,0,1,0,0,0),(4225,0,252812,0,0,0,1,0,0,0),
(4225,0,252811,0,0,0,1,0,0,0);
-- 7854 Jangdor Swiftstrider (wowhead.com/forever/npc=7854; wow-professions.com/forever/npc/jangdor-swiftstrider): 252784 Pattern: Forceful Medium Armor Kit, 252783 Pattern: Mystic Medium Armor Kit, 252812 Pattern: Forceful Heavy Armor Kit, 252811 Pattern: Mystic Heavy Armor Kit, 252942 Pattern: Forceful Rugged Armor Kit
DELETE FROM `npc_vendor` WHERE `entry`=7854 AND `item` IN (252784,252783,252812,252811,252942);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(7854,0,252784,0,0,0,1,0,0,0),(7854,0,252783,0,0,0,1,0,0,0),(7854,0,252812,0,0,0,1,0,0,0),(7854,0,252811,0,0,0,1,0,0,0),
(7854,0,252942,0,0,0,1,0,0,0);

-- == Enchanting supplies ==
-- 15419 Kania (wowhead.com/forever/npc=15419; wow-professions.com/forever/npc/kania): 247786 Mote of Magic
DELETE FROM `npc_vendor` WHERE `entry`=15419 AND `item` IN (247786);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(15419,0,247786,0,0,0,1,0,0,0);

-- == Fishing supplies: Chef's Knife (new) ==
-- 1678 Vernon Hale (wowhead.com/forever/npc=1678; wow-professions.com/forever/npc/vernon-hale): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=1678 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1678,0,273636,0,0,0,1,0,0,0);
-- 1684 Khara Deepwater (wowhead.com/forever/npc=1684; wow-professions.com/forever/npc/khara-deepwater): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=1684 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1684,0,273636,0,0,0,1,0,0,0);
-- 2383 Lindea Rabonne (wowhead.com/forever/npc=2383; wow-professions.com/forever/npc/lindea-rabonne): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=2383 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2383,0,273636,0,0,0,1,0,0,0);
-- 3178 Stuart Fleming (wowhead.com/forever/npc=3178; wow-professions.com/forever/npc/stuart-fleming): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=3178 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3178,0,273636,0,0,0,1,0,0,0);
-- 4222 Voloren (wowhead.com/forever/npc=4222; wow-professions.com/forever/npc/voloren): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=4222 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4222,0,273636,0,0,0,1,0,0,0);
-- 5162 Tansy Puddlefizz (wowhead.com/forever/npc=5162; wow-professions.com/forever/npc/tansy-puddlefizz): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=5162 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5162,0,273636,0,0,0,1,0,0,0);
-- 5494 Catherine Leland (wowhead.com/forever/npc=5494; wow-professions.com/forever/npc/catherine-leland): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=5494 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5494,0,273636,0,0,0,1,0,0,0);
-- 5940 Harn Longcast (wowhead.com/forever/npc=5940; wow-professions.com/forever/npc/harn-longcast): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=5940 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5940,0,273636,0,0,0,1,0,0,0);
-- 5942 Zansoa (wowhead.com/forever/npc=5942; wow-professions.com/forever/npc/zansoa): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=5942 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5942,0,273636,0,0,0,1,0,0,0);
-- 8508 Gretta Ganter (wowhead.com/forever/npc=8508; wow-professions.com/forever/npc/gretta-ganter): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=8508 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8508,0,273636,0,0,0,1,0,0,0);
-- 10118 Nessa Shadowsong (wowhead.com/forever/npc=10118; wow-professions.com/forever/npc/nessa-shadowsong): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=10118 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(10118,0,273636,0,0,0,1,0,0,0);
-- 12962 Wik'Tar (wowhead.com/forever/npc=12962; wow-professions.com/forever/npc/wiktar): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=12962 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(12962,0,273636,0,0,0,1,0,0,0);
-- 4574 Lizbeth Cromwell (wowhead.com/forever/npc=4574; wow-professions.com/forever/npc/lizbeth-cromwell): 273636 Chef's Knife
DELETE FROM `npc_vendor` WHERE `entry`=4574 AND `item` IN (273636);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4574,0,273636,0,0,0,1,0,0,0);

-- == Cooking supplies: camp fuel and Thick Logs (new) ==
-- 3085 Gloria Femmel (wowhead.com/forever/npc=3085; wow-professions.com/forever/npc/gloria-femmel): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=3085 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3085,0,4471,0,0,0,1,0,0,0),(3085,0,4536,0,0,0,1,0,0,0),(3085,0,10284,0,0,0,1,0,0,0),(3085,0,4470,0,0,0,1,0,0,0),
(3085,0,11291,0,0,0,1,0,0,0),(3085,0,272941,0,0,0,1,0,0,0);
-- 2381 Micha Yance (wowhead.com/forever/npc=2381; wow-professions.com/forever/npc/micha-yance): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=2381 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2381,0,4471,0,0,0,1,0,0,0),(2381,0,4536,0,0,0,1,0,0,0),(2381,0,10284,0,0,0,1,0,0,0),(2381,0,4470,0,0,0,1,0,0,0),
(2381,0,11291,0,0,0,1,0,0,0),(2381,0,272941,0,0,0,1,0,0,0);
-- 2664 Kelsey Yance (wowhead.com/forever/npc=2664; wow-professions.com/forever/npc/kelsey-yance): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=2664 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(2664,0,4471,0,0,0,1,0,0,0),(2664,0,4536,0,0,0,1,0,0,0),(2664,0,10284,0,0,0,1,0,0,0),(2664,0,4470,0,0,0,1,0,0,0),
(2664,0,11291,0,0,0,1,0,0,0),(2664,0,272941,0,0,0,1,0,0,0);
-- 4223 Fyldan (wowhead.com/forever/npc=4223; wow-professions.com/forever/npc/fyldan): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=4223 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4223,0,4471,0,0,0,1,0,0,0),(4223,0,4536,0,0,0,1,0,0,0),(4223,0,10284,0,0,0,1,0,0,0),(4223,0,4470,0,0,0,1,0,0,0),
(4223,0,11291,0,0,0,1,0,0,0),(4223,0,272941,0,0,0,1,0,0,0);
-- 4265 Nyoma (wowhead.com/forever/npc=4265; wow-professions.com/forever/npc/nyoma): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=4265 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4265,0,4471,0,0,0,1,0,0,0),(4265,0,4536,0,0,0,1,0,0,0),(4265,0,10284,0,0,0,1,0,0,0),(4265,0,4470,0,0,0,1,0,0,0),
(4265,0,11291,0,0,0,1,0,0,0),(4265,0,272941,0,0,0,1,0,0,0);
-- 4553 Ronald Burch (wowhead.com/forever/npc=4553; wow-professions.com/forever/npc/ronald-burch): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=4553 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4553,0,4471,0,0,0,1,0,0,0),(4553,0,4536,0,0,0,1,0,0,0),(4553,0,10284,0,0,0,1,0,0,0),(4553,0,4470,0,0,0,1,0,0,0),
(4553,0,11291,0,0,0,1,0,0,0),(4553,0,272941,0,0,0,1,0,0,0);
-- 5160 Emrul Riknussun (wowhead.com/forever/npc=5160; wow-professions.com/forever/npc/emrul-riknussun): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=5160 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5160,0,4471,0,0,0,1,0,0,0),(5160,0,4536,0,0,0,1,0,0,0),(5160,0,10284,0,0,0,1,0,0,0),(5160,0,4470,0,0,0,1,0,0,0),
(5160,0,11291,0,0,0,1,0,0,0),(5160,0,272941,0,0,0,1,0,0,0);
-- 5483 Erika Tate (wowhead.com/forever/npc=5483; wow-professions.com/forever/npc/erika-tate): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=5483 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(5483,0,4471,0,0,0,1,0,0,0),(5483,0,4536,0,0,0,1,0,0,0),(5483,0,10284,0,0,0,1,0,0,0),(5483,0,4470,0,0,0,1,0,0,0),
(5483,0,11291,0,0,0,1,0,0,0),(5483,0,272941,0,0,0,1,0,0,0);
-- 12033 Wulan (wowhead.com/forever/npc=12033; wow-professions.com/forever/npc/wulan): 4471 Flint and Tinder, 4536 Shiny Red Apple, 10284 Simple Flour, 4470 Simple Wood, 11291 Star Wood, 272941 Thick Logs
DELETE FROM `npc_vendor` WHERE `entry`=12033 AND `item` IN (4471,4536,10284,4470,11291,272941);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(12033,0,4471,0,0,0,1,0,0,0),(12033,0,4536,0,0,0,1,0,0,0),(12033,0,10284,0,0,0,1,0,0,0),(12033,0,4470,0,0,0,1,0,0,0),
(12033,0,11291,0,0,0,1,0,0,0),(12033,0,272941,0,0,0,1,0,0,0);

-- == Trade/general goods: apprentice gathering bags (new) ==
-- 66 Tharynn Bouden (wowhead.com/forever/npc=66; wow-professions.com/forever/npc/tharynn-bouden): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=66 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(66,0,277113,0,0,0,1,0,0,0),(66,0,277115,0,0,0,1,0,0,0),(66,0,277114,0,0,0,1,0,0,0),(66,0,4536,0,0,0,1,0,0,0);
-- 1250 Drake Lindgren (wowhead.com/forever/npc=1250; wow-professions.com/forever/npc/drake-lindgren): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=1250 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1250,0,277113,0,0,0,1,0,0,0),(1250,0,277115,0,0,0,1,0,0,0),(1250,0,277114,0,0,0,1,0,0,0),(1250,0,4536,0,0,0,1,0,0,0);
-- 1285 Thurman Mullby (wowhead.com/forever/npc=1285; wow-professions.com/forever/npc/thurman-mullby): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 2901 Mining Pick, 7005 Skinning Knife, 11291 Star Wood
DELETE FROM `npc_vendor` WHERE `entry`=1285 AND `item` IN (277113,277115,277114,2901,7005,11291);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1285,0,277113,0,0,0,1,0,0,0),(1285,0,277115,0,0,0,1,0,0,0),(1285,0,277114,0,0,0,1,0,0,0),(1285,0,2901,0,0,0,1,0,0,0),
(1285,0,7005,0,0,0,1,0,0,0),(1285,0,11291,0,0,0,1,0,0,0);
-- 1286 Edna Mullby (wowhead.com/forever/npc=1286; wow-professions.com/forever/npc/edna-mullby): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=1286 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1286,0,277113,0,0,0,1,0,0,0),(1286,0,277115,0,0,0,1,0,0,0),(1286,0,277114,0,0,0,1,0,0,0),(1286,0,4536,0,0,0,1,0,0,0);
-- 3081 Wunna Darkmane (wowhead.com/forever/npc=3081; wow-professions.com/forever/npc/wunna-darkmane): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=3081 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3081,0,277113,0,0,0,1,0,0,0),(3081,0,277115,0,0,0,1,0,0,0),(3081,0,277114,0,0,0,1,0,0,0),(3081,0,4536,0,0,0,1,0,0,0);
-- 3158 Duokna (wowhead.com/forever/npc=3158; wow-professions.com/forever/npc/duokna): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 2901 Mining Pick, 7005 Skinning Knife
DELETE FROM `npc_vendor` WHERE `entry`=3158 AND `item` IN (277113,277115,277114,2901,7005);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3158,0,277113,0,0,0,1,0,0,0),(3158,0,277115,0,0,0,1,0,0,0),(3158,0,277114,0,0,0,1,0,0,0),(3158,0,2901,0,0,0,1,0,0,0),
(3158,0,7005,0,0,0,1,0,0,0);
-- 3187 Tai'tasi (wowhead.com/forever/npc=3187; wow-professions.com/forever/npc/taitasi): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=3187 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3187,0,277113,0,0,0,1,0,0,0),(3187,0,277115,0,0,0,1,0,0,0),(3187,0,277114,0,0,0,1,0,0,0),(3187,0,4536,0,0,0,1,0,0,0);
-- 3587 Lyrai (wowhead.com/forever/npc=3587; wow-professions.com/forever/npc/lyrai): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 2901 Mining Pick, 7005 Skinning Knife
DELETE FROM `npc_vendor` WHERE `entry`=3587 AND `item` IN (277113,277115,277114,2901,7005);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3587,0,277113,0,0,0,1,0,0,0),(3587,0,277115,0,0,0,1,0,0,0),(3587,0,277114,0,0,0,1,0,0,0),(3587,0,2901,0,0,0,1,0,0,0),
(3587,0,7005,0,0,0,1,0,0,0);
-- 3614 Narret Shadowgrove (wowhead.com/forever/npc=3614; wow-professions.com/forever/npc/narret-shadowgrove): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=3614 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(3614,0,277113,0,0,0,1,0,0,0),(3614,0,277115,0,0,0,1,0,0,0),(3614,0,277114,0,0,0,1,0,0,0),(3614,0,4536,0,0,0,1,0,0,0);
-- 4561 Daniel Bartlett (wowhead.com/forever/npc=4561; wow-professions.com/forever/npc/daniel-bartlett): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=4561 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4561,0,277113,0,0,0,1,0,0,0),(4561,0,277115,0,0,0,1,0,0,0),(4561,0,277114,0,0,0,1,0,0,0),(4561,0,4536,0,0,0,1,0,0,0);
-- 4775 Felicia Doan (wowhead.com/forever/npc=4775; wow-professions.com/forever/npc/felicia-doan): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=4775 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(4775,0,277113,0,0,0,1,0,0,0),(4775,0,277115,0,0,0,1,0,0,0),(4775,0,277114,0,0,0,1,0,0,0),(4775,0,4536,0,0,0,1,0,0,0);
-- 8934 Christopher Hewen (wowhead.com/forever/npc=8934; wow-professions.com/forever/npc/christopher-hewen): 277113 Apprentice's Herb Pouch, 277115 Apprentice's Mining Pack, 277114 Apprentice's Skinning Satchel, 4536 Shiny Red Apple
DELETE FROM `npc_vendor` WHERE `entry`=8934 AND `item` IN (277113,277115,277114,4536);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8934,0,277113,0,0,0,1,0,0,0),(8934,0,277115,0,0,0,1,0,0,0),(8934,0,277114,0,0,0,1,0,0,0),(8934,0,4536,0,0,0,1,0,0,0);

-- == Cenarion Hold and Kirin Tor recipe vendors (new) ==
-- 15176 Vargus (wowhead.com/forever/npc=15176; wow-professions.com/forever/npc/vargus): 275601 Plans: Clutchlord's Grips, 275598 Plans: Goregasher Grips, 275602 Plans: Clutchlord's Support, 275599 Plans: Goregasher Support, 275603 Plans: Clutchlord's Stompers, 275600 Plans: Goregasher Stompers
DELETE FROM `npc_vendor` WHERE `entry`=15176 AND `item` IN (275601,275598,275602,275599,275603,275600);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(15176,0,275601,0,0,0,1,0,0,0),(15176,0,275598,0,0,0,1,0,0,0),(15176,0,275602,0,0,0,1,0,0,0),(15176,0,275599,0,0,0,1,0,0,0),
(15176,0,275603,0,0,0,1,0,0,0),(15176,0,275600,0,0,0,1,0,0,0);
-- 15179 Mishta (wowhead.com/forever/npc=15179; wow-professions.com/forever/npc/mishta): 275583 Pattern: Swarmtender's Gloves, 275580 Pattern: Venomspew Gloves, 275584 Pattern: Swarmtender's Cord, 275581 Pattern: Venomspew Cord, 275585 Pattern: Swarmtender's Footpads, 275582 Pattern: Venomspew Footpads
DELETE FROM `npc_vendor` WHERE `entry`=15179 AND `item` IN (275583,275580,275584,275581,275585,275582);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(15179,0,275583,0,0,0,1,0,0,0),(15179,0,275580,0,0,0,1,0,0,0),(15179,0,275584,0,0,0,1,0,0,0),(15179,0,275581,0,0,0,1,0,0,0),
(15179,0,275585,0,0,0,1,0,0,0),(15179,0,275582,0,0,0,1,0,0,0);
-- 32287 Archmage Alvareaux (wowhead.com/forever/npc=32287; wow-professions.com/forever/npc/archmage-alvareaux): 276984 Plans: Azure Skyforged Helm, 276985 Plans: Azure Skyforged Pauldrons, 276986 Pattern: Azure Gustwoven Hood, 276987 Pattern: Azure Gustwoven Spaulders, 276988 Pattern: Azure Windraveled Cover, 276989 Pattern: Azure Windraveled Drapes, 276990 Pattern: Azure Stormsewn Cowl, 276991 Pattern: Azure Stormsewn Epaulets
DELETE FROM `npc_vendor` WHERE `entry`=32287 AND `item` IN (276984,276985,276986,276987,276988,276989,276990,276991);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(32287,0,276984,0,0,0,1,0,0,0),(32287,0,276985,0,0,0,1,0,0,0),(32287,0,276986,0,0,0,1,0,0,0),(32287,0,276987,0,0,0,1,0,0,0),
(32287,0,276988,0,0,0,1,0,0,0),(32287,0,276989,0,0,0,1,0,0,0),(32287,0,276990,0,0,0,1,0,0,0),(32287,0,276991,0,0,0,1,0,0,0);

-- == Pet and novelty vendors: sole vendor of a New-in-Forever item ==
-- 268745 Mayhoa Skyhoof (wowhead.com/forever/npc=268745; item page: sold only in Orgrimmar): 277508 Prairie Wolf Pup Carrier
DELETE FROM `npc_vendor` WHERE `entry`=268745 AND `item` IN (277508);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(268745,0,277508,0,0,0,1,0,0,0);
-- 8403 Jeremiah Payson (wowhead.com/forever/npc=8403; item page: sold only in Undercity): 280697 Plagued Cockroach, 280699 Undercity Cockroach
DELETE FROM `npc_vendor` WHERE `entry`=8403 AND `item` IN (280697,280699);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8403,0,280697,0,0,0,1,0,0,0),(8403,0,280699,0,0,0,1,0,0,0);
-- 8404 Xan'tish (wowhead.com/forever/npc=8404; item page: sold only in Orgrimmar): 280703 Jungle Boa
DELETE FROM `npc_vendor` WHERE `entry`=8404 AND `item` IN (280703);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8404,0,280703,0,0,0,1,0,0,0);
-- 8401 Halpa (wowhead.com/forever/npc=8401; item page: sold only in Thunder Bluff): 280794 Brown Ground Squirrel Whistle, 280795 Red Ground Squirrel Whistle
DELETE FROM `npc_vendor` WHERE `entry`=8401 AND `item` IN (280794,280795);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8401,0,280794,0,0,0,1,0,0,0),(8401,0,280795,0,0,0,1,0,0,0);
-- 1263 Yarlyn Amberstill (wowhead.com/forever/npc=1263; item page: sold only in Dun Morogh): 280797 Rabbit Crate (Artic), 280801 Rabbit Crate (Brown)
DELETE FROM `npc_vendor` WHERE `entry`=1263 AND `item` IN (280797,280801);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1263,0,280797,0,0,0,1,0,0,0),(1263,0,280801,0,0,0,1,0,0,0);
-- 8665 Shylenai (wowhead.com/forever/npc=8665; item page: sold only in Darnassus): 280803 Eagle Owl
DELETE FROM `npc_vendor` WHERE `entry`=8665 AND `item` IN (280803);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(8665,0,280803,0,0,0,1,0,0,0);
-- 29547 Applebough (wowhead.com/forever/npc=29547; item page: sold only in Alterac Mountains (Dalaran)): 285359 Warm Apple Juice
DELETE FROM `npc_vendor` WHERE `entry`=29547 AND `item` IN (285359);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(29547,0,285359,0,0,0,1,0,0,0);
-- 256372 Windle Sparkshine (wowhead.com/forever/npc=256372; item page: sold only in Alterac Mountains (Dalaran)): 285104 Windle's Lighter
DELETE FROM `npc_vendor` WHERE `entry`=256372 AND `item` IN (285104);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(256372,0,285104,0,0,0,1,0,0,0);
