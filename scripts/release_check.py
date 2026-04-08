#!/usr/bin/env python3
"""Release readiness check for EmbeddedGUI Widgets."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys
import time


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent

ALL_STEP_NAMES = ["catalog", "touch", "compile", "unit_test", "runtime", "wasm"]
STEP_DESCRIPTIONS = {
    "catalog": "Widget catalog consistency check",
    "touch": "Custom widget touch release semantics check",
    "compile": "HelloCustomWidgets compile sweep",
    "unit_test": "HelloUnitTest build and run",
    "runtime": "HelloCustomWidgets runtime verification",
    "wasm": "WASM demo build verification",
}

BANNER_WIDTH = 72
STATUS_PASS = "PASS"
STATUS_FAIL = "FAIL"
STATUS_SKIP = "SKIP"
DEFAULT_RUNTIME_JOBS = 2
DEFAULT_WASM_OUTPUT_DIR = PROJECT_ROOT / "output" / "release_check_wasm" / "demos"


def banner(text: str) -> None:
    print("\n" + "=" * BANNER_WIDTH, flush=True)
    print(f"  {text}", flush=True)
    print("=" * BANNER_WIDTH, flush=True)


def format_duration(seconds: float) -> str:
    if seconds < 60:
        return f"{seconds:.1f}s"
    minutes = int(seconds) // 60
    secs = seconds - minutes * 60
    if minutes < 60:
        return f"{minutes}m {secs:.0f}s"
    hours = minutes // 60
    minutes %= 60
    return f"{hours}h {minutes}m {secs:.0f}s"


def format_command(cmd: list[str]) -> str:
    return " ".join(f'"{part}"' if " " in part else part for part in cmd)


def find_emsdk_path() -> str | None:
    for candidate in (
        PROJECT_ROOT / "tools" / "emsdk",
        PROJECT_ROOT / "sdk" / "EmbeddedGUI" / "tools" / "emsdk",
        PROJECT_ROOT.parent / "EmbeddedGUI" / "tools" / "emsdk",
    ):
        if candidate.exists():
            return str(candidate)
    for key in ("EMSDK_PATH", "EMSDK"):
        value = os.environ.get(key)
        if value and Path(value).exists():
            return value
    return None


def get_custom_widgets_list(category: str | None = None) -> list[str]:
    base = PROJECT_ROOT / "example" / "HelloCustomWidgets"
    widget_list: list[str] = []
    if not base.exists():
        return widget_list

    for cat_dir in sorted(base.iterdir()):
        if not cat_dir.is_dir():
            continue
        if category and cat_dir.name != category:
            continue
        for widget_dir in sorted(cat_dir.iterdir()):
            if widget_dir.is_dir() and (widget_dir / "test.c").exists():
                widget_list.append(f"{cat_dir.name}/{widget_dir.name}")
    return widget_list


def build_steps(args: argparse.Namespace) -> list[tuple[str, str, list[list[str]]]]:
    py = sys.executable
    category_args = ["--category", args.category] if args.category else []
    bits64_args = ["--bits64"] if args.bits64 else []
    compile_case_args = ["--case-jobs", str(args.compile_case_jobs)] if args.compile_case_jobs > 0 else []
    runtime_job_args = ["--jobs", str(args.runtime_jobs)] if args.runtime_jobs > 0 else []

    catalog_cmd = [py, str(SCRIPT_DIR / "checks" / "check_widget_catalog.py")]
    touch_cmd = [py, str(SCRIPT_DIR / "checks" / "check_touch_release_semantics.py"), "--scope", "custom"] + category_args
    compile_cmd = [py, str(SCRIPT_DIR / "code_compile_check.py"), "--custom-widgets"] + category_args + bits64_args + compile_case_args
    unit_test_cmd = [py, str(SCRIPT_DIR / "code_compile_check.py"), "--unit-tests-only"] + bits64_args
    runtime_cmd = [py, str(SCRIPT_DIR / "code_runtime_check.py"), "--app", "HelloCustomWidgets"] + category_args + bits64_args + runtime_job_args

    wasm_common_args = [py, str(SCRIPT_DIR / "web" / "wasm_build_demos.py"), "--output-dir", str(DEFAULT_WASM_OUTPUT_DIR)]
    emsdk_path = find_emsdk_path()
    if emsdk_path:
        wasm_common_args += ["--emsdk-path", emsdk_path]
    if args.wasm_jobs > 0:
        wasm_common_args += ["--jobs", str(args.wasm_jobs)]
    if args.wasm_make_jobs > 0:
        wasm_common_args += ["--make-jobs", str(args.wasm_make_jobs)]

    if args.category:
        widget_list = get_custom_widgets_list(args.category)
        if not widget_list:
            raise ValueError(f"no HelloCustomWidgets demos found for category={args.category}")
        wasm_commands = []
        for index, widget_sub in enumerate(widget_list):
            cmd = list(wasm_common_args)
            cmd += ["--app-sub", widget_sub]
            if index == 0:
                cmd.append("--clean")
            wasm_commands.append(cmd)
    else:
        wasm_commands = [wasm_common_args + ["--clean"]]

    return [
        ("catalog", STEP_DESCRIPTIONS["catalog"], [catalog_cmd]),
        ("touch", STEP_DESCRIPTIONS["touch"], [touch_cmd]),
        ("compile", STEP_DESCRIPTIONS["compile"], [compile_cmd]),
        ("unit_test", STEP_DESCRIPTIONS["unit_test"], [unit_test_cmd]),
        ("runtime", STEP_DESCRIPTIONS["runtime"], [runtime_cmd]),
        ("wasm", STEP_DESCRIPTIONS["wasm"], wasm_commands),
    ]


def run_step_commands(step_name: str, commands: list[list[str]], env: dict[str, str]) -> int:
    total = len(commands)
    for index, cmd in enumerate(commands, start=1):
        prefix = f"  Command [{index}/{total}]" if total > 1 else "  Command"
        print(f"{prefix}: {format_command(cmd)}", flush=True)
        result = subprocess.run(cmd, cwd=PROJECT_ROOT, env=env)
        if result.returncode != 0:
            print(f"  {step_name} failed at command {index}/{total}.", flush=True)
            return result.returncode
    return 0


def print_summary(results: list[tuple[str, str, str, float]], total_elapsed: float) -> bool:
    print("\n" + "=" * BANNER_WIDTH, flush=True)
    print("  RELEASE CHECK SUMMARY", flush=True)
    print("=" * BANNER_WIDTH, flush=True)

    desc_width = max(len(desc) for _, desc, _, _ in results) + 2
    for _, desc, status, elapsed in results:
        if status == STATUS_PASS:
            mark = "[PASS]"
        elif status == STATUS_FAIL:
            mark = "[FAIL]"
        else:
            mark = "[SKIP]"
        time_str = format_duration(elapsed) if elapsed > 0 else "-"
        print(f"  {mark}  {desc:<{desc_width}}  {time_str:>10}", flush=True)

    print("-" * BANNER_WIDTH, flush=True)
    print(f"  Total time: {format_duration(total_elapsed)}", flush=True)

    passed = sum(1 for _, _, status, _ in results if status == STATUS_PASS)
    failed = sum(1 for _, _, status, _ in results if status == STATUS_FAIL)
    skipped = sum(1 for _, _, status, _ in results if status == STATUS_SKIP)
    print(f"  {passed} passed, {failed} failed, {skipped} skipped", flush=True)

    if failed > 0:
        print("\n  ** RELEASE CHECK FAILED **", flush=True)
    else:
        print("\n  ** ALL CHECKS PASSED **", flush=True)
    print("=" * BANNER_WIDTH, flush=True)
    return failed == 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="EmbeddedGUI Widgets release readiness check.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            f"Available steps: {', '.join(ALL_STEP_NAMES)}\n"
            "\nExamples:\n"
            "  python scripts/release_check.py\n"
            "  python scripts/release_check.py --skip wasm\n"
            "  python scripts/release_check.py --category input --skip wasm\n"
            "  python scripts/release_check.py --keep-going --skip runtime\n"
        ),
    )
    parser.add_argument("--skip", type=str, default="", help=f"Comma-separated list of steps to skip. Available: {', '.join(ALL_STEP_NAMES)}")
    parser.add_argument("--keep-going", action="store_true", default=False, help="Continue executing subsequent steps after a failure.")
    parser.add_argument("--category", type=str, default=None, help="Only run one HelloCustomWidgets category.")
    parser.add_argument("--bits64", action="store_true", default=True, help="Use 64-bit build (default: enabled).")
    parser.add_argument("--no-bits64", action="store_true", default=False, help="Disable 64-bit build flag.")
    parser.add_argument("--compile-case-jobs", type=int, default=0, help="Forward to scripts/code_compile_check.py --case-jobs.")
    parser.add_argument("--runtime-jobs", type=int, default=DEFAULT_RUNTIME_JOBS, help=f"Forward to scripts/code_runtime_check.py --jobs (default: {DEFAULT_RUNTIME_JOBS}).")
    parser.add_argument("--wasm-jobs", type=int, default=0, help="Forward to scripts/web/wasm_build_demos.py --jobs.")
    parser.add_argument("--wasm-make-jobs", type=int, default=0, help="Forward to scripts/web/wasm_build_demos.py --make-jobs.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.no_bits64:
        args.bits64 = False

    skip_set: set[str] = set()
    if args.skip:
        for name in args.skip.split(","):
            normalized = name.strip()
            if normalized not in ALL_STEP_NAMES:
                print(f"Error: unknown step '{normalized}'. Available: {', '.join(ALL_STEP_NAMES)}")
                return 1
            skip_set.add(normalized)

    try:
        steps = build_steps(args)
    except ValueError as exc:
        print(f"Error: {exc}")
        return 1

    env = os.environ.copy()
    env.setdefault("PYTHONUNBUFFERED", "1")
    results: list[tuple[str, str, str, float]] = []
    had_failure = False
    total_start = time.time()

    for name, desc, commands in steps:
        if name in skip_set:
            results.append((name, desc, STATUS_SKIP, 0))
            continue
        if had_failure and not args.keep_going:
            results.append((name, desc, STATUS_SKIP, 0))
            continue

        banner(f"[{name}] {desc}")
        step_start = time.time()
        try:
            retcode = run_step_commands(name, commands, env)
        except FileNotFoundError as exc:
            print(f"  Error: {exc}", flush=True)
            retcode = 1
        elapsed = time.time() - step_start

        if retcode == 0:
            results.append((name, desc, STATUS_PASS, elapsed))
        else:
            results.append((name, desc, STATUS_FAIL, elapsed))
            had_failure = True

    success = print_summary(results, time.time() - total_start)
    return 0 if success else 1


if __name__ == "__main__":
    raise SystemExit(main())
