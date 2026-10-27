-- Launch simulation (doc/LaunchSim.md): the other Alliance starting zones join spawn group 2 "Dynamic Scaling (Quest
-- objectives)" as Dun Morogh did (2026_10_27_00): Elwynn Forest with Northshire, Teldrassil with Shadowglen, and Zephras Isle
-- (map 2991). Nothing changes while Respawn.DynamicMode is 0. Creatures already in a spawn group and NPCs are left alone.
INSERT IGNORE INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`)
SELECT 2, 0, c.`guid`
FROM `creature` c
JOIN `creature_template` t ON t.`entry` = c.`id`
WHERE ((c.`map` = 0 AND c.`position_x` BETWEEN -10050 AND -8400 AND c.`position_y` BETWEEN -1600 AND 800)
    OR (c.`map` = 1 AND c.`position_x` BETWEEN 8500 AND 11200 AND c.`position_y` BETWEEN -500 AND 2600)
    OR c.`map` = 2991)
  AND c.`spawntimesecs` BETWEEN 1 AND 1800
  AND t.`npcflag` = 0
  AND NOT EXISTS (SELECT 1 FROM `spawn_group` g WHERE g.`spawnType` = 0 AND g.`spawnId` = c.`guid`);
