-- Skinning for the vanilla beasts (playtest 28 Oct: "you can't skin with skinning"). The vanilla import (VMaNGOS) brought
-- the skinning loot tables but left creature_template_difficulty.SkinLootID at 0 for most beasts, so no corpse became
-- skinnable: 655 beasts of the vanilla world had none (Ragged Young Wolf, Small Crag Boar, Mottled Boar, Young Wolf ...),
-- 24 pointed at a missing table (100008), 20 had their own table unlinked.
-- 1. A creature whose own skinning table exists gets it.
-- 2. Every other vanilla beast gets VMaNGOS' generic table of its level (100007 Light Leather / Light Hide, 100006
--    Medium, 100005 Heavy, 100004 Thick, 100003 Rugged); levels 1-10 a new one like the low beasts that have their own
--    (Stonetusk Boar 113: Ruined Leather Scraps / Light Leather).
-- Safe to run again.
DELETE FROM `skinning_loot_template` WHERE `Entry` = 100010;
INSERT INTO `skinning_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(100010, 0, 2934, 60, 0, 1, 1, 1, 1, 'levels 1-10: Ruined Leather Scraps'),
(100010, 0, 2318, 40, 0, 1, 1, 1, 1, 'levels 1-10: Light Leather');

UPDATE `creature_template_difficulty` d
SET d.`SkinLootID` = d.`Entry`
WHERE d.`DifficultyID` = 0 AND d.`SkinLootID` = 0
  AND EXISTS (SELECT 1 FROM `skinning_loot_template` s WHERE s.`Entry` = d.`Entry`);

UPDATE `creature_template_difficulty` d
JOIN `creature_template` t ON t.`entry` = d.`Entry`
JOIN `creature_classic_level` l ON l.`entry` = d.`Entry`
SET d.`SkinLootID` = CASE
    WHEN l.`level_max` <= 10 THEN 100010
    WHEN l.`level_max` <= 19 THEN 100007
    WHEN l.`level_max` <= 29 THEN 100006
    WHEN l.`level_max` <= 39 THEN 100005
    WHEN l.`level_max` <= 49 THEN 100004
    ELSE 100003 END
WHERE d.`DifficultyID` = 0 AND t.`type` = 1
  AND (d.`SkinLootID` = 0 OR (d.`SkinLootID` = 100008 AND NOT EXISTS (SELECT 1 FROM `skinning_loot_template` s WHERE s.`Entry` = 100008)));
