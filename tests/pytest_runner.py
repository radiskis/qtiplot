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

class Tee:
    def __init__(self, *files):
        self.files = [f for f in files if f is not None]
    def write(self, obj):
        for f in self.files:
            try:
                f.write(obj)
                f.flush()
            except Exception:
                pass
    def flush(self):
        for f in self.files:
            try:
                f.flush()
            except Exception:
                pass
    def isatty(self):
        return any(getattr(f, 'isatty', lambda: False)() for f in self.files)

with open(log_file, "w", encoding="utf-8") as f:
    old_stdout = sys.stdout
    old_stderr = sys.stderr
    sys.stdout = Tee(old_stdout, f)
    sys.stderr = Tee(old_stderr, f)
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
    import gc
    import qti
    gc.collect()
    if qti.app:
        qti.app.savedProject()
        qti.app.closeProject()
    gc.collect()
except Exception as e:
    with open(log_file, "a", encoding="utf-8") as f:
        f.write(f"\nCleanup error: {e}\n")

if retcode != 0:
    sys.exit(int(retcode))
