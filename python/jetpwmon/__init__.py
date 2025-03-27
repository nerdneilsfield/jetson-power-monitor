"""
jetpwmon - A simple power monitor for Jetson.
"""

try:
    # Attempt to import the core functionality from the compiled C++/pybind11 extension module
    # !!! Important: The '_core' in '. _core' needs to be replaced with the actual name of your pybind11 module !!!
    # This name is usually specified when defining the Extension in your setup.py or CMakeLists.txt.
    # For example, if your .so file is _jetpwmon_core.cpython-XYZ.so, use ._jetpwmon_core
    # If it's jetpwmon.cpython-XYZ.so, use .jetpwmon
    # Assuming your module is compiled to _core
    from ._core import PowerMonitor #, SensorType, ErrorCode # If there are other exports needed

    # (Optional) If your pybind11 module is directly named 'jetpwmon'
    # from .jetpwmon import PowerMonitor

    # (Optional) Set which symbols are imported by `from jetpwmon import *`
    __all__ = ['PowerMonitor'] # Add ErrorCode, SensorType etc. if they exist and are needed

except ImportError as e:
    # If the compiled module does not exist, provide a useful error or warning
    import warnings
    warnings.warn(f"Could not import the compiled C++ core of jetpwmon: {e}\n"
                  "Please ensure the package is built and installed correctly.",
                  ImportWarning)
    # You can choose to define a placeholder PowerMonitor class here or let the import fail
    PowerMonitor = None # Example placeholder

# Define package-level metadata (optional)
__version__ = "0.1.1" # Should match pyproject.toml