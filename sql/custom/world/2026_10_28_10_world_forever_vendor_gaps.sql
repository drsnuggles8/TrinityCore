-- WoW Forever vendor gaps found in play (28 Oct): items the Forever vendors sell that our world DB lacks.
-- Mote of Magic (247786, new enchanting reagent, 9c) at the classic city enchanting suppliers (Wowhead Forever, e.g. npc=1318).
DELETE FROM `npc_vendor` WHERE `item`=247786 AND `entry` IN (1318,3012,3346,4228,4617,5158,5757,5758);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(1318,0,247786,0,0,0,1,0,0,0),(3012,0,247786,0,0,0,1,0,0,0),(3346,0,247786,0,0,0,1,0,0,0),(4228,0,247786,0,0,0,1,0,0,0),
(4617,0,247786,0,0,0,1,0,0,0),(5158,0,247786,0,0,0,1,0,0,0),(5757,0,247786,0,0,0,1,0,0,0),(5758,0,247786,0,0,0,1,0,0,0);
-- Anvilmar's junior gathering trainers sold nothing: the same goods as the other junior skinning (267330) and mining (267331) trainers.
DELETE FROM `npc_vendor` WHERE `entry` IN (267336,267337);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`,`PlayerConditionID`,`IgnoreFiltering`,`VerifiedBuild`) VALUES
(267336,0,7005,0,0,0,1,0,0,0),(267336,0,4289,0,0,0,1,0,0,0),(267336,0,2320,0,0,0,1,0,0,0),(267336,0,277114,0,0,0,1,0,0,0),
(267337,0,2901,0,0,0,1,0,0,0),(267337,0,277115,0,0,0,1,0,0,0),(267337,0,5956,0,0,0,1,0,0,0);
