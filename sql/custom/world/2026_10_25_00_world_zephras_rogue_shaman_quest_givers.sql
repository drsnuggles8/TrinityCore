-- Остров Зефрис (Thendal Grove): классовые задания разбойника и шамана не брались у Rorian the Dayseeker.
-- Все четыре классовых задания устроены одинаково (проверено по живой базе 25.10.2026):
--   выдаёт Rorian the Dayseeker (251361), предыдущее задание 92461 (его же), одна цель «получить записку»
--   (type 1, предмет = StartItem, flags 32), сдать тренеру своего класса, который назван в описании задания.
-- Для охотника (92482) и друида (92485) в снифах всё было: выдаёт Рориан, принимают Tai'ree Farsight (251376)
-- и Xyton Silverwind (251373). У разбойника (92483) и шамана (92484) записаны только шаблоны заданий:
-- ни выдающего, ни принимающего, ни предыдущего задания, поэтому сервер их не предлагал, хотя клиент над Рорианом
-- показывал «!» (жалоба игрока-разбойника, 25.10).
-- Принимающие по описанию заданий: «Read the Simple Note then speak with Akeri Duskblade» (251389) и
-- «Examine the Humming Recall Crystal then speak with Windshaper Boro» (251374); оба стоят на острове.
-- Показ «!» над Рорианом клиентом не проверялся: откуда клиент берёт эту подсказку, неизвестно.
-- Предыдущее задание 92461 взято по образцу охотника и друида (в снифах для этих двух его нет).
-- Тексты награды и запроса (quest_offer_reward / quest_request_items) в снифах для этих двух заданий отсутствуют и не
-- выдумываются: сдача пройдёт без текста награды, пока их не запишет сниф.
-- Ограничение по классу (AllowableClasses) ставит 2026_10_23_00_world_zephras_class_quests_class_mask.sql.
-- Можно запускать повторно.

INSERT IGNORE INTO `creature_queststarter` (`id`, `quest`) VALUES
(251361, 92483),   -- Rorian the Dayseeker -> At Home in the Shadows
(251361, 92484);   -- Rorian the Dayseeker -> Embracing the Elements

INSERT IGNORE INTO `creature_questender` (`id`, `quest`) VALUES
(251389, 92483),   -- Akeri Duskblade (тренер разбойников)
(251374, 92484);   -- Windshaper Boro (тренер шаманов)

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(92483, 92461),
(92484, 92461)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);
