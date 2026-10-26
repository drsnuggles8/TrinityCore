-- Quest 870 "The Forgotten Pools" (The Barrens, Horde, Tonga Runetotem): the quest log never shows a
-- finished objective line, so a player who completes the quest believes it is still open.
--
-- Reported: "I swam through the pool, saw the objective-complete message, but the quest log still
-- shows 'explore the waters...'".
--
-- Why: quest 870 has ZERO `quest_objectives` rows and completes through areatrigger 216
-- (`quest_template.Flags` 12 = COMPLETION_AREA_TRIGGER|sharable, `areatrigger_involvedrelation`
-- 216 -> 870, `Player::AreaExploredOrEventHappens` + `CompleteQuest`). The only text the client has
-- for the objective pane is `quest_template.QuestCompletionLog` = "Explore the waters of the
-- Forgotten Pools", which is *flavor text*: it carries no state, so it reads identically before and
-- after the quest turns complete. The server side is fine (status 1 / explored 1 was written for
-- the reporting character), the client just had nothing to tick off.
--
-- The client has no objective data of its own: data/dbc/enUS/QuestObjective.db2, QuestV2.db2 and
-- QuestInfo.db2 were shipped stripped (QuestObjective record count = 0), so every objective the
-- quest log shows comes from this table via SMSG_QUEST_QUERY_RESPONSE. Adding the row is therefore
-- the whole fix; it is display-only, the completion path (trigger 216) is untouched and still fires
-- `AreaExploredOrEventHappens` for the QUEST_FLAGS_COMPLETION_AREA_TRIGGER branch.
--
-- Shape, copied from the rows the vanilla->modern conversion produced for the rest of the Barrens
-- (quest 848 "Fungal Spores", 354 "Deaths in the Family"): id = 90000000 + QuestID*10 + `Order`,
-- `Type` 10 = QUEST_OBJECTIVE_AREATRIGGER (QuestDef.h:364), `ObjectID` = the trigger that completes
-- the quest so `Player::IsQuestObjectiveComplete`'s flag branch (Player.cpp:18185) is satisfied by the
-- same packet that already completes the quest. Amount is ignored for flag-type objectives
-- (data != 0), set to 1 to match the vanilla "one thing to find" wording.
-- VerifiedBuild 0 = Forever-authored row (not a retail dump).
--
-- Idempotent: PK insert with ON DUPLICATE KEY UPDATE. Safe to re-apply.
INSERT INTO `quest_objectives`
    (`ID`, `QuestID`, `Type`, `Order`, `StorageIndex`, `ObjectID`, `Amount`, `ConditionalAmount`,
     `Flags`, `Flags2`, `ProgressBarWeight`, `ParentObjectiveID`, `Visible`, `Description`, `VerifiedBuild`)
VALUES
    (90008700, 870, 10, 0, 0, 216, 1, 0, 0, 0, 0, 0, 1,
     'Explore the waters of the Forgotten Pools', 0)
ON DUPLICATE KEY UPDATE
    `QuestID` = VALUES(`QuestID`),
    `Type` = VALUES(`Type`),
    `Order` = VALUES(`Order`),
    `StorageIndex` = VALUES(`StorageIndex`),
    `ObjectID` = VALUES(`ObjectID`),
    `Amount` = VALUES(`Amount`),
    `Visible` = VALUES(`Visible`),
    `Description` = VALUES(`Description`);

-- Rollback: DELETE FROM `quest_objectives` WHERE `ID` = 90008700 AND `QuestID` = 870;
