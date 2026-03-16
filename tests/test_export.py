import os
import pytest

def test_png_export():
    import qti
    res_dir = "build/tests/results"
    if not os.path.exists(res_dir):
        os.makedirs(res_dir)
    
    png_path = os.path.join(res_dir, "pytest_sine.png")
    if os.path.exists(png_path):
        os.remove(png_path)
    
    # Create plot
    t = qti.app.newTable("ExportTable", 10, 2)
    for i in range(1, 11):
        t.setCell(1, i, float(i))
        t.setCell(2, i, 1.0)
    
    g = qti.app.newGraph()
    l = g.activeLayer()
    l.insertCurve(t, "ExportTable_2", 1)
    
    # Export
    l.exportImage(png_path)
    
    assert os.path.exists(png_path)
    size = os.path.getsize(png_path)
    assert size > 0
    # Verify optimization (should be around 5-10KB, definitely not 600KB)
    assert size < 50000 

def test_eps_export():
    import qti
    res_dir = "build/tests/results"
    eps_path = os.path.join(res_dir, "pytest_sine.eps")
    
    g = qti.app.currentGraph()
    if not g:
        pytest.skip("No active graph for EPS export")
        
    l = g.activeLayer()
    l.export(eps_path)
    assert os.path.exists(eps_path)
    assert os.path.getsize(eps_path) > 0

def test_emf_export():
    import qti
    res_dir = "build/tests/results"
    emf_path = os.path.join(res_dir, "pytest_sine.emf")
    
    g = qti.app.currentGraph()
    if not g:
        pytest.skip("No active graph for EMF export")
        
    l = g.activeLayer()
    l.export(emf_path)
    assert os.path.exists(emf_path)
    assert os.path.getsize(emf_path) > 0
