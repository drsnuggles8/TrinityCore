-- Migration: 0900_launch_sim (Olo fork; numbered 0900+ so upstream's own migrations never collide)
-- Date:    2026-10-26
-- Purpose: The launch simulation's roster (Fleet/LaunchSim, `.playerbot launch`): the fresh level-1 characters that start
--          together with a real player, one row per character. The character itself is a normal V2 bot (playerbot_v2_character,
--          distribution_level 0); this row adds what makes it a launch player.
--
--          Columns:
--            character_guid_low   the character (characters.guid)
--            name, race, class, gender   kept so `.playerbot launch reset` can recreate the same character fresh at level 1
--            persona              index into LaunchSim's persona table (strategy x temperament x pace)
--            arrival_s            seconds after the start (T0) at which the bot logs in
--            session_s            seconds it plays before logging out for good (0 = plays on)
--            seed                 per-bot randomness for its pace, pauses and temperament
-- Reverts: yes (DROP TABLE playerbot_launch_bot).

CREATE TABLE IF NOT EXISTS playerbot_launch_bot (
    character_guid_low BIGINT UNSIGNED NOT NULL PRIMARY KEY,
    name               VARCHAR(12)      NOT NULL,
    race               TINYINT UNSIGNED NOT NULL,
    class              TINYINT UNSIGNED NOT NULL,
    gender             TINYINT UNSIGNED NOT NULL,
    persona            TINYINT UNSIGNED NOT NULL,
    arrival_s          INT UNSIGNED     NOT NULL,
    session_s          INT UNSIGNED     NOT NULL DEFAULT 0,
    seed               INT UNSIGNED     NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO playerbot_v2_schema_version (version, sha256) VALUES
    (900, REPEAT('0', 64))
ON DUPLICATE KEY UPDATE applied_at = CURRENT_TIMESTAMP;
