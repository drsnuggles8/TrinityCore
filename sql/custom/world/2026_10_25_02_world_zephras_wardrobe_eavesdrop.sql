-- Остров Зефрис (Деревня Фалаат): шкаф «Wardrobe — A Very Convenient Obstruction» (254128) из задания 92528 «Among the
-- Faithful» (цель «Узнайте о планах сектантов», засчитывается за существо 252078) нельзя было нажать (баг-репорт №61
-- от 25.10.2026, игрок у шкафа на (2958, 1513) с активным заданием).
-- Причина (по живой базе и снифам): у шкафа npcflag = 0, при этом меню беседы 41095 в снифах есть, а единственный пункт
-- в нём — «<Hide behind the wardrobe and try to listen in on the cultists' conversation.>». Скрипта на выбор пункта
-- нет, существа 252078 в creature_template тоже нет (это только «кредит» для цели задания), поэтому цель никто не
-- засчитывал. Откуда в оригинале берётся флаг беседы (по заданию для игрока или иначе), снифы не показывают.
-- Что сделано:
--   1) шкафу ставится флаг беседы (npcflag 1) и SmartAI;
--   2) пункт показывается только пока задание 92528 взято и не выполнено (условие QUESTTAKEN, как у других
--      квестовых пунктов в базе);
--   3) выбор пункта засчитывает цель задания (кредит 252078) и закрывает окно.
-- Не сделано: сцены подслушивания (разговор сектантов, перемещение игрока за шкаф) в снифах нет, она не выдумывается —
-- цель засчитывается сразу.
-- Можно запускать повторно.

UPDATE `creature_template` SET `npcflag` = (`npcflag` | 1), `AIName` = 'SmartAI' WHERE `entry` = 254128;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 41095 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`,
    `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`,
    `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 41095, 0, 0, 0, 9, 0, 92528, 0, 0, 0, 0, 0, '', 'Show gossip menu 41095 option id 0 if quest Among the Faithful has been taken.');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 254128 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`,
    `event_flags`, `event_param1`, `event_param2`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(254128, 0, 0, 1, 62, 0, 100, 0, 41095, 0, 33, 252078, 7, 'Wardrobe - On Gossip Option 0 Selected - Quest Credit Learn of the Cultists'' Plans'),
(254128, 0, 1, 0, 61, 0, 100, 0, 0, 0, 72, 0, 7, 'Wardrobe - On Link - Close Gossip');
