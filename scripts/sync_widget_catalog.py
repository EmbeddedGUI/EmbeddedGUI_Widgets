#!/usr/bin/env python3
"""Rewrite HelloCustomWidgets widget_catalog.json into canonical synchronized form."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent

from widget_catalog import CATALOG_PATH, build_synchronized_catalog_entries, format_catalog_entries, write_widget_catalog


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Sync widget_catalog.json with actual HelloCustomWidgets directories and rewrite it in canonical order."
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Only check whether widget_catalog.json is already synchronized and canonical.",
    )
    parser.add_argument(
        "--output",
        default=str(CATALOG_PATH),
        help="Output catalog path (default: example/HelloCustomWidgets/widget_catalog.json).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_path = Path(args.output).resolve()
    canonical_entries = build_synchronized_catalog_entries()
    canonical_text = format_catalog_entries(canonical_entries)

    current_text = ""
    if output_path.exists():
        current_text = output_path.read_text(encoding="utf-8")

    if args.check:
        if current_text == canonical_text:
            print("Widget catalog is already synchronized (%d entries)" % len(canonical_entries))
            return 0
        print("Widget catalog needs synchronization: %s" % output_path)
        return 1

    written_entries = write_widget_catalog(output_path)
    print("Wrote synchronized widget catalog: %s (%d entries)" % (output_path, len(written_entries)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
