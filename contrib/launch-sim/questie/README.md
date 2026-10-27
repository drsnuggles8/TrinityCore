# Checking the world DB against the Forever QuestieDB

Questie's Forever database (the QuestieDB addon, GPL-3.0, built from Wowhead Forever) knows each quest's givers, enders,
class and race masks, pre-quests and objective NPCs. These scripts compare it with the server's world DB. Questie is
evidence, not truth: review every finding before it becomes SQL (its pre-quest chains have been wrong before).

1. `questie_export.py OUT.json`: reads `Interface/AddOns/QuestieDB/QuestieDB_Forever.toc` with a CBOR reader
   (`questiedb.py`, not in this repository; the path is at the top of the script) and writes quests, NPCs and objects.
2. Dump the world DB tables into `ourdb/*.tsv` (one `mysql -N` query per file): `creature_queststarter`,
   `creature_questender`, `gameobject_queststarter`, `gameobject_questender`, `quests` (ID, AllowableClasses,
   PrevQuestID, AllowableRaces), `creature_spawns` / `go_spawns` (id, count), `npcflags` (entry, npcflag),
   `creature_names` / `go_names` (entry, name).
3. `questie_check.py DIR [launch|all]`: the gaps, by kind (quest missing, giver/ender not linked or not spawned, kill
   target not spawned, class mask, pre-quests).
4. `questie_fix.py DIR OUT.sql [launch|all]`: SQL for the giver/ender links and quest-giver flags only, for quests the
   DB has and NPCs/objects spawned here (sql/custom/world/2026_10_27_50_world_questie_quest_links.sql came from it).
