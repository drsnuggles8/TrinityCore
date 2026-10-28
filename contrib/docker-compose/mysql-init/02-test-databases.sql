-- The test worldserver (compose.yaml, worldserver-test): its own login and character databases, so its bots and launch
-- roster never touch the players' realm. Its first start fills them (Updates.AutoSetup).
CREATE DATABASE IF NOT EXISTS `auth_test` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE DATABASE IF NOT EXISTS `characters_test` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
GRANT ALL PRIVILEGES ON `auth_test`.* TO 'trinity'@'%';
GRANT ALL PRIVILEGES ON `characters_test`.* TO 'trinity'@'%';
