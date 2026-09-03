import sys
import qti

print("--- START GATE CURVE LEDGER TEST ---")
t = qti.app.newTable("GateTable", 100, 2)
for i in range(1, 101):
    t.setCell(1, i, float(i))
    t.setCell(2, i, float(i) * 2.0)

ml = qti.app.newGraph("GateGraph", 1, 1, 1)
g = ml.activeLayer()
g.insertCurve(t, "GateTable_2", 1)

print("Performing 1,000 curve add and remove cycles...")
for _ in range(1000):
    g.removeCurve(0)
    g.addCurve(t, "GateTable_2")

# Remove the curve so zero curves should be alive
g.removeCurve(0)

print("Performing Phase 2: column deletion curve removal and table cleanup...")
for _ in range(200):
    t2 = qti.app.newTable("Tmp", 50, 2)
    g.addCurve(t2, "Tmp_2")
    t2.removeCol("Tmp_2")
    t2.confirmClose(False)
    t2.close()

# Check ledger state for curve leaks
report = qti.app.ledgerReport()
curve_leaks = ("PlotCurve" in report) or ("DataCurve" in report)
print(f"Curve Ledger Clean: {not curve_leaks}")
if curve_leaks:
    print(f"LEDGER LEAK DETECTED:\n{report}", file=sys.stderr)
    sys.exit(70)

print("Gate test passed: Zero curve leaks detected after 1,000 add/remove cycles.")

t.confirmClose(False)
t.close()
ml.confirmClose(False)
ml.close()
qti.app.savedProject()
qti.app.closeProject()
