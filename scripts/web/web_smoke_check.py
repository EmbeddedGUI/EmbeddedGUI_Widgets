#!/usr/bin/env python3
"""Headless browser smoke check for EmbeddedGUI web demos."""

from __future__ import annotations

import argparse
import base64
import errno
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import threading
import time
from datetime import datetime
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse
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

    def handle(self) -> None:
        try:
            super().handle()
        except (BrokenPipeError, ConnectionResetError):
            return

    def copyfile(self, source, outputfile) -> None:
        try:
            super().copyfile(source, outputfile)
        except OSError as exc:
            if exc.errno in (errno.EPIPE, errno.ECONNRESET):
                return
            raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Headless browser smoke check for web demos.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Entry note:\n"
            "  HelloCustomWidgets / HelloUnitTest use uicode_disp0.c / uicode_disp0.h\n"
            "  directly as the multi-display SDK entry.\n"
        ),
    )
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="Path to demos.json.")
    parser.add_argument("--web-root", default=str(DEFAULT_WEB_ROOT), help="Static web root directory.")
    parser.add_argument("--browser", default="", help="Browser executable path. Defaults to Edge/Chrome auto-detection.")
    parser.add_argument("--browser-arg", action="append", default=[], help="Extra browser launch arg. Repeatable.")
    parser.add_argument("--demo", action="append", default=[], help="Only check selected demo name(s). Repeatable.")
    parser.add_argument("--name-filter", default="", help="Only check demos whose names contain this substring.")
    parser.add_argument("--category", default="", help="Only check demos in one manifest category.")
    parser.add_argument("--port", type=int, default=0, help="Server port. 0 chooses a free port automatically.")
    parser.add_argument("--window-size", default="900,980", help="Browser window size, for example 900,980.")
    parser.add_argument("--virtual-time-budget", type=int, default=12000, help="Headless browser virtual time budget in ms.")
    parser.add_argument("--browser-timeout", type=int, default=90, help="Per-demo browser timeout in seconds.")
    parser.add_argument(
        "--capture-mode",
        choices=["cli", "cdp"],
        default="cli",
        help="Capture implementation. cli uses Chrome --dump-dom/--screenshot; cdp uses the DevTools protocol.",
    )
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


def default_browser_args() -> list[str]:
    args = [
        "--headless=new",
        "--disable-gpu",
        "--no-first-run",
        "--no-default-browser-check",
    ]

    if os.name != "nt":
        args.extend([
            "--disable-dev-shm-usage",
            "--disable-background-networking",
            "--disable-component-update",
            "--run-all-compositor-stages-before-draw",
        ])

    if os.environ.get("GITHUB_ACTIONS", "").lower() == "true":
        args.extend(["--no-sandbox", "--disable-setuid-sandbox"])

    return args


def merge_browser_args(extra_args: list[str]) -> list[str]:
    merged: list[str] = []
    seen: set[str] = set()

    for arg in default_browser_args() + list(extra_args):
        if arg in seen:
            continue
        merged.append(arg)
        seen.add(arg)

    return merged


def find_free_tcp_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return int(sock.getsockname()[1])


def read_http_json(url: str, timeout: float = 2.0):
    with urlopen(url, timeout=timeout) as response:
        return json.loads(response.read().decode("utf-8", errors="replace"))


def wait_for_devtools_page(debug_port: int, target_url: str, timeout: float) -> dict:
    endpoint = f"http://127.0.0.1:{debug_port}/json/list"
    deadline = time.time() + timeout
    last_error: Exception | None = None
    while time.time() < deadline:
        try:
            targets = read_http_json(endpoint, timeout=2)
            pages = [target for target in targets if target.get("type") == "page"]
            for target in pages:
                if target.get("url") == target_url:
                    return target
            if len(pages) == 1 and pages[0].get("webSocketDebuggerUrl"):
                return pages[0]
        except Exception as exc:  # pragma: no cover - runtime env dependent
            last_error = exc
        time.sleep(0.2)
    raise TimeoutError(f"devtools_page_not_ready: {last_error}")


