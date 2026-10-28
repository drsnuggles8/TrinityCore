"""Tiny reader for QuestieDB_Forever.toc baked CBOR data (read-only)."""
import base64, struct, re, json, sys

TOC = r"C:\Program Files (x86)\World of Warcraft\_forever_\Interface\AddOns\QuestieDB\QuestieDB_Forever.toc"

def cbor(b):
    pos = 0
    def rd():
        nonlocal pos
        ib = b[pos]; pos += 1
        mt, ai = ib >> 5, ib & 31
        if mt == 7:
            if ai == 20: return False
            if ai == 21: return True
            if ai in (22, 23): return None
            if ai == 25:
                v = struct.unpack('>e', b[pos:pos+2])[0]; pos += 2; return v
            if ai == 26:
                v = struct.unpack('>f', b[pos:pos+4])[0]; pos += 4; return v
            if ai == 27:
                v = struct.unpack('>d', b[pos:pos+8])[0]; pos += 8; return v
            raise ValueError('simple %d' % ai)
        if ai < 24: n = ai
        elif ai == 24: n = b[pos]; pos += 1
        elif ai == 25: n = int.from_bytes(b[pos:pos+2], 'big'); pos += 2
        elif ai == 26: n = int.from_bytes(b[pos:pos+4], 'big'); pos += 4
        elif ai == 27: n = int.from_bytes(b[pos:pos+8], 'big'); pos += 8
        else: raise ValueError('indef')
        if mt == 0: return n
        if mt == 1: return -1 - n
        if mt == 2:
            v = b[pos:pos+n]; pos += n
            try: return v.decode('utf-8')
            except UnicodeDecodeError: return v
        if mt == 3:
            v = b[pos:pos+n].decode('utf-8'); pos += n; return v
        if mt == 4: return [rd() for _ in range(n)]
        if mt == 5:
            d = {}
            for _ in range(n):
                k = rd(); d[k] = rd()
            return d
        if mt == 6:
            return rd()  # ignore tag
        raise ValueError
    return rd()

_store = None
def store():
    global _store
    if _store is None:
        _store = {}
        with open(TOC, encoding='utf-8') as f:
            for line in f:
                if line.startswith('## X-'):
                    k, _, v = line[3:].partition(': ')
                    _store[k] = v.strip()
    return _store

def get(key):
    s = store()
    v = s.get(key)
    if v is None: return None
    m = re.match(r'^~(\d+)~$', v)
    if m:
        v = ''.join(s[key + '-' + str(i)] for i in range(1, int(m.group(1)) + 1))
    return v

NAMES = {
 'Quest': {1:'name',2:'startedBy',3:'finishedBy',4:'requiredLevel',5:'questLevel',6:'requiredRaces',7:'requiredClasses',8:'objectivesText',9:'triggerEnd',10:'objectives',11:'sourceItemId',12:'preQuestGroup',13:'preQuestSingle',14:'childQuests',15:'inGroupWith',16:'exclusiveTo',17:'zoneOrSort',18:'requiredSkill',19:'requiredMinRep',20:'requiredMaxRep',21:'requiredSourceItems',22:'nextQuestInChain',23:'questFlags',24:'specialFlags',25:'parentQuest',26:'reputationReward',27:'breadcrumbForQuestId',28:'breadcrumbs',29:'extraObjectives',30:'requiredSpell',31:'requiredSpecialization',32:'requiredMaxLevel',33:'availableUntilCompleted',34:'availableStartingWith',35:'requiredRanks',36:'disabledByQuest'},
 'Npc': {1:'name',2:'minLevelHealth',3:'maxLevelHealth',4:'minLevel',5:'maxLevel',6:'rank',7:'spawns',8:'waypoints',9:'zoneID',10:'questStarts',11:'questEnds',12:'factionID',13:'friendlyToFaction',14:'subName',15:'npcFlags'},
 'Object': {1:'name',2:'questStarts',3:'questEnds',4:'spawns',5:'zoneID',6:'factionID',7:'waypoints'},
 'Item': {1:'name',2:'npcDrops',3:'objectDrops',4:'itemDrops',5:'startQuest',6:'questRewards',7:'flags',8:'foodType',9:'itemLevel',10:'requiredLevel',11:'ammoType',12:'class',13:'subClass',14:'vendors',15:'relatedQuests',16:'teachesSpell'},
}

def entity(kind, id_, skip=('waypoints',)):
    pre = 'X-%s-%d-' % (kind, id_)
    srow = get(pre + 'S')
    if srow is None: return None
    row = cbor(base64.b64decode(srow))
    out = {}
    names = NAMES[kind]
    if isinstance(row, list):
        row = {i + 1: v for i, v in enumerate(row)}
    p = row.get('p', 0) if isinstance(row, dict) else 0
    for k, v in row.items():
        if k == 'p': continue
        out[names.get(k, k)] = v
    for idx, nm in names.items():
        if nm in skip: continue
        if p and (p >> (idx - 1)) & 1:
            raw = get(pre + str(idx))
            if raw: out[nm] = cbor(base64.b64decode(raw))
    return out

def fix(o):
    if isinstance(o, dict): return {str(k): fix(v) for k, v in o.items()}
    if isinstance(o, list): return [fix(v) for v in o]
    if isinstance(o, bytes): return o.hex()
    return o

if __name__ == '__main__':
    kind = sys.argv[1]
    for i in sys.argv[2:]:
        print(i, json.dumps(fix(entity(kind, int(i))), ensure_ascii=False))
