-- Quest money as on Wowhead Forever (quest list rows, Olo's page cache of 23 Oct 2026). The six Forever quests added on
-- 28 Oct (2026_10_28_04) paid nothing; vanilla quests whose Forever level or money changed; 234 and 752 (the profession
-- book quests) pay no money on Forever. Wowhead's money agrees with ours for 2371 quests paying nothing and 101 Forever
-- quests paying by difficulty. Fixed amounts go into quest_classic_money (it overrides the difficulty). Safe to run again.
INSERT INTO `quest_classic_money` (`ID`, `RewardMoney`, `RewardMoneyMaxLevel`) VALUES
(86574, 1700, 0), -- Stonetalon Supply Run, was difficulty
(95217, 175, 0), -- The Quarry's Smith, was difficulty
(96875, 250, 0), -- Beasts of Thunder Ridge, was difficulty
(97914, 90, 0), -- Expanding Horizons, was difficulty
(97926, 90, 0), -- Making Do, was difficulty
(98298, 800, 0), -- Arugal's Folly, was difficulty
(299, 2200, 0), -- Uncovering the Past, was 2500
(430, 40, 0), -- Return to Quinn, was 0
(544, 6500, 0), -- Prison Break In, was 7000
(556, 3500, 0), -- Stone Tokens, was 3000
(635, 1800, 0), -- Crystal in the Mountains, was 0
(642, 4000, 0), -- The Princess Trapped, was 0
(651, 4500, 0), -- Stones of Binding, was 0
(692, 5500, 0), -- The Lost Fragments, was 0
(745, 225, 0), -- Sharing the Land, was 125
(1090, 1400, 0), -- Gerenzo's Orders, was 0
(234, 0, 0), -- Coldridge Valley Mail Delivery, was 7
(752, 0, 0) -- A Humble Task, was 17
ON DUPLICATE KEY UPDATE `RewardMoney` = VALUES(`RewardMoney`);
