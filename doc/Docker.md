# Running the server in Docker

[`contrib/docker-compose`](../contrib/docker-compose) runs this branch in Docker: MySQL, worldserver and bnetserver,
plus three one-shot helpers. `tdb` downloads the Forever database release, `realm` sets the realm's address, and
`extractor` extracts the game data from a client. MySQL is not published. The game uses ports 1119, 8081 and the
realm's world port (8085 for Normal), and the console's SOAP port 7878 is published on the host itself only.
Based on advocaite/TrinityCore#131 by forvpvp, without its spell test module.

You need:

- Docker with Compose v2: Docker Desktop on macOS and Windows, or Docker Engine on Linux.
- Memory: about 8 GB to build (each compile job needs 2.5 GB; with less memory the build runs fewer jobs), and about
  5 GB to run.
- The WoW Classic beta client 1.60.1.70450, for the game data and to play.

All commands below run in `contrib/docker-compose`.

## First start

1. **Settings.** `cp .env.example .env`, fill in the two passwords (`openssl rand -hex 16`), and set `WOW_DIR` to the
   client's World of Warcraft folder. `.env` is git-ignored.
2. **Certificates.** `./make-certs.sh` creates your own certificate authority and the bnetserver certificate in
   `certs/`, which is git-ignored. Players import `certs/ca.cert.pem`; `certs/ca.key.pem` never leaves this machine.
3. **Build:** `docker compose build worldserver`. The first build takes about an hour on four cores. After a
   `git pull`, only what changed is compiled again.
4. **Game data** from the client into `DATA_DIR` (about 700 MB): `docker compose run --rm extractor`
5. **Start:** `docker compose up -d`. The first start downloads and imports the databases (about 1 GB), which takes a
   few minutes; `up` waits for it. Follow it with `docker compose logs -f worldserver`.
6. **Account.** `docker compose attach worldserver`, then:
   ```
   bnetaccount create you@example.com yourpassword
   account set gmlevel 1#1 3 -1
   ```
   Detach with **Ctrl+P, Ctrl+Q**: Ctrl+C would shut the server down.
7. **Check the login TLS.** Both commands must end with `Verify return code: 0 (ok)`:
   ```
   openssl s_client -connect 127.0.0.1:1119 -servername trinity.actual.battle.net -CAfile certs/ca.cert.pem </dev/null
   openssl s_client -connect 127.0.0.1:8081 -servername trinity.actual.battle.net -CAfile certs/ca.cert.pem </dev/null
   ```

## Playing

Each player needs:

- the 1.60.1.70450 client;
- `certs/ca.cert.pem` imported as a trusted root certificate;
- a hosts entry `PUBLIC_IP trinity.actual.battle.net`.

They then start the client with the Forever Launcher, as described in [Building from source](../README.md#building-from-source).
The worldserver runs one realm, set in `.env` (`REALM_ID` and the values listed there; Normal by default). Flag the
other realms offline so the client does not offer their rulesets:
`docker compose exec db sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" -e "UPDATE auth.realmlist SET flag = flag | 2 WHERE id <> REALM_ID"'`
(with your realm's id).

For players on other machines:

1. Set `PUBLIC_IP` to the address they reach this machine at.
2. Set `PUBLISH_IP` to the address the ports are published on. `0.0.0.0` means every network; then limit who can connect
   with a firewall.
3. Run `docker compose up -d` again.

## Day to day

- **Update:** `git pull`, `docker compose build worldserver`, `docker compose up -d`. The worldserver applies new SQL
  updates when it starts.
- **Logs:** `docker compose logs -f worldserver`. The server's log files are in the `logs` volume.
- **Backup** of accounts and characters:
  `docker compose exec db sh -c 'mysqldump -uroot -p"$MYSQL_ROOT_PASSWORD" --databases auth characters' > backup.sql`
- `docker compose down` stops everything. With `-v` it also **deletes the databases**.


## Pathfinding

Without navigation meshes (mmaps) creatures walk straight lines, through walls and up cliffs, and the bots cannot move. Generate
them once from the extracted data (hours; `--threads` as your machine allows), then set `MMAP_PATHFINDING=1` in `.env` and run
`docker compose up -d`:

```
docker compose run --rm --user root --entrypoint sh extractor -c 'mkdir -p mmaps && for m in 0 1 2991; do mmaps_generator $m --threads 8 --silent; done'
```

Maps 0 (Eastern Kingdoms), 1 (Kalimdor) and 2991 (Zephras Isle) cover the open world; `mmaps_generator --threads 8 --silent`
without a map number builds every map, dungeons included. Build the meshes with the same image the server runs: the bots build
changes the generator (roads, off-mesh bridges).

## Bots

The WoW Forever Bots module (`INSTALL-BOTS.md`, `src/modules/PlayerbotV2`): bots that quest and fight around real players,
party companions (`.bot add`), dungeon and battleground bots, an auction house bot.

1. `BUILD_PLAYERBOT_V2=1` in `.env`, then build the image again.
2. The shared bot database, once (the `playerbot` grant is in `mysql-init`; an older db volume needs it by hand):
   ```
   docker compose exec -T db sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD"' < ../../sql/playerbot_shared/playerbot_shared_schema.sql
   docker compose exec -T db sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" playerbot' < ../../sql/playerbot_shared/playerbot_shared_data.sql
   ```
3. Pathfinding (above): bots need the mmaps.
4. `cp worldserver.env.example worldserver.env` and adjust, then `docker compose up -d`. The image's
   `etc/worldserver.conf.d/zz-forever-bots.conf` holds the defaults; `worldserver.env` overrides them.
5. GM accounts get the `.bot` commands; the in-game window is the `ForeverBots` addon from `src/modules/PlayerbotV2/Addon`.
