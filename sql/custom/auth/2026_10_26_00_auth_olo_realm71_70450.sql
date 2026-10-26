-- Olo fork: keep the PvP realm (71) on the 1.60.1.70450 client after 2026_10_24_00_auth_classic_beta_70532.sql moved
-- realms 70-73 to 70532. 70532 clients cannot log in yet (advocaite/TrinityCore#116), so our players stay on 70450 there.
-- Realm 70 (Normal) stays on 70532 for testing a 70532 client once that works. Safe to run again.
UPDATE `realmlist` SET `gamebuild`=70450 WHERE `id`=71;
