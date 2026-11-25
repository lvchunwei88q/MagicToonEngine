"""Git and Git LFS availability checks."""

from __future__ import annotations

import shutil
import subprocess

from .model import CheckResult, CheckSpec, Status

COMMAND_TIMEOUT_SECONDS = 10


def _run_git(args: tuple[str, ...]) -> tuple[subprocess.CompletedProcess[str] | None, str | None]:
    git_path = shutil.which("git")
    if git_path is None:
        return None, "Git executable was not found on PATH"

    try:
        result = subprocess.run(
            (git_path, *args),
            capture_output=True,
            text=True,
            errors="replace",
            timeout=COMMAND_TIMEOUT_SECONDS,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return None, f"`git {' '.join(args)}` timed out"
    except OSError as exc:
        return None, f"Could not run Git: {exc}"
    return result, None


def check_git_installation() -> CheckResult:
    result, error = _run_git(("--version",))
    if error is not None:
        return CheckResult(
            check_id="git.installation",
            title="Git Installation",
            status=Status.FAIL,
            summary="Git is not available",
            detail=(error,),
            data={"installed": False},
        )

    assert result is not None
    version = result.stdout.strip()
    if result.returncode != 0:
        return CheckResult(
            check_id="git.installation",
            title="Git Installation",
            status=Status.FAIL,
            summary="Git command failed",
            detail=(result.stderr.strip() or f"Exit code: {result.returncode}",),
            data={"installed": False, "exit_code": result.returncode},
        )

    return CheckResult(
        check_id="git.installation",
        title="Git Installation",
        status=Status.OK,
        summary=version or "Git is installed",
        detail=(f"Executable: {shutil.which('git')}",),
        data={"installed": True, "version": version, "executable": shutil.which("git")},
    )


def check_git_lfs() -> CheckResult:
    result, error = _run_git(("lfs", "version"))
    if error is not None:
        return CheckResult(
            check_id="git.lfs",
            title="Git LFS",
            status=Status.FAIL,
            summary="Git LFS is not available",
            detail=(error,),
            data={"available": False},
        )

    assert result is not None
    version = (result.stdout or result.stderr).strip()
    if result.returncode != 0:
        return CheckResult(
            check_id="git.lfs",
            title="Git LFS",
            status=Status.FAIL,
            summary="Git LFS is not available",
            detail=(version or f"`git lfs version` exited with code {result.returncode}",),
            data={"available": False, "exit_code": result.returncode},
        )

    return CheckResult(
        check_id="git.lfs",
        title="Git LFS",
        status=Status.OK,
        summary=version or "Git LFS is available",
        detail=("Git LFS command is available through Git",),
        data={"available": True, "version": version},
    )


def git_checks() -> tuple[CheckSpec, ...]:
    """Return checks for Git and its Git LFS extension."""
    return (
        CheckSpec("git.installation", "Git Installation", check_git_installation),
        CheckSpec("git.lfs", "Git LFS", check_git_lfs),
    )
