"""Collect Wowhead Forever's quest listview rows (levels, XP, money, fixed and choice reward items with counts) from Olo's
local page cache (Olo/tools/data/wowhead_cache, fetched 23 Oct 2026; read-only) into wh_quests.json."""
import glob, json, os, re

CACHE = r"C:\Program Files (x86)\World of Warcraft\_forever_\Olo\tools\data\wowhead_cache"
KEYS = ('itemrewards', 'itemchoices', 'level', 'reqlevel', 'money', 'xp', 'side', 'reqclass', 'name', 'category',
        'category2', 'reqrace', 'type')
DEC = json.JSONDecoder()
out = {}
srcs = {}
for p in glob.glob(os.path.join(CACHE, 'www.wowhead.com_forever_quest*')):
    s = open(p, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'\{"category":', s):
        try:
            row, _ = DEC.raw_decode(s, m.start())
        except Exception:
            continue
        if 'id' not in row or 'itemrewards' not in row:
            continue
        r = {k: row.get(k) for k in KEYS}
        q = row['id']
        if q in out and out[q] != r:
            srcs.setdefault(q, []).append(os.path.basename(p))
        out.setdefault(q, r)
json.dump(out, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'wh_quests.json'), 'w'), indent=0)
print(len(out), 'quests;', len(srcs), 'with differing rows across pages')
