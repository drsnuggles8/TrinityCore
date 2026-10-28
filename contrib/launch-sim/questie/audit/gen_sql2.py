"""Generate the 28 Oct quest audit SQL (rewards, levels/XP, money) from Wowhead Forever's quest rows (wh_quests.json,
wh_scaling.json) and the world DB dump (ourdb/quests.tsv). Writes OUT_DIR/2026_10_28_0{6,7,8}_*.sql."""
import json, os, sys, ast

D = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1]
W = {int(k): v for k, v in json.load(open(os.path.join(D, 'wh_quests.json'))).items()}
S = {int(k): v for k, v in json.load(open(os.path.join(D, 'wh_scaling.json'))).items()}
N = {int(k): v for k, v in json.load(open(os.path.join(D, 'item_names.json'))).items()}
XPR = json.load(open(os.path.join(D, 'report_xp.json')))
ours = {}
for l in open(os.path.join(D, 'ourdb', 'quests.tsv'), encoding='utf-8'):
    r = l.rstrip('\n').split('\t')
    ours[int(r[0])] = r
qxp = {}
for l in open(os.path.join(D, 'qxp.txt')):
    p = l.split(' ', 1)
    if p[0].isdigit() and p[1].startswith('[['):
        qxp[int(p[0])] = ast.literal_eval(p[1])[0]

def title(q):
    return ours[q][1]

def nm(i):
    return '%d %s' % (i, N.get(i, '?'))

def cur_guard(q):
    """WHERE clause pinning the row's current reward columns, so a statement runs only on the row it was made for."""
    r = ours[q]
    cols = ['RewardItem%d' % k for k in range(1, 5)] + ['RewardChoiceItemID%d' % k for k in range(1, 7)]
    vals = [r[2 + 2 * k] for k in range(4)] + [r[10 + 2 * k] for k in range(6)]
    return ' AND '.join('`%s` = %s' % (c, v) for c, v in zip(cols, vals))

def cur_desc(q):
    r = ours[q]
    f = [(int(r[2 + 2 * k]), int(r[3 + 2 * k])) for k in range(4) if int(r[2 + 2 * k])]
    c = [(int(r[10 + 2 * k]), int(r[11 + 2 * k])) for k in range(6) if int(r[10 + 2 * k])]
    return f, c

def fmt(lst):
    return '[' + ', '.join(('%s x%d' % (nm(i), n)) if n != 1 else nm(i) for i, n in lst) + ']'

# ------------------------------------------------------------------ 06 rewards
REWARD_Q = [98, 202, 208, 231, 296, 304, 378, 504, 515, 547, 613, 679, 680, 682, 684, 685, 709, 1012, 1101, 1102, 1203,
            1371, 5088, 5943, 97275]
L = ["-- Quest rewards as Wowhead Forever lists them (quest list rows: fixed and choice items with counts; Olo's page cache of",
     "-- 23 Oct 2026, checked on the live pages for 202, 1101 and 1203 on 28 Oct). QuestieDB's item data lacks most of these",
     "-- Forever items, so the 28 Oct pass (2026_10_28_03) missed them. Where Wowhead prints an item both under \"You will also",
     "-- receive\" and among the choices (682), it is a choice only (the same page quirk as 962 and 1199). Each statement sets",
     "-- the whole reward block and is guarded on the row's current rewards: it runs once and is safe to run again.", ""]
for q in REWARD_Q:
    w = W[q]
    wc = [(i, n) for i, n in w['itemchoices']]
    wf = [(i, n) for i, n in w['itemrewards'] if i not in {c for c, _ in wc}]
    of, oc = cur_desc(q)
    L.append('-- %d %s: fixed %s choice %s -> fixed %s choice %s' % (q, title(q), fmt(of), fmt(oc), fmt(wf), fmt(wc)))
    sets = []
    for k in range(4):
        i, n = wf[k] if k < len(wf) else (0, 0)
        sets.append('`RewardItem%d` = %d, `RewardAmount%d` = %d' % (k + 1, i, k + 1, n))
    for k in range(6):
        i, n = wc[k] if k < len(wc) else (0, 0)
        sets.append('`RewardChoiceItemID%d` = %d, `RewardChoiceItemQuantity%d` = %d, `RewardChoiceItemDisplayID%d` = 0'
                    % (k + 1, i, k + 1, n, k + 1))
    if q == 97275:   # counts only: the guard on items alone would not see the change
        L.append('UPDATE `quest_template` SET `RewardChoiceItemQuantity1` = 2, `RewardChoiceItemQuantity2` = 2 WHERE `ID` = 97275;')
        L.append('')
        continue
    L.append('UPDATE `quest_template` SET\n  ' + ',\n  '.join(sets) + '\nWHERE `ID` = %d AND %s;' % (q, cur_guard(q)))
    L.append('')
