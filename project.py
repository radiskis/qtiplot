import os
from sipbuild import Project

class QtiProject(Project):
    def update(self, tool):
        # Dynamically find PyQt5 bindings
        try:
            import PyQt5
            qt_path = os.path.dirname(PyQt5.__file__)
            bindings_path = os.path.join(qt_path, "bindings")
            # Also check for alternate locations (e.g. Qt/bin issue seen earlier)
            # But normally bindings are in site-packages/PyQt5/bindings or similar
            
            if os.path.exists(bindings_path):
                print(f"Project.py: Found PyQt5 bindings at: {bindings_path}")
                self.sip_include_dirs.append(bindings_path)
            else:
                print(f"Project.py: Warning: PyQt5 bindings directory not found at {bindings_path}")
                
        except ImportError:
            print("Project.py: Warning: Could not import PyQt5 to find bindings path")

        super().update(tool)
