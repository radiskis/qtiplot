import sys
import qti

print("--- START GATE CURVE LEDGER TEST ---")
t = qti.app.newTable("GateTable", 100, 2)
for i in range(1, 101):
    t.setCell(1, i, float(i))
    t.setCell(2, i, float(i) * 2.0)

g = qti.app.newGraph("GateGraph", 1, 1, 1).activeLayer()

# Reset ledger to start tracking curve add/remove cycles cleanly
qti.app.resetLedger()

g.insertCurve(t, "GateTable_2", 1)

print("Performing 1,000 curve add and remove cycles...")
for _ in range(1000):
    g.removeCurve(0)
    g.addCurve(t, "GateTable_2")

# Remove the curve so zero curves should be alive
g.removeCurve(0)

# Check ledger state
clean = qti.app.isLedgerClean()
report = qti.app.ledgerReport()
print(f"Ledger Clean: {clean}")
if not clean:
    print(f"LEDGER LEAK DETECTED:\n{report}", file=sys.stderr)
    sys.exit(70)

print("Gate test passed: Zero curve leaks detected after 1,000 add/remove cycles.")
