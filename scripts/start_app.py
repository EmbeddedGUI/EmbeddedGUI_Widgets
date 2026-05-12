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
CUSTOM_WIDGETS_APP = "HelloCustomWidgets"
CUSTOM_WIDGETS_ROOT = EXAMPLE_DIR / CUSTOM_WIDGETS_APP
CATEGORY_ORDER = ["input", "layout", "navigation", "display", "feedback"]
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
    if not (CUSTOM_WIDGETS_ROOT / "build.mk").exists():
        return []
    return [CUSTOM_WIDGETS_APP]


def sort_category_ids(category_ids: list[str]) -> list[str]:
    def sort_key(category_id: str) -> tuple[int, str]:
        try:
            return (CATEGORY_ORDER.index(category_id), category_id)
        except ValueError:
            return (len(CATEGORY_ORDER), category_id)

    return sorted(category_ids, key=sort_key)


def get_custom_widget_category_index() -> dict[str, list[str]]:
    if not CUSTOM_WIDGETS_ROOT.exists():
        return {}

    categories: dict[str, list[str]] = {}
    if (CUSTOM_WIDGETS_ROOT / "showcase" / "test.c").exists():
        categories["showcase"] = ["showcase"]

    for category_dir in sorted(path for path in CUSTOM_WIDGETS_ROOT.iterdir() if path.is_dir()):
        if category_dir.name.startswith(".") or category_dir.name.startswith("__"):
            continue
        if category_dir.name == "showcase":
            continue
        widgets = []
        for widget_dir in sorted(path for path in category_dir.iterdir() if path.is_dir()):
            if (widget_dir / "test.c").exists():
                widgets.append(widget_dir.name)
        if widgets:
            categories[category_dir.name] = widgets

    return {category: categories[category] for category in sort_category_ids(list(categories))}


def get_custom_widget_sub_apps() -> list[str]:
    sub_apps = []
    for category, widgets in get_custom_widget_category_index().items():
        if category == "showcase":
            sub_apps.append("showcase")
            continue
        for widget in widgets:
            sub_apps.append(f"{category}/{widget}")
    return sub_apps


def get_example_sub_list(app: str) -> list[str]:
    if app == CUSTOM_WIDGETS_APP:
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


def print_cases(category_index: dict[str, list[str]]) -> None:
    for category, widgets in category_index.items():
        if category == "showcase":
            print("showcase")
            continue
        print(f"{category}:")
        for widget in widgets:
            print(f"  {category}/{widget}")


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


def choose_case(category_index: dict[str, list[str]]) -> AppCase | None:
    while True:
        categories = list(category_index)
        print_numbered("HelloCustomWidgets categories", categories)
        category_index_value = read_choice("Select category", len(categories))
        if category_index_value is None:
            return None

        category = categories[category_index_value]
        widgets = category_index[category]
        if category == "showcase":
            return AppCase(CUSTOM_WIDGETS_APP, "showcase")

        while True:
            print_numbered(f"{category} widgets", widgets)
            widget_index = read_choice("Select widget", len(widgets), allow_back=True)
            if widget_index is None:
                return None
            if widget_index == "back":
                break
            return AppCase(CUSTOM_WIDGETS_APP, f"{category}/{widgets[widget_index]}")


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


def validate_custom_widget_sub_app(app_sub: str, sub_apps: list[str]) -> AppCase | None:
    if app_sub not in sub_apps:
        print(f"Unknown APP_SUB for {CUSTOM_WIDGETS_APP}: {app_sub}", file=sys.stderr)
        print("Available sub apps:", file=sys.stderr)
        for item in sub_apps:
            print(f"  {item}", file=sys.stderr)
        return None
    return AppCase(CUSTOM_WIDGETS_APP, app_sub)


def run_case(case: AppCase, port: str, make_jobs: int, extra_make_args: list[str], dry_run: bool) -> int:
    cmd = build_run_command(case, port, make_jobs, extra_make_args)
    print()
    print("Running:")
    print("  " + " ".join(cmd))
    if dry_run:
        return 0
    return subprocess.call(cmd, cwd=ROOT_DIR)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run HelloCustomWidgets examples from an interactive menu.")
    parser.add_argument("--list", action="store_true", help="List available categories and widgets.")
    parser.add_argument("--app", help="Run a specific app. Only HelloCustomWidgets is supported.")
    parser.add_argument("--app-sub", help="Run a specific HelloCustomWidgets APP_SUB.")
    parser.add_argument("--port", default="pc", help="Target port, default: pc.")
    parser.add_argument("-j", "--jobs", type=int, default=MAKE_JOBS, help=f"make -j value, default: {MAKE_JOBS}.")
    parser.add_argument("--dry-run", action="store_true", help="Print the make command without running it.")
    parser.add_argument("make_args", nargs="*", help="Extra make variables, for example BITS=32.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    apps, sub_apps = build_app_index()
    category_index = get_custom_widget_category_index()
    if not apps:
        print("HelloCustomWidgets was not found.", file=sys.stderr)
        return 1
    if not category_index:
        print("No HelloCustomWidgets demos found.", file=sys.stderr)
        return 1

    if args.list:
        print_cases(category_index)
        return 0

    if args.app:
        case = validate_case(args.app, args.app_sub, apps, sub_apps)
        if case is None:
            return 2
        return run_case(case, args.port, max(1, args.jobs), args.make_args, args.dry_run)

    if args.app_sub:
        case = validate_custom_widget_sub_app(args.app_sub, sub_apps[CUSTOM_WIDGETS_APP])
        if case is None:
            return 2
        return run_case(case, args.port, max(1, args.jobs), args.make_args, args.dry_run)

    case = choose_case(category_index)
    if case is None:
        return 0
    return run_case(case, args.port, max(1, args.jobs), args.make_args, args.dry_run)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
