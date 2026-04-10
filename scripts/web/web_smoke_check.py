#!/usr/bin/env python3
"""Headless browser smoke check for EmbeddedGUI web demos."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from datetime import datetime
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.request import urlopen

from PIL import Image, ImageDraw, ImageFont


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent.parent
DEFAULT_WEB_ROOT = PROJECT_ROOT / "web"
DEFAULT_MANIFEST = DEFAULT_WEB_ROOT / "demos" / "demos.json"

UTF8_CONTENT_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".md": "text/markdown; charset=utf-8",
    ".rst": "text/x-rst; charset=utf-8",
    ".txt": "text/plain; charset=utf-8",
}

WINDOW_CANDIDATE_BROWSERS = [
    Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"),
    Path(r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"),
    Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
    Path(r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"),
]
PATH_CANDIDATE_BROWSERS = [
    "msedge",
    "chrome",
    "chromium",
    "google-chrome",
    "chromium-browser",
]


class Utf8StaticHandler(SimpleHTTPRequestHandler):
    extensions_map = SimpleHTTPRequestHandler.extensions_map.copy()
    extensions_map.update(UTF8_CONTENT_TYPES)

    def guess_type(self, path: str) -> str:
        suffix = Path(path).suffix.lower()
        if suffix in UTF8_CONTENT_TYPES:
            return UTF8_CONTENT_TYPES[suffix]

        content_type = super().guess_type(path)
        if content_type in ("text/javascript", "application/x-javascript"):
            return "application/javascript; charset=utf-8"
        if content_type.startswith("text/") and "charset=" not in content_type:
            return content_type + "; charset=utf-8"
        return content_type

    def log_message(self, format: str, *args) -> None:
        return


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Headless browser smoke check for web demos.")
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="Path to demos.json.")
    parser.add_argument("--web-root", default=str(DEFAULT_WEB_ROOT), help="Static web root directory.")
    parser.add_argument("--browser", default="", help="Browser executable path. Defaults to Edge/Chrome auto-detection.")
    parser.add_argument("--demo", action="append", default=[], help="Only check selected demo name(s). Repeatable.")
    parser.add_argument("--name-filter", default="", help="Only check demos whose names contain this substring.")
    parser.add_argument("--category", default="", help="Only check demos in one manifest category.")
    parser.add_argument("--port", type=int, default=0, help="Server port. 0 chooses a free port automatically.")
    parser.add_argument("--window-size", default="900,980", help="Browser window size, for example 900,980.")
    parser.add_argument("--virtual-time-budget", type=int, default=12000, help="Headless browser virtual time budget in ms.")
    parser.add_argument(
        "--output-dir",
        default="",
        help="Directory for smoke outputs. Defaults to output/web_smoke_check_<timestamp>.",
    )
    parser.add_argument("--no-contact-sheet", action="store_true", help="Skip contact sheet generation.")
    parser.add_argument("--max-demos", type=int, default=0, help="Limit checked demos for quick local debugging.")
    return parser.parse_args()


def find_browser(browser_override: str) -> str:
    if browser_override:
        browser_path = Path(browser_override)
        if browser_path.exists():
            return str(browser_path)
        resolved = shutil.which(browser_override)
        if resolved:
            return resolved
        raise FileNotFoundError(f"browser not found: {browser_override}")

    for candidate in WINDOW_CANDIDATE_BROWSERS:
        if candidate.exists():
            return str(candidate)

    for candidate in PATH_CANDIDATE_BROWSERS:
        resolved = shutil.which(candidate)
        if resolved:
            return resolved

    raise FileNotFoundError("no supported browser found; pass --browser explicitly")


def load_manifest(path: Path) -> list[dict]:
    return json.loads(path.read_text(encoding="utf-8"))


def filter_manifest(entries: list[dict], args: argparse.Namespace) -> list[dict]:
    filtered = list(entries)
    if args.demo:
        selected = set(args.demo)
        filtered = [entry for entry in filtered if entry["name"] in selected]
    if args.name_filter:
        filtered = [entry for entry in filtered if args.name_filter in entry["name"]]
    if args.category:
        filtered = [entry for entry in filtered if entry.get("category") == args.category]
    if args.max_demos > 0:
        filtered = filtered[: args.max_demos]
    return filtered


def parse_window_size(window_size: str) -> tuple[int, int]:
    parts = window_size.split(",", 1)
    if len(parts) != 2:
        raise ValueError(f"invalid --window-size: {window_size}")
    width = int(parts[0].strip())
    height = int(parts[1].strip())
    if width <= 0 or height <= 0:
        raise ValueError(f"invalid --window-size: {window_size}")
    return width, height


def wait_server(port: int, timeout: float = 15.0) -> None:
    end_time = time.time() + timeout
    url = f"http://127.0.0.1:{port}/demos/demos.json"
    last_error: Exception | None = None
    while time.time() < end_time:
        try:
            with urlopen(url, timeout=3) as response:
                if response.status == 200:
                    return
        except Exception as exc:  # pragma: no cover - runtime env dependent
            last_error = exc
        time.sleep(0.3)
    raise RuntimeError(f"server_not_ready: {last_error}")


def start_server(web_root: Path, port: int) -> tuple[ThreadingHTTPServer, threading.Thread]:
    handler = partial(Utf8StaticHandler, directory=str(web_root))
    server = ThreadingHTTPServer(("127.0.0.1", port), handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, thread


def parse_dom(dom_text: str) -> tuple[str, str, list[int]]:
    status_match = re.search(r'<div id="status">(.*?)</div>', dom_text, re.S)
    status = re.sub(r"<.*?>", "", status_match.group(1)).strip() if status_match else ""
    title_match = re.search(r"<title>(.*?)</title>", dom_text, re.S)
    title = title_match.group(1).strip() if title_match else ""
    canvas_match = re.search(r'<canvas id="canvas"[^>]*width="(\d+)"[^>]*height="(\d+)"', dom_text, re.S)
    canvas_size = [int(canvas_match.group(1)), int(canvas_match.group(2))] if canvas_match else [0, 0]
    return status, title, canvas_size


def screenshot_metrics(path: Path) -> dict:
    image = Image.open(path).convert("RGB")
    width, height = image.size
    pixels = image.load()

    non_black = 0
    min_x = width
    min_y = height
    max_x = -1
    max_y = -1
    unique_colors: set[tuple[int, int, int]] = set()

    for y in range(height):
        for x in range(width):
            pixel = pixels[x, y]
            unique_colors.add(pixel)
            if pixel[0] + pixel[1] + pixel[2] > 24:
                non_black += 1
                if x < min_x:
                    min_x = x
                if y < min_y:
                    min_y = y
                if x > max_x:
                    max_x = x
                if y > max_y:
                    max_y = y

    if max_x >= min_x and max_y >= min_y:
        bbox = [min_x, min_y, max_x + 1, max_y + 1]
        bbox_area = (bbox[2] - bbox[0]) * (bbox[3] - bbox[1])
    else:
        bbox = None
        bbox_area = 0

    return {
        "imageWidth": width,
        "imageHeight": height,
        "nonBlackRatio": round(non_black / float(width * height), 4),
        "bbox": bbox,
        "bboxArea": bbox_area,
        "uniqueColors": len(unique_colors),
    }


def project_relative(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(PROJECT_ROOT)).replace("\\", "/")
    except ValueError:
        return str(path.resolve())


def build_output_dir(path_arg: str) -> Path:
    if path_arg:
        return (PROJECT_ROOT / path_arg).resolve() if not Path(path_arg).is_absolute() else Path(path_arg).resolve()
    stamp = datetime.now().strftime("web_smoke_check_%Y%m%d_%H%M%S")
    return PROJECT_ROOT / "output" / stamp


def render_summary_markdown(summary: dict, summary_path: Path, summary_md_path: Path) -> str:
    lines = [
        "## Web Smoke Check",
        "",
        f"- Generated: `{summary['generatedAt']}`",
        f"- Browser: `{summary['browser']}`",
        f"- Manifest: `{summary['manifest']}`",
        f"- Web root: `{summary['webRoot']}`",
        f"- Output: `{project_relative(summary_path.parent)}`",
        f"- Result: `{summary['passed']}/{summary['total']}` passed",
        f"- Window size: `{summary['windowSize'][0]}x{summary['windowSize'][1]}`",
        f"- Virtual time budget: `{summary['virtualTimeBudgetMs']} ms`",
        f"- JSON summary: `{project_relative(summary_path)}`",
        f"- Markdown summary: `{project_relative(summary_md_path)}`",
    ]

    contact_sheet = summary.get("contactSheet")
    if contact_sheet:
        lines.append(f"- Contact sheet: `{contact_sheet}`")

    if summary["failed"] > 0:
        lines.extend(["", "### Failed Demos", ""])
        for item in summary["results"]:
            if not item["ok"]:
                lines.append(
                    f"- `{item['name']}`: status=`{item['status'] or 'missing'}`, "
                    f"canvas=`{item['canvasSize'][0]}x{item['canvasSize'][1]}`, "
                    f"ratio=`{item['metrics']['nonBlackRatio']}`, "
                    f"colors=`{item['metrics']['uniqueColors']}`, "
                    f"screenshot=`{item['screenshot']}`, stderr=`{item['stderr']}`"
                )
    else:
        lines.extend(["", "All selected demos passed smoke check."])

    return "\n".join(lines) + "\n"


def create_contact_sheet(output_dir: Path, results: list[dict], window_size: tuple[int, int]) -> Path | None:
    if not results:
        return None

    sheet_cols = 4
    thumb_width = max(220, window_size[0] // 4)
    thumb_height = max(240, window_size[1] // 4)
    rows = (len(results) + sheet_cols - 1) // sheet_cols
    label_height = 36
    sheet = Image.new("RGB", (sheet_cols * thumb_width, rows * thumb_height), (245, 247, 250))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()

    for index, result in enumerate(results):
        screenshot_path = PROJECT_ROOT / result["screenshot"]
        if not screenshot_path.exists():
            continue
        image = Image.open(screenshot_path).convert("RGB")
        image.thumbnail((thumb_width - 16, thumb_height - label_height - 12))

        cell_x = (index % sheet_cols) * thumb_width
        cell_y = (index // sheet_cols) * thumb_height
        paste_x = cell_x + (thumb_width - image.width) // 2
        paste_y = cell_y + 6

        sheet.paste(image, (paste_x, paste_y))
        draw.rectangle(
            [cell_x + 4, cell_y + thumb_height - label_height, cell_x + thumb_width - 4, cell_y + thumb_height - 4],
            fill=(255, 255, 255),
            outline=(210, 216, 224),
        )
        label = result["name"].replace("HelloCustomWidgets_", "")
        draw.text((cell_x + 8, cell_y + thumb_height - label_height + 8), label[:36], fill=(35, 48, 63), font=font)

    output_path = output_dir / "contact_sheet.png"
    sheet.save(output_path)
    return output_path


def run_demo(browser: str, port: int, entry: dict, output_dir: Path, profile_dir: Path, window_size: str, virtual_time_budget: int) -> dict:
    name = entry["name"]
    app = entry["app"]
    demo_dir = output_dir / name
    demo_dir.mkdir(parents=True, exist_ok=True)
    profile_dir.mkdir(parents=True, exist_ok=True)

    dom_path = demo_dir / "dom.html"
    stderr_path = demo_dir / "stderr.txt"
    screenshot_path = demo_dir / "screenshot.png"
    url = f"http://127.0.0.1:{port}/demos/{name}/{app}.html"

    command = [
        browser,
        "--headless",
        "--disable-gpu",
        "--no-first-run",
        "--no-default-browser-check",
        f"--virtual-time-budget={virtual_time_budget}",
        f"--window-size={window_size}",
        f"--user-data-dir={profile_dir}",
        "--dump-dom",
        f"--screenshot={screenshot_path}",
        url,
    ]

    completed = subprocess.run(
        command,
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=40,
    )

    dom_path.write_text(completed.stdout or "", encoding="utf-8")
    stderr_path.write_text(completed.stderr or "", encoding="utf-8")

    status, title, canvas_size = parse_dom(completed.stdout or "")
    shot_exists = screenshot_path.exists() and screenshot_path.stat().st_size > 0
    metrics = screenshot_metrics(screenshot_path) if shot_exists else {
        "imageWidth": 0,
        "imageHeight": 0,
        "nonBlackRatio": 0.0,
        "bbox": None,
        "bboxArea": 0,
        "uniqueColors": 0,
    }
    ok = (
        completed.returncode == 0
        and status == "Running"
        and canvas_size[0] > 0
        and canvas_size[1] > 0
        and shot_exists
        and metrics["bboxArea"] >= 40000
        and metrics["nonBlackRatio"] >= 0.03
        and metrics["uniqueColors"] >= 20
    )

    return {
        "name": name,
        "app": app,
        "category": entry.get("category"),
        "url": url,
        "returncode": completed.returncode,
        "status": status,
        "title": title,
        "canvasSize": canvas_size,
        "screenshot": project_relative(screenshot_path),
        "dom": project_relative(dom_path),
        "stderr": project_relative(stderr_path),
        "metrics": metrics,
        "ok": ok,
    }


def main() -> int:
    args = parse_args()
    manifest_path = Path(args.manifest).resolve()
    web_root = Path(args.web_root).resolve()
    output_dir = build_output_dir(args.output_dir)
    profiles_dir = output_dir / "profiles"
    output_dir.mkdir(parents=True, exist_ok=True)
    profiles_dir.mkdir(parents=True, exist_ok=True)

    window_dims = parse_window_size(args.window_size)
    browser = find_browser(args.browser)
    manifest = filter_manifest(load_manifest(manifest_path), args)
    if not manifest:
        print("No demos selected.")
        return 1

    server, thread = start_server(web_root, args.port)
    try:
        wait_server(server.server_address[1])
        results = []
        failures = []

        print(f"Checking {len(manifest)} demo(s) with browser: {browser}", flush=True)
        for index, entry in enumerate(manifest, start=1):
            print(f"[{index}/{len(manifest)}] {entry['name']}", flush=True)
            result = run_demo(
                browser=browser,
                port=server.server_address[1],
                entry=entry,
                output_dir=output_dir,
                profile_dir=profiles_dir / entry["name"],
                window_size=args.window_size,
                virtual_time_budget=args.virtual_time_budget,
            )
            results.append(result)
            if result["ok"]:
                print(
                    "  PASS "
                    f"status={result['status']} "
                    f"canvas={result['canvasSize'][0]}x{result['canvasSize'][1]} "
                    f"ratio={result['metrics']['nonBlackRatio']} "
                    f"colors={result['metrics']['uniqueColors']}",
                    flush=True,
                )
            else:
                failures.append(result["name"])
                print(
                    "  FAIL "
                    f"rc={result['returncode']} "
                    f"status={result['status']!r} "
                    f"canvas={result['canvasSize']} "
                    f"ratio={result['metrics']['nonBlackRatio']} "
                    f"colors={result['metrics']['uniqueColors']}",
                    flush=True,
                )

        contact_sheet_path = None
        if not args.no_contact_sheet:
            contact_sheet_path = create_contact_sheet(output_dir, results, window_dims)

        summary = {
            "generatedAt": datetime.now().isoformat(timespec="seconds"),
            "browser": browser,
            "manifest": str(manifest_path),
            "webRoot": str(web_root),
            "port": server.server_address[1],
            "windowSize": [window_dims[0], window_dims[1]],
            "virtualTimeBudgetMs": args.virtual_time_budget,
            "total": len(results),
            "passed": sum(1 for item in results if item["ok"]),
            "failed": len(failures),
            "failedNames": failures,
            "contactSheet": project_relative(contact_sheet_path) if contact_sheet_path is not None else None,
            "results": results,
        }
        summary_path = output_dir / "summary.json"
        summary_md_path = output_dir / "summary.md"
        summary["summaryMarkdown"] = project_relative(summary_md_path)
        summary_path.write_text(json.dumps(summary, indent=2, ensure_ascii=False), encoding="utf-8")
        summary_md_path.write_text(render_summary_markdown(summary, summary_path, summary_md_path), encoding="utf-8")
        print(f"SUMMARY {summary_path}", flush=True)
        print(f"SUMMARY_MD {summary_md_path}", flush=True)
        print(f"PASSED {summary['passed']}/{summary['total']}", flush=True)
        if contact_sheet_path is not None:
            print(f"CONTACT_SHEET {contact_sheet_path}", flush=True)
        if failures:
            print("FAILED_NAMES " + ", ".join(failures), flush=True)
            return 1
        return 0
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)


if __name__ == "__main__":
    raise SystemExit(main())
