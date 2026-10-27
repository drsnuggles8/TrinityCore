"""Export the Forever QuestieDB (read-only) to JSON for checking the server's world DB: per quest the givers, enders,
levels, class/race masks, pre-quests and objective NPCs/objects/items; per NPC/object used, its spawn zones."""
import json, sys
sys.path.insert(0, r"C:\Program Files (x86)\World of Warcraft\_forever_\Olo\tools")
import questiedb

def lua(v):
    """A Lua table as {1-based key: value}: dense tables arrive as lists, sparse ones as dicts."""
    if isinstance(v, dict):
        return {int(k): x for k, x in v.items()}
    if isinstance(v, (list, tuple)):
        return {i + 1: x for i, x in enumerate(v)}
    return {}

def idlist(v):
    return [int(x) for x in lua(v).values() if isinstance(x, (int, float))]

db = questiedb.load()
out = {"quests": {}, "npcs": {}, "objects": {}}
for qid in db.quest_ids():
    q = db.quest(qid)
    if not q:
        continue
    st, fi, ob = lua(q.get("startedBy")), lua(q.get("finishedBy")), lua(q.get("objectives"))
    first = lambda t: [int(o[0]) for o in lua(t).values() if isinstance(o, (list, tuple)) and o and o[0]]
    out["quests"][qid] = {
        "name": q.get("name"), "level": q.get("questLevel"), "req": q.get("requiredLevel"),
        "classes": q.get("requiredClasses") or 0, "races": q.get("requiredRaces") or 0,
        "start_npc": idlist(st.get(1)), "start_obj": idlist(st.get(2)), "start_item": idlist(st.get(3)),
        "end_npc": idlist(fi.get(1)), "end_obj": idlist(fi.get(2)),
        "pre_all": idlist(q.get("preQuestGroup")), "pre_any": idlist(q.get("preQuestSingle")),
        "kill": first(ob.get(1)), "objs": first(ob.get(2)), "items": first(ob.get(3)),
        "zone": q.get("zoneOrSort"),
    }
need_n = {n for q in out["quests"].values() for k in ("start_npc", "end_npc", "kill") for n in q[k]}
need_o = {o for q in out["quests"].values() for k in ("start_obj", "end_obj", "objs") for o in q[k]}
for kind, need, get, key in (("npcs", need_n, db.npc, "npcs"), ("objects", need_o, db.obj, "objects")):
    for i in need:
        r = get(i)
        if r:
            out[key][i] = {"name": r.get("name"), "zones": sorted(lua(r.get("spawns")).keys())}
json.dump(out, open(sys.argv[1], "w"), separators=(",", ":"))
print(len(out["quests"]), "quests,", len(out["npcs"]), "npcs,", len(out["objects"]), "objects")
