import os
import tempfile
import pytest

def test_table_creation():
    import qti
    t = qti.app.newTable("APITestTable", 10, 3)
    assert t is not None
    assert "APITestTable" in qti.app.tableNames()

def test_set_cell_values():
    import qti
    t = qti.app.table("APITestTable")
    if not t:
        t = qti.app.newTable("APITestTable", 10, 3)
    
    for i in range(1, 11):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i * i))
    
    assert t.cell(1, 5) == 5.0
    assert t.cell(2, 5) == 25.0

def test_save_project():
    import qti
    save_path = "build/tests/results/test_api_project.qti"
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    qti.app.saveProjectAs(save_path)
    assert os.path.exists(save_path)
    # Cleanup
    try:
        os.remove(save_path)
    except:
        pass