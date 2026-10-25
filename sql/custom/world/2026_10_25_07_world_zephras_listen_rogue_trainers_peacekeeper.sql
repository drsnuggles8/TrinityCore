-- Баг-репорты игрока (Героический мир, остров Зефрис, 25.10.2026): №79, №81, №84 («выслушать» по квесту — нажимаю, ничего не
-- засчитывается), №75–77 и №86–88 (страж лежит и двигается), №83 (у учителя разбойников нечему учиться), №72 (диалог на английском).
-- Источники: снифы (dump_1.60.1.70338_2026-10-11_10-57-34.pkt), данные клиента (DB2 заклинаний), страницы wowhead.com/forever.
--   1) «Выслушать». Цели квестов 92708 «A Grand Adventure» (273972), 93737 «The Broken Construct» (256083) и 92679 «Blood Tithe» (252448)
--      засчитываются заклинаниями-кредитами 1314159, 1267046 и 1256875 (эффект «кредит за существо»). В снифе 1314159 кастует существо
--      252475 на игрока сразу после начала «беседы» (CMSG_CONVERSATION_LINE_STARTED); самих бесед (диалоги с репликами) в данных нет,
--      поэтому, как у шкафа, кредит даётся сразу при выборе пункта: Айесса (251968) — меню 40960 «Understood.», Риаани (256083) —
--      меню 42848 «Can you remind me…» (единственный пункт; у неё в снифе беседы нет вообще), Альварион (252448) — меню 40790 «Question
--      for you, Alvarion…». У Альвариона и Риаани в базе стояло «при сбросе кастовать 1256875 / 1267046 на себя» — оно остаётся
--      (кредит на существо ничего не засчитывает), добавлены только строки на выбор пункта.
--   2) Страж 252477 «Peacekeeper Elite»: скрипт каждую секунду (повтор 0,9–1,5 с) заново кастовал «Постоянную ложную смерть» (1288546),
--      поэтому он «лежал и двигался» (аура обновлялась). По снифу они должны лежать. Добавлен признак «кастовать, только если ауры нет».
--   3) Учителя разбойников Зефриса (252379 Eltheen Nightbreeze, 251389 Akeri Duskblade, 254087 Miriaan Mistblade) делят меню 41124, в
--      котором нет ни одного пункта, и не привязаны к тренеру. Wowhead Forever: Eltheen обучает 85+20+2 способностям (полный список
--      разбойника) — привязан полный список trainer 26 (как у Keryn Sylvius и др.), добавлен пункт «Хочу обучиться!» (как у других
--      классовых тренеров острова) и флаг тренера. Номер GossipOptionID 95100001 вне диапазона базы.
--   4) Пункт 41313/1 у Valennia Stormfist («Your armor looks much different…») в русской локали остался английским — переведён (ручной
--      перевод, как в остальном проекте). Перевод реплики 316366 (hotfixes) лежит в 2026_10_25_07_hotfixes_broadcast_ru_316366.sql.
-- Можно запускать повторно.

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` = 251968 AND `id` IN (2, 3)) OR (`entryorguid` = 252448 AND `id` = 1) OR (`entryorguid` = 256083 AND `id` IN (1, 2)));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`,
    `event_flags`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(251968, 0, 2, 3, 62, 0, 100, 0, 40960, 0, 85, 1314159, 2, 7, 'Ayessa Dawnsinger - On Gossip Option 0 Selected - Invoker Self Cast Listen to Ayessa (kill credit 273972)'),
(251968, 0, 3, 0, 61, 0, 100, 0, 0, 0, 72, 0, 0, 7, 'Ayessa Dawnsinger - On Link - Close Gossip'),
(252448, 0, 1, 0, 62, 0, 100, 0, 40790, 0, 85, 1256875, 2, 7, 'Alvarion Windfield - On Gossip Option 0 Selected - Invoker Self Cast Sit Down (kill credit 252448)'),
(256083, 0, 1, 2, 62, 0, 100, 0, 42848, 0, 85, 1267046, 2, 7, 'Riaani Nightwind - On Gossip Option 0 Selected - Invoker Self Cast Listen (kill credit 256083)'),
(256083, 0, 2, 0, 61, 0, 100, 0, 0, 0, 72, 0, 0, 7, 'Riaani Nightwind - On Link - Close Gossip');

UPDATE `smart_scripts` SET `action_param2` = 32 WHERE `entryorguid` = 252477 AND `source_type` = 0 AND `id` = 0 AND `action_type` = 11 AND `action_param1` = 1288546;

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 41124 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `GossipOptionID`, `OptionNpc`, `OptionText`, `VerifiedBuild`) VALUES
(41124, 0, 95100001, 3, 'I''d like training!', 0);
DELETE FROM `gossip_menu_option_locale` WHERE `MenuID` = 41124 AND `OptionID` = 0 AND `Locale` = 'ruRU';
INSERT INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`) VALUES
(41124, 0, 'ruRU', 'Хочу обучиться!');
UPDATE `creature_template` SET `npcflag` = (`npcflag` | 16) WHERE `entry` IN (252379, 251389, 254087);
DELETE FROM `creature_trainer` WHERE `CreatureID` IN (252379, 251389, 254087);
INSERT INTO `creature_trainer` (`CreatureID`, `TrainerID`, `MenuID`, `OptionID`) VALUES
(252379, 26, 41124, 0),
(251389, 26, 41124, 0),
(254087, 26, 41124, 0);

UPDATE `gossip_menu_option_locale` SET `OptionText` = 'Твоя броня сильно отличается от брони других миротворцев, которых я видел на острове Зефрис...'
WHERE `MenuID` = 41313 AND `OptionID` = 1 AND `Locale` = 'ruRU' AND `OptionText` LIKE 'Your armor looks much different%';
