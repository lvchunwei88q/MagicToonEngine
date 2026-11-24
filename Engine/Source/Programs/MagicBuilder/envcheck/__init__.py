"""Shared core for the environmental-testing console tool.

`env_console.py` presents results from the shared cache. Check definitions are
registered through `all_checks()` so new checks need no entry-point changes.
"""

from .cache import EnvironmentCache, get_cache
from .checks import MIN_PYTHON, CheckSpec, all_checks, python_checks
from .model import CacheEvent, CheckResult, EventKind, Status
from .runner import CheckRunner

__all__ = [
    "MIN_PYTHON",
    "CacheEvent",
    "CheckResult",
    "CheckRunner",
    "CheckSpec",
    "EnvironmentCache",
    "EventKind",
    "Status",
    "all_checks",
    "get_cache",
    "python_checks",
]