class DevToolsWebSocket:
    """Small WebSocket client for Chrome DevTools Protocol text frames."""

    def __init__(self, ws_url: str):
        parsed = urlparse(ws_url)
        if parsed.scheme != "ws":
            raise ValueError(f"unsupported DevTools URL: {ws_url}")
        self.host = parsed.hostname or "127.0.0.1"
        self.port = parsed.port or 80
        self.path = parsed.path
        if parsed.query:
            self.path += "?" + parsed.query
        self.sock: socket.socket | None = None
        self.next_id = 1

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def connect(self) -> None:
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        sock = socket.create_connection((self.host, self.port), timeout=10)
        request = (
            f"GET {self.path} HTTP/1.1\r\n"
            f"Host: {self.host}:{self.port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n"
        )
        sock.sendall(request.encode("ascii"))
        response = b""
        while b"\r\n\r\n" not in response:
            chunk = sock.recv(4096)
            if not chunk:
                raise ConnectionError("DevTools websocket closed during handshake")
            response += chunk
        status_line = response.split(b"\r\n", 1)[0]
        if b" 101 " not in status_line:
            raise ConnectionError(status_line.decode("ascii", errors="replace"))
        sock.settimeout(10)
        self.sock = sock

    def close(self) -> None:
        if self.sock is not None:
            try:
                self.sock.close()
            finally:
                self.sock = None

    def _recv_exact(self, length: int) -> bytes:
        assert self.sock is not None
        data = b""
        while len(data) < length:
            chunk = self.sock.recv(length - len(data))
            if not chunk:
                raise ConnectionError("DevTools websocket closed")
            data += chunk
        return data

    def _send_frame(self, opcode: int, payload: bytes) -> None:
        assert self.sock is not None
        header = bytearray()
        header.append(0x80 | opcode)
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < 65536:
            header.extend([0x80 | 126, (length >> 8) & 0xFF, length & 0xFF])
        else:
            header.append(0x80 | 127)
            header.extend(length.to_bytes(8, "big"))
        mask = os.urandom(4)
        masked = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        self.sock.sendall(bytes(header) + mask + masked)

    def _recv_frame(self) -> tuple[int, bool, bytes]:
        header = self._recv_exact(2)
        first, second = header[0], header[1]
        fin = bool(first & 0x80)
        opcode = first & 0x0F
        masked = bool(second & 0x80)
        length = second & 0x7F
        if length == 126:
            length = int.from_bytes(self._recv_exact(2), "big")
        elif length == 127:
            length = int.from_bytes(self._recv_exact(8), "big")
        mask = self._recv_exact(4) if masked else b""
        payload = self._recv_exact(length) if length else b""
        if masked:
            payload = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        return opcode, fin, payload

    def _recv_message(self) -> str:
        chunks: list[bytes] = []
        while True:
            opcode, fin, payload = self._recv_frame()
            if opcode == 0x8:
                raise ConnectionError("DevTools websocket closed by browser")
            if opcode == 0x9:
                self._send_frame(0xA, payload)
                continue
            if opcode in (0x1, 0x0):
                chunks.append(payload)
                if fin:
                    return b"".join(chunks).decode("utf-8", errors="replace")

    def command(self, method: str, params: dict | None = None, timeout: float = 10.0) -> dict:
        assert self.sock is not None
        message_id = self.next_id
        self.next_id += 1
        payload = {"id": message_id, "method": method}
        if params is not None:
            payload["params"] = params
        self._send_frame(0x1, json.dumps(payload, separators=(",", ":")).encode("utf-8"))

        old_timeout = self.sock.gettimeout()
        self.sock.settimeout(timeout)
        try:
            while True:
                message = json.loads(self._recv_message())
                if message.get("id") != message_id:
                    continue
                if "error" in message:
                    raise RuntimeError(f"{method}: {message['error']}")
                return message.get("result", {})
        finally:
            self.sock.settimeout(old_timeout)


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
    server.daemon_threads = True
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
        f"- Browser args: `{' '.join(summary['browserArgs'])}`",
        f"- Manifest: `{summary['manifest']}`",
        f"- Web root: `{summary['webRoot']}`",
        f"- Output: `{project_relative(summary_path.parent)}`",
        f"- Result: `{summary['passed']}/{summary['total']}` passed",
        f"- Capture mode: `{summary['captureMode']}`",
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


