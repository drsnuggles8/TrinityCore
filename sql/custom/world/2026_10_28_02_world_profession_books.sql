-- The profession books Forever added as reward choices (Wowhead Forever, 28 Oct 2026): Mining for Dummies (247840), Wild
-- Harvest (247841) and Pelt Collecting for Beginners (247846). Use: +2 skill up to 15, or learn the profession when it is
-- not trained and fewer than two are known. The quests came from an older import without them (playtest 28 Oct: Coldridge
-- Valley Mail Delivery gave nothing). Coldridge Valley Mail Delivery (234) and Galgar's Cactus Apple Surprise (4402).
-- Safe to run again.
UPDATE `quest_template` SET
  `RewardChoiceItemID1` = 247840, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 247841, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 247846, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0
WHERE `ID` IN (234, 4402) AND `RewardChoiceItemID1` = 0;
