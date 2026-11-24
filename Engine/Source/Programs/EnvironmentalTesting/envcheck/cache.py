"""Process-wide result store with change notification.

The runner writes here, subscribers are notified, and every view re-reads the
entries it displays. Neither entry point owns a private copy of the data.
"""

from __future__ import annotations

import threading
import traceback
from collections.abc import Callable, Iterable
from datetime import datetime
from typing import Any

from .model import CacheEvent, CheckResult, EventKind, Status


class EnvironmentCache:
    """Thread-safe store of check results shared by the whole process."""

    def __init__(self) -> None:
        self._lock = threading.RLock()
        self._results: dict[str, CheckResult] = {}
        self._order: list[str] = []
        self._listeners: list[Callable[[CacheEvent], None]] = []
        self._revision = 0

    @property
    def revision(self) -> int:
        """Monotonic counter; views can cheaply detect that something changed."""
        with self._lock:
            return self._revision

    def put(self, result: CheckResult) -> None:
        """Store a result and notify every subscriber outside the lock."""
        with self._lock:
            if result.check_id not in self._results:
                self._order.append(result.check_id)
            self._results[result.check_id] = result
            self._revision += 1
            event = CacheEvent(EventKind.UPDATED, result.check_id, self._revision)
            listeners = tuple(self._listeners)
        self._broadcast(listeners, event)

    def put_many(self, results: Iterable[CheckResult]) -> None:
        for result in results:
            self.put(result)

    def get(self, check_id: str) -> CheckResult | None:
        with self._lock:
            return self._results.get(check_id)

    def snapshot(self) -> tuple[CheckResult, ...]:
        """Results in registration order."""
        with self._lock:
            return tuple(self._results[check_id] for check_id in self._order)

    def clear(self) -> None:
        with self._lock:
            self._results.clear()
            self._order.clear()
            self._revision += 1
            event = CacheEvent(EventKind.CLEARED, None, self._revision)
            listeners = tuple(self._listeners)
        self._broadcast(listeners, event)

    def subscribe(self, listener: Callable[[CacheEvent], None]) -> Callable[[], None]:
        """Register a listener, returning a callable that unsubscribes it.

        The listener may be called from a worker thread; GUI subscribers must
        marshal back to the UI thread themselves.
        """
        with self._lock:
            self._listeners.append(listener)

        def unsubscribe() -> None:
            with self._lock:
                if listener in self._listeners:
                    self._listeners.remove(listener)

        return unsubscribe

    def counts(self) -> dict[str, int]:
        """Number of results per status."""
        counts = {status.value: 0 for status in Status}
        for result in self.snapshot():
            counts[result.status.value] += 1
        return counts

    def as_dict(self) -> dict[str, Any]:
        """Serialisable snapshot used by `--json` and the GUI copy button."""
        return {
            "generated_at": datetime.now().astimezone().isoformat(timespec="seconds"),
            "revision": self.revision,
            "summary": self.counts(),
            "checks": [
                {
                    "id": result.check_id,
                    "title": result.title,
                    "status": result.status.value,
                    "summary": result.summary,
                    "detail": list(result.detail),
                    "data": result.data,
                    "duration_ms": round(result.duration_ms, 2),
                }
                for result in self.snapshot()
            ],
        }

    @staticmethod
    def _broadcast(listeners: tuple[Callable[[CacheEvent], None], ...], event: CacheEvent) -> None:
        for listener in listeners:
            try:
                listener(event)
            except Exception:  # a broken view must never abort the check run
                traceback.print_exc()


_CACHE = EnvironmentCache()


def get_cache() -> EnvironmentCache:
    """The single cache instance shared by every entry point."""
    return _CACHE
