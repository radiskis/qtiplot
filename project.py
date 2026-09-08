import os
import sys
from sipbuild import Project

class QtiProject(Project):
    def update(self, tool):
        self.target_abi = (13, 9)
        self.abi_version = "13.9"
        # Dynamically find PyQt6 bindings
        try:
            import PyQt6
            qt_path = os.path.dirname(PyQt6.__file__)
            bindings_path = os.path.join(qt_path, "bindings")
            
            # Check standard path
            if os.path.exists(bindings_path):
                print(f"Project.py: Found PyQt6 bindings at: {bindings_path}")
                if bindings_path not in self.sip_include_dirs:
                    self.sip_include_dirs.append(bindings_path)
            else:
                # Check distro locations for Linux
                dist_bindings = [
                    "/usr/lib/python3/dist-packages/PyQt6/bindings",
                    "/usr/share/sip/PyQt6",
                ]
                found = False
                for p in dist_bindings:
                    if os.path.exists(p):
                        print(f"Project.py: Found distro PyQt6 bindings at: {p}")
                        if p not in self.sip_include_dirs:
                            self.sip_include_dirs.append(p)
                        found = True
                        break
                if not found:
                    print(f"Project.py: Warning: PyQt6 bindings directory not found at {bindings_path}")
                
        except ImportError:
            print("Project.py: Warning: Could not import PyQt6 to find bindings path")

        super().update(tool)
