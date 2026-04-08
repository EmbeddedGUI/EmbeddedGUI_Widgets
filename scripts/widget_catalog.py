"""Shared widget catalog helpers for HelloCustomWidgets."""

from __future__ import annotations

import json
from collections import Counter
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
CATALOG_PATH = ROOT_DIR / "example" / "HelloCustomWidgets" / "widget_catalog.json"
WEB_CATALOG_POLICY_PATH = ROOT_DIR / "web" / "catalog-policy.json"
VALID_TRACKS = {"reference", "showcase", "deprecated"}
VALID_VISIBILITIES = {"public", "internal", "hidden"}
EXPECTED_VISIBILITY_BY_TRACK = {
    "reference": "public",
    "showcase": "internal",
    "deprecated": "hidden",
}
CATALOG_FIELD_ORDER = [
    "id",
    "track",
    "visibility",
    "reference_system",
    "reference_library",
    "reference_component",
    "replacement",
    "doc_state",
]
CATEGORY_ORDER = ["input", "layout", "navigation", "display", "feedback", "decoration", "chart", "media", "misc"]


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


def make_default_entry(widget_id: str) -> dict:
    return {
        "id": widget_id,
        "track": "showcase",
        "visibility": EXPECTED_VISIBILITY_BY_TRACK["showcase"],
        "reference_system": "",
        "reference_library": "",
        "reference_component": "",
        "replacement": None,
        "doc_state": "ok",
    }


def load_widget_catalog() -> list[dict]:
    if not CATALOG_PATH.exists():
        return [make_default_entry(widget_id) for widget_id in scan_custom_widgets()]

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


def sort_catalog_entries(entries: list[dict]) -> list[dict]:
    return sorted(entries, key=lambda entry: entry["id"])


def format_catalog_entries(entries: list[dict]) -> str:
    normalized_entries = []
    for entry in sort_catalog_entries(entries):
        normalized_entries.append({field: entry[field] for field in CATALOG_FIELD_ORDER})
    return json.dumps(normalized_entries, indent=2, ensure_ascii=False) + "\n"


def build_synchronized_catalog_entries() -> list[dict]:
    existing = build_widget_catalog_map()
    synchronized = []
    for widget_id in scan_custom_widgets():
        synchronized.append(existing.get(widget_id, make_default_entry(widget_id)))
    return sort_catalog_entries(synchronized)


def write_widget_catalog(path: Path | None = None) -> list[dict]:
    target_path = path or CATALOG_PATH
    entries = build_synchronized_catalog_entries()
    target_path.parent.mkdir(parents=True, exist_ok=True)
    target_path.write_text(format_catalog_entries(entries), encoding="utf-8")
    return entries


def sort_category_ids(category_ids: set[str]) -> list[str]:
    def sort_key(category_id: str) -> tuple[int, str]:
        try:
            return (CATEGORY_ORDER.index(category_id), category_id)
        except ValueError:
            return (len(CATEGORY_ORDER), category_id)

    return sorted(category_ids, key=sort_key)


def build_catalog_policy_summary(entries: list[dict] | None = None) -> dict:
    resolved_entries = sort_catalog_entries(entries or load_widget_catalog())
    track_counts = Counter(entry["track"] for entry in resolved_entries)
    visibility_counts = Counter(entry["visibility"] for entry in resolved_entries)
    category_ids = {entry["id"].split("/", 1)[0] for entry in resolved_entries}
    categories = []
    for category_id in sort_category_ids(category_ids):
        category_entries = [entry for entry in resolved_entries if entry["id"].startswith(category_id + "/")]
        category_tracks = Counter(entry["track"] for entry in category_entries)
        categories.append(
            {
                "id": category_id,
                "total": len(category_entries),
                "reference": category_tracks["reference"],
                "showcase": category_tracks["showcase"],
                "deprecated": category_tracks["deprecated"],
            }
        )

    replacements = []
    for entry in resolved_entries:
        if not entry["replacement"]:
            continue
        replacements.append(
            {
                "source": entry["id"],
                "target": entry["replacement"],
            }
        )

    return {
        "generatedFrom": "example/HelloCustomWidgets/widget_catalog.json",
        "total": len(resolved_entries),
        "defaultWebTotal": track_counts["reference"] + track_counts["showcase"],
        "tracks": {
            "reference": track_counts["reference"],
            "showcase": track_counts["showcase"],
            "deprecated": track_counts["deprecated"],
        },
        "visibility": {
            "public": visibility_counts["public"],
            "internal": visibility_counts["internal"],
            "hidden": visibility_counts["hidden"],
        },
        "categories": categories,
        "replacements": replacements,
    }


def format_catalog_policy_summary(summary: dict) -> str:
    return json.dumps(summary, indent=2, ensure_ascii=False) + "\n"


def write_catalog_policy_summary(path: Path | None = None, entries: list[dict] | None = None) -> dict:
    target_path = path or WEB_CATALOG_POLICY_PATH
    summary = build_catalog_policy_summary(entries=entries)
    target_path.parent.mkdir(parents=True, exist_ok=True)
    target_path.write_text(format_catalog_policy_summary(summary), encoding="utf-8")
    return summary


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
