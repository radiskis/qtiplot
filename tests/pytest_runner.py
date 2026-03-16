import pytest
import os
import sys

# Redirect stdout/stderr to a file for capturing pytest output
log_file = "build/tests/pytest_results.txt"

with open(log_file, "w") as f:
    # Save original stdout
    old_stdout = sys.stdout
    old_stderr = sys.stderr
    sys.stdout = f
    sys.stderr = f
    try:
        print("--- PYTEST START ---")
        # Run pytest on the tests directory
        retcode = pytest.main(["-v", "tests"])
        print(f"--- PYTEST END, EXIT CODE: {retcode} ---")
    finally:
        sys.stdout = old_stdout
        sys.stderr = old_stderr

# NOW import qti to close the app
import qti
qti.app.savedProject()
qti.app.close()
