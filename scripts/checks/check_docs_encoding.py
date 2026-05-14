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
    ROOT_DIR / "web" / "lib",
    ROOT_DIR / "web" / "demos",
}
README_PATTERN = re.compile(r"\?{4,}")
MOJIBAKE_TOKENS = (
    "��",
    "鏈",
    "褰撳",
    "鍙傝",
    "鏂囨。",
    "鐩綍",
    "杩愯",
    "璇诲彇",
    "鎺т欢",
)
LEGACY_REFERENCE_PATTERNS = (
    (
        re.compile(r"\bpython\s+scripts[\\/]+tools[\\/]"),
        "uses root scripts/tools; use sdk/EmbeddedGUI/scripts/tools or Make resource targets",
    ),
    (
        re.compile(r"\bcheck_example_icon_font\.py\b"),
        "references the removed example icon font check",
    ),
    (
        re.compile(r"\bcode_format\.py\b"),
        "references the removed code format helper",
    ),
    (
        re.compile(r"\b(?:perf_doc|ui_package)\b"),
        "references a release_check step that is not available in this repository",
    ),
    (
        re.compile(r"\b(?:Keil|UI Designer)\b"),
        "references SDK release tooling that is not part of this repository",
    ),
    (
        re.compile(r"\bbasic\.html\b"),
        "references the old HelloBasic web page",
    ),
    (
        re.compile(r"\b(?:HelloBasic|HelloSimple|HelloStyleDemo|HelloShowcase|MyNewApp)\b"),
        "references an app family outside the active widgets repository workflow",
    ),
    (
        re.compile(r"\bsrc[\\/]build\.mk\b"),
        "references the old SDK-root build layout",
    ),
    (
        re.compile(r"(?<!sdk/EmbeddedGUI/)porting[\\/]pc[\\/]Makefile\.base\b"),
        "references an SDK path without the sdk/EmbeddedGUI prefix",
    ),
)


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


def path_is_under(path: Path, base: Path) -> bool:
    try:
        path.relative_to(base)
        return True
    except ValueError:
        return False


def path_matches_scan(path: Path, scan_paths: list[Path]) -> bool:
    for base in scan_paths:
        if base.is_file():
            if path == base:
                return True
            continue
        if path_is_under(path, base):
            return True
    return False


def iter_active_collaboration_docs(scan_paths: list[Path]) -> list[Path]:
    """Docs that guide new work; historical progress logs are intentionally skipped."""
    docs = [
        ROOT_DIR / "AGENTS.md",
        ROOT_DIR / "CLAUDE.md",
        ROOT_DIR / "README.md",
        ROOT_DIR / "scripts" / "README.md",
        ROOT_DIR / ".claude" / "workflow" / "widget_acceptance_workflow.md",
    ]
    skills_dir = ROOT_DIR / ".claude" / "skills"
    if skills_dir.exists():
        docs.extend(skills_dir.glob("*.md"))
    return sorted({path for path in docs if path.exists() and path_matches_scan(path, scan_paths)})


def is_readme(path: Path) -> bool:
    name = path.name.lower()
    return name == "readme.md" or name == "readme.rst" or name == "readme.txt"


def check_readme_quality(content: str) -> str | None:
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


def check_legacy_references(scan_paths: list[Path]) -> list[str]:
    failures: list[str] = []
    for path in iter_active_collaboration_docs(scan_paths):
        relative = path.relative_to(ROOT_DIR).as_posix()
        try:
            content = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue

        for pattern, message in LEGACY_REFERENCE_PATTERNS:
            match = pattern.search(content)
            if not match:
                continue
            line = content.count("\n", 0, match.start()) + 1
            failures.append("%s:%d: %s (%r)" % (relative, line, message, match.group(0)))
    return failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check documentation UTF-8 quality and stale active-doc references.")
    parser.add_argument("paths", nargs="*", help="Optional files or directories to scan. Defaults to repository root.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    scan_paths = [Path(path).resolve() for path in args.paths] if args.paths else [ROOT_DIR]
    files = iter_doc_files(scan_paths)

    decode_failures: list[str] = []
    quality_failures: list[str] = []
    legacy_failures = check_legacy_references(scan_paths)

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
            quality_issue = check_readme_quality(content)
            if quality_issue:
                quality_failures.append("%s: %s" % (relative, quality_issue))

    if decode_failures:
        print("UTF-8 decode failures:")
        for item in decode_failures:
            print("  - %s" % item)

    if quality_failures:
        print("Suspicious documentation content:")
        for item in quality_failures:
            print("  - %s" % item)

    if legacy_failures:
        print("Stale active documentation references:")
        for item in legacy_failures:
            print("  - %s" % item)

    if decode_failures or quality_failures or legacy_failures:
        total = len(decode_failures) + len(quality_failures) + len(legacy_failures)
        print("Documentation quality check FAILED (%d issues)" % total)
        return 1

    print("Documentation quality check passed (%d files)" % len(files))
    return 0


if __name__ == "__main__":
    sys.exit(main())
