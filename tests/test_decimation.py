import pytest
import qti
import math

def test_decimation_enum_and_api():
    DM = qti.Layer.DecimationMethod
    assert hasattr(DM, 'NoDecimation')
    assert hasattr(DM, 'LTTB')
    assert hasattr(DM, 'MinMax')
    assert hasattr(DM, 'DouglasPeucker')
    
    assert DM.NoDecimation.value == 0
    assert DM.LTTB.value == 1
    assert DM.MinMax.value == 2
    assert DM.DouglasPeucker.value == 3

    # Test ApplicationWindow defaults
    app = qti.app
    app.setDefaultDecimationMethod(DM.LTTB)
    assert app.defaultDecimationMethod() == DM.LTTB
    app.setDefaultDecimationMethod(DM.MinMax)
    assert app.defaultDecimationMethod() == DM.MinMax
    
    app.setSpeedMaxPoints(2500)
    assert app.speedModeMaxPoints() == 2500

def test_lttb_and_minmax_curve_decimation():
    DM = qti.Layer.DecimationMethod
    # Create large dataset: 5000 points
    n_points = 5000
    t = qti.app.newTable("DecimationData", n_points, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    
    for i in range(1, n_points + 1):
        x = float(i)
        # Sine wave with a sharp spike in the middle
        y = math.sin(x * 0.05)
        if i == 2500:
            y = 100.0 # Spike
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("DecimationPlot", 1, 1, 1)
    l = g.activeLayer()
    assert l is not None
    
    assert l.insertCurve(t, "DecimationData_Y", 1)
    c = l.curve(0)
    assert c is not None
    
    # 1. No decimation: curve should contain all 5000 points
    l.enableSpeedMode(DM.NoDecimation, 500, 0.0)
    assert l.decimationMethod() == DM.NoDecimation
    assert c.dataSize() == n_points
    
    # 2. LTTB Decimation: speed mode is enabled, but owned curve samples remain pristine (5000 points)
    # T6 fix ensures decimation is screen-only and does not mutate owned data
    l.enableSpeedMode(DM.LTTB, 500, 0.0)
    assert l.decimationMethod() == DM.LTTB
    assert l.speedModeMaxPoints() == 500
    assert c.dataSize() == n_points
    # Endpoints must match exactly
    assert abs(c.x(0) - 1.0) < 1e-6
    assert abs(c.x(n_points - 1) - float(n_points)) < 1e-6
    
    # 3. Min-Max Decimation: owned curve samples still remain pristine
    l.enableSpeedMode(DM.MinMax, 500, 0.0)
    assert l.decimationMethod() == DM.MinMax
    assert c.dataSize() == n_points
    # The 100.0 spike is preserved in curve data
    assert c.maxYValue() >= 99.9
    
    # 4. Disable decimation: retains all 5000 points
    l.enableSpeedMode(DM.NoDecimation, 500, 0.0)
    assert c.dataSize() == n_points
