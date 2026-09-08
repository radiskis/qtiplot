import pytest
import qti
import math


def test_speed_mode_api():
    app = qti.app

    app.setSpeedMaxPoints(2500)
    assert app.speedModeMaxPoints() == 2500

    app.setDouglasPeukerTolerance(0.0)
    assert app.getDouglasPeukerTolerance() == 0.0

    app.setSpeedModeExport(False)
    assert not app.speedModeExport()
    app.setSpeedModeExport(True)
    assert app.speedModeExport()


def _spiked_sine_table(name, n_points, spike_row):
    t = qti.app.newTable(name, n_points, 2)
    t.setColName(1, "X")
    t.setColName(2, "Y")
    for i in range(1, n_points + 1):
        y = 100.0 if i == spike_row else math.sin(i * 0.05)
        t.setCell(1, i, float(i))
        t.setCell(2, i, y)
    return t


def test_speed_mode_never_mutates_curve_data():
    """Speed mode is a Qwt paint attribute, so the curve keeps every sample.

    This is the invariant that matters: analysis, fitting and the Python API
    must always see the full series no matter how the layer is rendered.
    """
    n_points = 5000
    t = _spiked_sine_table("SpeedModeData", n_points, 2500)

    g = qti.app.newGraph("SpeedModePlot", 1, 1, 1)
    l = g.activeLayer()
    assert l is not None
    assert l.insertCurve(t, "SpeedModeData_Y", 1)
    c = l.curve(0)
    assert c is not None

    # Off: a point budget of 0 disables speed mode.
    l.enableDouglasPeukerSpeedMode(0.0, 0)
    assert not l.speedModeEnabled()
    assert c.dataSize() == n_points

    # On, pixel filtering only (tolerance 0).
    l.enableDouglasPeukerSpeedMode(0.0, 500)
    assert l.speedModeEnabled()
    assert l.speedModeMaxPoints() == 500
    assert c.dataSize() == n_points
    assert abs(c.x(0) - 1.0) < 1e-6
    assert abs(c.x(n_points - 1) - float(n_points)) < 1e-6
    assert c.maxYValue() >= 99.9

    # On, with a Douglas-Peucker tolerance.
    l.enableDouglasPeukerSpeedMode(1.0, 500)
    assert l.speedModeEnabled()
    assert abs(l.getDouglasPeukerTolerance() - 1.0) < 1e-12
    assert c.dataSize() == n_points
    assert c.maxYValue() >= 99.9

    # Off again.
    l.enableDouglasPeukerSpeedMode(0.0, 0)
    assert not l.speedModeEnabled()
    assert c.dataSize() == n_points


def _round_trip(tmp_path, name, tolerance, max_points):
    """Save a one-layer project with the given speed mode, reopen it, return the layer."""
    n_points = 4000
    t = _spiked_sine_table(name + "Data", n_points, 2000)

    g = qti.app.newGraph(name, 1, 1, 1)
    l = g.activeLayer()
    assert l.insertCurve(t, name + "Data_Y", 1)
    l.enableDouglasPeukerSpeedMode(tolerance, max_points)

    path = str(tmp_path / (name + ".qti"))
    qti.app.saveProjectAs(path, False)
    qti.app.open(path)

    g2 = qti.app.graph(name)
    assert g2 is not None
    l2 = g2.activeLayer()
    assert l2.curve(0).dataSize() == n_points
    return l2


def test_speed_mode_on_survives_project_round_trip(tmp_path):
    l = _round_trip(tmp_path, "RoundTripOn", 2.5, 1500)
    assert l.speedModeEnabled()
    assert l.speedModeMaxPoints() == 1500
    assert abs(l.getDouglasPeukerTolerance() - 2.5) < 1e-9


def test_speed_mode_off_survives_project_round_trip(tmp_path):
    """An absent <SpeedMode> tag inherits the application preference, so a layer
    with speed mode switched off must persist that explicitly."""
    qti.app.setSpeedMaxPoints(3000)   # preference is ON, so "off" cannot be implicit
    l = _round_trip(tmp_path, "RoundTripOff", 0.0, 0)
    assert not l.speedModeEnabled()
