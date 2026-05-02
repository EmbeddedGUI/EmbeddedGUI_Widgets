"""Interactive launcher for EmbeddedGUI widget examples."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
EXAMPLE_DIR = ROOT_DIR / "example"
MAKE_JOBS = max(1, min(8, (os.cpu_count() or 1)))


@dataclass(frozen=True)
class AppCase:
    app: str
    app_sub: str | None = None

    @property
    def label(self) -> str:
        if self.app_sub:
            return f"{self.app}/{self.app_sub}"
        return self.app


def get_example_list() -> list[str]:
    if not EXAMPLE_DIR.exists():
        return []
    return sorted(path.name for path in EXAMPLE_DIR.iterdir() if path.is_dir() and (path / "build.mk").exists())


def get_custom_widget_sub_apps() -> list[str]:
    root = EXAMPLE_DIR / "HelloCustomWidgets"
    if not root.exists():
        return []

    sub_apps = ["showcase"] if (root / "showcase").is_dir() else []
    for category_dir in sorted(path for path in root.iterdir() if path.is_dir()):
        if category_dir.name.startswith(".") or category_dir.name.startswith("__"):
            continue
        if category_dir.name == "showcase":
            continue
        for widget_dir in sorted(path for path in category_dir.iterdir() if path.is_dir()):
            if (widget_dir / "test.c").exists():
                sub_apps.append(f"{category_dir.name}/{widget_dir.name}")
    return sub_apps


def get_example_sub_list(app: str) -> list[str]:
    if app == "HelloCustomWidgets":
        return get_custom_widget_sub_apps()
    return []


def build_app_index() -> tuple[list[str], dict[str, list[str]]]:
    apps = get_example_list()
    sub_apps = {app: get_example_sub_list(app) for app in apps}
    return apps, sub_apps


def build_run_command(case: AppCase, port: str, make_jobs: int, extra_make_args: list[str]) -> list[str]:
    cmd = ["make", "run", f"-j{make_jobs}", f"APP={case.app}", f"PORT={port}", *extra_make_args]
    if case.app_sub:
        cmd.append(f"APP_SUB={case.app_sub}")
    return cmd


def print_cases(apps: list[str], sub_apps: dict[str, list[str]]) -> None:
    for app in apps:
        app_sub_list = sub_apps.get(app, [])
        if not app_sub_list:
            print(app)
            continue
        print(f"{app}:")
        for app_sub in app_sub_list:
            print(f"  {app}/{app_sub}")


def print_numbered(title: str, items: list[str]) -> None:
    print()
    print(title)
    print("-" * len(title))
    for index, item in enumerate(items, start=1):
        print(f"{index:3d}. {item}")


def read_choice(prompt: str, max_value: int, allow_back: bool = False) -> int | str | None:
    while True:
        suffix = " [number, b=back, q=quit]: " if allow_back else " [number, q=quit]: "
        try:
            value = input(prompt + suffix).strip()
        except EOFError:
            print()
            return None

        if value.lower() == "q":
            return None
        if allow_back and value.lower() == "b":
            return "back"

        try:
            number = int(value)
        except ValueError:
            print("Invalid input.")
            continue

        if 1 <= number <= max_value:
            return number - 1
        print("Selection out of range.")


def choose_case(apps: list[str], sub_apps: dict[str, list[str]]) -> AppCase | None:
    while True:
        print_numbered("Applications", apps)
        app_index = read_choice("Select app", len(apps))
        if app_index is None:
            return None

        app = apps[app_index]
        app_sub_list = sub_apps.get(app, [])
        if not app_sub_list:
            return AppCase(app)

        while True:
            print_numbered(f"{app} sub applications", app_sub_list)
            sub_index = read_choice("Select sub app", len(app_sub_list), allow_back=True)
            if sub_index is None:
                return None
            if sub_index == "back":
                break
            return AppCase(app, app_sub_list[sub_index])


def validate_case(app: str, app_sub: str | None, apps: list[str], sub_apps: dict[str, list[str]]) -> AppCase | None:
    if app not in apps:
        print(f"Unknown app: {app}", file=sys.stderr)
        print("Available apps:", file=sys.stderr)
        for item in apps:
            print(f"  {item}", file=sys.stderr)
        return None

    app_sub_list = sub_apps.get(app, [])
    if app_sub_list:
        if not app_sub:
            print(f"App requires APP_SUB: {app}", file=sys.stderr)
            print("Available sub apps:", file=sys.stderr)
            for item in app_sub_list:
                print(f"  {item}", file=sys.stderr)
            return None
        if app_sub not in app_sub_list:
            print(f"Unknown APP_SUB for {app}: {app_sub}", file=sys.stderr)
            print("Available sub apps:", file=sys.stderr)
            for item in app_sub_list:
                print(f"  {item}", file=sys.stderr)
            return None
        return AppCase(app, app_sub)

    if app_sub:
        print(f"App does not use APP_SUB: {app}", file=sys.stderr)
        return None
    return AppCase(app)


def run_case(case: AppCase, port: str, make_jobs: int, extra_make_args: list[str], dry_run: bool) -> int:
    cmd = build_run_command(case, port, make_jobs, extra_make_args)
    print()
    print("Running:")
    print("  " + " ".join(cmd))
    if dry_run:
        return 0
    return subprocess.call(cmd, cwd=ROOT_DIR)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run EmbeddedGUI widget examples from an interactive menu.")
    parser.add_argument("--list", action="store_true", help="List available apps and sub apps.")
    parser.add_argument("--app", help="Run a specific app.")
    parser.add_argument("--app-sub", help="Run a specific APP_SUB.")
    parser.add_argument("--port", default="pc", help="Target port, default: pc.")
    parser.add_argument("-j", "--jobs", type=int, default=MAKE_JOBS, help=f"make -j value, default: {MAKE_JOBS}.")
    parser.add_argument("--dry-run", action="store_true", help="Print the make command without running it.")
    parser.add_argument("make_args", nargs="*", help="Extra make variables, for example BITS=32.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    apps, sub_apps = build_app_index()
    if not apps:
        print("No example apps found.", file=sys.stderr)
        return 1

    if args.list:
        print_cases(apps, sub_apps)
        return 0

    if args.app:
        case = validate_case(args.app, args.app_sub, apps, sub_apps)
        if case is None:
            return 2
        return run_case(case, args.port, max(1, args.jobs), args.make_args, args.dry_run)

    if args.app_sub:
        print("--app-sub requires --app.", file=sys.stderr)
        return 2

    case = choose_case(apps, sub_apps)
    if case is None:
        return 0
    return run_case(case, args.port, max(1, args.jobs), args.make_args, args.dry_run)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
