import pytest

def test_app_available():
    import qti
    assert qti.app is not None

def test_new_table():
    import qti
    t = qti.app.newTable("SmokeTest", 2, 2)
    assert t is not None
    assert "SmokeTest" in qti.app.tableNames()
