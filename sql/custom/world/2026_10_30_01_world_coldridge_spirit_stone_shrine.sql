-- Call of Earth (94374, dwarf shaman, Coldridge Valley): "drink the Earth Sapta at the Spirit Stone" - the Sapta's spell needs
-- a Shaman Shrine spell focus (63) close by, and Coldridge had none: "Requires Shaman Shrine" at the stone (Olo, 30 Oct). The
-- stone itself is part of the terrain. Every other shrine site pairs the invisible 50 y focus (63674) with a visible shrine
-- rock, the Minor Manifestation of Earth 8-11 y off (Durotar, Mulgore); here the manifestation (5891) and four Redrock Earth
-- Spirits (5890) were spawned but not the focus. It goes on the manifestation's spot, so its 50 y reach covers the stone.
DELETE FROM `gameobject` WHERE `guid`=34100010;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnDifficulties`,`phaseUseFlags`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`StringId`,`VerifiedBuild`) VALUES
(34100010,63674,0,1,132,'0',0,0,0,-1,-5908.1,576.5,500.6,0,0,0,0,1,7200,100,1,'',NULL,0);
