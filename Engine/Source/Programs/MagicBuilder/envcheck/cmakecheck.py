"""CMake installation and version checks."""

from __future__ import annotations

import re
import shutil
import subprocess

from .model import CheckResult, CheckSpec, Status

MIN_CMAKE = (3, 20, 0)
COMMAND_TIMEOUT_SECONDS = 10
VERSION_PATTERN = re.compile(r"cmake version\s+(\d+)\.(\d+)(?:\.(\d+))?", re.IGNORECASE)


def check_cmake() -> CheckResult:
    executable = shutil.which("cmake")
    if executable is None:
        return CheckResult(
            check_id="cmake.installation",
            title="CMake",
            status=Status.FAIL,
            summary="CMake was not found on PATH",
            detail=("CMake 3.20 or later is required by this project",),
            data={"installed": False, "required": "3.20+"},
        )

    try:
        result = subprocess.run(
            (executable, "--version"),
            capture_output=True,
            text=True,
            errors="replace",
            timeout=COMMAND_TIMEOUT_SECONDS,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return CheckResult(
            check_id="cmake.installation",
            title="CMake",
            status=Status.FAIL,
            summary="CMake version check timed out",
            detail=(f"Command: {executable} --version",),
            data={"installed": True, "executable": executable, "required": "3.20+"},
        )
    except OSError as exc:
        return CheckResult(
            check_id="cmake.installation",
            title="CMake",
            status=Status.FAIL,
            summary="Could not run CMake",
            detail=(str(exc),),
            data={"installed": False, "executable": executable, "required": "3.20+"},
        )

    output = (result.stdout or result.stderr).strip()
    if result.returncode != 0:
        return CheckResult(
            check_id="cmake.installation",
            title="CMake",
            status=Status.FAIL,
            summary="CMake version command failed",
            detail=(output or f"Exit code: {result.returncode}",),
            data={
                "installed": True,
                "executable": executable,
                "exit_code": result.returncode,
                "required": "3.20+",
            },
        )

    match = VERSION_PATTERN.search(output)
    if match is None:
        return CheckResult(
            check_id="cmake.installation",
            title="CMake",
            status=Status.FAIL,
            summary="Could not determine the CMake version",
            detail=(output or "CMake returned no version information",),
            data={"installed": True, "executable": executable, "required": "3.20+"},
        )

    version = tuple(int(part or 0) for part in match.groups())
    satisfies = version >= MIN_CMAKE
    version_text = ".".join(str(part) for part in version)
    return CheckResult(
        check_id="cmake.installation",
        title="CMake",
        status=Status.OK if satisfies else Status.FAIL,
        summary=f"CMake {version_text}",
        detail=(
            f"Executable: {executable}",
            f"Required: CMake {MIN_CMAKE[0]}.{MIN_CMAKE[1]} or later",
        ),
        data={
            "installed": True,
            "executable": executable,
            "version": version_text,
            "required": f"{MIN_CMAKE[0]}.{MIN_CMAKE[1]}+",
            "satisfies_requirement": satisfies,
        },
    )


def cmake_checks() -> tuple[CheckSpec, ...]:
    """Return CMake checks required by this project."""
    return (CheckSpec("cmake.installation", "CMake", check_cmake),)
