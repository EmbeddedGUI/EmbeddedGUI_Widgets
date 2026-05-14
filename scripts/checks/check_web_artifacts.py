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
DEFAULT_WEB_ROOT = ROOT_DIR / "web"
DEFAULT_RENDER_GALLERY_DIR = DEFAULT_WEB_ROOT / "render-gallery"
APP_NAME = "HelloCustomWidgets"
APP_SOURCE_DIR = ROOT_DIR / "example" / APP_NAME

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
    parser.add_argument(
        "--web-root",
        default=str(DEFAULT_WEB_ROOT),
        help="Web root that contains demos/ and render-gallery/ (default: web).",
    )
    parser.add_argument(
        "--manifest",
        default="",
        help="Path to demos.json. Defaults to <web-root>/demos/demos.json.",
    )
    parser.add_argument(
        "--render-gallery",
        default="",
        help="Path to render-gallery directory. Defaults to <web-root>/render-gallery.",
    )
    parser.add_argument(
        "--category",
        default="",
        help="Only validate one HelloCustomWidgets category, for release_check.py category runs.",
    )
    return parser.parse_args()


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        raise ValueError("missing file: %s" % project_relative(path)) from None
    except json.JSONDecodeError as exc:
        raise ValueError("%s: invalid JSON: %s" % (project_relative(path), exc)) from None


def load_text(path: Path, errors: list[str], context: str) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        errors.append("%s missing file: %s" % (context, project_relative(path)))
    except UnicodeDecodeError as exc:
        errors.append("%s is not valid UTF-8: %s" % (project_relative(path), exc))
    return ""


def project_relative(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT_DIR).as_posix()
    except ValueError:
        return str(path.resolve())


def demo_name_for_widget(widget_id: str) -> str:
    return APP_NAME + "_" + widget_id.replace("/", "_")


def expected_published_widgets(catalog_map: dict[str, dict], category: str = "") -> dict[str, dict]:
    return {
        widget_id: entry
        for widget_id, entry in catalog_map.items()
        if entry["track"] == "reference" and entry["visibility"] == "public"
        and (not category or widget_id.split("/", 1)[0] == category)
    }


def is_selected_widget(widget_id: str, category: str) -> bool:
    return not category or widget_id.split("/", 1)[0] == category


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


def require_text_fragment(errors: list[str], text: str, fragment: str, path: Path, context: str) -> None:
    if fragment not in text:
        errors.append("%s missing %s in %s" % (context, fragment, project_relative(path)))


def validate_bundled_readme(errors: list[str], widget_id: str, bundled_path: Path, context: str) -> None:
    source_path = APP_SOURCE_DIR / widget_id / "readme.md"
    if not source_path.is_file():
        errors.append("%s missing source README: %s" % (context, project_relative(source_path)))
        return
    if not bundled_path.is_file():
        return
    if bundled_path.read_bytes() != source_path.read_bytes():
        errors.append(
            "%s README is stale; refresh from %s"
            % (context, project_relative(source_path))
        )


