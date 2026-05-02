#!/usr/bin/env python3
"""Generate the doc-side HelloCustomWidgets render gallery."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


DOC_SCRIPT_DIR = Path(__file__).resolve().parent
DOC_ROOT = DOC_SCRIPT_DIR.parent
PROJECT_ROOT = DOC_ROOT.parent
WEB_GALLERY_SCRIPT = PROJECT_ROOT / "scripts" / "web" / "widget_render_gallery.py"
DEFAULT_DOC_OUTPUT_DIR = DOC_ROOT / "source" / "images"
DEFAULT_WORK_OUTPUT_DIR = PROJECT_ROOT / "output" / "doc_widget_render_gallery"


def has_option(args: list[str], option: str) -> bool:
    return any(arg == option or arg.startswith(option + "=") for arg in args)


def main() -> int:
    forwarded = sys.argv[1:]
    if not has_option(forwarded, "--doc-output-dir"):
        forwarded = ["--doc-output-dir", str(DEFAULT_DOC_OUTPUT_DIR), *forwarded]
    if not has_option(forwarded, "--output-dir"):
        forwarded = ["--output-dir", str(DEFAULT_WORK_OUTPUT_DIR), *forwarded]
    if not has_option(forwarded, "--skip-web-output"):
        forwarded = ["--skip-web-output", *forwarded]

    command = [sys.executable, str(WEB_GALLERY_SCRIPT), *forwarded]
    return subprocess.run(command, cwd=PROJECT_ROOT).returncode


if __name__ == "__main__":
    raise SystemExit(main())
