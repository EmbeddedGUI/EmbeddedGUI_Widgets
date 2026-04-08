#!/usr/bin/env python3
"""Run the locally built EmbeddedGUI app from an absolute output path."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the built EmbeddedGUI application from one output directory.")
    parser.add_argument("--output-dir", required=True, help="Build output directory that contains main/main.exe.")
    parser.add_argument("--resource-path", default=None, help="Optional resource bundle path. Defaults to app_egui_resource_merge.bin inside output-dir.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir).resolve()
    exe_name = "main.exe" if os.name == "nt" else "main"
    exe_path = output_dir / exe_name
    resource_path = Path(args.resource_path).resolve() if args.resource_path else (output_dir / "app_egui_resource_merge.bin")

    if not exe_path.exists():
        print(f"[run-app] executable not found: {exe_path}", file=sys.stderr)
        return 1

    cmd = [str(exe_path)]
    if resource_path.exists():
        cmd.append(str(resource_path))

    print("[run-app] " + " ".join(f'"{part}"' if " " in part else part for part in cmd))
    result = subprocess.run(cmd, cwd=str(output_dir))
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
