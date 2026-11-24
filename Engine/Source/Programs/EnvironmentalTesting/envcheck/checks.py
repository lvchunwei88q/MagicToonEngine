"""Check registry.

This first slice only inspects Python itself (version and interpreter path).
Later checks (Visual Studio, MSVC, Windows SDK, CMake, Ninja, Git LFS, libclang)
can be added to `all_checks()` without changing the console entry point.
"""

from __future__ import annotations

import platform
import sys
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from .model import CheckResult, Status

# Minimum version documented in README.md / README.en.md.
MIN_PYTHON = (3, 12)


@dataclass(frozen=True, slots=True)
class CheckSpec:
    """A named check plus the callable that produces its result."""

    check_id: str
    title: str
    run: Callable[[], CheckResult]


def check_python_version() -> CheckResult:
    info = sys.version_info
    satisfied = (info.major, info.minor) >= MIN_PYTHON
    bits = 64 if sys.maxsize > 2**32 else 32
    gil_enabled = getattr(sys, "_is_gil_enabled", lambda: True)()
    return CheckResult(
        check_id="python.version",
        title="Python 版本",
        status=Status.OK if satisfied else Status.FAIL,
        summary=f"{platform.python_version()} · {platform.python_implementation()} · {bits}-bit",
        detail=(
            f"要求 Python {MIN_PYTHON[0]}.{MIN_PYTHON[1]} 或更高",
            sys.version.splitlines()[0].strip(),
            "GIL 已启用" if gil_enabled else "GIL 已禁用（free-threading 构建）",
        ),
        data={
            "version": platform.python_version(),
            "required": f"{MIN_PYTHON[0]}.{MIN_PYTHON[1]}+",
            "satisfied": satisfied,
            "implementation": platform.python_implementation(),
            "architecture": f"{bits}-bit",
            "gil_enabled": gil_enabled,
        },
    )


def check_python_interpreter() -> CheckResult:
    executable = Path(sys.executable)
    exists = executable.is_file()
    in_venv = sys.prefix != sys.base_prefix
    return CheckResult(
        check_id="python.interpreter",
        title="解释器路径",
        status=Status.OK if exists else Status.WARN,
        summary=str(executable),
        detail=(
            f"sys.prefix      {sys.prefix}",
            f"sys.base_prefix {sys.base_prefix}",
            "虚拟环境：是" if in_venv else "虚拟环境：否",
            f"编译器：{platform.python_compiler()}",
        ),
        data={
            "executable": str(executable),
            "exists": exists,
            "prefix": sys.prefix,
            "base_prefix": sys.base_prefix,
            "virtual_env": in_venv,
        },
    )


def python_checks() -> tuple[CheckSpec, ...]:
    """The Python slice of the registry."""
    return (
        CheckSpec("python.version", "Python 版本", check_python_version),
        CheckSpec("python.interpreter", "解释器路径", check_python_interpreter),
    )


def all_checks() -> tuple[CheckSpec, ...]:
    """Everything the tool knows how to check. Entry points use this."""
    return python_checks()
