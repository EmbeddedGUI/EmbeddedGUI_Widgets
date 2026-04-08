"""Shared widget catalog helpers for HelloCustomWidgets."""

from __future__ import annotations

import json
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
CATALOG_PATH = ROOT_DIR / "example" / "HelloCustomWidgets" / "widget_catalog.json"
VALID_TRACKS = {"reference", "showcase", "deprecated"}
VALID_VISIBILITIES = {"public", "internal", "hidden"}
EXPECTED_VISIBILITY_BY_TRACK = {
    "reference": "public",
    "showcase": "internal",
    "deprecated": "hidden",
}


def scan_custom_widgets() -> list[str]:
    base = ROOT_DIR / "example" / "HelloCustomWidgets"
    if not base.is_dir():
        return []

    result = []
    for source in sorted(base.glob("*/*/test.c")):
        result.append(source.parent.relative_to(base).as_posix())
    return result


def _normalize_entry(entry: dict) -> dict:
    widget_id = str(entry.get("id", "")).strip().replace("\\", "/")
    track = str(entry.get("track", "showcase")).strip() or "showcase"
    visibility = str(entry.get("visibility", "internal")).strip() or "internal"
    replacement = entry.get("replacement")

    if track not in VALID_TRACKS:
        raise ValueError("invalid track for %s: %s" % (widget_id or "<missing>", track))
    if visibility not in VALID_VISIBILITIES:
        raise ValueError("invalid visibility for %s: %s" % (widget_id or "<missing>", visibility))
    if replacement is not None:
        replacement = str(replacement).strip().replace("\\", "/") or None

    return {
        "id": widget_id,
        "track": track,
        "visibility": visibility,
        "reference_system": str(entry.get("reference_system", "") or ""),
        "reference_library": str(entry.get("reference_library", "") or ""),
        "reference_component": str(entry.get("reference_component", "") or ""),
        "replacement": replacement,
        "doc_state": str(entry.get("doc_state", "ok") or "ok"),
    }


def load_widget_catalog() -> list[dict]:
    if not CATALOG_PATH.exists():
        return [{"id": widget_id, "track": "showcase", "visibility": "internal", "reference_system": "", "reference_library": "",
                 "reference_component": "", "replacement": None, "doc_state": "ok"} for widget_id in scan_custom_widgets()]

    entries = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    if not isinstance(entries, list):
        raise ValueError("widget catalog must be a JSON array")

    normalized = []
    seen_ids = set()
    for raw_entry in entries:
        if not isinstance(raw_entry, dict):
            raise ValueError("widget catalog entries must be JSON objects")
        entry = _normalize_entry(raw_entry)
        if not entry["id"]:
            raise ValueError("widget catalog entry is missing id")
        if entry["id"] in seen_ids:
            raise ValueError("widget catalog contains duplicate id: %s" % entry["id"])
        seen_ids.add(entry["id"])
        normalized.append(entry)

    return normalized


def build_widget_catalog_map() -> dict[str, dict]:
    return {entry["id"]: entry for entry in load_widget_catalog()}


def get_catalog_track_counts() -> dict[str, int]:
    counts = {track: 0 for track in sorted(VALID_TRACKS)}
    for entry in load_widget_catalog():
        counts[entry["track"]] += 1
    return counts


def filter_widget_ids(category: str | None = None, track: str = "all", include_deprecated: bool = False) -> list[str]:
    normalized_track = (track or "all").strip().lower()
    if normalized_track not in VALID_TRACKS and normalized_track != "all":
        raise ValueError("unknown widget track: %s" % track)

    normalized_category = category.strip().strip("/\\") if category else None
    result = []
    for entry in load_widget_catalog():
        widget_id = entry["id"]
        widget_category = widget_id.split("/", 1)[0]
        if normalized_category and widget_category != normalized_category:
            continue
        if not include_deprecated and entry["track"] == "deprecated":
            continue
        if normalized_track != "all" and entry["track"] != normalized_track:
            continue
        result.append(widget_id)

    return sorted(result)
