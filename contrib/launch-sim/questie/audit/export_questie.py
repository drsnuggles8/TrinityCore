"""Export the Forever QuestieDB quests (levels, givers, enders, pre-quests) and the item -> quest reward lists to JSON."""
import json, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import qdb

def lua(v):
    if isinstance(v, dict):
        return {int(k): x for k, x in v.items()}
    if isinstance(v, (list, tuple)):
        return {i + 1: x for i, x in enumerate(v)}
    return {}

def ids(v):
    if isinstance(v, (int, float)):
        return [int(v)]
    return [int(x) for x in lua(v).values() if isinstance(x, (int, float))]

s = qdb.store()
qids = sorted(int(k.split('-')[2]) for k in s if re.match(r'^X-Quest-\d+-S$', k))
iids = sorted(int(k.split('-')[2]) for k in s if re.match(r'^X-Item-\d+-S$', k))
out = {'quests': {}, 'rewards': {}}
for q in qids:
    e = qdb.entity('Quest', q, skip=('waypoints', 'objectivesText', 'extraObjectives'))
    if not e:
        continue
    st, fi = lua(e.get('startedBy')), lua(e.get('finishedBy'))
    out['quests'][q] = {
        'name': e.get('name'), 'level': e.get('questLevel'), 'req': e.get('requiredLevel'),
        'races': e.get('requiredRaces') or 0, 'classes': e.get('requiredClasses') or 0,
        'start_npc': ids(st.get(1)), 'start_obj': ids(st.get(2)), 'start_item': ids(st.get(3)),
        'end_npc': ids(fi.get(1)), 'end_obj': ids(fi.get(2)),
        'pre_all': ids(e.get('preQuestGroup')), 'pre_any': ids(e.get('preQuestSingle')),
        'next': e.get('nextQuestInChain') or 0, 'exclusive': ids(e.get('exclusiveTo')),
        'breadcrumb_for': e.get('breadcrumbForQuestId') or 0, 'zone': e.get('zoneOrSort'),
        'flags': e.get('questFlags') or 0, 'special': e.get('specialFlags') or 0,
    }
for i in iids:
    e = qdb.entity('Item', i, skip=('waypoints', 'npcDrops', 'objectDrops', 'itemDrops', 'vendors', 'relatedQuests'))
    if not e or not e.get('questRewards'):
        continue
    for q in ids(e['questRewards']):
        out['rewards'].setdefault(q, []).append(i)
json.dump(out, open(sys.argv[1], 'w'), separators=(',', ':'))
print(len(out['quests']), 'quests,', len(out['rewards']), 'quests with item rewards')
