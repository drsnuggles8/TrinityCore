-- Downstream (91733, Elwynn Forest, level 10, from 7), per Wowhead Forever and its mirrors: the quest was in the database but
-- nothing else was (a guild mate, 30 Oct: "some NPC seems to be completely missing").
-- 1. Ormin Pelford (248265, level 35 humanoid) did not exist: template copied from the Eastvale Peasant (11328, Stormwind faction),
--    quest giver, the Westfall lumberjack's model (308; Forever's own is not in this client's data). Spawn at Wowhead's 76.4, 72.0.
-- 2. The Waterlogged Saw (247813), Axe (247817) and Toolbox (247818) had no source. Wowhead lists none either, so they are quest
--    pickups (lock 43, like the vanilla ones) at the guide's spots: the Saw at 74.3, 76.3 below the Eastvale bridge, the Axe at
--    76.8, 82.4 on the island where the river splits, the Toolbox at 77.3, 86.9 on the riverbed by the Duskwood border. Models
--    from this client's data: a pile of tools (1208, there is no saw), The Shattered Axe (707), a toolbox (317).
-- 3. Quest POIs as for the Misty Pine quests: giver point, a 48 yd square per pickup, turn-in point.
-- Map coordinates (uiMap 1429) to world: Elwynn's bounds (x = -7939.58 - v * 2314.62, y = 1535.42 - u * 3470.84; Guard Thomas at
-- 73.9, 72.2 comes out 2.5 yd from his spawn). Z from the extracted .map terrain at each point. Approximate, not sniffed.
START TRANSACTION;

-- 1. Ormin Pelford
DELETE FROM `creature_template` WHERE `entry` = 248265;
INSERT INTO `creature_template` (`entry`, `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `TitleAlt`, `IconName`, `RequiredExpansion`, `VignetteID`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `Classification`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `family`, `trainer_class`, `type`, `VehicleId`, `AIName`, `MovementType`, `ExperienceModifier`, `RacialLeader`, `movementId`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `StringId`, `VerifiedBuild`)
SELECT 248265, 0, 0, 'Ormin Pelford', `femaleName`, NULL, `TitleAlt`, `IconName`, `RequiredExpansion`, `VignetteID`, 12, 2, `speed_walk`, `speed_run`, `scale`, `Classification`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `family`, `trainer_class`, `type`, `VehicleId`, '', 0, `ExperienceModifier`, `RacialLeader`, `movementId`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, '', NULL, 0
  FROM `creature_template` WHERE `entry` = 11328;

DELETE FROM `creature_template_difficulty` WHERE `Entry` = 248265;
CREATE TEMPORARY TABLE `tmp_ormin` AS SELECT * FROM `creature_template_difficulty` WHERE `Entry` = 11328;
UPDATE `tmp_ormin` SET `Entry` = 248265, `LootID` = 0, `VerifiedBuild` = 0;
INSERT INTO `creature_template_difficulty` SELECT * FROM `tmp_ormin`;
DROP TEMPORARY TABLE `tmp_ormin`;

DELETE FROM `creature_classic_level` WHERE `entry` = 248265;
INSERT INTO `creature_classic_level` (`entry`, `level_min`, `level_max`) VALUES (248265, 35, 35);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 248265;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(248265, 0, 308, 1, 1, 0);

DELETE FROM `creature_queststarter` WHERE `id` = 248265 AND `quest` = 91733;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES (248265, 91733, 0);
DELETE FROM `creature_questender` WHERE `id` = 248265 AND `quest` = 91733;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES (248265, 91733, 0);

-- 2. The tools
DELETE FROM `gameobject_template` WHERE `entry` IN (900100, 900101, 900102);
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data3`, `Data8`, `ContentTuningId`, `RequiredLevel`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(900100, 3, 1208, 'Waterlogged Saw',     '', '', '', 1, 43, 900100, 1, 91733, 0, 0, '', '', 0),
(900101, 3, 707,  'Waterlogged Axe',     '', '', '', 1, 43, 900101, 1, 91733, 0, 0, '', '', 0),
(900102, 3, 317,  'Waterlogged Toolbox', '', '', '', 1, 43, 900102, 1, 91733, 0, 0, '', '', 0);

DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (900100, 900101, 900102);
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(900100, 0, 247813, 100, 1, 1, 0, 1, 1, 'Waterlogged Saw'),
(900101, 0, 247817, 100, 1, 1, 0, 1, 1, 'Waterlogged Axe'),
(900102, 0, 247818, 100, 1, 1, 0, 1, 1, 'Waterlogged Toolbox');

-- Spawns (creature guids 26300100-26300109 and gameobject guids 34100020-34100029 reserved for this file)
DELETE FROM `creature` WHERE `guid` BETWEEN 26300100 AND 26300109;
DELETE FROM `gameobject` WHERE `guid` BETWEEN 34100020 AND 34100029;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`, `VerifiedBuild`) VALUES
(26300100, 248265, 0, 0, 0, '0', -9606.1, -1116.3, 42.41, 2.51, 300, 0, 0, 0);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(34100020, 900100, 0, 0, 0, '0', -9705.6, -1043.4, 39.38, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(34100021, 900101, 0, 0, 0, '0', -9846.8, -1130.2, 26.97, 0, 0, 0, 0, 1, 60, 255, 1, 0),
(34100022, 900102, 0, 0, 0, '0', -9951.0, -1147.5, 18.21, 0, 0, 0, 0, 1, 60, 255, 1, 0);

-- 3. Quest POIs
DELETE FROM `quest_poi` WHERE `QuestID` = 91733;
DELETE FROM `quest_poi_points` WHERE `QuestID` = 91733;
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `UiMapID`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `NavigationPlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`) VALUES
(91733, 0, 0, -1, 0, 0, 0, 1429, 0, 0, 0, 0, 0, 0, 0, 0),
(91733, 0, 1, 0, 463348, 247817, 0, 1429, 0, 0, 0, 0, 0, 0, 0, 0),
(91733, 0, 2, 1, 463349, 247813, 0, 1429, 0, 0, 0, 0, 0, 0, 0, 0),
(91733, 0, 3, 2, 463350, 247818, 0, 1429, 0, 0, 0, 0, 0, 0, 0, 0),
(91733, 0, 4, 32, 0, 0, 0, 1429, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `Z`, `VerifiedBuild`) VALUES
(91733, 0, 0, -9606, -1116, 42, 0),
(91733, 1, 0, -9871, -1154, 0, 0),
(91733, 1, 1, -9823, -1154, 0, 0),
(91733, 1, 2, -9823, -1106, 0, 0),
(91733, 1, 3, -9871, -1106, 0, 0),
(91733, 2, 0, -9730, -1067, 0, 0),
(91733, 2, 1, -9682, -1067, 0, 0),
(91733, 2, 2, -9682, -1019, 0, 0),
(91733, 2, 3, -9730, -1019, 0, 0),
(91733, 3, 0, -9975, -1172, 0, 0),
(91733, 3, 1, -9927, -1172, 0, 0),
(91733, 3, 2, -9927, -1124, 0, 0),
(91733, 3, 3, -9975, -1124, 0, 0),
(91733, 4, 0, -9606, -1116, 42, 0);
COMMIT;
