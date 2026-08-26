import os
import tempfile
import pytest

def test_roundtrip_data():
    import qti
    project_file = "build/tests/results/rt_pytest.qti"
    os.makedirs(os.path.dirname(project_file), exist_ok=True)
    
    # 1. Setup
    t = qti.app.newTable("RTTable", 5, 2)
    for i in range(1, 6):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i * 1.5))
    
    # 2. Save
    qti.app.saveProjectAs(project_file)
    assert os.path.exists(project_file)
    
    # 3. Reload
    qti.app.open(project_file, False, False)
    
    # 4. Verify
    t_verify = qti.app.table("RTTable")
    assert t_verify is not None, f"Table RTTable not found. Available: {qti.app.tableNames()}"
    
    for i in range(1, 6):
        assert t_verify.cell(1, i) == float(i)
        assert abs(t_verify.cell(2, i) - (i * 1.5)) < 1e-9
    
    # Cleanup
    if os.path.exists(project_file):
        os.remove(project_file)
