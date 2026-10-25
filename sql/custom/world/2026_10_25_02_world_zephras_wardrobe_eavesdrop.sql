-- Остров Зефрис (Деревня Фалаат): шкаф «Wardrobe — A Very Convenient Obstruction» (254128) из задания 92528 «Among the
-- Faithful» (цель «Узнайте о планах сектантов», кредит за существо 252078) нельзя было нажать (баг-репорт №61 от
-- 25.10.2026: игрок у шкафа на (2958, 1513) с активным заданием, шкаф только выделяется).
-- Что показывает дамп dump_1.60.1.70338_2026-10-11_10-57-34.pkt (проверено по пакетам, tick в миллисекундах):
--   4031354  SMSG_GOSSIP_MESSAGE от Vehicle-2991-254128 (шкаф — «техника»), меню 41095, один пункт 136768
--            «<Спрятаться за шкафом и постараться подслушать разговор сектантов.>»;
--   4033797  CMSG_GOSSIP_SELECT_OPTION (меню 41095, пункт 136768);
--   4034071  игрок сам кастует заклинание 1261177 «Подслушивание» (cast_start + cast_go);
--   4034199  SMSG_GOSSIP_COMPLETE (окно закрывается);
--   4047063  SMSG_QUEST_UPDATE_ADD_CREDIT: задание 92528, объект 252078, 1/1 — через 12,99 с после заклинания.
-- Заклинание 1261177 по данным клиента: один эффект — аура 236 «управление техникой» (игрок садится в технику), цель
-- «ближайшее существо заданного вида» (тип 38), длительность 30 с. Откуда берётся флаг беседы у шкафа (в снифе он у
-- шаблона 0), и что делает сам шкаф, кроме посадки, дамп не показывает.
-- Что исправлено:
--   1) шкафу ставится флаг беседы (npcflag 1) и SmartAI;
--   2) пункт беседы показывается только пока задание 92528 взято и не выполнено (условие QUESTTAKEN);
--   3) выбор пункта: игрок кастует 1261177 на себя, окно закрывается, через 13 с игрок кастует на себя 1254871
--      (тоже «Подслушивание»: эффект «кредит за существо» 252078 + засчитывание задания 92543, которого в базе нет);
--      заклинанию 1261177 задано условие цели — шкаф 254128. Кредит скриптом (действие 33) ядро запрещает, если для
--      существа есть заклинание-кредит («There is a killcredit spell for creatureEntry 252078 (SpellId: 1254871)»,
--      DBErrors.log после рестарта 25.10), поэтому шаблон существа 252078 не нужен.
-- Не проверено в игре: посадит ли заклинание игрока в шкаф. Если нет, цель всё равно засчитается через 13 с.
-- Можно запускать повторно.

UPDATE `creature_template` SET `npcflag` = (`npcflag` | 1), `AIName` = 'SmartAI' WHERE `entry` = 254128;

DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 41095 AND `SourceEntry` = 0)
    OR (`SourceTypeOrReferenceId` = 13 AND `SourceGroup` = 1 AND `SourceEntry` = 1261177);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`,
    `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`,
    `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 41095, 0, 0, 0, 9, 0, 92528, 0, 0, 0, 0, 0, '', 'Show gossip menu 41095 option id 0 if quest Among the Faithful has been taken.'),
(13, 1, 1261177, 0, 0, 51, 0, 5, 254128, 0, 0, 0, 0, '', 'Eavesdropping can only target the Wardrobe (254128).');

DELETE FROM `smart_scripts` WHERE (`entryorguid` = 254128 AND `source_type` = 0) OR (`entryorguid` = 25412800 AND `source_type` = 9);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`,
    `event_flags`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `action_param2`, `action_param3`,
    `target_type`, `comment`) VALUES
-- действие 80 (запуск списка): параметр 1 — ID списка, 2 — тип таймера (2 = всегда), 3 — разрешить перезапись (0/1)
(254128, 0, 0, 1, 62, 0, 100, 0, 41095, 0, 85, 1261177, 2, 0, 7, 'Wardrobe - On Gossip Option 0 Selected - Invoker Self Cast Eavesdropping'),
(254128, 0, 1, 2, 61, 0, 100, 0, 0, 0, 72, 0, 0, 0, 7, 'Wardrobe - On Link - Close Gossip'),
(254128, 0, 2, 0, 61, 0, 100, 0, 0, 0, 80, 25412800, 2, 0, 1, 'Wardrobe - On Link - Run Timed Action List'),
(25412800, 9, 0, 0, 0, 0, 100, 0, 13000, 13000, 85, 1254871, 2, 0, 7, 'Wardrobe Eavesdrop - After 13 s - Invoker Self Cast Eavesdropping (kill credit 252078)');
