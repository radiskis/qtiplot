import pytest
import math
import qti

def test_threaded_linear_fit():
    t = qti.app.newTable("ThreadLinearTable", 20, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 21):
        x = float(i)
        y = 3.5 * x - 1.2
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadLinearGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadLinearTable_Y", 1)
    
    fit = qti.LinearFit(qti.app, l, "ThreadLinearTable_Y")
    assert fit is not None
    success = fit.run()
    assert success is True
    
    res = fit.results()
    assert len(res) == 2
    intercept, slope = res[0], res[1]
    assert abs(slope - 3.5) < 1e-4
    assert abs(intercept - (-1.2)) < 1e-4

def test_threaded_polynomial_fit():
    t = qti.app.newTable("ThreadPolyTable", 25, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 26):
        x = float(i)
        y = 2.0 + 1.5 * x - 0.1 * (x ** 2)
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadPolyGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadPolyTable_Y", 1)
    
    fit = qti.PolynomialFit(qti.app, l, "ThreadPolyTable_Y", 2)
    assert fit is not None
    success = fit.run()
    assert success is True
    
    res = fit.results()
    assert len(res) == 3
    a0, a1, a2 = res[0], res[1], res[2]
    assert abs(a0 - 2.0) < 1e-2
    assert abs(a1 - 1.5) < 1e-2
    assert abs(a2 - (-0.1)) < 1e-3

def test_threaded_exponential_fit():
    t = qti.app.newTable("ThreadExpTable", 30, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 31):
        x = float(i) * 0.2
        y = 5.0 * math.exp(-x / 2.0) + 1.0
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadExpGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadExpTable_Y", 1)
    
    fit = qti.ExponentialFit(qti.app, l, "ThreadExpTable_Y")
    assert fit is not None
    fit.setInitialValues(5.0, 2.0, 1.0)
    success = fit.run()
    assert success is True
    
    res = fit.results()
    assert len(res) == 3
    a, t_const, y0 = res[0], res[1], res[2]
    assert abs(a - 5.0) < 0.1
    assert abs(t_const - 2.0) < 0.1
    assert abs(y0 - 1.0) < 0.1

def test_threaded_smooth_filter():
    t = qti.app.newTable("ThreadSmoothTable", 50, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 51):
        x = float(i)
        y = math.sin(x * 0.1)
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadSmoothGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadSmoothTable_Y", 1)
    
    # SmoothFilter method 1 = Savitzky-Golay
    sf = qti.SmoothFilter(qti.app, l, "ThreadSmoothTable_Y", 1)
    sf.setSmoothPoints(5, 2)
    sf.setPolynomOrder(2)
    success = sf.run()
    assert success is True

def test_threaded_interpolation():
    t = qti.app.newTable("ThreadInterpTable", 20, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 21):
        x = float(i)
        y = x * x
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadInterpGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadInterpTable_Y", 1)
    
    # Interpolation method 1 = Cubic spline
    interp = qti.Interpolation(qti.app, l, "ThreadInterpTable_Y", 1.0, 20.0, 1)
    interp.setOutputPoints(100)
    success = interp.run()
    assert success is True

def test_threaded_fft():
    t = qti.app.newTable("ThreadFFTTable", 64, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 65):
        x = float(i) * 0.1
        y = math.sin(2.0 * math.pi * 1.0 * x)
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("ThreadFFTGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "ThreadFFTTable_Y", 1)
    
    fft = qti.FFT(qti.app, l, "ThreadFFTTable_Y")
    success = fft.run()
    assert success is True

def test_fit_cancellation_flag():
    t = qti.app.newTable("CancelFitTable", 20, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 21):
        x = float(i)
        y = 2.0 * x + 1.0
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("CancelFitGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "CancelFitTable_Y", 1)
    
    fit = qti.LinearFit(qti.app, l, "CancelFitTable_Y")
    assert fit.isCanceled() is False
    fit.cancel()
    assert fit.isCanceled() is True
