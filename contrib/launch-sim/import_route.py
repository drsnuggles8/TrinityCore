#!/usr/bin/env python3
"""Turns a leveling guide in RestedXP's guide format into a launch-simulation route (doc/LaunchSim.md, "Routes").

    python3 import_route.py [--races dwarf,gnome|human|nightelf] ROUTE_ID GUIDE.lua [GUIDE.lua ...] > route.sql

ROUTE_ID is a short name (e.g. rxp, olo; rxp-human, rxp-nightelf for the other Alliance starting zones: the launch sim picks
a route for a bot by this suffix). Every RegisterGuide block of every file is read in order and becomes one ordered list of
quest steps: accept / complete (quest objective) / turn in, plus deathskip and hearth steps. Class and race filters
(`step << Dwarf Hunter`, `<< !Gnome`, `.accept 1234 << Warrior/Rogue`) are evaluated for each class of the route's races
(default dwarf,gnome) and stored as a bit mask (bit = race slot * 12 + class id; the first race slot 0, the second slot 1),
so one route serves every class.

The output is SQL for the characters database (table playerbot_launch_route_step, sql/playerbot_v2/0901_launch_routes.sql).
It holds the guide's content: keep it on the machine that runs the server, never commit it (guides have their own licences).
"""
import re
import sys

RACES = {"human": 1, "dwarf": 3, "nightelf": 4, "gnome": 7}
CLASSES = {"warrior": 1, "paladin": 2, "hunter": 3, "rogue": 4, "priest": 5, "shaman": 7, "mage": 8, "warlock": 9,
           "druid": 11, "deathknight": 6, "monk": 10, "demonhunter": 12, "evoker": 13}
# Classes of the Alliance starting races in Forever (Fleet/BotComposition.cpp IsVanillaCombo)
RACE_CLASSES = {1: (1, 2, 4, 5, 8, 9), 3: (1, 2, 3, 4, 5, 7), 4: (1, 3, 4, 5, 11), 7: (1, 4, 8, 9)}
ROUTE_RACES = [3, 7]   # set by --races
COMBOS = []
ACTIONS = {"accept": 1, "complete": 2, "turnin": 3, "deathskip": 4, "hearth": 5}


def bit(race, cls):
    return ROUTE_RACES.index(race) * 12 + cls


def term_true(term, race, cls):
    neg = term.startswith("!")
    t = term.lstrip("!").lower()
    if t in RACES:
        val = RACES[t] == race
    elif t in CLASSES:
        val = CLASSES[t] == cls
    elif t in ("alliance",):
        val = True
    elif t in ("horde",):
        val = False
    else:
        val = True   # tags we do not model (season, softcore, professions): do not filter on them
    return val != neg


def allowed(expr, race, cls):
    """RestedXP filter: space-separated AND of terms, each term a /-separated OR of names, ! negates a name."""
    for group in expr.split():
        if not any(term_true(alt, race, cls) for alt in group.split("/") if alt):
            return False
    return True


def mask_for(*exprs):
    m = 0
    for race, cls in COMBOS:
        if all(allowed(e, race, cls) for e in exprs if e):
            m |= 1 << bit(race, cls)
    return m


def guide_bodies(text):
    parts = re.split(r"RegisterGuide\(\s*\[\[", text)
    return [p.split("]])")[0] for p in parts[1:]] if len(parts) > 1 else [text]


def parse(text):
    steps = []
    for body in guide_bodies(text):
        for block in re.split(r"\n[ \t]*(?=step\b)", "\n" + body)[1:]:
            head, _, rest = block.partition("\n")
            m = re.search(r"<<\s*(.+)$", head)
            step_filter = m.group(1).strip() if m else ""
            optional = False
            for raw in rest.split("\n"):
                t = raw.strip()
                if t.startswith("--"):
                    continue
                if t.startswith("#optional") or t.startswith("#hardcore") or t.startswith("#som"):
                    optional = True
                    continue
                line_filter = ""
                lm = re.search(r"\s<<\s*([^-]+?)\s*(--.*)?$", t)
                if lm:
                    line_filter = lm.group(1).strip()
                    t = t[:lm.start()].strip()
                if optional:
                    continue
                mask = mask_for(step_filter, line_filter)
                if not mask:
                    continue
                if (a := re.match(r"\.accept\s+-?(\d+)", t)):
                    steps.append(("accept", int(a.group(1)), 0, mask))
                elif (a := re.match(r"\.turnin\s+-?(\d+)", t)):
                    steps.append(("turnin", int(a.group(1)), 0, mask))
                elif (a := re.match(r"\.complete\s+(\d+)\s*,\s*(\d+)", t)):
                    steps.append(("complete", int(a.group(1)), int(a.group(2)), mask))
                elif t.startswith(".deathskip"):
                    steps.append(("deathskip", 0, 0, mask))
                elif re.match(r"\.hs\b", t):
                    steps.append(("hearth", 0, 0, mask))
    return steps


def main():
    global ROUTE_RACES, COMBOS
    args = sys.argv[1:]
    if args[:1] == ["--races"]:
        ROUTE_RACES = [RACES[r] for r in args[1].split(",")]
        if not 1 <= len(ROUTE_RACES) <= 2:
            sys.exit("--races: one or two races")
        args = args[2:]
    COMBOS = [(r, c) for r in ROUTE_RACES for c in RACE_CLASSES[r]]
    if len(args) < 2:
        sys.exit(__doc__)
    route = args[0]
    if not re.fullmatch(r"[a-z0-9_-]{1,16}", route):
        sys.exit("ROUTE_ID: 1-16 lowercase letters, digits, _ or -")
    steps = []
    for path in args[1:]:
        steps += parse(open(path, encoding="utf-8", errors="replace").read())
    print(f"DELETE FROM playerbot_launch_route_step WHERE route = '{route}';")
    for i in range(0, len(steps), 200):
        rows = ",\n".join(f"('{route}', {n}, {mask}, {ACTIONS[a]}, {q}, {o})"
                          for n, (a, q, o, mask) in enumerate(steps[i:i + 200], start=i + 1))
        print("INSERT INTO playerbot_launch_route_step (route, step, combos, action, quest, objective) VALUES\n" + rows + ";")
    print(f"-- {len(steps)} steps", file=sys.stderr)


if __name__ == "__main__":
    main()
