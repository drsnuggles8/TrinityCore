"""XP multiplier / coin of each cached Wowhead Forever quest page (setupScalingRewards) -> wh_scaling.json."""
import glob, json, os, re
C = r"C:\Program Files (x86)\World of Warcraft\_forever_\Olo\tools\data\wowhead_cache"
out = {}
for p in glob.glob(os.path.join(C, 'www.wowhead.com_forever_quest=*.html')):
    q = int(re.search(r'quest=(\d+)', p).group(1))
    t = open(p, encoding='utf-8', errors='replace').read()
    m = re.search(r'setupScalingRewards\((\{.*?\})\)', t)
    if m:
        try:
            out[q] = json.loads(m.group(1))
        except ValueError:
            pass
json.dump(out, open('wh_scaling.json', 'w'))
import collections
print(len(out), collections.Counter(v.get('xp', {}).get('multiplier') for v in out.values()).most_common(15))
