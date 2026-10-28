"""Compare the world DB's quests with the Forever QuestieDB export (questie.json) and Factoruide's forever.sqlite
(Wowhead Forever XP, observed XP, levels; reward counts / fixed-or-choice). Writes report.json with one list per kind."""
import json, os, sqlite3, collections

D = os.path.dirname(os.path.abspath(__file__))
FS = os.environ.get('FOREVER_SQLITE', os.path.join(D, 'forever.sqlite'))

def tsv(name):
    with open(os.path.join(D, 'ourdb', name), encoding='utf-8') as f:
        return [l.rstrip('\n').split('\t') for l in f if l.strip()]

Q = json.load(open(os.path.join(D, 'questie.json')))
QQ = {int(k): v for k, v in Q['quests'].items()}
QR = {int(k): set(v) for k, v in Q['rewards'].items()}

ours = {}
for r in tsv('quests.tsv'):
    i = int(r[0])
    fixed = [(int(r[2 + 2 * k]), int(r[3 + 2 * k])) for k in range(4) if int(r[2 + 2 * k])]
    choice = [(int(r[10 + 2 * k]), int(r[11 + 2 * k])) for k in range(6) if int(r[10 + 2 * k])]
    ours[i] = dict(title=r[1], fixed=fixed, choice=choice, xpdiff=int(r[22]), xpmult=float(r[23]), ct=int(r[24]),
                   level=int(r[27]), minlevel=int(r[28]), money=int(r[29]), prev=int(r[30]), next=int(r[31]),
                   excl=int(r[32]), flags=int(r[35]), spell=int(r[36]), startitem=int(r[37]))
disabled = {int(r[0]) for r in tsv('disabled.tsv')}
cspawn = {int(r[0]) for r in tsv('creature_spawns.tsv')}
gspawn = {int(r[0]) for r in tsv('go_spawns.tsv')}
rel = collections.defaultdict(set)
for t, k in (('creature_queststarter', 'sn'), ('creature_questender', 'en'), ('gameobject_queststarter', 'so'),
             ('gameobject_questender', 'eo')):
    for a, b in tsv(t + '.tsv'):
        rel[(k, int(b))].add(int(a))

db = sqlite3.connect(FS)
mq = {r[0]: dict(level=r[1], min=r[2], xp=r[3], obs=r[4], status=r[5], hidden=r[6], reason=r[7])
      for r in db.execute('select id, level, min_level, xp, xp_observed, status, hidden, status_reason from m_quest')}
mr = collections.defaultdict(dict)
for q, i, c, ch in db.execute('select quest_id, item_id, count, choice from m_quest_reward'):
    mr[q][i] = (c, ch)
qxp = {r[0]: r[1:] for r in db.execute('select * from client_questxp')}
items_known = None

rep = collections.defaultdict(list)
both = sorted(set(QQ) & set(ours))
for q in both:
    qq, o, m = QQ[q], ours[q], mq.get(q, {})
    live = q not in disabled and m.get('status', 'available') == 'available'
    if not live:
        rep['_not_live'].append(q)
        continue
    giver_spawned = any(n in cspawn for n in rel[('sn', q)]) or any(g in gspawn for g in rel[('so', q)]) or o['startitem']
    base = dict(id=q, name=qq['name'], spawned=bool(giver_spawned))
    # rewards
    oi = {i for i, _ in o['fixed']} | {i for i, _ in o['choice']}
    qi = QR.get(q, set())
    if qi - oi:
        rep['reward_missing'].append(dict(base, missing=sorted(qi - oi), ours_fixed=o['fixed'], ours_choice=o['choice'],
                                          mr=mr.get(q)))
    if oi - qi:
        rep['reward_extra'].append(dict(base, extra=sorted(oi - qi), questie=sorted(qi), ours_fixed=o['fixed'],
                                        ours_choice=o['choice'], mr=mr.get(q)))
    for kind, lst in (('fixed', o['fixed']), ('choice', o['choice'])):
        for i, c in lst:
            if i in mr.get(q, {}):
                mc, mch = mr[q][i]
                if (mch == 1) != (kind == 'choice'):
                    rep['reward_kind'].append(dict(base, item=i, ours=kind, vmangos='choice' if mch else 'fixed'))
                elif mc and mc != c:
                    rep['reward_count'].append(dict(base, item=i, ours=c, vmangos=mc, kind=kind))
    # levels
    if qq.get('level') and o['level'] > 0 and qq['level'] != o['level']:
        rep['level'].append(dict(base, ours=o['level'], questie=qq['level'], wowhead=m.get('level')))
    if o['level'] <= 0:
        rep['level_unset'].append(dict(base, questie=qq.get('level')))
    if qq.get('req') and o['minlevel'] > 0 and qq['req'] != o['minlevel']:
        rep['minlevel'].append(dict(base, ours=o['minlevel'], questie=qq['req'], wowhead=m.get('min')))
    if o['minlevel'] < 0:
        rep['minlevel_unset'].append(dict(base, questie=qq.get('req'), ours_level=o['level']))
    # XP at the quest's own level
    lvl = o['level'] if o['level'] > 0 else (qq.get('level') or 0)
    if m.get('xp') and lvl in qxp:
        ox = int(-(-qxp[lvl][o['xpdiff']] * o['xpmult'] // 1)) if o['xpdiff'] < 10 else 0
        if ox != m['xp']:
            cand = [k for k in range(10) if qxp[lvl][k] == m['xp']]
            rep['xp'].append(dict(base, ours=ox, wowhead=m['xp'], observed=m['obs'], level=lvl, diff=o['xpdiff'],
                                  mult=o['xpmult'], ct=o['ct'], exact_diff=cand))
    # givers / enders
    for qk, ok, sp in (('start_npc', 'sn', cspawn), ('end_npc', 'en', cspawn), ('start_obj', 'so', gspawn),
                       ('end_obj', 'eo', gspawn)):
        miss = [e for e in qq[qk] if e not in rel[(ok, q)]]
        if miss:
            rep['link_missing_' + ok].append(dict(base, missing=miss, missing_spawned=[e for e in miss if e in sp],
                                                  ours=sorted(rel[(ok, q)])))
        extra = [e for e in rel[(ok, q)] if e not in qq[qk]]
        if extra and qq[qk]:
            rep['link_extra_' + ok].append(dict(base, extra=extra, questie=qq[qk]))
    # pre-quests
    pa, py = qq['pre_all'], qq['pre_any']
    if (pa or py) and not o['prev']:
        rep['pre_missing'].append(dict(base, pre_all=pa, pre_any=py))
    elif o['prev'] and abs(o['prev']) not in pa + py:
        rep['pre_differs'].append(dict(base, ours=o['prev'], pre_all=pa, pre_any=py))

rep['_only_questie'] = sorted(set(QQ) - set(ours))
rep['_both'] = len(both)
json.dump(rep, open(os.path.join(D, 'report.json'), 'w'), indent=0)
for k, v in sorted(rep.items()):
    if isinstance(v, list):
        print(k, len(v), '' if k.startswith('_') else
              '(giver spawned: %d)' % sum(1 for x in v if isinstance(x, dict) and x.get('spawned')))
print('both', len(both))
