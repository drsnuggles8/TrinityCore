-- Migration: 0901_launch_routes (Olo fork)
-- Date:    2026-10-26
-- Purpose: Leveling routes for launch-simulation bots that follow a guide (LaunchStrategy::GuideRoute, Fleet/LaunchRoutes).
--          Filled locally by contrib/launch-sim/import_route.py from guide files on the server's own disk; the table ships
--          empty (guides carry their own licences and never go into the repository).
--
--          Columns:
--            route        short route name (rxp, olo, ...)
--            step         order within the route, from 1
--            combos       which dwarf/gnome classes do the step: bit (race slot * 12 + class id), dwarf slot 0, gnome slot 1
--            action       1 accept, 2 complete an objective, 3 turn in, 4 deathskip, 5 hearth
--            quest        quest id (0 for deathskip / hearth)
--            objective    1-based objective of a complete step (the guide's ".complete Q,N"), else 0
-- Reverts: yes (DROP TABLE playerbot_launch_route_step).

CREATE TABLE IF NOT EXISTS playerbot_launch_route_step (
    route      VARCHAR(16)      NOT NULL,
    step       INT UNSIGNED     NOT NULL,
    combos     INT UNSIGNED     NOT NULL,
    action     TINYINT UNSIGNED NOT NULL,
    quest      INT UNSIGNED     NOT NULL DEFAULT 0,
    objective  TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (route, step)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO playerbot_v2_schema_version (version, sha256) VALUES
    (901, REPEAT('0', 64))
ON DUPLICATE KEY UPDATE applied_at = CURRENT_TIMESTAMP;
