import pytest
@pytest.fixture(autouse=True)
def per_test_cleanup():
    import qti
    if qti.app:
        qti.app.savedProject()
    yield
    try:
        import qti
        if qti.app:
            qti.app.savedProject()
    except:
        pass
