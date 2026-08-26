import os
import pytest
import qti

def test_qti_app_singleton():
    assert qti.app is not None
    assert hasattr(qti.app, "newTable")
    assert hasattr(qti.app, "newMatrix")
    assert hasattr(qti.app, "newGraph")
    assert hasattr(qti.app, "newNote")

def test_table_extended_api():
    t = qti.app.newTable("ExtTable", 15, 4)
    assert t is not None
    assert t.numRows() == 15
    assert t.numCols() == 4
    
    # Column naming & comments
    t.setColName(1, "Time")
    t.setColName(2, "Voltage")
    t.setComment(1, "Seconds")
    t.setComment(2, "Volts")
    assert t.colName(1) == "Time"
    assert t.colName(2) == "Voltage"
    assert t.comment(1) == "Seconds"
    assert t.comment(2) == "Volts"
    
    # Cell values
    for row in range(1, 16):
        t.setCell(1, row, float(row) * 0.5)
        t.setCell(2, row, float(row) ** 2)
    
    assert abs(t.cell(1, 4) - 2.0) < 1e-6
    assert abs(t.cell(2, 4) - 16.0) < 1e-6

def test_table_sorting_and_math():
    t = qti.app.newTable("SortTable", 5, 2)
    t.setColName(1, "Data")
    values = [4.0, 1.0, 5.0, 2.0, 3.0]
    for i, v in enumerate(values, 1):
        t.setCell(1, i, v)
        
    t.sortColumn(1, 0) # Ascending sort
    assert abs(t.cell(1, 1) - 1.0) < 1e-6
    assert abs(t.cell(1, 5) - 5.0) < 1e-6

def test_matrix_operations():
    m = qti.app.newMatrix("TestMatrix", 10, 10)
    assert m is not None
    assert m.numRows() == 10
    assert m.numCols() == 10
    
    # Set coordinates
    m.setCoordinates(0.0, 1.0, 0.0, 1.0)
    assert abs(m.xStart() - 0.0) < 1e-6
    assert abs(m.xEnd() - 1.0) < 1e-6
    
    # Set cell value
    m.setCell(2, 3, 42.5)
    assert abs(m.cell(2, 3) - 42.5) < 1e-6
    
    # Transpose
    m.transpose()
    assert abs(m.cell(3, 2) - 42.5) < 1e-6

def test_graph_and_layer():
    t = qti.app.newTable("PlotData", 10, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 11):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i) * 2.0)
        
    g = qti.app.newGraph("PlotTest", 1, 1, 1)
    assert g is not None
    
    l = g.activeLayer()
    assert l is not None
    
    # Insert curve
    curve_added = l.insertCurve(t, "PlotData_Y", 1)
    assert curve_added is not None
    assert l.numCurves() >= 1
    
    # Layer styling
    l.setTitle("Test Plot Title")
    l.setAxisTitle(0, "X-Axis") # Bottom
    l.setAxisTitle(2, "Y-Axis") # Left
    
    # Check export image
    res_dir = "build/tests/results"
    os.makedirs(res_dir, exist_ok=True)
    out_img = os.path.join(res_dir, "test_plot.png")
    l.exportImage(out_img)
    assert os.path.exists(out_img)
    assert os.path.getsize(out_img) > 0

def test_note_window():
    note = qti.app.newNote("ExperimentNotes")
    assert note is not None
    test_content = "QtiPlot automated test run note\nLine 2 with data."
    note.setText(test_content)
    assert "automated test run note" in note.text()
