-- High Priestess Mims (258785), Priest Trainer in Ironforge's Mystic Ward, and the two gnome priest racial quests she gives (a guild
-- mate, 31 Oct: "a gnome priest NPC missing in Ironforge"). Per Wowhead Forever and its mirrors (60.tools, foreverdb.app):
-- 1. Mims did not exist: template copied from High Priest Rohan (11406: priest trainer, trainer 8, gossip menu 3642), Bink's faction
--    (5144, a gnome trainer of the same ward), level 60. Model: Juli Stormkettle's (10214, a gnome woman in a robe; Forever's own is not
--    in this client's data). Spawn at Wowhead's 24.8, 10.0 (Ironforge: x = -4569.241 - v * 527.605, y = -713.591 - u * 790.625, which
--    puts High Priest Rohan at 24.7, 8.2), on the trainers' floor (z 501.15, as Rohan and Toldren Deepiron).
-- 2. Confounding Flash (94817, level 10) and Contingency Plan (94818, level 20) were in the database (gnome priests only, reward spells
--    1277662 / 1277663 teach 1277455 / 1277462) with no giver or ender, so a gnome priest never got its racials. Mims starts and ends
--    both (no objectives: the lesson is the turn-in). Texts from Wowhead's quest tooltips; who gives them on live is not on any page
--    found, Mims is the one gnome priest trainer.
START TRANSACTION;

SET @faction := (SELECT `faction` FROM `creature_template` WHERE `entry` = 5144);

DELETE FROM `creature_template` WHERE `entry` = 258785;
INSERT INTO `creature_template` (`entry`, `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `TitleAlt`, `IconName`, `RequiredExpansion`, `VignetteID`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `Classification`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `family`, `trainer_class`, `type`, `VehicleId`, `AIName`, `MovementType`, `ExperienceModifier`, `RacialLeader`, `movementId`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `StringId`, `VerifiedBuild`)
SELECT 258785, 0, 0, 'High Priestess Mims', NULL, 'Priest Trainer', `TitleAlt`, `IconName`, `RequiredExpansion`, `VignetteID`, @faction, `npcflag`, `speed_walk`, `speed_run`, `scale`, `Classification`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `family`, `trainer_class`, `type`, `VehicleId`, '', 0, `ExperienceModifier`, `RacialLeader`, `movementId`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, '', NULL, 0
  FROM `creature_template` WHERE `entry` = 11406;

DELETE FROM `creature_template_difficulty` WHERE `Entry` = 258785;
CREATE TEMPORARY TABLE `tmp_mims` AS SELECT * FROM `creature_template_difficulty` WHERE `Entry` = 11406;
UPDATE `tmp_mims` SET `Entry` = 258785, `VerifiedBuild` = 0;
INSERT INTO `creature_template_difficulty` SELECT * FROM `tmp_mims`;
DROP TEMPORARY TABLE `tmp_mims`;

DELETE FROM `creature_classic_level` WHERE `entry` = 258785;
INSERT INTO `creature_classic_level` (`entry`, `level_min`, `level_max`) VALUES (258785, 60, 60);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 258785;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(258785, 0, 10214, 1, 1, 0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID` = 258785;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES (258785, 3642, 0);
DELETE FROM `creature_trainer` WHERE `CreatureID` = 258785;
INSERT INTO `creature_trainer` (`CreatureID`, `TrainerID`, `MenuID`, `OptionID`) VALUES (258785, 8, 3642, 0);

-- Spawn (creature guids 26300110-26300119 reserved for this file)
DELETE FROM `creature` WHERE `guid` BETWEEN 26300110 AND 26300119;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`, `VerifiedBuild`) VALUES
(26300110, 258785, 0, 0, 0, '0', -4622.0, -909.7, 501.15, 4.4, 300, 0, 0, 0);

-- 2. The racial quests
DELETE FROM `creature_queststarter` WHERE `id` = 258785;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES (258785, 94817, 0), (258785, 94818, 0);
DELETE FROM `creature_questender` WHERE `id` = 258785;
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES (258785, 94817, 0), (258785, 94818, 0);

UPDATE `quest_template` SET
  `LogDescription` = 'You took your time arriving, $N. But you can be excused given your dedicated service to the Light. Please pay close attention to this lesson.',
  `QuestDescription` = 'You took your time arriving, $N. But you can be excused given your dedicated service to the Light. Please pay close attention to this lesson.'
  WHERE `ID` = 94817;
UPDATE `quest_template` SET
  `LogDescription` = 'Continue your service to the Light and you will become a great $C, $N. Maintain your vigilance and dedication!$B$BYou are ready for your next lesson in service.',
  `QuestDescription` = 'Continue your service to the Light and you will become a great $C, $N. Maintain your vigilance and dedication!$B$BYou are ready for your next lesson in service.'
  WHERE `ID` = 94818;
DELETE FROM `quest_offer_reward` WHERE `ID` IN (94817, 94818);
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES
(94817, 1, 0, 0, 0, 0, 0, 0, 0, 'You took your time arriving, $N. But you can be excused given your dedicated service to the Light. Please pay close attention to this lesson.', 0),
(94818, 1, 0, 0, 0, 0, 0, 0, 0, 'Continue your service to the Light and you will become a great $C, $N. Maintain your vigilance and dedication!$B$BYou are ready for your next lesson in service.', 0);

COMMIT;
