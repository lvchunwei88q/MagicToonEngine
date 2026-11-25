"""Shared core for the environmental-testing console tool.

`environment_stage.py` presents results from the shared cache. Check
definitions are registered through `all_checks()` so the pipeline entry point
does not need to change when new checks are added.
"""

from .cache import EnvironmentCache, get_cache
from .checks import MIN_PYTHON, CheckSpec, all_checks, cmake_checks, git_checks, python_checks
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
    "cmake_checks",
    "get_cache",
    "git_checks",
    "python_checks",
]