def validate_demo_manifest(
    expected: dict[str, dict],
    errors: list[str],
    demos_dir: Path,
    manifest_path: Path,
    category: str,
) -> dict[str, dict]:
    data = load_json(manifest_path)
    if not isinstance(data, list):
        errors.append("%s must be a JSON array" % project_relative(manifest_path))
        return {}

    demos_by_widget: dict[str, dict] = {}
    names: set[str] = set()
    duplicate_names: set[str] = set()
    duplicate_widgets: set[str] = set()

    for index, item in enumerate(data):
        context = "%s[%d]" % (project_relative(manifest_path), index)
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
        if not is_selected_widget(widget_id, category):
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
            demo_dir = demos_dir / name
            require_file(errors, demo_dir / f"{APP_NAME}.html", name)
            require_file(errors, demo_dir / f"{APP_NAME}.js", name)
            require_file(errors, demo_dir / f"{APP_NAME}.wasm", name)
            readme_path = demo_dir / "README.md"
            require_file(errors, readme_path, name)
            validate_bundled_readme(errors, widget_id, readme_path, name)
            expected_doc = "demos/%s/README.md" % name
            if item.get("doc") != expected_doc:
                errors.append("%s doc must be %s (got %s)" % (context, expected_doc, item.get("doc")))

    for name in sorted(duplicate_names):
        errors.append("%s contains duplicate name: %s" % (project_relative(manifest_path), name))
    for widget_id in sorted(duplicate_widgets):
        errors.append("%s contains duplicate widgetId: %s" % (project_relative(manifest_path), widget_id))

    add_set_diff_errors(
        errors,
        project_relative(manifest_path),
        set(expected),
        set(demos_by_widget),
        "published widget",
        "widget",
    )

    expected_demo_dirs = {demo_name_for_widget(widget_id) for widget_id in expected}
    actual_demo_dirs = {
        path.name
        for path in demos_dir.iterdir()
        if path.is_dir()
        and path.name.startswith(APP_NAME + "_")
        and (not category or path.name.startswith(APP_NAME + "_" + category + "_"))
    } if demos_dir.is_dir() else set()
    add_set_diff_errors(
        errors,
        project_relative(demos_dir),
        expected_demo_dirs,
        actual_demo_dirs,
        "demo directory",
        "demo directory",
    )

    return demos_by_widget


def validate_render_gallery(
    expected: dict[str, dict],
    demos_by_widget: dict[str, dict],
    errors: list[str],
    render_gallery_dir: Path,
    category_filter: str,
) -> None:
    gallery_index_path = render_gallery_dir / "widget-render-gallery.json"
    data = load_json(gallery_index_path)
    if not isinstance(data, dict):
        errors.append("%s must be a JSON object" % project_relative(gallery_index_path))
        return

    entries = data.get("entries")
    if not isinstance(entries, list):
        errors.append("%s entries must be a JSON array" % project_relative(gallery_index_path))
        return

    if data.get("total") != len(entries):
        errors.append(
            "%s total must equal entries count %d (got %r)"
            % (project_relative(gallery_index_path), len(entries), data.get("total"))
        )

    sheet_value = str(data.get("sheet", "") or "")
    if not sheet_value:
        errors.append("%s is missing sheet" % project_relative(gallery_index_path))
    else:
        require_file(errors, render_gallery_dir / sheet_value, "render gallery")
    require_file(errors, render_gallery_dir / "index.html", "render gallery")
    require_file(errors, render_gallery_dir / "README.md", "render gallery")
    html_path = render_gallery_dir / "index.html"
    markdown_path = render_gallery_dir / "README.md"
    html_text = load_text(html_path, errors, "render gallery") if html_path.is_file() else ""
    markdown_text = load_text(markdown_path, errors, "render gallery") if markdown_path.is_file() else ""

    entries_by_widget: dict[str, dict] = {}
    duplicate_widgets: set[str] = set()
    skipped_entry_count = 0

    for index, item in enumerate(entries):
        context = "%s entries[%d]" % (project_relative(gallery_index_path), index)
        if not isinstance(item, dict):
            errors.append("%s must be a JSON object" % context)
            continue

        widget_id = str(item.get("widgetId", "") or "").replace("\\", "/")
        if not widget_id:
            errors.append("%s is missing widgetId" % context)
            continue
        if not is_selected_widget(widget_id, category_filter):
            skipped_entry_count += 1
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

        require_file(errors, render_gallery_dir / expected_thumbnail, expected_name)
        require_text_fragment(errors, html_text, expected_demo, html_path, expected_name)
        require_text_fragment(errors, html_text, expected_thumbnail, html_path, expected_name)
        require_text_fragment(errors, html_text, widget_id, html_path, expected_name)
        require_text_fragment(errors, markdown_text, expected_demo, markdown_path, expected_name)
        require_text_fragment(errors, markdown_text, widget_id, markdown_path, expected_name)

    for widget_id in sorted(duplicate_widgets):
        errors.append("%s contains duplicate widgetId: %s" % (project_relative(gallery_index_path), widget_id))

    if category_filter:
        selected_total = len(entries) - skipped_entry_count
        if selected_total != len(entries_by_widget):
            errors.append(
                "%s selected category %s has %d entries but %d unique widgetIds"
                % (project_relative(gallery_index_path), category_filter, selected_total, len(entries_by_widget))
            )

    add_set_diff_errors(
        errors,
        project_relative(gallery_index_path),
        set(expected),
        set(entries_by_widget),
        "published widget",
        "widget",
    )

    entry_category_counts = Counter(widget_id.split("/", 1)[0] for widget_id in entries_by_widget)
    category_summary = data.get("categories")
    if not isinstance(category_summary, list):
        errors.append("%s categories must be a JSON array" % project_relative(gallery_index_path))
    else:
        summary_counts: dict[str, int] = {}
        for index, item in enumerate(category_summary):
            context = "%s categories[%d]" % (project_relative(gallery_index_path), index)
            if not isinstance(item, dict):
                errors.append("%s must be a JSON object" % context)
                continue
            category_id = str(item.get("id", "") or "")
            total = item.get("total")
            if not category_id:
                errors.append("%s is missing id" % context)
                continue
            if category_filter and category_id != category_filter:
                continue
            if not isinstance(total, int):
                errors.append("%s total must be an integer" % context)
                continue
            summary_counts[category_id] = total
        if summary_counts != dict(entry_category_counts):
            errors.append(
                "%s category totals must match gallery entries (got %r, expected %r)"
                % (project_relative(gallery_index_path), summary_counts, dict(entry_category_counts))
            )

    expected_thumbs = {"%s.png" % demo_name_for_widget(widget_id) for widget_id in expected}
    actual_thumbs = {
        path.name
        for path in (render_gallery_dir / "thumbs").glob("*.png")
        if path.is_file()
        and (not category_filter or path.name.startswith(APP_NAME + "_" + category_filter + "_"))
    }
    add_set_diff_errors(
        errors,
        project_relative(render_gallery_dir / "thumbs"),
        expected_thumbs,
        actual_thumbs,
        "thumbnail",
        "thumbnail",
    )