def empty_metrics() -> dict:
    return {
        "imageWidth": 0,
        "imageHeight": 0,
        "nonBlackRatio": 0.0,
        "bbox": None,
        "bboxArea": 0,
        "uniqueColors": 0,
    }


def is_smoke_ok(result: dict) -> bool:
    metrics = result["metrics"]
    return (
        not result["timedOut"]
        and result["returncode"] == 0
        and result["status"] == "Running"
        and result["canvasSize"][0] > 0
        and result["canvasSize"][1] > 0
        and result["shotExists"]
        and metrics["bboxArea"] >= 40000
        and metrics["nonBlackRatio"] >= 0.03
        and metrics["uniqueColors"] >= 20
    )


def make_demo_result(
    entry: dict,
    url: str,
    returncode: int,
    timed_out: bool,
    status: str,
    title: str,
    canvas_size: list[int],
    screenshot_path: Path,
    dom_path: Path,
    stderr_path: Path,
    metrics: dict,
    capture_mode: str,
) -> dict:
    result = {
        "name": entry["name"],
        "app": entry["app"],
        "category": entry.get("category"),
        "url": url,
        "returncode": returncode,
        "timedOut": timed_out,
        "status": status,
        "title": title,
        "canvasSize": canvas_size,
        "shotExists": screenshot_path.exists() and screenshot_path.stat().st_size > 0,
        "screenshot": project_relative(screenshot_path),
        "dom": project_relative(dom_path),
        "stderr": project_relative(stderr_path),
        "metrics": metrics,
        "captureMode": capture_mode,
    }
    result["ok"] = is_smoke_ok(result)
    return result


def read_stderr_excerpt(path: Path, max_chars: int = 220) -> str:
    if not path.exists():
        return ""
    text = path.read_text(encoding="utf-8", errors="replace").strip()
    if not text:
        return ""
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    excerpt = " | ".join(lines[-3:])
    if len(excerpt) > max_chars:
        excerpt = excerpt[: max_chars - 3] + "..."
    return excerpt


def run_demo_cli(browser: str, browser_args: list[str], port: int, entry: dict, output_dir: Path, profile_dir: Path, window_size: str, virtual_time_budget: int, browser_timeout: int) -> dict:
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
        *browser_args,
        f"--virtual-time-budget={virtual_time_budget}",
        f"--window-size={window_size}",
        f"--user-data-dir={profile_dir}",
        "--dump-dom",
        f"--screenshot={screenshot_path}",
        url,
    ]

    timed_out = False
    returncode = 0
    stdout = ""
    stderr = ""
    try:
        completed = subprocess.run(
            command,
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=max(1, browser_timeout),
        )
        returncode = completed.returncode
        stdout = completed.stdout or ""
        stderr = completed.stderr or ""
    except subprocess.TimeoutExpired as exc:
        timed_out = True
        returncode = -9
        stdout_data = exc.stdout or ""
        stderr_data = exc.stderr or ""
        stdout = stdout_data.decode("utf-8", errors="replace") if isinstance(stdout_data, bytes) else stdout_data
        stderr = stderr_data.decode("utf-8", errors="replace") if isinstance(stderr_data, bytes) else stderr_data
        stderr = (stderr + f"\nBROWSER_TIMEOUT after {browser_timeout}s\n").lstrip()

    dom_path.write_text(stdout, encoding="utf-8")
    stderr_path.write_text(stderr, encoding="utf-8")

    status, title, canvas_size = parse_dom(stdout)
    shot_exists = screenshot_path.exists() and screenshot_path.stat().st_size > 0
    metrics = screenshot_metrics(screenshot_path) if shot_exists else empty_metrics()
    return make_demo_result(
        entry=entry,
        url=url,
        returncode=returncode,
        timed_out=timed_out,
        status=status,
        title=title,
        canvas_size=canvas_size,
        screenshot_path=screenshot_path,
        dom_path=dom_path,
        stderr_path=stderr_path,
        metrics=metrics,
        capture_mode="cli",
    )


