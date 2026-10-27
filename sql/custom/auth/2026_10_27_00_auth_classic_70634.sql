-- WoW Classic 1.60 "Forever" release client 1.60.1.70634 (the beta, 70450, ended on 23 Oct).
DELETE FROM `build_info` WHERE `build`=70634;
INSERT INTO `build_info` (`build`,`majorVersion`,`minorVersion`,`bugfixVersion`,`hotfixVersion`) VALUES
(70634,1,60,1,NULL);

-- No auth key is known for Win-x64 70634; see Network.SkipBuildAuthKeyCheck in worldserver.conf (on in contrib/docker-compose).

-- The Forever realms 70-73 move to the release client. A realm serves one build: 70450 (beta) clients no longer see them.
UPDATE `realmlist` SET `gamebuild`=70634 WHERE `id` IN (70,71,72,73) AND `gamebuild` IN (70450,70532);
