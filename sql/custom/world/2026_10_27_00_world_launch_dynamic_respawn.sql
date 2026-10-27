-- Launch simulation (doc/LaunchSim.md): the hostile creatures of the dwarf and gnome leveling area (Coldridge Valley,
-- Dun Morogh, Ironforge surroundings, the west of Loch Modan) join spawn group 2 "Dynamic Scaling (Quest objectives)", so
-- that with Respawn.DynamicMode = 1 a crowd gets faster respawns, as the 2019 Classic launch did. Nothing changes while
-- Respawn.DynamicMode is 0 (the default). Creatures already in a spawn group and NPCs with any npcflag are left alone.
INSERT IGNORE INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`)
SELECT 2, 0, c.`guid`
FROM `creature` c
JOIN `creature_template` t ON t.`entry` = c.`id`
WHERE c.`map` = 0
  AND c.`position_x` BETWEEN -6800 AND -4500
  AND c.`position_y` BETWEEN -1500 AND 1100
  AND c.`spawntimesecs` BETWEEN 1 AND 1800
  AND t.`npcflag` = 0
  AND NOT EXISTS (SELECT 1 FROM `spawn_group` g WHERE g.`spawnType` = 0 AND g.`spawnId` = c.`guid`);
