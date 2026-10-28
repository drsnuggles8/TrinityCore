-- The profession books Forever added as reward choices (Wowhead Forever, 28 Oct 2026): Mining for Dummies (247840), Wild
-- Harvest (247841) and Pelt Collecting for Beginners (247846). Use: +2 skill up to 15, or learn the profession when it is
-- not trained and fewer than two are known. The quests came from an older import without them (playtest 28 Oct: Coldridge
-- Valley Mail Delivery gave nothing). One quest per starting zone (QuestieDB Forever, the books' questRewards): Coldridge
-- Valley Mail Delivery (234), The Woodland Protector (458), A Humble Task (752), Galgar's Cactus Apple Surprise (4402),
-- Marla's Last Wish (6395), Rascally Rodents (91743); Harvesting Windstones (93552) has them from its sniff. Safe to run again.
UPDATE `quest_template` SET
  `RewardChoiceItemID1` = 247840, `RewardChoiceItemQuantity1` = 1, `RewardChoiceItemDisplayID1` = 0,
  `RewardChoiceItemID2` = 247841, `RewardChoiceItemQuantity2` = 1, `RewardChoiceItemDisplayID2` = 0,
  `RewardChoiceItemID3` = 247846, `RewardChoiceItemQuantity3` = 1, `RewardChoiceItemDisplayID3` = 0
WHERE `ID` IN (234, 458, 752, 4402, 6395, 91743) AND `RewardChoiceItemID1` = 0;
