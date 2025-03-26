# jetpwmon/__init__.py
"""Jetson Power Monitor Package"""

import os
import sys
try:
    from .jetpwmon import *
except ImportError as e:
    print(f"Error importing _core module: {e}", file=sys.stderr)
    print(f"Python path: {sys.path}", file=sys.stderr)
    print(f"Package location: {os.path.dirname(__file__)}", file=sys.stderr)
    print(f"Files in package directory:", os.listdir(os.path.dirname(__file__)), file=sys.stderr)
    raise