import pytest
import qti
import os
import tempfile

def test_table_series_data_binding():
    t = qti.app.newTable("QwtConventionTable", 10, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")

    for i in range(1, 11):
        t.setCell(1, i, float(i * 10))
        t.setCell(2, i, float(i * 2.5))

    g = qti.app.newGraph("QwtConventionPlot", 1, 1, 1)
    l = g.activeLayer()
    assert l is not None

    assert l.insertCurve(t, "QwtConventionTable_Y", 1)
    c = l.dataCurve(0)
    assert c is not None

    # Verify initial series data
    assert c.dataSize() == 10
    assert abs(c.x(0) - 10.0) < 1e-6
    assert abs(c.y(0) - 2.5) < 1e-6
    assert abs(c.x(9) - 100.0) < 1e-6
    assert abs(c.y(9) - 25.0) < 1e-6
    # PlotCurve::boundingRect() expands boundaries by 1% margin
    assert abs(c.minXValue() - 9.1) < 1e-4
    assert abs(c.maxXValue() - 100.9) < 1e-4
    assert c.minYValue() <= 2.5
    assert c.maxYValue() >= 25.0

    # Mutate a cell and reload: verify dynamic update
    t.setCell(2, 5, 500.0)
    c.loadData()
    assert c.dataSize() == 10
    assert abs(c.y(4) - 500.0) < 1e-6
    assert c.maxYValue() >= 500.0

def test_table_series_empty_cells_skipping():
    t = qti.app.newTable("QwtGapsTable", 5, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")

    t.setCell(1, 1, 1.0)
    t.setCell(2, 1, 10.0)
    # Row 2 is left blank
    t.setCell(1, 3, 3.0)
    t.setCell(2, 3, 30.0)
    t.setCell(1, 4, 4.0)
    t.setCell(2, 4, 40.0)
    # Row 5 is left blank

    g = qti.app.newGraph("QwtGapsPlot", 1, 1, 1)
    l = g.activeLayer()
    assert l.insertCurve(t, "QwtGapsTable_Y", 1)
    c = l.curve(0)
    assert c is not None

    # Curve should only contain the 3 valid rows
    assert c.dataSize() == 3
    assert abs(c.x(0) - 1.0) < 1e-6
    assert abs(c.x(1) - 3.0) < 1e-6
    assert abs(c.x(2) - 4.0) < 1e-6

def test_qwt_export_rendering():
    t = qti.app.newTable("ExportTable", 20, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 21):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i * i))

    g = qti.app.newGraph("ExportPlot", 1, 1, 1)
    l = g.activeLayer()
    assert l.insertCurve(t, "ExportTable_Y", 1)

    with tempfile.TemporaryDirectory() as tmpdir:
        png_path = os.path.join(tmpdir, "test_render.png")
        svg_path = os.path.join(tmpdir, "test_render.svg")

        g.exportImage(png_path)
        assert os.path.exists(png_path)
        assert os.path.getsize(png_path) > 0

        g.exportSVG(svg_path)
        assert os.path.exists(svg_path)
        assert os.path.getsize(svg_path) > 0
