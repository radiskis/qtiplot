import pytest
import math
import qti

def test_linear_fit():
    # Create test data: y = 2*x + 3
    t = qti.app.newTable("FitLinearData", 10, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 11):
        x = float(i)
        y = 2.0 * x + 3.0
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("FitLinearGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "FitLinearData_Y", 1)
    
    # Run Linear Fit
    fit = qti.LinearFit(qti.app, l, "FitLinearData_Y")
    assert fit is not None
    success = fit.run()
    assert success is True
    
    # Verify results: slope ~ 2.0, intercept ~ 3.0
    res = fit.results()
    assert len(res) == 2
    intercept, slope = res[0], res[1]
    assert abs(slope - 2.0) < 1e-4
    assert abs(intercept - 3.0) < 1e-4
    assert fit.rSquare() > 0.999

def test_polynomial_fit():
    # Create test data: y = 1.0 + 2.0*x + 0.5*x^2
    t = qti.app.newTable("FitPolyData", 15, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 16):
        x = float(i)
        y = 1.0 + 2.0 * x + 0.5 * (x ** 2)
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("FitPolyGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "FitPolyData_Y", 1)
    
    # Run Polynomial Fit (order 2)
    fit = qti.PolynomialFit(qti.app, l, "FitPolyData_Y", 2)
    assert fit is not None
    success = fit.run()
    assert success is True
    
    res = fit.results()
    assert len(res) == 3
    a0, a1, a2 = res[0], res[1], res[2]
    assert abs(a0 - 1.0) < 1e-3
    assert abs(a1 - 2.0) < 1e-3
    assert abs(a2 - 0.5) < 1e-3
    assert fit.rSquare() > 0.999

def test_exponential_fit():
    # Create test data: y = 100 * exp(-x / 5.0)
    t = qti.app.newTable("FitExpData", 20, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 21):
        x = float(i)
        y = 100.0 * math.exp(-x / 5.0)
        t.setCell(1, i, x)
        t.setCell(2, i, y)
        
    g = qti.app.newGraph("FitExpGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "FitExpData_Y", 1)
    
    # Run Exponential Fit (growth = False)
    fit = qti.ExponentialFit(qti.app, l, "FitExpData_Y", False)
    assert fit is not None
    fit.guessInitialValues()
    success = fit.run()
    assert success is True
    assert fit.rSquare() > 0.99

def test_fft_phase_atan2_and_shift():
    # Test FFT on odd number of points with shift and normalization (C5, C6)
    t = qti.app.newTable("FFTData", 9, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 10):
        t.setCell(1, i, float(i))
        t.setCell(2, i, math.sin(2.0 * math.pi * float(i) / 9.0))
    
    fft = qti.FFT(qti.app, t, "FFTData_Y")
    assert fft is not None
    fft.shiftFrequencies(True)
    fft.normalizeAmplitudes(True)
    success = fft.run()
    assert success is True

def test_smooth_filter_window_larger_than_data():
    # Test smoothing when window exceeds data size (C4)
    t = qti.app.newTable("SmoothData", 10, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 11):
        t.setCell(1, i, float(i))
        t.setCell(2, i, float(i * i))
        
    smooth = qti.SmoothFilter(qti.app, t, "SmoothData_X", "SmoothData_Y", 1, 10, 3) # Average
    assert smooth is not None
    smooth.setSmoothPoints(30) # larger than 10 points
    success = smooth.run()
    assert success is True

def test_differentiation_short_and_constant_data():
    # Test differentiation on small input with identical values (C11)
    t = qti.app.newTable("DiffData", 5, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, 6):
        t.setCell(1, i, 1.0) # identical x
        t.setCell(2, i, 2.0)
        
    diff = qti.Differentiation(qti.app, t, "DiffData_X", "DiffData_Y", 1, 5)
    assert diff is not None
    success = diff.run()
    assert success is True

def test_fit_degrees_of_freedom_guard():
    # Test fit with more parameters than points (C2)
    t = qti.app.newTable("FitDofData", 2, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    t.setCell(1, 1, 1.0)
    t.setCell(2, 1, 2.0)
    t.setCell(1, 2, 2.0)
    t.setCell(2, 2, 4.0)
    
    g = qti.app.newGraph("FitDofGraph", 1, 1, 1)
    l = g.activeLayer()
    l.insertCurve(t, "FitDofData_Y", 1)
    
    # 2 points with polynomial order 2 requires 3 parameters (d_p = 3 > d_n = 2)
    poly = qti.PolynomialFit(qti.app, l, "FitDofData_Y", 2)
    assert poly is not None
    # Must fail safely without divide-by-zero or crash
    success = poly.run()
    assert success is False

def test_large_dataset_linear_fit_decimation_parity():
    # T10: Verify 10,000 points fitted with and without decimation enabled on the graph
    # yields identical fit parameters and fits all 10,000 points (T6 verification)
    n_points = 10000
    t = qti.app.newTable("LargeFitData", n_points, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, n_points + 1):
        x = float(i)
        y = 2.5 * x + 10.0
        t.setCell(1, i, x)
        t.setCell(2, i, y)

    g = qti.app.newGraph("LargeFitGraph", 1, 1, 1)
    l = g.activeLayer()
    assert l.insertCurve(t, "LargeFitData_Y", 1)
    c = l.curve(0)
    assert c.dataSize() == n_points

    # Fit without decimation
    fit_raw = qti.LinearFit(qti.app, l, "LargeFitData_Y")
    assert fit_raw.run() is True
    assert len(fit_raw.residuals()) == n_points
    slope_raw = fit_raw.results()[1]
    intercept_raw = fit_raw.results()[0]

    # Now enable speed mode (3000 point budget) on the layer
    l.enableDouglasPeukerSpeedMode(0.0, 3000)
    assert l.speedModeEnabled()
    # Verify curve data was NOT decimated in memory!
    assert c.dataSize() == n_points

    # Fit with decimation enabled on the plot
    fit_dec = qti.LinearFit(qti.app, l, "LargeFitData_Y")
    assert fit_dec.run() is True
    # Crucial: Fit must operate on all 10,000 points, NOT 3,000 decimated points!
    assert len(fit_dec.residuals()) == n_points
    slope_dec = fit_dec.results()[1]
    intercept_dec = fit_dec.results()[0]

    assert abs(slope_raw - slope_dec) < 1e-9
    assert abs(intercept_raw - intercept_dec) < 1e-9
    assert abs(slope_dec - 2.5) < 1e-3
    assert abs(intercept_dec - 10.0) < 0.2


