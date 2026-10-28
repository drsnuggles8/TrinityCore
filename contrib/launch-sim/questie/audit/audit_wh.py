"""Compare the world DB's quest rewards, levels and money with Wowhead Forever's quest listview rows (wh_quests.json)."""
import json, os, collections

D = os.path.dirname(os.path.abspath(__file__))
def tsv(name):
    with open(os.path.join(D, 'ourdb', name), encoding='utf-8') as f:
        return [l.rstrip('\n').split('\t') for l in f if l.strip()]

W = {int(k): v for k, v in json.load(open(os.path.join(D, 'wh_quests.json'))).items()}
disabled = {int(r[0]) for r in tsv('disabled.tsv')}
starts = {int(r[1]) for t in ('creature_queststarter.tsv', 'gameobject_queststarter.tsv') for r in tsv(t)}
ours = {}
for r in tsv('quests.tsv'):
    i = int(r[0])
    ours[i] = dict(title=r[1], fixed=[(int(r[2 + 2 * k]), int(r[3 + 2 * k])) for k in range(4) if int(r[2 + 2 * k])],
                   choice=[(int(r[10 + 2 * k]), int(r[11 + 2 * k])) for k in range(6) if int(r[10 + 2 * k])],
                   level=int(r[27]), minlevel=int(r[28]), money=int(r[29]), startitem=int(r[37]),
                   moneydiff=int(r[25]), moneymult=float(r[26]))
rep = collections.defaultdict(list)
for q in sorted(set(W) & set(ours)):
    w, o = W[q], ours[q]
    if q in disabled:
        continue
    live = q in starts or o['startitem'] > 0
    base = dict(id=q, name=o['title'], live=live)
    wf = {i: c for i, c in (w.get('itemrewards') or [])}
    wc = {i: c for i, c in (w.get('itemchoices') or [])}
    of, oc = dict(o['fixed']), dict(o['choice'])
    if wf != of or wc != oc:
        kind = []
        allw, allo = set(wf) | set(wc), set(of) | set(oc)
        if allw - allo: kind.append('missing')
        if allo - allw: kind.append('extra')
        if not kind:
            if set(wf) != set(of): kind.append('fixed_choice')
            else: kind.append('count')
        rep['reward_' + '+'.join(kind)].append(dict(base, wh_fixed=sorted(wf.items()), wh_choice=sorted(wc.items()),
                                                    ours_fixed=sorted(of.items()), ours_choice=sorted(oc.items())))
    if w.get('level') and o['level'] > 0 and w['level'] != o['level']:
        rep['level'].append(dict(base, ours=o['level'], wh=w['level']))
    if w.get('reqlevel') and o['minlevel'] > 0 and w['reqlevel'] != o['minlevel']:
        rep['minlevel'].append(dict(base, ours=o['minlevel'], wh=w['reqlevel']))
    if w.get('money') and o['money'] != -999999 and w['money'] != max(o['money'], 0):
        rep['money'].append(dict(base, ours=o['money'], wh=w['money']))
    if w.get('money') and o['money'] == -999999:
        rep['money_nofixed'].append(dict(base, wh=w['money'], diff=o['moneydiff'], mult=o['moneymult']))
json.dump(rep, open(os.path.join(D, 'report_wh.json'), 'w'), indent=0)
for k, v in sorted(rep.items()):
    print(k, len(v), 'live', sum(1 for x in v if x['live']))
print('compared', len(set(W) & set(ours) - disabled), 'wh only', len(set(W) - set(ours)))
