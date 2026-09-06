import pytest
import os
import tempfile
import qti

def test_table_calculate_operations():
    t = qti.app.newTable("CalcTable", 100, 2)
    t.setColName(1, "A")
    t.setColName(2, "B")
    
    # Calculate with column formula/command
    t.setCommand(1, "i * 2")
    success = t.recalculate(1, 1, 100)
    assert abs(t.cell(1, 1) - 2.0) < 1e-4
    assert abs(t.cell(1, 50) - 100.0) < 1e-4
    assert abs(t.cell(1, 100) - 200.0) < 1e-4

def test_table_ascii_export_cancellation_path():
    t = qti.app.newTable("ExportTable", 500, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 501):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i * 3))
        
    with tempfile.TemporaryDirectory() as tmpdir:
        export_file = os.path.join(tmpdir, "test_export.txt")
        res = t.exportASCII(export_file, "\t", True, False, False)
        assert res is True
        assert os.path.exists(export_file)
        assert os.path.getsize(export_file) > 0

def test_matrix_calculate_operations():
    m = qti.app.newMatrix("CalcMatrix", 50, 50)
    m.setFormula("i + j")
    success = m.calculate()
    assert success is True
    assert abs(m.cell(1, 1) - 2.0) < 1e-4
    assert abs(m.cell(25, 25) - 50.0) < 1e-4
    assert abs(m.cell(50, 50) - 100.0) < 1e-4

def test_matrix_ascii_export_cancellation_path():
    m = qti.app.newMatrix("ExportMatrix", 50, 50)
    m.setFormula("i * 10 + j")
    m.calculate()
    
    with tempfile.TemporaryDirectory() as tmpdir:
        export_file = os.path.join(tmpdir, "matrix_export.txt")
        res = m.exportASCII(export_file, "\t", False)
        assert res is True
        assert os.path.exists(export_file)
        assert os.path.getsize(export_file) > 0
