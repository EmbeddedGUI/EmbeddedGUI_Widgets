#!/usr/bin/env python3
"""Validate checked-in web demo and render-gallery artifacts."""

from __future__ import annotations

import argparse
import json
import sys
from collections import Counter
from pathlib import Path
from typing import Any


SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_ROOT = SCRIPT_DIR.parent
ROOT_DIR = SCRIPTS_ROOT.parent
WEB_DIR = ROOT_DIR / "web"
DEMOS_DIR = WEB_DIR / "demos"
DEMOS_MANIFEST_PATH = DEMOS_DIR / "demos.json"
RENDER_GALLERY_DIR = WEB_DIR / "render-gallery"
RENDER_GALLERY_INDEX_PATH = RENDER_GALLERY_DIR / "widget-render-gallery.json"
APP_NAME = "HelloCustomWidgets"

if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))

from widget_catalog import build_widget_catalog_map


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check web/demos and web/render-gallery consistency with the widget catalog.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Entry note:\n"
            "  HelloCustomWidgets / HelloUnitTest use uicode_disp0.c / uicode_disp0.h\n"
            "  directly as the multi-display SDK entry.\n"
        ),
    )
    return parser.parse_args()


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        raise ValueError("missing file: %s" % project_relative(path)) from None
    except json.JSONDecodeError as exc:
        raise ValueError("%s: invalid JSON: %s" % (project_relative(path), exc)) from None


def project_relative(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT_DIR).as_posix()
    except ValueError:
        return str(path.resolve())


def demo_name_for_widget(widget_id: str) -> str:
    return APP_NAME + "_" + widget_id.replace("/", "_")


def expected_published_widgets(catalog_map: dict[str, dict]) -> dict[str, dict]:
    return {
        widget_id: entry
        for widget_id, entry in catalog_map.items()
        if entry["track"] == "reference" and entry["visibility"] == "public"
    }


def add_set_diff_errors(
    errors: list[str],
    label: str,
    expected: set[str],
    actual: set[str],
    missing_text: str,
    orphan_text: str,
) -> None:
    for item in sorted(expected - actual):
        errors.append("%s missing %s: %s" % (label, missing_text, item))
    for item in sorted(actual - expected):
        errors.append("%s has unexpected %s: %s" % (label, orphan_text, item))


def require_file(errors: list[str], path: Path, context: str) -> None:
    if not path.is_file():
        errors.append("%s missing file: %s" % (context, project_relative(path)))


