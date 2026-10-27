-- Quest givers and enders of the Forever QuestieDB (Wowhead Forever data, GPL-3.0 addon; read 27 Oct 2026) that the
-- world DB lacked, for quests it has and NPCs/objects spawned here (contrib/launch-sim/questie_check.py lists the gaps).
-- Links only, plus the quest-giver flag on NPCs that give or take one of these quests. Safe to run again.

-- creature_queststarter: 17
DELETE FROM `creature_queststarter` WHERE `id`=2081 AND `quest`=489;
DELETE FROM `creature_queststarter` WHERE `id`=2151 AND `quest`=489;
DELETE FROM `creature_queststarter` WHERE `id`=2155 AND `quest`=489;
DELETE FROM `creature_queststarter` WHERE `id`=2155 AND `quest`=490;
DELETE FROM `creature_queststarter` WHERE `id`=5116 AND `quest`=6721;
DELETE FROM `creature_queststarter` WHERE `id`=167 AND `quest`=86776;
DELETE FROM `creature_queststarter` WHERE `id`=240 AND `quest`=91777;
DELETE FROM `creature_queststarter` WHERE `id`=251374 AND `quest`=92466;
DELETE FROM `creature_queststarter` WHERE `id`=251374 AND `quest`=92467;
DELETE FROM `creature_queststarter` WHERE `id`=254151 AND `quest`=93459;
DELETE FROM `creature_queststarter` WHERE `id`=1265 AND `quest`=95212;
DELETE FROM `creature_queststarter` WHERE `id`=1256 AND `quest`=95214;
DELETE FROM `creature_queststarter` WHERE `id`=1376 AND `quest`=96408;
DELETE FROM `creature_queststarter` WHERE `id`=271546 AND `quest`=98323;
DELETE FROM `creature_queststarter` WHERE `id`=383 AND `quest`=99129;
DELETE FROM `creature_queststarter` WHERE `id`=241 AND `quest`=99130;
DELETE FROM `creature_queststarter` WHERE `id`=241 AND `quest`=99131;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(2081, 489),  -- Sentinel Kyra Starsong - Seek Redemption! (Teldrassil)
(2151, 489),  -- Moon Priestess Amara - Seek Redemption! (Teldrassil)
(2155, 489),  -- Sentinel Shayla Nightbreeze - Seek Redemption! (Teldrassil)
(2155, 490),  -- Sentinel Shayla Nightbreeze - Bounty: Gnarlpine Furbolg (Teldrassil)
(5116, 6721),  -- Olmin Burningbeard - The Hunters Path (Ironforge)
(167, 86776),  -- Morhan Coppertongue - Ingredients for the Forge (Loch Modan)
(240, 91777),  -- Marshal Dughan - Rare Books (Elwynn)
(251374, 92466),  -- Windshaper Boro - Call of Earth (Zephras Isle)
(251374, 92467),  -- Windshaper Boro - Call of Earth (Zephras Isle)
(254151, 93459),  -- Vayn Moongaze - More AlAketh Ears (Zephras Isle)
(1265, 95212),  -- Rudra Amberstill - Never Saddle on Quality (Dun Morogh)
(1256, 95214),  -- Quarrymaster Thesten - Stolen Blasting Powder (Dun Morogh)
(1376, 96408),  -- Beldin Steelgrill - A Visitor to Dun Morogh (Dun Morogh)
(271546, 98323),  -- Mountaineer Gretchen - Secure the Mountain (Dun Morogh)
(383, 99129),  -- Jason Mathers - A Man About a Murloc (Elwynn)
(241, 99130),  -- Remy "Two Times" - An Enticing Offer (Elwynn)
(241, 99131);  -- Remy "Two Times" - Baited for Success (Elwynn)

-- creature_questender: 13
DELETE FROM `creature_questender` WHERE `id`=2155 AND `quest`=490;
DELETE FROM `creature_questender` WHERE `id`=167 AND `quest`=86776;
DELETE FROM `creature_questender` WHERE `id`=951 AND `quest`=91777;
DELETE FROM `creature_questender` WHERE `id`=251374 AND `quest`=92466;
DELETE FROM `creature_questender` WHERE `id`=251374 AND `quest`=92468;
DELETE FROM `creature_questender` WHERE `id`=254151 AND `quest`=93459;
DELETE FROM `creature_questender` WHERE `id`=1265 AND `quest`=95212;
DELETE FROM `creature_questender` WHERE `id`=1256 AND `quest`=95213;
DELETE FROM `creature_questender` WHERE `id`=1256 AND `quest`=95214;
DELETE FROM `creature_questender` WHERE `id`=1252 AND `quest`=98323;
DELETE FROM `creature_questender` WHERE `id`=241 AND `quest`=99129;
DELETE FROM `creature_questender` WHERE `id`=241 AND `quest`=99130;
DELETE FROM `creature_questender` WHERE `id`=383 AND `quest`=99131;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(2155, 490),  -- Sentinel Shayla Nightbreeze - Bounty: Gnarlpine Furbolg (Teldrassil)
(167, 86776),  -- Morhan Coppertongue - Ingredients for the Forge (Loch Modan)
(951, 91777),  -- Brother Paxton - Rare Books (Elwynn)
(251374, 92466),  -- Windshaper Boro - Call of Earth (Zephras Isle)
(251374, 92468),  -- Windshaper Boro - Call of Earth (Zephras Isle)
(254151, 93459),  -- Vayn Moongaze - More AlAketh Ears (Zephras Isle)
(1265, 95212),  -- Rudra Amberstill - Never Saddle on Quality (Dun Morogh)
(1256, 95213),  -- Quarrymaster Thesten - Stolen Blasting Powder (Dun Morogh)
(1256, 95214),  -- Quarrymaster Thesten - Stolen Blasting Powder (Dun Morogh)
(1252, 98323),  -- Senir Whitebeard - Secure the Mountain (Dun Morogh)
(241, 99129),  -- Remy "Two Times" - A Man About a Murloc (Elwynn)
(241, 99130),  -- Remy "Two Times" - An Enticing Offer (Elwynn)
(383, 99131);  -- Jason Mathers - Baited for Success (Elwynn)

-- gameobject_questender: 1
DELETE FROM `gameobject_questender` WHERE `id`=581822 AND `quest`=92727;
INSERT INTO `gameobject_questender` (`id`, `quest`) VALUES
(581822, 92727);  -- Bloodstained Satchel - The Missing Scholar (Zephras Isle)

-- quest-giver flag (npcflag 2): 4
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 167;  -- Morhan Coppertongue (Loch Modan, quest 86776 Ingredients for the Forge)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 251371;  -- Falorne Fallwind (Zephras Isle, quest 92597 Reading the Leylines)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 251374;  -- Windshaper Boro (Zephras Isle, quest 92468 Call of Earth)
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 259084;  -- Denaaris Stargale (Stormwind, quest 94947 Welcome to Azeroth)
