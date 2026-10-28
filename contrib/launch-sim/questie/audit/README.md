# Quest audit (28 Oct 2026)

Compares every quest of the world DB with three Forever sources and writes the SQL in
`sql/custom/world/2026_10_28_06..09_*.sql` (see `doc/ForeverQuestGaps.md`). Read-only towards the game install.

Sources:
- the Forever QuestieDB (`QuestieDB_Forever.toc`, base64 CBOR; `qdb.py` reads it): levels, givers, enders, pre-quests,
  and each item's `questRewards`;
- Wowhead Forever's quest list rows (fixed and choice items with counts, level, required level, XP, money) and each
  quest page's XP multiplier (`setupScalingRewards`), read from a local page cache (`CACHE` in `wh_lists.py`, `C` in
  `wh_scaling.py`; nothing is fetched);
- Factoruide's `forever.sqlite` (`FOREVER_SQLITE`): the XP its collector saw in game, the client QuestXP table, the
  checked pre-quest overrides.

Run order (the world DB is read over ssh through `db.py`; `ourdb/` holds the dumps, see the queries in the audit notes):

1. `export_questie.py questie.json`, `wh_lists.py`, `wh_scaling.py`
2. `audit.py` (Questie: rewards, levels, links, pre-quests) -> `report.json`;
   `audit_wh.py` (Wowhead: rewards, levels, money) -> `report_wh.json`;
   `audit_xp.py` (XP at the quest's level) -> `report_xp.json`
3. `gen_sql2.py OUT_DIR` (rewards, levels/XP, money; needs `qxp.txt` = `wdc5b.py QuestXP.db2`), `gen_links.py OUT.sql`
4. `dryrun.py FILE.sql...`: runs each statement inside a rolled-back transaction and prints the rows it would change.