def validate_demo_manifest(expected: dict[str, dict], errors: list[str]) -> dict[str, dict]:
    data = load_json(DEMOS_MANIFEST_PATH)
    if not isinstance(data, list):
        errors.append("%s must be a JSON array" % project_relative(DEMOS_MANIFEST_PATH))
        return {}

    demos_by_widget: dict[str, dict] = {}
    names: set[str] = set()
    duplicate_names: set[str] = set()
    duplicate_widgets: set[str] = set()

    for index, item in enumerate(data):
        context = "%s[%d]" % (project_relative(DEMOS_MANIFEST_PATH), index)
        if not isinstance(item, dict):
            errors.append("%s must be a JSON object" % context)
            continue

        name = str(item.get("name", "") or "")
        widget_id = str(item.get("widgetId", "") or "").replace("\\", "/")
        if not name:
            errors.append("%s is missing name" % context)
        elif name in names:
            duplicate_names.add(name)
        else:
            names.add(name)

        if not widget_id:
            errors.append("%s is missing widgetId" % context)
            continue
        if widget_id in demos_by_widget:
            duplicate_widgets.add(widget_id)
        else:
            demos_by_widget[widget_id] = item

        expected_name = demo_name_for_widget(widget_id)
        if name and name != expected_name:
            errors.append("%s name must be %s (got %s)" % (context, expected_name, name))
        if item.get("app") != APP_NAME:
            errors.append("%s app must be %s (got %s)" % (context, APP_NAME, item.get("app")))
        if str(item.get("appSub", "") or "").replace("\\", "/") != widget_id:
            errors.append("%s appSub must match widgetId %s" % (context, widget_id))
        if item.get("category") != APP_NAME:
            errors.append("%s category must be %s (got %s)" % (context, APP_NAME, item.get("category")))

        catalog_entry = expected.get(widget_id)
        if catalog_entry is not None:
            metadata_checks = [
                ("track", "track"),
                ("visibility", "visibility"),
                ("referenceSystem", "reference_system"),
                ("referenceLibrary", "reference_library"),
                ("referenceComponent", "reference_component"),
                ("replacement", "replacement"),
            ]
            for manifest_key, catalog_key in metadata_checks:
                if item.get(manifest_key) != catalog_entry[catalog_key]:
                    errors.append(
                        "%s %s must match catalog value %r (got %r)"
                        % (context, manifest_key, catalog_entry[catalog_key], item.get(manifest_key))
                    )

        width = item.get("width")
        height = item.get("height")
        if not isinstance(width, int) or width <= 0:
            errors.append("%s width must be a positive integer" % context)
        if not isinstance(height, int) or height <= 0:
            errors.append("%s height must be a positive integer" % context)

        if name:
            demo_dir = DEMOS_DIR / name
            require_file(errors, demo_dir / f"{APP_NAME}.html", name)
            require_file(errors, demo_dir / f"{APP_NAME}.js", name)
            require_file(errors, demo_dir / f"{APP_NAME}.wasm", name)
            require_file(errors, demo_dir / "README.md", name)
            expected_doc = "demos/%s/README.md" % name
            if item.get("doc") != expected_doc:
                errors.append("%s doc must be %s (got %s)" % (context, expected_doc, item.get("doc")))

    for name in sorted(duplicate_names):
        errors.append("%s contains duplicate name: %s" % (project_relative(DEMOS_MANIFEST_PATH), name))
    for widget_id in sorted(duplicate_widgets):
        errors.append("%s contains duplicate widgetId: %s" % (project_relative(DEMOS_MANIFEST_PATH), widget_id))

    add_set_diff_errors(
        errors,
        project_relative(DEMOS_MANIFEST_PATH),
        set(expected),
        set(demos_by_widget),
        "published widget",
        "widget",
    )

    expected_demo_dirs = {demo_name_for_widget(widget_id) for widget_id in expected}
    actual_demo_dirs = {
        path.name
        for path in DEMOS_DIR.iterdir()
        if path.is_dir() and path.name.startswith(APP_NAME + "_")
    }
    add_set_diff_errors(
        errors,
        project_relative(DEMOS_DIR),
        expected_demo_dirs,
        actual_demo_dirs,
        "demo directory",
        "demo directory",
    )

    return demos_by_widget


