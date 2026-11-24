"""Environment-check stage for the MagicToonEngine build pipeline.

Usage:
    py -3 main.py
    py -3 main.py --json
    py -3 main.py --no-color

Results are printed as they arrive in the shared cache; the exit code is 1 when
any check failed, 2 when the interpreter is too old to run the tool at all.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import unicodedata

MIN_PYTHON = (3, 12)
RESET = "\x1b[0m"
STATUS_COLOR = {
    "ok": "\x1b[32m",
    "warn": "\x1b[33m",
    "fail": "\x1b[1;31m",
    "pending": "\x1b[90m",
}
TITLE_WIDTH = 16
# "  [ OK ] title         " -> keep detail lines aligned under the title column.
INDENT = " " * (2 + 6 + 1 + TITLE_WIDTH + 1)


def _enable_ansi() -> bool:
    """Turn on VT processing; returns False when output is redirected."""
    if os.environ.get("NO_COLOR"):
        return False
    if not sys.stdout.isatty():
        return False
    if os.name != "nt":
        return True
    try:
        import ctypes

        kernel32 = ctypes.windll.kernel32
        handle = kernel32.GetStdHandle(-11)  # STD_OUTPUT_HANDLE
        mode = ctypes.c_ulong()
        if not kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
            return False
        enable_vt = 0x0004  # ENABLE_VIRTUAL_TERMINAL_PROCESSING
        if mode.value & enable_vt:
            return True
        return bool(kernel32.SetConsoleMode(handle, mode.value | enable_vt))
    except Exception:
        return False


def _display_width(text: str) -> int:
    """Terminal cell width; CJK characters occupy two cells."""
    return sum(2 if unicodedata.east_asian_width(ch) in ("W", "F") else 1 for ch in text)


def _pad(text: str, width: int) -> str:
    return text + " " * max(0, width - _display_width(text))


class _Palette:
    """Tiny ANSI painter, disabled automatically for redirected output."""

    def __init__(self, enabled: bool) -> None:
        self.enabled = enabled

    def tag(self, status, text: str) -> str:
        if not self.enabled:
            return text
        return STATUS_COLOR.get(status.value, "") + text + RESET


def _print_result(result, palette: _Palette) -> None:
    tag = "[" + result.status.tag + "]"
    print(f"  {palette.tag(result.status, tag)} {_pad(result.title, TITLE_WIDTH)} {result.summary}")
    for line in result.detail:
        print(INDENT + line)


def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Register options owned by the environment-check stage."""
    parser.add_argument("--print-json", action="store_true", help="Output results as JSON for CI and scripts")


def run_environment_stage(options: argparse.Namespace) -> int:
    if sys.version_info < MIN_PYTHON:
        sys.stderr.write(
            "Python {}.{} or later is required (see README for environment requirements). "
            "Current version: {}\nInterpreter path: {}\n".format(
                MIN_PYTHON[0],
                MIN_PYTHON[1],
                sys.version.split()[0],
                sys.executable,
            )
        )
        return 2

    here = os.path.dirname(os.path.abspath(__file__))
    if here not in sys.path:
        sys.path.insert(0, here)

    from envcheck import CheckRunner, Status, all_checks, get_cache

    cache = get_cache()
    specs = all_checks()
    runner = CheckRunner(specs, cache)

    if options.print_json:
        runner.run_sync()
        sys.stdout.write(json.dumps(cache.as_dict(), ensure_ascii=False, indent=2) + "\n")
        return 1 if cache.counts()["fail"] else 0

    palette = _Palette(not options.no_color and _enable_ansi())

    def on_cache_event(event) -> None:
        # The event carries no payload: read the latest value from the cache.
        if event.check_id is None:
            return
        result = cache.get(event.check_id)
        if result is None or result.status is Status.PENDING:
            return
        _print_result(result, palette)

    cache.subscribe(on_cache_event)

    print()
    print("MagicToonEngine - Build Environment Check")
    print("-" * 62)
    print(f"  Running {len(specs)} checks; results are streamed from the shared cache\n")

    runner.start()
    runner.wait()

    counts = cache.counts()
    print("-" * 62)
    print(f"  {counts['ok']} passed | {counts['warn']} warnings | {counts['fail']} failed")
    return 1 if counts["fail"] else 0