def get_runtime_state(cdp: DevToolsWebSocket) -> dict:
    expression = r"""
(() => {
  const statusEl = document.getElementById('status');
  const canvas = document.getElementById('canvas');
  return {
    status: statusEl ? statusEl.textContent.trim() : '',
    title: document.title || '',
    canvasSize: canvas ? [canvas.width || 0, canvas.height || 0] : [0, 0],
    html: document.documentElement ? document.documentElement.outerHTML : ''
  };
})()
"""
    result = cdp.command(
        "Runtime.evaluate",
        {"expression": expression, "returnByValue": True},
        timeout=5,
    )
    value = result.get("result", {}).get("value")
    return value if isinstance(value, dict) else {}


def wait_two_animation_frames(cdp: DevToolsWebSocket) -> None:
    expression = "new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(() => resolve(true))))"
    try:
        cdp.command(
            "Runtime.evaluate",
            {"expression": expression, "awaitPromise": True, "returnByValue": True},
            timeout=4,
        )
    except Exception:
        return


def run_demo_cdp(browser: str, browser_args: list[str], port: int, entry: dict, output_dir: Path, profile_dir: Path, window_size: str, browser_timeout: int) -> dict:
    name = entry["name"]
    app = entry["app"]
    demo_dir = output_dir / name
    demo_dir.mkdir(parents=True, exist_ok=True)
    profile_dir.mkdir(parents=True, exist_ok=True)

    dom_path = demo_dir / "dom.html"
    stderr_path = demo_dir / "stderr.txt"
    screenshot_path = demo_dir / "screenshot.png"
    url = f"http://127.0.0.1:{port}/demos/{name}/{app}.html"
    debug_port = find_free_tcp_port()

    command = [
        browser,
        *browser_args,
        f"--remote-debugging-port={debug_port}",
        f"--window-size={window_size}",
        f"--user-data-dir={profile_dir}",
        url,
    ]

    process: subprocess.Popen | None = None
    stderr_extra = ""
    timed_out = False
    returncode = 0
    status = ""
    title = ""
    canvas_size = [0, 0]
    dom_text = ""
    metrics = empty_metrics()

    stderr_file = stderr_path.open("w", encoding="utf-8")
    try:
        process = subprocess.Popen(
            command,
            cwd=PROJECT_ROOT,
            stdout=subprocess.DEVNULL,
            stderr=stderr_file,
        )

        deadline = time.time() + max(1, browser_timeout)
        target = wait_for_devtools_page(debug_port, url, min(15.0, max(1.0, deadline - time.time())))
        with DevToolsWebSocket(target["webSocketDebuggerUrl"]) as cdp:
            cdp.command("Page.enable", timeout=5)
            cdp.command("Runtime.enable", timeout=5)
            if target.get("url") != url:
                cdp.command("Page.navigate", {"url": url}, timeout=5)

            ready = False
            last_state: dict = {}
            while time.time() < deadline:
                if process.poll() is not None:
                    returncode = process.returncode if process.returncode is not None else 1
                    stderr_extra += f"\nBROWSER_EXITED before page became ready rc={returncode}\n"
                    break

                try:
                    last_state = get_runtime_state(cdp)
                except Exception as exc:
                    stderr_extra += f"\nCDP_STATE_RETRY {type(exc).__name__}: {exc}\n"
                    time.sleep(0.25)
                    continue
                status = str(last_state.get("status") or "")
                title = str(last_state.get("title") or "")
                size_value = last_state.get("canvasSize")
                if isinstance(size_value, list) and len(size_value) == 2:
                    canvas_size = [int(size_value[0] or 0), int(size_value[1] or 0)]
                dom_text = str(last_state.get("html") or "")
                if status == "Running" and canvas_size[0] > 0 and canvas_size[1] > 0:
                    ready = True
                    break
                time.sleep(0.25)

            if ready:
                wait_two_animation_frames(cdp)
                while time.time() < deadline:
                    screenshot = cdp.command(
                        "Page.captureScreenshot",
                        {"format": "png", "fromSurface": True},
                        timeout=10,
                    )
                    data = screenshot.get("data")
                    if isinstance(data, str):
                        screenshot_path.write_bytes(base64.b64decode(data))
                        metrics = screenshot_metrics(screenshot_path)
                        if (
                            metrics["bboxArea"] >= 40000
                            and metrics["nonBlackRatio"] >= 0.03
                            and metrics["uniqueColors"] >= 20
                        ):
                            break
                    time.sleep(0.25)
                if not (
                    metrics["bboxArea"] >= 40000
                    and metrics["nonBlackRatio"] >= 0.03
                    and metrics["uniqueColors"] >= 20
                ):
                    stderr_extra += (
                        "\nSCREENSHOT_METRICS_LOW "
                        f"bboxArea={metrics['bboxArea']} "
                        f"ratio={metrics['nonBlackRatio']} "
                        f"colors={metrics['uniqueColors']}\n"
                    )
            elif returncode == 0:
                timed_out = True
                returncode = -9
                state_note = json.dumps(last_state, ensure_ascii=False)[:500] if last_state else "no runtime state"
                stderr_extra += f"\nBROWSER_TIMEOUT after {browser_timeout}s waiting for page ready: {state_note}\n"
    except TimeoutError as exc:
        timed_out = True
        returncode = -9
        stderr_extra += f"\nCDP_TIMEOUT after {browser_timeout}s: {exc}\n"
    except Exception as exc:
        if process is not None and process.poll() is not None:
            returncode = process.returncode if process.returncode is not None else 1
        else:
            returncode = 1
        stderr_extra += f"\nCDP_ERROR {type(exc).__name__}: {exc}\n"
    finally:
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5)
        stderr_file.close()

    if stderr_extra:
        with stderr_path.open("a", encoding="utf-8") as f:
            f.write(stderr_extra)

    dom_path.write_text(dom_text, encoding="utf-8")
    return make_demo_result(
        entry=entry,
        url=url,
        returncode=returncode,
        timed_out=timed_out,
        status=status,
        title=title,
        canvas_size=canvas_size,
        screenshot_path=screenshot_path,
        dom_path=dom_path,
        stderr_path=stderr_path,
        metrics=metrics,
        capture_mode="cdp",
    )


