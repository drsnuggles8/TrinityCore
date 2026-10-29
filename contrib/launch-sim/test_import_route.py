#!/usr/bin/env python3
"""Tests for import_route.py on a made-up guide in RestedXP's format (no real guide content).

    python3 -m unittest contrib/launch-sim/test_import_route.py
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_route as ir  # noqa: E402

GUIDE = """
RXPGuides.RegisterGuide([[
#name 1-5 Test Valley
step
    .accept 100 >> Accept First Steps
step << Warlock
    #season 2
    .xp 2 >> Season of Discovery only
    .accept 77001
step << Warlock
    #season 0,1
    .accept 104
step << Hunter
    .accept 101 >> Accept Hunter Training
step
    .complete 100,1
    .complete 100,2 << Warrior
step
    #optional
    .xp 2 >> Grind to level 2
step
    #hardcore
    .xp 9 >> Hardcore only
step << Shaman
    #loop
    .xp 3+1130 >> Grind to 1130+/1400xp
step
    .xp >6,1
    .accept 102
step
    .xp 10-1720 >> Grind until you are 1720xp away from level 10
step
    #optional
    .isQuestComplete 103
    .turnin 103
step
    .turnin 100
    .deathskip
]])
"""


class ImportRouteTest(unittest.TestCase):
    def setUp(self):
        ir.ROUTE_RACES = [3, 7]
        ir.COMBOS = [(r, c) for r in ir.ROUTE_RACES for c in ir.RACE_CLASSES[r]]
        self.steps = ir.parse(GUIDE)

    def mask(self, *classes):
        return sum(1 << ir.bit(r, c) for r, c in ir.COMBOS if c in classes)

    def test_order_and_actions(self):
        self.assertEqual([(a, q, o) for a, q, o, _ in self.steps], [
            ("accept", 100, 0),
            ("accept", 104, 0),               # "#season 0,1" is Era: kept; the "#season 2" (SoD) step before it is not
            ("accept", 101, 0),
            ("complete", 100, 1),
            ("complete", 100, 2),
            ("level", 2 << 24, 0),            # #optional grind step: imported (Olo, 29 Oct)
            ("level", 3 << 24 | 1130, 0),     # the #hardcore step before it is not
            ("accept", 102, 0),               # ".xp >6,1" only shows a step: not a grind step
            ("level", 9 << 24 | 1720, 1),     # counted back from level 10
            ("turnin_done", 103, 0),
            ("turnin", 100, 0),
            ("deathskip", 0, 0),
        ])

    def test_class_filters(self):
        by = {(a, q, o): m for a, q, o, m in self.steps}
        self.assertEqual(by[("accept", 101, 0)], self.mask(3))
        self.assertEqual(by[("complete", 100, 2)], self.mask(1))
        self.assertEqual(by[("level", 3 << 24 | 1130, 0)], self.mask(7))
        self.assertEqual(by[("level", 2 << 24, 0)], self.mask(*{c for _, c in ir.COMBOS}))

    def test_mode_tags(self):
        everyone = self.mask(*{c for _, c in ir.COMBOS})
        self.assertEqual(ir.mask_for("!sod/Warrior"), everyone)      # Era: not SoD, so every class
        self.assertEqual(ir.mask_for("sod"), 0)
        self.assertEqual(ir.mask_for("sod Warrior"), 0)
        self.assertEqual(ir.mask_for("!hardcore Hunter"), self.mask(3))
        self.assertEqual(ir.mask_for("!Herbalism"), everyone)        # an unknown tag filters nothing, negated or not

    def test_xp_step(self):
        self.assertEqual(ir.xp_step(".xp 4 >> Grind to level 4"), ("level", 4 << 24, 0))
        self.assertEqual(ir.xp_step(".xp 5+1325 >> Travel"), ("level", 5 << 24 | 1325, 0))
        self.assertEqual(ir.xp_step(".xp 10-2040"), ("level", 9 << 24 | 2040, 1))
        self.assertIsNone(ir.xp_step(".xp >6,1"))
        self.assertIsNone(ir.xp_step(".xp <11,1"))
        self.assertIsNone(ir.xp_step(".xpto 4"))


if __name__ == "__main__":
    unittest.main()
