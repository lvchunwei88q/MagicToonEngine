"""Ninja installation and version checks."""

from __future__ import annotations

import re
import shutil
import subprocess

from .model import CheckResult, CheckSpec, Status

COMMAND_TIMEOUT_SECONDS = 10
VERSION_PATTERN = re.compile(r"^\s*(\d+)\.(\d+)(?:\.(\d+))?\s*$")
REQUIREMENT = "Ninja must be installed and available on PATH for the Python-based MagicBuilder"


def check_ninja() -> CheckResult:
    executable = shutil.which("ninja")
    if executable is None:
        return CheckResult(
            check_id="ninja.installation",
            title="Ninja",
            status=Status.FAIL,
            summary="Required build dependency Ninja was not found on PATH",
            detail=(
                "Install Ninja and add its executable directory to PATH.",
                REQUIREMENT,
            ),
            data={"installed": False, "required": REQUIREMENT},
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
            check_id="ninja.installation",
            title="Ninja",
            status=Status.FAIL,
            summary="Required Ninja version check timed out",
            detail=(f"Command: {executable} --version", REQUIREMENT),
            data={"installed": True, "executable": executable, "required": REQUIREMENT},
        )
    except OSError as exc:
        return CheckResult(
            check_id="ninja.installation",
            title="Ninja",
            status=Status.FAIL,
            summary="Could not run required Ninja",
            detail=(str(exc), "Check the Ninja installation and PATH.", REQUIREMENT),
            data={"installed": False, "executable": executable, "required": REQUIREMENT},
        )

    output = (result.stdout or result.stderr).strip()
    if result.returncode != 0:
        return CheckResult(
            check_id="ninja.installation",
            title="Ninja",
            status=Status.FAIL,
            summary="Required Ninja version command failed",
            detail=(
                output or f"Exit code: {result.returncode}",
                "Check that the executable on PATH is a working Ninja installation.",
                REQUIREMENT,
            ),
            data={
                "installed": True,
                "executable": executable,
                "exit_code": result.returncode,
                "required": REQUIREMENT,
            },
        )

    match = VERSION_PATTERN.fullmatch(output)
    if match is None:
        return CheckResult(
            check_id="ninja.installation",
            title="Ninja",
            status=Status.FAIL,
            summary="Could not determine the required Ninja version",
            detail=(output or "Ninja returned no version information", REQUIREMENT),
            data={"installed": True, "executable": executable, "required": REQUIREMENT},
        )

    version = ".".join(part for part in match.groups() if part is not None)
    return CheckResult(
        check_id="ninja.installation",
        title="Ninja",
        status=Status.OK,
        summary=f"Ninja {version}",
        detail=(f"Executable: {executable}",),
        data={
            "installed": True,
            "executable": executable,
            "version": version,
            "required": REQUIREMENT,
            "satisfies_requirement": True,
        },
    )


def ninja_checks() -> tuple[CheckSpec, ...]:
    """Return Ninja checks required by this project."""
    return (CheckSpec("ninja.installation", "Ninja", check_ninja),)