def run_demo(browser: str, browser_args: list[str], port: int, entry: dict, output_dir: Path, profile_dir: Path, window_size: str, virtual_time_budget: int, browser_timeout: int, capture_mode: str) -> dict:
    if capture_mode == "cdp":
        return run_demo_cdp(
            browser=browser,
            browser_args=browser_args,
            port=port,
            entry=entry,
            output_dir=output_dir,
            profile_dir=profile_dir,
            window_size=window_size,
            browser_timeout=browser_timeout,
        )
    return run_demo_cli(
        browser=browser,
        browser_args=browser_args,
        port=port,
        entry=entry,
        output_dir=output_dir,
        profile_dir=profile_dir,
        window_size=window_size,
        virtual_time_budget=virtual_time_budget,
        browser_timeout=browser_timeout,
    )


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
    browser_args = merge_browser_args(args.browser_arg)
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
        print(f"Capture mode: {args.capture_mode}", flush=True)
        print("Browser args: " + " ".join(browser_args), flush=True)
        for index, entry in enumerate(manifest, start=1):
            print(f"[{index}/{len(manifest)}] {entry['name']}", flush=True)
            result = run_demo(
                browser=browser,
                browser_args=browser_args,
                port=server.server_address[1],
                entry=entry,
                output_dir=output_dir,
                profile_dir=profiles_dir / entry["name"],
                window_size=args.window_size,
                virtual_time_budget=args.virtual_time_budget,
                browser_timeout=args.browser_timeout,
                capture_mode=args.capture_mode,
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
                stderr_excerpt = read_stderr_excerpt(PROJECT_ROOT / result["stderr"])
                stderr_note = f" stderr={stderr_excerpt!r}" if stderr_excerpt else ""
                print(
                    "  FAIL "
                    f"rc={result['returncode']} "
                    f"status={result['status']!r} "
                    f"canvas={result['canvasSize']} "
                    f"ratio={result['metrics']['nonBlackRatio']} "
                    f"colors={result['metrics']['uniqueColors']}"
                    f"{stderr_note}",
                    flush=True,
                )

        contact_sheet_path = None
        if not args.no_contact_sheet:
            contact_sheet_path = create_contact_sheet(output_dir, results, window_dims)

        summary = {
            "generatedAt": datetime.now().isoformat(timespec="seconds"),
            "browser": browser,
            "browserArgs": browser_args,
            "captureMode": args.capture_mode,
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
