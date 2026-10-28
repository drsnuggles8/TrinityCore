-- Port Karrz v0.1.24's Grund/Gozwin correction, including the display prerequisite from its 2026_10_25_30 migration.
-- Quest 97277 already starts at creature 2756, but the imported retail row calls him Chao-Ju and uses display 46093.
-- Keep this self-contained: our baseline did not apply the release's earlier display replacement.
UPDATE `creature_template` SET `name`='Grund Drokda', `femaleName`=NULL, `subname`=NULL,
    `faction`=55, `npcflag`=(`npcflag` | 2) WHERE `entry`=2756;
UPDATE `creature_template` SET `name`='Gozwin Halfsprocket', `femaleName`=NULL, `subname`=NULL,
    `faction`=64 WHERE `entry`=6046;
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (2756,6046);
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) VALUES
(2756,0,3428,1,1,70634),
(6046,0,4775,1,1,70634);
