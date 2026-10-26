-- Exploration ("trigger") quests across the vanilla world: give each one the objective line it never had.
--
-- Follow-up to 2026_10_25_01_..._quest_objective_forgotten_pools.sql (quest 870, reported because the
-- quest log kept showing "Explore the waters of the Forgotten Pools" after the quest had already
-- completed). The defect is a whole class, not one quest:
--
--   * the quest is completed by walking into an area trigger — `quest_template.Flags & 4`
--     (QUEST_FLAGS_COMPLETION_AREA_TRIGGER) plus a matching `areatrigger_involvedrelation` row, so
--     `Player::AreaExploredOrEventHappens` + `CompleteQuest` fire from
--     `WorldSession::HandleAreaTriggerOpcode` (MiscHandler.cpp:551);
--   * the quest has ZERO `quest_objectives` rows, so the client's objective pane has nothing to
--     tick — `Player::CanCompleteQuest` is trivially satisfied and `CompleteQuest` sets the log slot
--     state, but the *visible* log text is only `quest_template.QuestCompletionLog`, which is flavor
--     text and never changes state. Players therefore read a finished quest as still open;
--   * the vanilla "Objectives" text was carried over into `QuestCompletionLog` by the conversion
--     instead of becoming a real objective row.
--
-- The 18 quests below are every quest in the vanilla range that is in that shape (no objectives, in
-- `areatrigger_involvedrelation`, `Flags & 4`), found with:
--
--   SELECT q.ID, q.LogTitle, q.QuestCompletionLog
--   FROM quest_template q
--   WHERE (q.Flags & 4) AND q.ID < 20000
--     AND NOT EXISTS (SELECT 1 FROM quest_objectives o WHERE o.QuestID = q.ID)
--     AND EXISTS (SELECT 1 FROM areatrigger_involvedrelation a WHERE a.quest = q.ID);
--
-- Shape of each row (same as the conversion's own rows for quests 848/354 and as 870's fix):
--   * `Type` 10 = QUEST_OBJECTIVE_AREATRIGGER (QuestDef.h:364). `Player::IsQuestObjectiveComplete`
--     takes the flag branch for it (Player.cpp:18185: data != 0), so the very packet that used to
--     complete the quest now also credits the objective.
--   * `ObjectID` = -1 on purpose: "any trigger that completes this quest credits this objective"
--     (MiscHandler.cpp:551 skips the ObjectID comparison for -1, and ObjectMgr.cpp:551's own comment
--     recommends it). Several of these quests own more than one trigger — 62 has 88+197, 76 has
--     87+342, 944 has 223..225, 984 has 231..238, 6025 has 3366+3367 — and pinning a single trigger
--     id would have blocked completion from the others. Quest 870 (single trigger 216) keeps its
--     explicit id in the previous file; both forms work.
--   * `Visible` 1, `StorageIndex` 0, `Order` 0, `Amount` 1 (ignored for flag objectives).
--   * `Description` = the quest's own `QuestCompletionLog` text, i.e. the line the player was already
--     reading, now attached to something that ticks. Quest 1149 "Test of Faith" has an empty
--     completion log, so its description is written from its `LogDescription`
--     ("If you have faith, leap from the planks overlooking Thousand Needles.").
--   * `ID` = 90000000 + QuestID*10 + `Order`, the conversion's own convention for vanilla quests
--     (quest 848 -> 90008484, quest 354 -> 90003544..46); verified free before writing this file.
--   * `VerifiedBuild` 0 = Forever-authored, not a retail dump.
--
-- No quest keeps a script path that could complete it without the objective: none of the 17 has
-- `quest_template_addon.ScriptName` or `SpecialFlags` set (checked), so the trigger loop is the only
-- completion path and adding the objective cannot strand a character.
--
-- Idempotent: PK insert with ON DUPLICATE KEY UPDATE. Safe to re-apply, safe on a second DB.
-- Rollback: DELETE FROM `quest_objectives` WHERE `ID` IN (the 18 ids below) AND `VerifiedBuild` = 0;

