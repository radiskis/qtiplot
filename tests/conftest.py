import pytest

@pytest.fixture(autouse=True)
def per_test_cleanup():
    try:
        import qti
        if qti.app:
            qti.app.savedProject()
    except ImportError:
        pass
    yield
    try:
        import qti
        if qti.app:
            qti.app.savedProject()
    except Exception:
        pass
