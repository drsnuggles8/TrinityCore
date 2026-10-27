-- Pre-quests of the Forever QuestieDB (Wowhead Forever data) that the world DB lacked, for clear chain steps in the starting
-- zones (contrib/launch-sim/questie: "pre-quest missing"). The Missing Scholar had itself as its pre-quest and could never be
-- taken. Left for review (Questie's chains have been wrong before): Coldridge Valley Mail Delivery (233), Riverpaw Gnoll
-- Bounty (11), Milly Osworth (3903), Teldrassil (940), the "one of several" pre-quests, and those where ours differs.
-- Safe to run again.
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(95214, 95213),  -- Stolen Blasting Powder <- Stolen Blasting Powder (Dun Morogh)
(99160, 99159),  -- Rime's Wrath <- Finding Warmth (Dun Morogh)
(99162, 99159),  -- Treacherous Cold <- Finding Warmth (Dun Morogh)
(46, 39),        -- Bounty on Murlocs <- Deliver Thomas' Report (Elwynn)
(99129, 99128),  -- A Man About a Murloc <- Slimy Menace (Elwynn)
(99130, 99129),  -- An Enticing Offer <- A Man About a Murloc (Elwynn)
(99131, 99130),  -- Baited for Success <- An Enticing Offer (Elwynn)
(930, 918),      -- The Glowing Fruit <- Timberling Seeds (Teldrassil)
(931, 918),      -- The Shimmering Frond <- Timberling Seeds (Teldrassil)
(92468, 92467),  -- Call of Earth <- Call of Earth (Zephras Isle)
(92597, 92461),  -- Reading the Leylines <- Harmony in Balance (Zephras Isle)
(92698, 92679),  -- What Is My Purpose? <- Blood Tithe (Zephras Isle)
(92741, 92699),  -- Unwelcome Visitors <- The Supreme Magister (Zephras Isle)
(93459, 93165),  -- More Al'Aketh Ears <- Mercy Falls on Deaf Ears (Zephras Isle)
(93736, 92700),  -- Unwelcome Spirits <- The Grand Skyseer (Zephras Isle)
(92727, 92699)   -- The Missing Scholar <- The Supreme Magister (was itself)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);
