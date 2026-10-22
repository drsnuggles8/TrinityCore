-- Баг-репорты игроков Forever (gm_bug), разбор 23.10.2026.

-- 1) gm_bug №91, №92 «Задублирован нпс» (Страж смерти – ветеран 275103, Тирисфальские леса).
-- В снифах официального сервера (сборка 70450) в этой точке стоят только 5 «Стражей смерти – ветеранов» (275103);
-- классический «Страж смерти – гвардеец» (7980) в снифах не встречается вовсе. У нас 4 ветерана стояли ровно поверх
-- 4 гвардейцев (расстояние 0–0,1 ярда), а пятой точки ветерана не было — там остался гвардеец.
-- Удаляем 4 гвардейца-дубля, пятого гвардейца превращаем в ветерана (его точка совпадает со снифом: 2162.7, 1309.1).
DELETE FROM `creature` WHERE `guid` IN (20028707, 20028710, 20028712, 20028713) AND `id` = 7980;
UPDATE `creature` SET `id` = 275103, `spawntimesecs` = 120 WHERE `guid` = 20028709 AND `id` = 7980;

-- 2) gm_bug №117 «Не работает кв при общении нет донесения от нпс», №182 «Говорю с нпс по кв но не засчитывается»
-- (задание 99141 «Терпение»: нужны 3 донесения — предметы 286200, 286201, 286202).
-- По снифам (70450): после выбора пункта «I need a report for Executor Zygand.» игрок сразу накладывает на себя
-- заклинание донесения (Кристоф — 1319489, Толстяк — 1319508). Заклинания уже записаны в gossip_menu_option.SpellID,
-- но ядро это поле только показывает клиенту и само ничего не накладывает, поэтому донесения не выдавались.
-- Добавляем SmartAI: выбор пункта -> игрок накладывает заклинание на себя (у Диллинджера — 1319468 из того же меню).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (1496, 251001) AND `AIName` = '';
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `event_type` = 62 AND
    ((`entryorguid` = 1496 AND `event_param1` = 44990) OR
     (`entryorguid` = 251001 AND `event_param1` = 44991) OR
     (`entryorguid` = 10666 AND `event_param1` = 44993));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`,
    `event_flags`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(1496,   0, 100, 0, 62, 0, 100, 0, 44990, 6, 134, 1319468, 2, 7, 'Deathguard Dillinger - On Gossip Select - Invoker Cast Report for Executor Zygand (quest 99141)'),
(251001, 0, 100, 0, 62, 0, 100, 0, 44991, 6, 134, 1319489, 2, 7, 'Deathguard Kristof - On Gossip Select - Invoker Cast Report for Executor Zygand (quest 99141)'),
(10666,  0, 100, 0, 62, 0, 100, 0, 44993, 0, 134, 1319508, 2, 7, 'Gordo - On Gossip Select - Invoker Cast Report for Executor Zygand (quest 99141)');
