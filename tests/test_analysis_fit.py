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
