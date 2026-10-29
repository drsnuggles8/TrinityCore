-- GlobalColor.db2 (table hash 0xB8BA5FB3): Forever client 70685's default bag frames (ContainerFrame.lua, ApplyEmptySlotAppearanceToTexture)
-- read EMPTY_BAG_SLOT_EFFECT_COLOR, which the 70685 data does not have: "attempt to index global 'EMPTY_BAG_SLOT_EFFECT_COLOR' (a nil
-- value)" and every bag drawn empty (playtest 29 Oct; bag addons were fine). The row as retail has it (wago.tools GlobalColor: 423,
-- EMPTY_BAG_SLOT_EFFECT_COLOR, 2141557887), sent as a hotfix; the store is registered in DB2Stores.cpp.
CREATE TABLE IF NOT EXISTS `global_color` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `LuaConstantName` text,
  `Color` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `global_color` WHERE `ID`=423;
INSERT INTO `global_color` (`ID`,`LuaConstantName`,`Color`,`VerifiedBuild`) VALUES
(423,'EMPTY_BAG_SLOT_EFFECT_COLOR',2141557887,0);

DELETE FROM `hotfix_data` WHERE `Id`=2000120 AND `TableHash`=3099222963;
INSERT INTO `hotfix_data` (`Id`,`UniqueId`,`TableHash`,`RecordId`,`Status`,`VerifiedBuild`) VALUES
(2000120,2000120,3099222963,423,1,0);
