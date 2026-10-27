-- Classic 1.60: Overpower 7384 / 7887 / 11584 / 11585 "Only useable after the target dodges." Overpower costs a combo point, which
-- Offensive State (DND) 1282735 (learned with Overpower, 2026_10_26_07) gives through 1282733 when it procs. Without a spell_proc row
-- the proc used the default hit mask for melee hits done (normal and critical hits), so every landed hit lit up Overpower. Only a
-- dodge of the warrior's swing or melee ability procs it now.
DELETE FROM `spell_proc` WHERE `SpellId` = 1282735;
INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`ProcFlags`,`ProcFlags2`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(1282735,0,0,0,0,0,0,0x14,0,0,0x2,0x10,0,0,0,100,0,0);
