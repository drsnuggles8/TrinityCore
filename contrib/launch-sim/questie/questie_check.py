"""Compare the server's world DB (ourdb/*.tsv) with the Forever QuestieDB export: what Questie says a quest needs that
our DB lacks. Questie is evidence, not truth (its chains misled twice on 11 Oct): every finding is for review."""
import json, sys, collections
D = sys.argv[1]
qd = json.load(open(D + "/questie_forever.json"))
def tsv(name):
    return [l.rstrip("\n").split("\t") for l in open(f"{D}/ourdb/{name}.tsv", encoding="utf-8") if l.strip()]
rel = {k: collections.defaultdict(set) for k in ("cs", "ce", "gs", "ge")}
for k, f in (("cs", "creature_queststarter"), ("ce", "creature_questender"), ("gs", "gameobject_queststarter"), ("ge", "gameobject_questender")):
    for i, q in tsv(f):
        rel[k][int(q)].add(int(i))
quests = {int(r[0]): (int(r[1]), int(r[2])) for r in tsv("quests")}
cspawn = {int(i): int(n) for i, n in tsv("creature_spawns")}
gspawn = {int(i): int(n) for i, n in tsv("go_spawns")}
npcflag = {int(i): int(f) for i, f in tsv("npcflags")}
ZONES = {1: "Dun Morogh", 132: "Coldridge", 12: "Elwynn", 9: "Northshire", 141: "Teldrassil", 188: "Shadowglen",
         38: "Loch Modan", 40: "Westfall", 148: "Darkshore", 1537: "Ironforge", 1519: "Stormwind", 1657: "Darnassus",
         16593: "Zephras Isle"}
only = sys.argv[2] if len(sys.argv) > 2 else "launch"
CLASSMASK_ALL = 0x5FF
find = collections.defaultdict(list)
for sq, q in qd["quests"].items():
    qid = int(sq)
    z = q.get("zone")
    if only == "launch" and z not in ZONES:
        continue
    tag = f"{qid} {q['name']} [{ZONES.get(z, z)}]"
    if qid not in quests:
        find["quest missing from quest_template"].append(tag); continue
    cls, prev = quests[qid]
    for n in q["start_npc"]:
        if n not in rel["cs"][qid]: find["giver not linked (creature_queststarter)"].append(f"{tag}: npc {n}")
        elif not cspawn.get(n): find["giver not spawned"].append(f"{tag}: npc {n} {qd['npcs'].get(str(n), {}).get('name')}")
        elif not (npcflag.get(n, 0) & 2): find["giver without quest-giver flag"].append(f"{tag}: npc {n}")
    for n in q["end_npc"]:
        if n not in rel["ce"][qid]: find["ender not linked (creature_questender)"].append(f"{tag}: npc {n}")
        elif not cspawn.get(n): find["ender not spawned"].append(f"{tag}: npc {n} {qd['npcs'].get(str(n), {}).get('name')}")
        elif not (npcflag.get(n, 0) & 2): find["ender without quest-giver flag"].append(f"{tag}: npc {n}")
    for o in q["start_obj"]:
        if o not in rel["gs"][qid]: find["object giver not linked"].append(f"{tag}: go {o}")
    for o in q["end_obj"]:
        if o not in rel["ge"][qid]: find["object ender not linked"].append(f"{tag}: go {o}")
    for n in q["kill"]:
        if not cspawn.get(n): find["kill target not spawned"].append(f"{tag}: npc {n} {qd['npcs'].get(str(n), {}).get('name')}")
    for o in q["objs"]:
        if not gspawn.get(o): find["objective object not spawned"].append(f"{tag}: go {o} {qd['objects'].get(str(o), {}).get('name')}")
    qc = q["classes"] or 0
    if qc and (qc & CLASSMASK_ALL) != CLASSMASK_ALL and cls != qc:
        find["class restriction differs"].append(f"{tag}: questie {qc}, ours {cls}")
    pre = set(q["pre_any"]) | set(q["pre_all"])
    if pre and prev and abs(prev) not in pre:
        find["pre-quest differs"].append(f"{tag}: questie {sorted(pre)}, ours {prev}")
    if pre and not prev:
        find["pre-quest missing (ours none)"].append(f"{tag}: questie {sorted(pre)}")
for k in sorted(find, key=lambda k: -len(find[k])):
    print(f"\n## {k}: {len(find[k])}")
    for line in find[k][:12]:
        print("  ", line)
    if len(find[k]) > 12:
        print("   ...")
