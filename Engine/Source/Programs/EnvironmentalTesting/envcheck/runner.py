"""Check execution.

Runs each check on a worker thread and publishes the result into the shared
cache as soon as it is available, so subscribers update progressively.
"""

from __future__ import annotations

import threading
import time
from collections.abc import Iterable, Iterator
from dataclasses import replace

from .cache import EnvironmentCache, get_cache
from .checks import CheckSpec
from .model import CheckResult, Status


class CheckRunner:
    """Executes `CheckSpec.run` callables and publishes each result."""

    def __init__(self, specs: Iterable[CheckSpec], cache: EnvironmentCache | None = None) -> None:
        self._specs = tuple(specs)
        self._cache = cache if cache is not None else get_cache()
        self._thread: threading.Thread | None = None

    @property
    def running(self) -> bool:
        return self._thread is not None and self._thread.is_alive()

    def start(self) -> None:
        """Publish pending placeholders, then run everything off the UI thread."""
        if self.running:
            return
        for spec in self._specs:
            self._cache.put(CheckResult(check_id=spec.check_id, title=spec.title))
        self._thread = threading.Thread(
            target=self._consume,
            name="environment-check",
            daemon=True,
        )
        self._thread.start()

    def run_sync(self) -> tuple[CheckResult, ...]:
        """Run everything on the calling thread (used by `--json` / CI)."""
        return tuple(self._run_all())

    def wait(self, timeout: float | None = None) -> None:
        thread = self._thread
        if thread is not None:
            thread.join(timeout)

    def _consume(self) -> None:
        # The generator publishes results as a side effect; draining it is enough.
        for _ in self._run_all():
            pass

    def _run_all(self) -> Iterator[CheckResult]:
        for spec in self._specs:
            started = time.perf_counter()
            try:
                result = spec.run()
            except Exception as exc:  # one broken check must not stop the others
                result = CheckResult(
                    check_id=spec.check_id,
                    title=spec.title,
                    status=Status.FAIL,
                    summary=f"检查抛出异常：{type(exc).__name__}",
                    detail=(str(exc),),
                )
            result = replace(
                result,
                checked_at=time.time(),
                duration_ms=(time.perf_counter() - started) * 1000.0,
            )
            self._cache.put(result)
            yield result
