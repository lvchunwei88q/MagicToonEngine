"""Result model shared by the console and GUI entry points."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import StrEnum
from typing import Any


class Status(StrEnum):
    """Outcome of a single check."""

    PENDING = "pending"
    OK = "ok"
    WARN = "warn"
    FAIL = "fail"

    @property
    def tag(self) -> str:
        """Fixed-width token for the console renderer."""
        return {
            Status.PENDING: " .. ",
            Status.OK: " OK ",
            Status.WARN: "WARN",
            Status.FAIL: "FAIL",
        }[self]

    @property
    def label(self) -> str:
        """Human readable label used by the GUI."""
        return {
            Status.PENDING: "检测中",
            Status.OK: "通过",
            Status.WARN: "警告",
            Status.FAIL: "失败",
        }[self]


class EventKind(StrEnum):
    """What happened to the cache."""

    UPDATED = "updated"
    CLEARED = "cleared"


@dataclass(frozen=True, slots=True)
class CheckResult:
    """One check's result.

    Immutable on purpose: the cache always stores the latest copy, and views
    render from that copy instead of mutating it in place.
    """

    check_id: str
    title: str
    status: Status = Status.PENDING
    summary: str = ""
    detail: tuple[str, ...] = ()
    data: dict[str, Any] = field(default_factory=dict)
    checked_at: float = 0.0
    duration_ms: float = 0.0


@dataclass(frozen=True, slots=True)
class CacheEvent:
    """Notification sent to subscribers.

    It deliberately carries no payload: subscribers re-read the cache so that
    console and GUI can never display different values.
    """

    kind: EventKind
    check_id: str | None = None
    revision: int = 0
