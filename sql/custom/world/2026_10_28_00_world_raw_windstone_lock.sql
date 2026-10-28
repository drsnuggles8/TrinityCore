-- Raw Windstone (613286, Harvesting Windstones 93552, Zephras Isle): the sniffed chest template uses Lock.db2 record 4088,
-- the lock that needs skill 42 (see 2026_10_24_00_world_forever_resaan_heirloom_gossip.sql, the same lock on the
-- "Abandoned Belongings" chest), so ordinary players cannot harvest it. Our bots opened it only because the bot API ignores
-- chest locks. Safe to run again.
UPDATE `gameobject_template` SET `Data0` = 0 WHERE `entry` = 613286 AND `type` = 3 AND `Data0` = 4088;
