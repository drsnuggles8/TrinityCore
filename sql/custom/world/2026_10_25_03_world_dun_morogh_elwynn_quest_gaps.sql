-- Баг-репорты игроков 25.10.2026: №1077 (Отец Гавин), №1063 (ящики груза огнекремния), №941 и №991 («Book Return»).
-- Источники: база Questie для Forever в клиенте (QuestieDB_Forever.toc, данные Wowhead Forever) и страницы
-- wowhead.com/forever (проверено 25.10.2026); в снифах этих данных нет.
--   1) Отец Гавин (1253) принимает задание 99158 (creature_questender есть), но npcflag = 0: ядро не отправляет ему
--      «?» и не даёт сдать. Wowhead Forever: начинает 4 и принимает 5 заданий. Тот же дефект (задания привязаны, флага
--      «выдаёт задания» нет) у ещё трёх существ: Lee Brown (1651, задание 99143), существо 2756 (97277) и Bareth
--      Dawnstone (275954, 99144) — у них бит квестодателя добавлен по тому же признаку. Привязки заданий 99159–99162
--      к Гавину не добавлены: у них нет цепочки требований, а без неё они открылись бы все сразу.
--   2) Объект «Flintfire's Shipment» (672508): таблицы лута нет, поэтому «Добычи нет» и ящик пропадал. Лут этого объекта
--      задан «личным» полем 30 (chestPersonalLoot = 142903), ядро берёт его, когда обычного lootid (поле 1) нет; строк с
--      этим номером в gameobject_loot_template не было (DBErrors.log: «Entry 142903 does not exist but it is used by
--      Gameobject 672508»). Wowhead Forever: предмет 280413 лежит в ящике в 100% случаев (2961 из 2961), по заданию
--      98321 нужно 8 штук. Шаблон объекта не меняется.
--   3) «Lost Book» (248001, задание 91775 «Book Return», нужно 6): падает с Kobold Miner (40) и Kobold Tunneler (475), в
--      таблицах лута его не было. Шанс в данных Wowhead не восстановить (в счёт попадают убийства игроков без задания),
--      взят 40% — как у остальных квестовых предметов Forever с неизвестным шансом в этой базе.
-- Не сделано: «Picture Book: Fun with Elementals» (248002) падает с Nimsy (248464, элитка 6 уровня в Elwynn, 62% по
-- Wowhead) — в базе её нет, а модели, фракции и точных координат нет ни в снифах, ни в Questie; Корнелиус (98319):
-- цель «вход в область 44117» без серверной области (таких целей в базе 505 из 512).
-- Можно запускать повторно.

UPDATE `creature_template` SET `npcflag` = (`npcflag` | 2) WHERE `entry` IN (1253, 1651, 2756, 275954);

DELETE FROM `gameobject_loot_template` WHERE `Entry` = 142903;
INSERT INTO `gameobject_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(142903, 0, 280413, 100, 1, 1, 0, 1, 1, 'Flintfire''s Shipment (672508) - Flintfire''s Shipment');

DELETE FROM `creature_loot_template` WHERE `Entry` IN (40, 475) AND `Item` = 248001;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(40, 0, 248001, 40, 1, 1, 0, 1, 1, 'Kobold Miner - Lost Book'),
(475, 0, 248001, 40, 1, 1, 0, 1, 1, 'Kobold Tunneler - Lost Book');
