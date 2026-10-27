"""SQL for the quest givers/enders the Forever QuestieDB has and our world DB lacks (links only, and the quest-giver flag
on NPCs that give or take a quest). Only for quests we have and NPCs/objects that exist and are spawned here."""
import json, sys, collections
D, out, scope = sys.argv[1], sys.argv[2], (sys.argv[3] if len(sys.argv) > 3 else "launch")
qd = json.load(open(D + "/questie_forever.json"))
def tsv(name):
    return [l.rstrip("\n").split("\t") for l in open(f"{D}/ourdb/{name}.tsv", encoding="utf-8") if l.strip()]
rel = {k: collections.defaultdict(set) for k in ("cs", "ce", "gs", "ge")}
for k, f in (("cs", "creature_queststarter"), ("ce", "creature_questender"), ("gs", "gameobject_queststarter"), ("ge", "gameobject_questender")):
    for i, q in tsv(f):
        rel[k][int(q)].add(int(i))
quests = {int(r[0]) for r in tsv("quests")}
cspawn = {int(i): int(n) for i, n in tsv("creature_spawns")}
gspawn = {int(i): int(n) for i, n in tsv("go_spawns")}
npcflag = {int(i): int(f) for i, f in tsv("npcflags")}
cname = {int(r[0]): r[1] for r in tsv("creature_names")}
gname = {int(r[0]): r[1] for r in tsv("go_names")}
ZONES = {1: "Dun Morogh", 132: "Coldridge", 12: "Elwynn", 9: "Northshire", 141: "Teldrassil", 188: "Shadowglen",
         38: "Loch Modan", 40: "Westfall", 148: "Darkshore", 1537: "Ironforge", 1519: "Stormwind", 1657: "Darnassus",
         16593: "Zephras Isle"}
rows = {"creature_queststarter": [], "creature_questender": [], "gameobject_queststarter": [], "gameobject_questender": []}
flag = {}
skipped = []
for sq, q in sorted(qd["quests"].items(), key=lambda kv: int(kv[0])):
    qid = int(sq)
    if scope == "launch" and q.get("zone") not in ZONES:
        continue
    if qid not in quests:
        continue
    where = ZONES.get(q.get("zone"), str(q.get("zone")))
    for table, key, ids, names, spawns in (("creature_queststarter", "cs", q["start_npc"], cname, cspawn),
                                           ("creature_questender", "ce", q["end_npc"], cname, cspawn),
                                           ("gameobject_queststarter", "gs", q["start_obj"], gname, gspawn),
                                           ("gameobject_questender", "ge", q["end_obj"], gname, gspawn)):
        for i in ids:
            if i in rel[key][qid]:
                if key in ("cs", "ce") and not (npcflag.get(i, 0) & 2) and spawns.get(i):
                    flag[i] = f"{names.get(i, '?')} ({where}, quest {qid} {q['name']})"
                continue
            if i not in names:
                skipped.append(f"{qid} {q['name']}: {key} {i} has no template here"); continue
            if not spawns.get(i):
                skipped.append(f"{qid} {q['name']}: {key} {i} {names[i]} not spawned here"); continue
            rows[table].append((i, qid, f"{names[i]} - {q['name']} ({where})"))
            if key in ("cs", "ce") and not (npcflag.get(i, 0) & 2):
                flag[i] = f"{names[i]} ({where}, quest {qid} {q['name']})"
with open(out, "w", encoding="utf-8", newline="\n") as f:
    f.write("-- Quest givers and enders of the Forever QuestieDB (Wowhead Forever data, GPL-3.0 addon; read 27 Oct 2026) that the\n"
            "-- world DB lacked, for quests it has and NPCs/objects spawned here (contrib/launch-sim/questie_check.py lists the gaps).\n"
            "-- Links only, plus the quest-giver flag on NPCs that give or take one of these quests. Safe to run again.\n")
    for table, lst in rows.items():
        if not lst:
            continue
        f.write(f"\n-- {table}: {len(lst)}\n")
        for i, qid, c in lst:
            f.write(f"DELETE FROM `{table}` WHERE `id`={i} AND `quest`={qid};\n")
        f.write(f"INSERT INTO `{table}` (`id`, `quest`) VALUES\n")
        for n, (i, qid, c) in enumerate(lst):
            sep = ";" if n == len(lst) - 1 else ","
            f.write(f"({i}, {qid}){sep}  -- {c.replace(chr(39), '')}\n")
    if flag:
        f.write(f"\n-- quest-giver flag (npcflag 2): {len(flag)}\n")
        for i, c in sorted(flag.items()):
            f.write(f"UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = {i};  -- {c.replace(chr(39), '')}\n")
print({t: len(l) for t, l in rows.items()}, "flags", len(flag), "skipped", len(skipped))
for s in skipped:
    print("  skipped:", s)
