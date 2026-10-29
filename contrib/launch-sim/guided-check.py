#!/usr/bin/env python3
"""Checks that guide followers follow their guide, from the [route] lines of a Playerbot.log (doc/LaunchSim.md, "Routes").

    python3 guided-check.py PLAYERBOT.LOG [--routes-dir ~/forever/routes] [--section-slack 0.5] [--max-skip-share 0.3]
                            [--max-step-min 15]

The snapshot builder logs every route step a bot finishes or passes over ("[route] NAME ROUTE step I/N action=A quest=Q HOW
level=L xp=X/NEXT on_step_s=S min=M area=AREA in_valley=0|1"). The checks, each PASS or FAIL:

- guide sections: a bot that starts the next guide of its route ("5-11 Dun Morogh", from the "-- section" lines import_route.py
  writes into ROUTE.sql) has that guide's start level, less --section-slack (Olo, 29 Oct: the RestedXP Coldridge guide ends at
  about 5 1/3; bots that dropped its ".xp" grind steps walked out at level 3). Where bots first stepped out of their starting
  valley is printed too, but not checked: the guides step out briefly (Shadowglen's Iverron stands outside the gate).
- skipped steps: no step is passed over by more than --max-skip-share of the bots that reached it ("skipped", "not_in_log",
  "not_turned_in": the bot moved on without doing it; "cannot_take" is a class or race quest, or prerequisites, and allowed).
- stuck steps: no bot spends more than --max-step-min minutes on one step (a grind step to a level counts too; it is reported
  but not failed, as grinding a level in a crowd takes long).

Exit code 0 when every check passes, 1 otherwise.
"""
import argparse
import collections
import glob
import os
import re
import statistics
import sys

LINE = re.compile(r"\[route\] (\S+) (\S+) step (\d+)/(\d+) action=(\d+) quest=(\d+) (\S+) level=(\d+) xp=(\d+)/(\d+) "
                  r"on_step_s=(\d+) min=(\d+) area=(\d+) in_valley=([01])")
FAILED_AS = ("skipped", "skipped:not_in_log", "skipped:not_turned_in", "skipped:no_giver", "skipped:no_quest")
LEVEL_ACTION = 7


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log")
    ap.add_argument("--routes-dir", default="", help="directory of the imported ROUTE.sql files (their '-- section' lines)")
    ap.add_argument("--section-slack", type=float, default=0.5, help="how far below a section's start level a bot may be")
    ap.add_argument("--max-skip-share", type=float, default=0.3)
    ap.add_argument("--max-step-min", type=float, default=15.0)
    a = ap.parse_args()
    sections = collections.defaultdict(list)
    if a.routes_dir:
        for path in glob.glob(os.path.join(a.routes_dir, "*.sql")):
            route = os.path.basename(path)[:-4]
            with open(path, encoding="utf-8", errors="replace") as f:
                for line in f:
                    m = re.match(r"-- section (\d+) (\d+) (.+)", line)
                    if m:
                        sections[route].append((int(m.group(1)), int(m.group(2)), m.group(3).strip()))

    steps = []
    with open(a.log, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = LINE.search(line)
            if m:
                g = m.groups()
                steps.append(dict(bot=g[0], route=g[1], step=int(g[2]), n=int(g[3]), action=int(g[4]), quest=int(g[5]),
                                  how=g[6], level=int(g[7]), xp=int(g[8]), next=int(g[9]), on_s=int(g[10]), min=int(g[11]),
                                  valley=g[13] == "1"))
    if not steps:
        print("no [route] lines: no guide followers, or a build without the route log")
        return 1
    ok = True

    by_route = collections.defaultdict(list)
    for s in steps:
        by_route[s["route"]].append(s)
    for route, rs in sorted(by_route.items()):
        bots = sorted({s["bot"] for s in rs})
        hows = collections.Counter(s["how"] for s in rs)
        print(f"== {route}: {len(bots)} bots, {len(rs)} steps logged, furthest step {max(s['step'] for s in rs)}/{rs[0]['n']}: "
              + ", ".join(f"{h} {c}" for h, c in hows.most_common()))

        # valley exit
        exits = {}
        for s in rs:
            if not s["valley"] and s["bot"] not in exits:
                exits[s["bot"]] = s["level"] + (s["xp"] / s["next"] if s["next"] else 0.0)
        if exits:
            print(f"  [info] valley exit: {len(exits)} of {len(bots)} bots stepped out, level min {min(exits.values()):.2f} "
                  f"median {statistics.median(exits.values()):.2f} (guides step out briefly, e.g. Iverron at Shadowglen's gate)")

        # guide sections: a bot starting "5-11 Dun Morogh" should be about level 5 (Olo, 29 Oct: Coldridge ends at about 5 1/3)
        for first, level, name in sections.get(route, []):
            if level <= 1:
                continue
            at = {}
            for s in rs:
                if s["step"] >= first and s["bot"] not in at:
                    at[s["bot"]] = s["level"] + (s["xp"] / s["next"] if s["next"] else 0.0)
            if not at:
                print(f"  [----] section '{level}-.. {name}' (step {first}): no bot there yet")
                continue
            want = level - a.section_slack
            low = {b: lv for b, lv in at.items() if lv < want}
            verdict = "FAIL" if low else "PASS"
            ok &= not low
            print(f"  [{verdict}] section '{name}' (step {first}, from level {level}): {len(at)} bots started it, level min "
                  f"{min(at.values()):.2f} median {statistics.median(at.values()):.2f} (want >= {want:g})"
                  + (": " + ", ".join(f"{b} {lv:.2f}" for b, lv in sorted(low.items(), key=lambda kv: kv[1])[:10]) if low else ""))

        # skipped steps
        # a step the bot came back for and did later (the route went back to it) is not passed over
        done_later = {(s["bot"], s["step"]) for s in rs if s["how"] == "done"}
        reached = collections.Counter(s["step"] for s in {(s["bot"], s["step"]): s for s in rs}.values())
        failed = collections.Counter(st for b, st in {(s["bot"], s["step"]) for s in rs if s["how"] in FAILED_AS}
                                     if (b, st) not in done_later)
        info = {s["step"]: (s["action"], s["quest"]) for s in rs}
        bad = [(st, failed[st], reached[st]) for st in failed if failed[st] / reached[st] > a.max_skip_share]
        verdict = "FAIL" if bad else "PASS"
        ok &= not bad
        print(f"  [{verdict}] skipped steps: {sum(failed.values())} passed over without doing them"
              + "".join(f"\n      step {st} action={info[st][0]} quest={info[st][1]}: {f} of {r} bots"
                        for st, f, r in sorted(bad)))

        # long steps
        long_ = [s for s in rs if s["on_s"] > a.max_step_min * 60]
        quest_long = [s for s in long_ if s["action"] != LEVEL_ACTION]
        verdict = "FAIL" if quest_long else "PASS"
        ok &= not quest_long
        print(f"  [{verdict}] long steps: {len(quest_long)} quest steps and {len(long_) - len(quest_long)} grind steps over "
              f"{a.max_step_min:g} min"
              + "".join(f"\n      {s['bot']} step {s['step']} action={s['action']} quest={s['quest']} {s['on_s'] // 60} min"
                        for s in sorted(long_, key=lambda s: -s["on_s"])[:8]))
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
