#!/usr/bin/env python3
"""Run the same checks that the widget CI expects for local iteration."""

import argparse
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent


def format_cmd(cmd):
    return " ".join('"%s"' % part if " " in part else part for part in cmd)


def run_step(title, cmd):
    print("=" * 80)
    print(title)
    print("=" * 80)
    print(format_cmd(cmd))
    subprocess.run(cmd, cwd=ROOT_DIR, check=True)
    print("")


def add_category_arg(cmd, category):
    if category and category.lower() != "all":
        cmd.extend(["--category", category])


def main():
    parser = argparse.ArgumentParser(
        description="Run the local widget CI flow: catalog audit, touch audit, compile, runtime, and HelloUnitTest.",
    )
    parser.add_argument(
        "--category",
        type=str,
        default=None,
        help="Only run one HelloCustomWidgets category, for example input.",
    )
    parser.add_argument(
        "--bits32",
        action="store_false",
        dest="bits64",
        help="Use 32-bit build settings instead of the CI default 64-bit.",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=None,
        help="Override runtime check timeout in seconds.",
    )
    parser.add_argument(
        "--compile-case-jobs",
        type=int,
        default=0,
        help="Forwarded to scripts/code_compile_check.py --case-jobs.",
    )
    parser.add_argument(
        "--runtime-jobs",
        type=int,
        default=0,
        help="Forwarded to scripts/code_runtime_check.py --jobs.",
    )
    parser.add_argument(
        "--skip-unit-tests",
        action="store_true",
        help="Skip the HelloUnitTest step.",
    )
    parser.set_defaults(bits64=True)
    args = parser.parse_args()

    python = sys.executable

    catalog_cmd = [python, "scripts/checks/check_widget_catalog.py"]
    run_step("Widget Catalog Policy", catalog_cmd)

    touch_cmd = [python, "scripts/checks/check_touch_release_semantics.py", "--scope", "custom"]
    add_category_arg(touch_cmd, args.category)
    run_step("Touch Release Semantics", touch_cmd)

    compile_cmd = [python, "scripts/code_compile_check.py", "--custom-widgets"]
    add_category_arg(compile_cmd, args.category)
    if args.bits64:
        compile_cmd.append("--bits64")
    if args.compile_case_jobs > 0:
        compile_cmd.extend(["--case-jobs", str(args.compile_case_jobs)])
    run_step("Compile Check", compile_cmd)

    runtime_cmd = [python, "scripts/code_runtime_check.py", "--app", "HelloCustomWidgets"]
    add_category_arg(runtime_cmd, args.category)
    if args.bits64:
        runtime_cmd.append("--bits64")
    if args.timeout is not None:
        runtime_cmd.extend(["--timeout", str(args.timeout)])
    if args.runtime_jobs > 0:
        runtime_cmd.extend(["--jobs", str(args.runtime_jobs)])
    run_step("Runtime Check", runtime_cmd)

    if not args.skip_unit_tests:
        unit_test_cmd = [python, "scripts/code_compile_check.py", "--unit-tests-only"]
        if args.bits64:
            unit_test_cmd.append("--bits64")
        run_step("HelloUnitTest", unit_test_cmd)

    print("Local CI checks completed successfully.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