open(os.path.join(OUT, '2026_10_28_06_world_forever_quest_rewards_wowhead.sql'), 'w', encoding='utf-8', newline='\n').write('\n'.join(L))

# ------------------------------------------------------------------ 07 levels and XP
lv = [x for x in XPR['level_differs'] if x['id'] not in (490, 79358, 79361)]
L = ["-- Quest levels and quest XP as on Wowhead Forever (quest list rows and each quest page's XP multiplier, Olo's page cache of",
     "-- 23 Oct 2026; the game XP Factoruide's collector saw on the beta agrees wherever it has one).",
     "--",
     "-- 1. Quest levels. The server reads the quest level from quest_template_classic_level only; 2026_10_26_25 wrote its",
     "--    Forever levels into quest_classic_level.QuestLevel, which nothing reads, so these quests kept the vanilla level",
     "--    (and its XP). Both tables get the Forever level.",
     "INSERT INTO `quest_template_classic_level` (`ID`, `LevelType`, `QuestLevel`) VALUES"]
rows = []
for x in lv:
    q = x['id']
    rows.append("(%d, 2, %d)%%s -- %s, was %d (XP %d -> %d)" % (q, x['wh_level'], title(q), x['level'],
                x['ours'], int(-(-qxp[x['wh_level']][x['diff']] * x['mult'] // 1))))
for i, r in enumerate(rows):
    L.append(r % (',' if i < len(rows) - 1 else ''))
L.append("ON DUPLICATE KEY UPDATE `QuestLevel` = VALUES(`QuestLevel`);")
L.append("UPDATE `quest_classic_level` SET `QuestLevel` = CASE `ID` %s END WHERE `ID` IN (%s);" % (
    ' '.join('WHEN %d THEN %d' % (x['id'], x['wh_level']) for x in lv), ', '.join(str(x['id']) for x in lv)))
L += ["", "-- 2. Dungeon quests: Forever multiplies their XP (Wowhead's per-quest XP multiplier, 1.75-2.675; the collector saw the",
      "--    multiplied XP on all 21 of these it has). Our rows had multiplier 1."]
for x in sorted(XPR['multiplier_missing'], key=lambda x: x['id']):
    L.append("UPDATE `quest_template` SET `RewardXPMultiplier` = %s WHERE `ID` = %d AND `RewardXPMultiplier` = 1; -- %s: %d -> %d"
             % (('%g' % x['wh_mult']), x['id'], title(x['id']), x['ours'], x['want']))
L += ["", "-- 3. XP difficulty that gives Wowhead Forever's XP (no collector figure contradicts these)."]
for x in XPR['other']:
    if x['id'] in (100, 606, 635, 745):
        d = x['diff_fits'][0]
        L.append("UPDATE `quest_template` SET `RewardXPDifficulty` = %d WHERE `ID` = %d AND `RewardXPDifficulty` = %d; -- %s: %d -> %d"
                 % (d, x['id'], x['diff'], title(x['id']), x['ours'], qxp[x['level']][d]))
open(os.path.join(OUT, '2026_10_28_07_world_forever_quest_levels_xp.sql'), 'w', encoding='utf-8', newline='\n').write('\n'.join(L) + '\n')

# ------------------------------------------------------------------ 08 money
MONEY = [86574, 95217, 96875, 97914, 97926, 98298, 299, 430, 544, 556, 635, 642, 651, 692, 745, 1090, 234, 752]
L = ["-- Quest money as on Wowhead Forever (quest list rows, Olo's page cache of 23 Oct 2026). The six Forever quests added on",
     "-- 28 Oct (2026_10_28_04) paid nothing; vanilla quests whose Forever level or money changed; 234 and 752 (the profession",
     "-- book quests) pay no money on Forever. Wowhead's money agrees with ours for 2371 quests paying nothing and 101 Forever",
     "-- quests paying by difficulty. Fixed amounts go into quest_classic_money (it overrides the difficulty). Safe to run again.",
     "INSERT INTO `quest_classic_money` (`ID`, `RewardMoney`, `RewardMoneyMaxLevel`) VALUES"]
rows = []
for q in MONEY:
    old = int(ours[q][29])
    rows.append("(%d, %d, 0)%%s -- %s, was %s" % (q, W[q]['money'], title(q), 'difficulty' if old == -999999 else old))
for i, r in enumerate(rows):
    L.append(r % (',' if i < len(rows) - 1 else ''))
L.append("ON DUPLICATE KEY UPDATE `RewardMoney` = VALUES(`RewardMoney`);")
open(os.path.join(OUT, '2026_10_28_08_world_forever_quest_money.sql'), 'w', encoding='utf-8', newline='\n').write('\n'.join(L) + '\n')
print('ok')