def main() -> int:
    args = parse_args()
    errors: list[str] = []
    web_root = Path(args.web_root).resolve()
    demos_dir = Path(args.manifest).resolve().parent if args.manifest else web_root / "demos"
    manifest_path = Path(args.manifest).resolve() if args.manifest else demos_dir / "demos.json"
    render_gallery_dir = Path(args.render_gallery).resolve() if args.render_gallery else web_root / "render-gallery"
    default_checked_in_scope = not args.manifest and not args.render_gallery and web_root == DEFAULT_WEB_ROOT.resolve()

    if default_checked_in_scope and not manifest_path.exists() and not render_gallery_dir.exists():
        print(
            "Web artifact check skipped: checked-in web artifacts are not present "
            "(web/demos and web/render-gallery are generated locally)."
        )
        return 0

    try:
        catalog_map = build_widget_catalog_map()
        expected = expected_published_widgets(catalog_map, args.category)
        if args.category and not expected:
            errors.append("no published widgets found for category: %s" % args.category)
        demos_by_widget = validate_demo_manifest(expected, errors, demos_dir, manifest_path, args.category)
        validate_render_gallery(expected, demos_by_widget, errors, render_gallery_dir, args.category)
    except ValueError as exc:
        errors.append(str(exc))

    if errors:
        print("Web artifact check FAILED (%d issues)" % len(errors))
        for item in errors:
            print("  - %s" % item)
        return 1

    category_counts = Counter(widget_id.split("/", 1)[0] for widget_id in expected)
    category_text = ", ".join("%s=%d" % (category, category_counts[category]) for category in sorted(category_counts))
    scope_text = " for category %s" % args.category if args.category else ""
    print(
        "Web artifact check passed%s (%d published widgets; %s)"
        % (scope_text, len(expected), category_text)
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
