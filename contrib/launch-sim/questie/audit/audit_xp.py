"""Quest XP at the quest's own level: ours (QuestXP[level][difficulty] x multiplier, our formula) against Wowhead Forever
(listview XP x the page's XP multiplier) and the XP Factoruide's collector saw in game (beta, no level penalty)."""
import json, os, sqlite3, collections

D = os.path.dirname(os.path.abspath(__file__))
def tsv(name):
    with open(os.path.join(D, 'ourdb', name), encoding='utf-8') as f:
        return [l.rstrip('\n').split('\t') for l in f if l.strip()]

def rnd(xp):
    """Retail RoundXPValue: steps of 5 / 10 / 25 / 50."""
    if xp <= 100: return 5 * ((xp + 2) // 5)
    if xp <= 500: return 10 * ((xp + 5) // 10)
    if xp <= 1000: return 25 * ((xp + 12) // 25)
    return 50 * ((xp + 25) // 50)

W = {int(k): v for k, v in json.load(open(os.path.join(D, 'wh_quests.json'))).items()}
S = {int(k): v for k, v in json.load(open(os.path.join(D, 'wh_scaling.json'))).items()}
fs = sqlite3.connect(os.environ.get('FOREVER_SQLITE', os.path.join(D, 'forever.sqlite')))
qxp = {r[0]: r[1:] for r in fs.execute('select * from client_questxp')}
obs = {r[0]: r[1] for r in fs.execute('select id, xp from m_quest where xp_observed = 1')}
disabled = {int(r[0]) for r in tsv('disabled.tsv')}
starts = {int(r[1]) for t in ('creature_queststarter.tsv', 'gameobject_queststarter.tsv') for r in tsv(t)}
rep = collections.defaultdict(list)
for r in tsv('quests.tsv'):
    q, diff, mult, lvl = int(r[0]), int(r[22]), float(r[23]), int(r[27])
    if q not in W or q in disabled or lvl <= 0 or lvl not in qxp or not W[q].get('xp'):
        continue
    base = qxp[lvl][diff] if diff < 10 else 0
    ours = int(-(-base * mult // 1))
    wm = (S.get(q, {}).get('xp') or {}).get('multiplier') or 1
    whx = W[q]['xp']
    want = rnd(int(round(whx * wm)))
    x = dict(id=q, name=r[1], live=q in starts, level=lvl, wh_level=W[q].get('level'), diff=diff, mult=mult, base=base,
             ours=ours, wh=whx, wh_mult=wm, want=want, observed=obs.get(q), paged=q in S)
    if ours == want:
        rep['same'].append(q)
    elif rnd(ours) == want:
        rep['rounding_only'].append(x)
    elif wm != 1 and rnd(int(round(base * wm))) == want:
        rep['multiplier_missing'].append(x)
    elif W[q].get('level') != lvl:
        rep['level_differs'].append(x)
    else:
        cand = [k for k in range(10) if rnd(int(round(qxp[lvl][k] * wm))) == want]
        x['diff_fits'] = cand
        rep['other'].append(x)
json.dump(rep, open(os.path.join(D, 'report_xp.json'), 'w'), indent=0)
for k, v in rep.items():
    print(k, len(v))
for x in rep['other']:
    print(x['id'], x['name'][:30], 'lvl', x['level'], 'd', x['diff'], x['mult'], 'ours', x['ours'], 'wh', x['wh'], 'x', x['wh_mult'],
          'want', x['want'], 'obs', x['observed'], 'fits', x['diff_fits'])
