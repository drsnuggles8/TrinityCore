-- Targeted port of Karrz v0.1.24's Valanaar guard correction: sniffed corpses gave living Peacekeeper Elites permanent
-- Feign Death. Remove only that cast and aura, preserving any unrelated SmartAI, auras and patrol paths.
DELETE FROM `smart_scripts` WHERE `entryorguid`=252477 AND `source_type`=0 AND `action_type`=11 AND `action_param1`=1288546;
UPDATE `creature_template_addon` SET `StandState`=0,
    `auras`=TRIM(REPLACE(CONCAT(' ',COALESCE(`auras`,''),' '),' 1288546 ',' ')) WHERE `entry`=252477;
UPDATE `creature_addon` AS a INNER JOIN `creature` AS c ON c.`guid`=a.`guid`
    SET a.`StandState`=0, a.`auras`=TRIM(REPLACE(CONCAT(' ',COALESCE(a.`auras`,''),' '),' 1288546 ',' ')) WHERE c.`id`=252477;
