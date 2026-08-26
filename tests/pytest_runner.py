import os
import sys
import traceback
import pytest

# Find project root robustly
cwd = os.getcwd()
candidate = cwd
project_root = cwd
while candidate and os.path.dirname(candidate) != candidate:
    if os.path.isdir(os.path.join(candidate, "tests")) and os.path.isfile(os.path.join(candidate, "CMakeLists.txt")):
        project_root = candidate
        break
    candidate = os.path.dirname(candidate)

log_file = os.path.join(project_root, "build", "tests", "pytest_results.txt")
os.makedirs(os.path.dirname(log_file), exist_ok=True)

with open(log_file, "w", encoding="utf-8") as f:
    old_stdout = sys.stdout
    old_stderr = sys.stderr
    sys.stdout = f
    sys.stderr = f
    retcode = -1
    try:
        print("--- PYTEST START ---")
        f.flush()
        retcode = pytest.main(["-v", os.path.join(project_root, "tests")])
        print(f"--- PYTEST END, EXIT CODE: {retcode} ---")
        f.flush()
    except Exception as e:
        print(f"Pytest Exception: {e}")
        traceback.print_exc(file=f)
        f.flush()
    finally:
        f.flush()
        sys.stdout = old_stdout
        sys.stderr = old_stderr

try:
    import qti
    if qti.app:
        qti.app.savedProject()
        qti.app.close()
except Exception as e:
    with open(log_file, "a", encoding="utf-8") as f:
        f.write(f"\nCleanup error: {e}\n")

if retcode != 0:
    sys.exit(int(retcode))
