"""Registry that combines check categories for the console entry point."""

from __future__ import annotations

from .model import CheckSpec
from .pycheck import MIN_PYTHON, python_checks
from .gitcheck import git_checks


def all_checks() -> tuple[CheckSpec, ...]:
    """Return all registered checks.

    Add each new check category here so entry points only need to call this
    registry.
    """
    return (*python_checks(), *git_checks())
