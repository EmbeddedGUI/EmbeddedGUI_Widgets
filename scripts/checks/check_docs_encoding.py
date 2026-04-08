#!/usr/bin/env python3
"""Validate documentation files can be decoded as UTF-8 and look sane."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent.parent
DOC_SUFFIXES = {".md", ".rst", ".txt", ".html", ".css", ".js", ".json"}
SKIP_DIR_NAMES = {
    ".git",
    ".venv",
    "__pycache__",
    "build",
    "build_cmake",
    "output",
    "runtime_check_output",
    "iteration_log",
}
SKIP_ROOTS = {
    ROOT_DIR / "sdk",
    ROOT_DIR / "tools",
    ROOT_DIR / "web" / "demos",
}
README_PATTERN = re.compile(r"\?{4,}")
MOJIBAKE_TOKENS = ("锟斤拷",)


def should_skip_dir(path: Path) -> bool:
    if path.name in SKIP_DIR_NAMES:
        return True
    return any(path == skip_root or skip_root in path.parents for skip_root in SKIP_ROOTS)


def iter_doc_files(paths: list[Path]) -> list[Path]:
    result = []
    for base in paths:
        if not base.exists():
            continue
        if base.is_file():
            if base.suffix.lower() in DOC_SUFFIXES:
                result.append(base)
            continue

        for path in base.rglob("*"):
            if path.is_dir():
                continue
            if path.suffix.lower() not in DOC_SUFFIXES:
                continue
            if any(parent.name in SKIP_DIR_NAMES for parent in path.parents):
                continue
            if any(path == skip_root or skip_root in path.parents for skip_root in SKIP_ROOTS):
                continue
            result.append(path)
    return sorted(set(result))


def is_readme(path: Path) -> bool:
    name = path.name.lower()
    return name == "readme.md" or name == "readme.rst" or name == "readme.txt"


def check_readme_quality(path: Path, content: str) -> str | None:
    suspicious_blocks = README_PATTERN.findall(content)
    question_count = content.count("?")

    if len(suspicious_blocks) >= 2:
        return "contains repeated '????' style corruption"
    if suspicious_blocks and question_count >= 20:
        return "contains too many '?' markers for a README"
    return None


def find_private_use_character(content: str) -> str | None:
    for char in content:
        codepoint = ord(char)
        if 0xE000 <= codepoint <= 0xF8FF:
            return "U+%04X" % codepoint
    return None


def check_general_quality(content: str) -> str | None:
    if "\ufffd" in content:
        return "contains replacement characters"

    private_use = find_private_use_character(content)
    if private_use:
        return "contains private-use character %s" % private_use

    for token in MOJIBAKE_TOKENS:
        if token in content:
            return "contains mojibake token %r" % token

    return None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check documentation files for UTF-8 decoding and obvious corruption.")
    parser.add_argument("paths", nargs="*", help="Optional files or directories to scan. Defaults to repository root.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    scan_paths = [Path(path).resolve() for path in args.paths] if args.paths else [ROOT_DIR]
    files = iter_doc_files(scan_paths)

    decode_failures: list[str] = []
    quality_failures: list[str] = []

    for path in files:
        relative = path.relative_to(ROOT_DIR).as_posix() if path.is_relative_to(ROOT_DIR) else str(path)
        try:
            content = path.read_text(encoding="utf-8")
        except UnicodeDecodeError as exc:
            decode_failures.append("%s: %s" % (relative, exc))
            continue

        general_issue = check_general_quality(content)
        if general_issue:
            quality_failures.append("%s: %s" % (relative, general_issue))

        if is_readme(path):
            quality_issue = check_readme_quality(path, content)
            if quality_issue:
                quality_failures.append("%s: %s" % (relative, quality_issue))

    if decode_failures:
        print("UTF-8 decode failures:")
        for item in decode_failures:
            print("  - %s" % item)

    if quality_failures:
        print("Suspicious README content:")
        for item in quality_failures:
            print("  - %s" % item)

    if decode_failures or quality_failures:
        total = len(decode_failures) + len(quality_failures)
        print("Documentation encoding check FAILED (%d issues)" % total)
        return 1

    print("Documentation encoding check passed (%d files)" % len(files))
    return 0


if __name__ == "__main__":
    sys.exit(main())
