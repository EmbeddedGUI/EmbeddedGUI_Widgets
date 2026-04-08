#!/usr/bin/env python3
"""Validate HelloCustomWidgets widget_catalog.json coverage and policy rules."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_ROOT = SCRIPT_DIR.parent
ROOT_DIR = SCRIPTS_ROOT.parent

if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))

from widget_catalog import (
    EXPECTED_VISIBILITY_BY_TRACK,
    build_widget_catalog_map,
    get_catalog_track_counts,
    scan_custom_widgets,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check HelloCustomWidgets widget_catalog.json for coverage and mainline policy consistency."
    )
    return parser.parse_args()


def main() -> int:
    parse_args()
    catalog_map = build_widget_catalog_map()
    actual_widget_ids = sorted(scan_custom_widgets())
    actual_widget_set = set(actual_widget_ids)
    catalog_widget_set = set(catalog_map)

    errors: list[str] = []

    missing_entries = sorted(actual_widget_set - catalog_widget_set)
    orphan_entries = sorted(catalog_widget_set - actual_widget_set)
    for widget_id in missing_entries:
        errors.append("missing catalog entry: %s" % widget_id)
    for widget_id in orphan_entries:
        errors.append("catalog entry has no widget directory: %s" % widget_id)

    for widget_id, entry in sorted(catalog_map.items()):
        expected_visibility = EXPECTED_VISIBILITY_BY_TRACK[entry["track"]]
        if entry["visibility"] != expected_visibility:
            errors.append(
                "%s: %s track must use visibility=%s (got %s)"
                % (widget_id, entry["track"], expected_visibility, entry["visibility"])
            )

        has_reference_metadata = any(
            entry[key] for key in ("reference_system", "reference_library", "reference_component")
        )
        if entry["track"] == "reference":
            missing_metadata = [
                key for key in ("reference_system", "reference_library", "reference_component") if not entry[key]
            ]
            if missing_metadata:
                errors.append("%s: reference track is missing %s" % (widget_id, ", ".join(missing_metadata)))
            if entry["replacement"]:
                errors.append("%s: reference track must not declare replacement=%s" % (widget_id, entry["replacement"]))
        elif has_reference_metadata:
            errors.append("%s: only reference track may carry reference metadata" % widget_id)

        replacement = entry["replacement"]
        if not replacement:
            continue
        if replacement == widget_id:
            errors.append("%s: replacement must not point to itself" % widget_id)
            continue
        replacement_entry = catalog_map.get(replacement)
        if replacement_entry is None:
            errors.append("%s: replacement target does not exist: %s" % (widget_id, replacement))
            continue
        if replacement_entry["track"] != "reference":
            errors.append(
                "%s: replacement target must be a reference widget (got %s -> %s)"
                % (widget_id, replacement, replacement_entry["track"])
            )
        if replacement_entry["replacement"]:
            errors.append(
                "%s: replacement target must be terminal and cannot point again (got %s -> %s)"
                % (widget_id, replacement, replacement_entry["replacement"])
            )

    if errors:
        print("Widget catalog check FAILED (%d issues)" % len(errors))
        for item in errors:
            print("  - %s" % item)
        return 1

    counts = get_catalog_track_counts()
    print(
        "Widget catalog check passed (%d widgets: reference=%d, showcase=%d, deprecated=%d)"
        % (
            len(catalog_map),
            counts["reference"],
            counts["showcase"],
            counts["deprecated"],
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