def validate_render_gallery(expected: dict[str, dict], demos_by_widget: dict[str, dict], errors: list[str]) -> None:
    data = load_json(RENDER_GALLERY_INDEX_PATH)
    if not isinstance(data, dict):
        errors.append("%s must be a JSON object" % project_relative(RENDER_GALLERY_INDEX_PATH))
        return

    entries = data.get("entries")
    if not isinstance(entries, list):
        errors.append("%s entries must be a JSON array" % project_relative(RENDER_GALLERY_INDEX_PATH))
        return

    if data.get("total") != len(entries):
        errors.append(
            "%s total must equal entries count %d (got %r)"
            % (project_relative(RENDER_GALLERY_INDEX_PATH), len(entries), data.get("total"))
        )

    sheet_value = str(data.get("sheet", "") or "")
    if not sheet_value:
        errors.append("%s is missing sheet" % project_relative(RENDER_GALLERY_INDEX_PATH))
    else:
        require_file(errors, RENDER_GALLERY_DIR / sheet_value, "render gallery")
    require_file(errors, RENDER_GALLERY_DIR / "index.html", "render gallery")
    require_file(errors, RENDER_GALLERY_DIR / "README.md", "render gallery")

    entries_by_widget: dict[str, dict] = {}
    duplicate_widgets: set[str] = set()

    for index, item in enumerate(entries):
        context = "%s entries[%d]" % (project_relative(RENDER_GALLERY_INDEX_PATH), index)
        if not isinstance(item, dict):
            errors.append("%s must be a JSON object" % context)
            continue

        widget_id = str(item.get("widgetId", "") or "").replace("\\", "/")
        if not widget_id:
            errors.append("%s is missing widgetId" % context)
            continue
        if widget_id in entries_by_widget:
            duplicate_widgets.add(widget_id)
        else:
            entries_by_widget[widget_id] = item

        catalog_entry = expected.get(widget_id)
        demo_entry = demos_by_widget.get(widget_id)
        category, _, widget = widget_id.partition("/")
        expected_name = demo_name_for_widget(widget_id)
        expected_thumbnail = "thumbs/%s.png" % expected_name
        expected_demo = "../demos/%s/%s.html" % (expected_name, APP_NAME)

        if item.get("name") != expected_name:
            errors.append("%s name must be %s (got %s)" % (context, expected_name, item.get("name")))
        if item.get("category") != category:
            errors.append("%s category must be %s (got %s)" % (context, category, item.get("category")))
        if item.get("widget") != widget:
            errors.append("%s widget must be %s (got %s)" % (context, widget, item.get("widget")))
        if item.get("thumbnail") != expected_thumbnail:
            errors.append("%s thumbnail must be %s (got %s)" % (context, expected_thumbnail, item.get("thumbnail")))
        if item.get("demo") != expected_demo:
            errors.append("%s demo must be %s (got %s)" % (context, expected_demo, item.get("demo")))
        if catalog_entry is not None and item.get("track") != catalog_entry["track"]:
            errors.append("%s track must match catalog value %s" % (context, catalog_entry["track"]))
        if demo_entry is not None and item.get("track") != demo_entry.get("track"):
            errors.append("%s track must match demos manifest value %s" % (context, demo_entry.get("track")))

        require_file(errors, RENDER_GALLERY_DIR / expected_thumbnail, expected_name)

    for widget_id in sorted(duplicate_widgets):
        errors.append("%s contains duplicate widgetId: %s" % (project_relative(RENDER_GALLERY_INDEX_PATH), widget_id))

    add_set_diff_errors(
        errors,
        project_relative(RENDER_GALLERY_INDEX_PATH),
        set(expected),
        set(entries_by_widget),
        "published widget",
        "widget",
    )

    entry_category_counts = Counter(widget_id.split("/", 1)[0] for widget_id in entries_by_widget)
    category_summary = data.get("categories")
    if not isinstance(category_summary, list):
        errors.append("%s categories must be a JSON array" % project_relative(RENDER_GALLERY_INDEX_PATH))
    else:
        summary_counts: dict[str, int] = {}
        for index, item in enumerate(category_summary):
            context = "%s categories[%d]" % (project_relative(RENDER_GALLERY_INDEX_PATH), index)
            if not isinstance(item, dict):
                errors.append("%s must be a JSON object" % context)
                continue
            category_id = str(item.get("id", "") or "")
            total = item.get("total")
            if not category_id:
                errors.append("%s is missing id" % context)
                continue
            if not isinstance(total, int):
                errors.append("%s total must be an integer" % context)
                continue
            summary_counts[category_id] = total
        if summary_counts != dict(entry_category_counts):
            errors.append(
                "%s category totals must match gallery entries (got %r, expected %r)"
                % (project_relative(RENDER_GALLERY_INDEX_PATH), summary_counts, dict(entry_category_counts))
            )

    expected_thumbs = {"%s.png" % demo_name_for_widget(widget_id) for widget_id in expected}
    actual_thumbs = {
        path.name
        for path in (RENDER_GALLERY_DIR / "thumbs").glob("*.png")
        if path.is_file()
    }
    add_set_diff_errors(
        errors,
        project_relative(RENDER_GALLERY_DIR / "thumbs"),
        expected_thumbs,
        actual_thumbs,
        "thumbnail",
        "thumbnail",
    )


def main() -> int:
    parse_args()
    errors: list[str] = []

    try:
        catalog_map = build_widget_catalog_map()
        expected = expected_published_widgets(catalog_map)
        demos_by_widget = validate_demo_manifest(expected, errors)
        validate_render_gallery(expected, demos_by_widget, errors)
    except ValueError as exc:
        errors.append(str(exc))

    if errors:
        print("Web artifact check FAILED (%d issues)" % len(errors))
        for item in errors:
            print("  - %s" % item)
        return 1

    category_counts = Counter(widget_id.split("/", 1)[0] for widget_id in expected)
    category_text = ", ".join("%s=%d" % (category, category_counts[category]) for category in sorted(category_counts))
    print(
        "Web artifact check passed (%d published widgets; %s)"
        % (len(expected), category_text)
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