INSERT INTO `quest_objectives`
    (`ID`, `QuestID`, `Type`, `Order`, `StorageIndex`, `ObjectID`, `Amount`, `ConditionalAmount`,
     `Flags`, `Flags2`, `ProgressBarWeight`, `ParentObjectiveID`, `Visible`, `Description`, `VerifiedBuild`)
VALUES
    -- 62  The Fargodeep Mine            (Elwynn, triggers 88 + 197)
    (90000620,   62, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Scout through the Fargodeep Mine', 0),
    -- 76  The Jasperlode Mine          (Elwynn, triggers 87 + 342)
    (90000760,   76, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Scout through the Jasperlode Mine', 0),
    -- 201 Investigate the Camp         (Stranglethorn, trigger 98)
    (90002010,  201, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Locate the hunters'' camp', 0),
    -- 273 Resupplying the Excavation   (Thousand Needles, trigger 171)
    (90002730,  273, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Find Huldar, Miran, and Saean', 0),
    -- 578 The Stone of the Tides       (Stranglethorn, trigger 196)
    (90005780,  578, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Locate the haunted island', 0),
    -- 944 The Master's Glaive          (Darkshore, triggers 223..225)
    (90009440,  944, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Enter the Master''s Glaive', 0),
    -- 984 How Big a Threat?            (Darkshore, triggers 231..238)
    (90009840,  984, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Find a corrupt furbolg camp', 0),
    -- 1149 Test of Faith               (Thousand Needles, trigger 246) — completion log is empty
    (90011490, 1149, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Leap from the planks overlooking Thousand Needles', 0),
    -- 1265 The Missing Diplomat        (Dustwallow, trigger 302)
    (90012650, 1265, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Sentry Point explored', 0),
    -- 1448 In Search of The Temple     (Dustwallow, trigger 362)
    (90014480, 1448, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Search for the Temple of Atal''Hakkar', 0),
    -- 1658 Crashing the Wickerman Festival (Hillsbrad, trigger 3991)
    (90016580, 1658, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Scout out the Forsaken''s Wickerman Festival.', 0),
    -- 1699 The Rethban Gauntlet        (Redridge, trigger 482)
    (90016990, 1699, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Enter the Rethban Caverns', 0),
    -- 2240 The Hidden Chamber          (Tirisfal, trigger 822)
    (90022400, 2240, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Explore the Hidden Chamber', 0),
    -- 2989 The Altar of Zul            (Hinterlands, trigger 1205)
    (90029890, 2989, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Search the Altar of Zul', 0),
    -- 4811 The Red Crystal             (Darkshore, trigger 2486)
    (90048110, 4811, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Locate the large, red crystal on Darkshore''s eastern mountain range', 0),
    -- 4842 Strange Sources             (Winterspring, trigger 2327)
    (90048420, 4842, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Discover Darkwhisper Gorge', 0),
    -- 6025 Unfinished Business         (Western Plaguelands, triggers 3366 + 3367)
    (90060250, 6025, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Overlook Hearthglen from a high vantage point', 0),
    -- 8286 What Tomorrow Brings        (Tanaris, trigger 3986)
    (90082860, 8286, 10, 0, 0, -1, 1, 0, 0, 0, 0, 0, 1, 'Discover the Brood of Nozdormu.', 0)
ON DUPLICATE KEY UPDATE
    `QuestID` = VALUES(`QuestID`),
    `Type` = VALUES(`Type`),
    `Order` = VALUES(`Order`),
    `StorageIndex` = VALUES(`StorageIndex`),
    `ObjectID` = VALUES(`ObjectID`),
    `Amount` = VALUES(`Amount`),
    `Visible` = VALUES(`Visible`),
    `Description` = VALUES(`Description`);
