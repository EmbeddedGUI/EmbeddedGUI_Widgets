#!/usr/bin/env python3
"""Start local web server for EmbeddedGUI demos.

Usage:
    python web/start_server.py --port 8080
"""

import argparse
import pathlib
import sys
import webbrowser
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer

UTF8_CONTENT_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".md": "text/markdown; charset=utf-8",
    ".rst": "text/x-rst; charset=utf-8",
    ".txt": "text/plain; charset=utf-8",
}


class Utf8StaticHandler(SimpleHTTPRequestHandler):
    extensions_map = SimpleHTTPRequestHandler.extensions_map.copy()
    extensions_map.update(UTF8_CONTENT_TYPES)

    def guess_type(self, path: str) -> str:
        suffix = pathlib.Path(path).suffix.lower()
        if suffix in UTF8_CONTENT_TYPES:
            return UTF8_CONTENT_TYPES[suffix]

        content_type = super().guess_type(path)
        if content_type in ("text/javascript", "application/x-javascript"):
            return "application/javascript; charset=utf-8"
        if content_type.startswith("text/") and "charset=" not in content_type:
            return content_type + "; charset=utf-8"
        return content_type


def parse_args() -> argparse.Namespace:
    script_dir = pathlib.Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description="Start EmbeddedGUI web demo server")
    parser.add_argument("port", nargs="?", type=int, default=None, help="Port to listen on (default: 8080)")
    parser.add_argument("--port", dest="port_opt", type=int, default=None, help="Port to listen on (default: 8080)")
    parser.add_argument("--web-dir", default=str(script_dir), help="Web root directory (default: script directory)")
    parser.add_argument("--no-browser", action="store_true", help="Do not open browser automatically")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    port = args.port_opt if args.port_opt is not None else (args.port if args.port is not None else 8080)
    web_dir = pathlib.Path(args.web_dir).resolve()
    index_file = web_dir / "index.html"

    if not index_file.exists():
        print("[!!] index.html not found")
        print("    Please build WASM demos first:")
        print("      python scripts/web/wasm_build_demos.py")
        return 1

    url = f"http://localhost:{port}"

    print("========================================")
    print("  EmbeddedGUI Web Demos")
    print("========================================")
    print()
    print(f"  Root   : {web_dir}")
    print(f"  URL    : {url}")
    print("  Stop   : Ctrl+C")
    print()

    if not args.no_browser:
        try:
            webbrowser.open(url)
        except Exception:
            pass

    handler = partial(Utf8StaticHandler, directory=str(web_dir))
    try:
        with ThreadingHTTPServer(("0.0.0.0", port), handler) as server:
            server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped")
        return 0
    except OSError as exc:
        print(f"[!!] failed to start server: {exc}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
