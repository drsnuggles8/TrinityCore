-- WoW Classic 1.60: druid and shaman class mechanics (classic_spells_druid_shaman.cpp).
-- Druid: Ferocious Bite (energy and attack power), Rip (attack power), Frenzied Regeneration, Swiftmend, Leader of the Pack and
-- Heart of the Wild in feral forms, Predatory Strikes refresh, Enrage armor reduction.
-- Shaman: Flametongue Weapon and Lightning Shield are 2026_10_25_00 / 2026_10_24_03 (classic_spell_scripts.cpp).
-- Safe to run again.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('classic_spell_dru_ferocious_bite', 'classic_spell_dru_rip', 'classic_spell_dru_frenzied_regeneration', 'classic_spell_dru_swiftmend', 'classic_spell_dru_feral_form_talents', 'classic_spell_dru_predatory_strikes', 'classic_spell_dru_enrage');
-- retail scripts on the same ids (fail Validate or hook retail effects)
DELETE FROM `spell_script_names` WHERE (`spell_id`, `ScriptName`) IN (
(22568, 'spell_dru_ferocious_bite'),
(1079, 'spell_dru_rip'),
(22842, 'spell_dru_guardian_of_elune_healing'));
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(22568, 'classic_spell_dru_ferocious_bite'),
(22827, 'classic_spell_dru_ferocious_bite'),
(22828, 'classic_spell_dru_ferocious_bite'),
(22829, 'classic_spell_dru_ferocious_bite'),
(31018, 'classic_spell_dru_ferocious_bite'),
(1079, 'classic_spell_dru_rip'),
(9492, 'classic_spell_dru_rip'),
(9493, 'classic_spell_dru_rip'),
(9752, 'classic_spell_dru_rip'),
(9894, 'classic_spell_dru_rip'),
(9896, 'classic_spell_dru_rip'),
(22842, 'classic_spell_dru_frenzied_regeneration'),
(18562, 'classic_spell_dru_swiftmend'),
(768, 'classic_spell_dru_feral_form_talents'),
(5487, 'classic_spell_dru_feral_form_talents'),
(9634, 'classic_spell_dru_feral_form_talents'),
(16972, 'classic_spell_dru_predatory_strikes'),
(5229, 'classic_spell_dru_enrage');
