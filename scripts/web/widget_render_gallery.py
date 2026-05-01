#!/usr/bin/env python3
"""Generate a visual gallery for HelloCustomWidgets WASM render outputs."""

from __future__ import annotations

import argparse
import html
import json
import math
import subprocess
import textwrap
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps


SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_ROOT = SCRIPT_DIR.parent
PROJECT_ROOT = SCRIPTS_ROOT.parent
DEFAULT_SUMMARY = PROJECT_ROOT / "output" / "ci_web_smoke" / "summary.json"
DEFAULT_MANIFEST = PROJECT_ROOT / "web" / "demos" / "demos.json"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "web" / "render-gallery"

CATEGORY_ORDER = ["input", "layout", "navigation", "display", "feedback", "misc"]
IMAGE_LANCZOS = getattr(Image, "Resampling", Image).LANCZOS

PAGE_BG = "#f6f8fb"
CARD_BG = "#ffffff"
CARD_BORDER = "#d8dee8"
TEXT_STRONG = "#111827"
TEXT_MUTED = "#5b6472"
SECTION_BG = "#162033"
SECTION_TEXT = "#ffffff"


@dataclass
class GalleryEntry:
    name: str
    app: str
    widget_id: str
    category: str
    widget: str
    track: str
    reference: str
    screenshot: Path
    thumb: Path
    demo_href: str
    ok: bool
    non_black_ratio: float
    unique_colors: int


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Build a render-result gallery from web_smoke_check.py screenshots. "
            "Run web_smoke_check.py first when the summary does not exist."
        )
    )
    parser.add_argument("--summary", default=str(DEFAULT_SUMMARY), help="Path to web smoke summary.json.")
    parser.add_argument("--manifest", default="", help="Path to demos.json. Defaults to the summary manifest or web/demos/demos.json.")
    parser.add_argument("--output-dir", default=str(DEFAULT_OUTPUT_DIR), help="Output directory for gallery assets.")
    parser.add_argument("--columns", type=int, default=5, help="Columns in the contact sheet.")
    parser.add_argument("--tile-width", type=int, default=220, help="Preview tile width in the contact sheet.")
    parser.add_argument("--thumb-size", type=int, default=320, help="Square thumbnail size for the HTML gallery.")
    parser.add_argument("--include-failures", action="store_true", help="Include failed smoke entries when screenshots exist.")
    parser.add_argument("--title", default="HelloCustomWidgets Render Gallery", help="Gallery title.")
    return parser.parse_args()


def project_relative(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(PROJECT_ROOT)).replace("\\", "/")
    except ValueError:
        return str(path.resolve())


def web_relative(path: Path, base: Path) -> str:
    try:
        return str(path.resolve().relative_to(base.resolve())).replace("\\", "/")
    except ValueError:
        return project_relative(path)


def get_git_commit() -> str:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
    except Exception:
        return "unknown"
    return (result.stdout or "").strip() or "unknown"


def load_json(path: Path) -> dict | list:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve_manifest_path(summary: dict, manifest_arg: str) -> Path:
    if manifest_arg:
        return Path(manifest_arg).resolve()

    summary_manifest = summary.get("manifest")
    if summary_manifest:
        candidate = Path(str(summary_manifest))
        if candidate.exists():
            return candidate.resolve()

    return DEFAULT_MANIFEST.resolve()


def load_manifest_map(path: Path) -> dict[str, dict]:
    if not path.exists():
        return {}
    data = load_json(path)
    if not isinstance(data, list):
        raise RuntimeError(f"manifest must be a JSON array: {path}")
    return {str(item.get("name", "")): item for item in data if isinstance(item, dict)}


def sort_category_key(category: str) -> tuple[int, str]:
    try:
        return (CATEGORY_ORDER.index(category), category)
    except ValueError:
        return (len(CATEGORY_ORDER), category)


def display_words(value: str) -> str:
    words = str(value or "").replace("/", " ").replace("_", " ").replace("-", " ").split()
    return " ".join(word[:1].upper() + word[1:] for word in words)


def parse_widget_id(name: str, manifest_entry: dict | None) -> str:
    if manifest_entry:
        widget_id = manifest_entry.get("widgetId") or manifest_entry.get("appSub")
        if widget_id:
            return str(widget_id).replace("\\", "/")

    prefix = "HelloCustomWidgets_"
    raw = name[len(prefix) :] if name.startswith(prefix) else name
    parts = raw.split("_", 1)
    if len(parts) == 2:
        return parts[0] + "/" + parts[1]
    return raw.replace("_", "/")


def reference_label(manifest_entry: dict | None) -> str:
    if not manifest_entry:
        return ""
    pieces = [
        str(manifest_entry.get("referenceSystem", "") or ""),
        str(manifest_entry.get("referenceLibrary", "") or ""),
        str(manifest_entry.get("referenceComponent", "") or ""),
    ]
    pieces = [piece for piece in pieces if piece]
    return " / ".join(pieces)


def resolve_project_path(path_value: str) -> Path:
    path = Path(path_value)
    if path.is_absolute():
        return path
    return PROJECT_ROOT / path


def expand_bbox(bbox: list[int], image_size: tuple[int, int], margin: int = 20) -> tuple[int, int, int, int]:
    width, height = image_size
    left, top, right, bottom = bbox
    return (
        max(0, left - margin),
        max(0, top - margin),
        min(width, right + margin),
        min(height, bottom + margin),
    )


def fallback_bbox(image: Image.Image) -> tuple[int, int, int, int] | None:
    pixels = image.convert("RGB")
    width, height = pixels.size
    min_x = width
    min_y = height
    max_x = -1
    max_y = -1

    for y in range(height):
        for x in range(width):
            red, green, blue = pixels.getpixel((x, y))
            if red + green + blue > 36:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)

    if max_x < min_x or max_y < min_y:
        return None
    return expand_bbox([min_x, min_y, max_x + 1, max_y + 1], image.size)


def make_thumb(source_path: Path, bbox: list[int] | None, thumb_path: Path, thumb_size: int) -> None:
    with Image.open(source_path) as image:
        rgb = image.convert("RGB")
        crop_box = None
        if bbox and len(bbox) == 4:
            crop_box = expand_bbox([int(value) for value in bbox], rgb.size)
        if crop_box is None:
            crop_box = fallback_bbox(rgb)
        cropped = rgb.crop(crop_box) if crop_box else rgb
        preview = ImageOps.contain(cropped, (thumb_size - 24, thumb_size - 24), IMAGE_LANCZOS)

    canvas = Image.new("RGB", (thumb_size, thumb_size), PAGE_BG)
    x = (thumb_size - preview.width) // 2
    y = (thumb_size - preview.height) // 2
    canvas.paste(preview, (x, y))
    thumb_path.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(thumb_path)


def load_font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    candidates = [
        "C:/Windows/Fonts/segoeuib.ttf" if bold else "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
    ]
    for candidate in candidates:
        try:
            return ImageFont.truetype(candidate, size)
        except OSError:
            continue
    return ImageFont.load_default()


def wrap_label(text: str, max_chars: int) -> list[str]:
    words = str(text or "").replace("/", " / ").replace("_", " ").split()
    if not words:
        return [""]
    lines = textwrap.wrap(" ".join(words), width=max_chars, break_long_words=True, break_on_hyphens=False)
    return lines[:3] or [""]


def read_entries(summary_path: Path, output_dir: Path, args: argparse.Namespace) -> tuple[list[GalleryEntry], dict]:
    if not summary_path.exists():
        raise RuntimeError(f"summary not found: {summary_path}")

    summary = load_json(summary_path)
    if not isinstance(summary, dict):
        raise RuntimeError(f"summary must be a JSON object: {summary_path}")

    manifest = load_manifest_map(resolve_manifest_path(summary, args.manifest))
    results = summary.get("results", [])
    if not isinstance(results, list):
        raise RuntimeError("summary results must be a JSON array")

    entries: list[GalleryEntry] = []
    thumbs_dir = output_dir / "thumbs"
    for result in results:
        if not isinstance(result, dict):
            continue
        ok = bool(result.get("ok"))
        if not ok and not args.include_failures:
            continue

        name = str(result.get("name", ""))
        app = str(result.get("app", "HelloCustomWidgets") or "HelloCustomWidgets")
        screenshot_value = str(result.get("screenshot", "") or "")
        if not name or not screenshot_value:
            continue

        screenshot = resolve_project_path(screenshot_value)
        if not screenshot.exists():
            continue

        manifest_entry = manifest.get(name)
        widget_id = parse_widget_id(name, manifest_entry)
        category, _, widget = widget_id.partition("/")
        category = category or "misc"
        widget = widget or widget_id
        metrics = result.get("metrics", {}) if isinstance(result.get("metrics"), dict) else {}
        bbox = metrics.get("bbox") if isinstance(metrics.get("bbox"), list) else None
        thumb = thumbs_dir / f"{name}.png"
        make_thumb(screenshot, bbox, thumb, max(96, args.thumb_size))

        track = str((manifest_entry or {}).get("track", "reference") or "reference")
        entries.append(
            GalleryEntry(
                name=name,
                app=app,
                widget_id=widget_id,
                category=category,
                widget=widget,
                track=track,
                reference=reference_label(manifest_entry),
                screenshot=screenshot,
                thumb=thumb,
                demo_href=f"../demos/{name}/{app}.html",
                ok=ok,
                non_black_ratio=float(metrics.get("nonBlackRatio", 0.0) or 0.0),
                unique_colors=int(metrics.get("uniqueColors", 0) or 0),
            )
        )

    entries.sort(key=lambda item: (sort_category_key(item.category), display_words(item.widget).lower(), item.name))
    return entries, summary


def grouped_entries(entries: list[GalleryEntry]) -> list[tuple[str, list[GalleryEntry]]]:
    groups: dict[str, list[GalleryEntry]] = {}
    for entry in entries:
        groups.setdefault(entry.category, []).append(entry)
    return [(category, groups[category]) for category in sorted(groups, key=sort_category_key)]


def build_contact_sheet(entries: list[GalleryEntry], output_path: Path, title: str, columns: int, tile_width: int, commit: str) -> None:
    if not entries:
        raise RuntimeError("no render entries to compose")
    if columns <= 0:
        raise RuntimeError("columns must be greater than zero")

    title_font = load_font(30, bold=True)
    subtitle_font = load_font(15)
    section_font = load_font(20, bold=True)
    label_font = load_font(13, bold=True)
    meta_font = load_font(12)

    margin = 24
    gap = 14
    card_pad = 8
    label_height = 70
    header_height = 38
    section_gap = 24
    tile_width = max(120, tile_width)
    tile_height = tile_width
    card_width = tile_width + card_pad * 2
    card_height = tile_height + label_height + card_pad * 2
    sheet_width = margin * 2 + columns * card_width + (columns - 1) * gap

    groups = grouped_entries(entries)
    total_height = margin + 86
    for _, items in groups:
        rows = math.ceil(len(items) / columns)
        total_height += header_height + 10 + rows * card_height + max(0, rows - 1) * gap + section_gap
    total_height += margin - section_gap

    sheet = Image.new("RGB", (sheet_width, total_height), PAGE_BG)
    draw = ImageDraw.Draw(sheet)
    generated = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    draw.text((margin, margin), title, fill=TEXT_STRONG, font=title_font)
    draw.text((margin, margin + 42), f"{len(entries)} widgets | commit {commit} | generated {generated}", fill=TEXT_MUTED, font=subtitle_font)

    y = margin + 86
    for category, items in groups:
        draw.rounded_rectangle((margin, y, sheet_width - margin, y + header_height), radius=8, fill=SECTION_BG)
        draw.text((margin + 14, y + 8), f"{display_words(category)} ({len(items)})", fill=SECTION_TEXT, font=section_font)
        y += header_height + 10

        for index, entry in enumerate(items):
            row = index // columns
            col = index % columns
            x = margin + col * (card_width + gap)
            cy = y + row * (card_height + gap)
            draw.rounded_rectangle((x, cy, x + card_width, cy + card_height), radius=10, fill=CARD_BG, outline=CARD_BORDER, width=1)

            with Image.open(entry.thumb) as thumb:
                preview = ImageOps.contain(thumb.convert("RGB"), (tile_width, tile_height), IMAGE_LANCZOS)
            px = x + card_pad + (tile_width - preview.width) // 2
            py = cy + card_pad + (tile_height - preview.height) // 2
            sheet.paste(preview, (px, py))

            lx = x + card_pad + 4
            ly = cy + card_pad + tile_height + 8
            name_lines = wrap_label(display_words(entry.widget), 24)
            draw.multiline_text((lx, ly), "\n".join(name_lines), fill=TEXT_STRONG, font=label_font, spacing=3)
            meta = entry.reference or entry.widget_id
            meta_lines = wrap_label(meta, 28)[:1]
            draw.multiline_text((lx, ly + 42), "\n".join(meta_lines), fill=TEXT_MUTED, font=meta_font, spacing=2)

        rows = math.ceil(len(items) / columns)
        y += rows * card_height + max(0, rows - 1) * gap + section_gap

    output_path.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output_path)


def write_index(entries: list[GalleryEntry], output_dir: Path, sheet_path: Path, summary_path: Path, summary: dict, commit: str) -> Path:
    payload = {
        "generatedAt": datetime.now().isoformat(timespec="seconds"),
        "gitCommit": commit,
        "sourceSummary": project_relative(summary_path),
        "smokeGeneratedAt": summary.get("generatedAt", ""),
        "sheet": web_relative(sheet_path, output_dir),
        "total": len(entries),
        "categories": [
            {"id": category, "total": len(items)}
            for category, items in grouped_entries(entries)
        ],
        "entries": [
            {
                "name": entry.name,
                "widgetId": entry.widget_id,
                "category": entry.category,
                "widget": entry.widget,
                "track": entry.track,
                "reference": entry.reference,
                "thumbnail": web_relative(entry.thumb, output_dir),
                "demo": entry.demo_href,
                "nonBlackRatio": entry.non_black_ratio,
                "uniqueColors": entry.unique_colors,
            }
            for entry in entries
        ],
    }
    index_path = output_dir / "widget-render-gallery.json"
    index_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return index_path


def write_markdown(entries: list[GalleryEntry], output_dir: Path, sheet_path: Path, summary_path: Path) -> Path:
    lines = [
        "# HelloCustomWidgets Render Gallery",
        "",
        f"- Generated: `{datetime.now().isoformat(timespec='seconds')}`",
        f"- Source summary: `{project_relative(summary_path)}`",
        f"- Widgets: `{len(entries)}`",
        "",
        f"![HelloCustomWidgets render gallery]({web_relative(sheet_path, output_dir)})",
        "",
    ]
    for category, items in grouped_entries(entries):
        lines.extend([f"## {display_words(category)} ({len(items)})", ""])
        for entry in items:
            lines.append(f"- [{display_words(entry.widget)}]({entry.demo_href}) - `{entry.widget_id}`")
        lines.append("")
    markdown_path = output_dir / "README.md"
    markdown_path.write_text("\n".join(lines), encoding="utf-8")
    return markdown_path


def render_card(entry: GalleryEntry, output_dir: Path) -> str:
    title = html.escape(display_words(entry.widget))
    category = html.escape(display_words(entry.category))
    widget_id = html.escape(entry.widget_id)
    reference = html.escape(entry.reference or entry.track)
    thumb = html.escape(web_relative(entry.thumb, output_dir))
    demo = html.escape(entry.demo_href)
    return (
        '<a class="gallery-card" href="{demo}">'
        '<img src="{thumb}" alt="{title} render output" loading="lazy">'
        '<span class="gallery-title">{title}</span>'
        '<span class="gallery-meta">{category} &middot; {widget_id}</span>'
        '<span class="gallery-ref">{reference}</span>'
        "</a>"
    ).format(demo=demo, thumb=thumb, title=title, category=category, widget_id=widget_id, reference=reference)


def write_html(entries: list[GalleryEntry], output_dir: Path, sheet_path: Path, index_path: Path, markdown_path: Path, title: str, commit: str) -> Path:
    generated = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    sections = []
    for category, items in grouped_entries(entries):
        cards = "\n".join(render_card(entry, output_dir) for entry in items)
        sections.append(
            '<section class="gallery-section">'
            f'<h2>{html.escape(display_words(category))}<span>{len(items)}</span></h2>'
            f'<div class="gallery-grid">{cards}</div>'
            "</section>"
        )

    document = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>{html.escape(title)}</title>
    <link rel="icon" type="image/svg+xml" href="../favicon.svg">
    <style>
        * {{ box-sizing: border-box; }}
        body {{
            margin: 0;
            color: #172033;
            font-family: "Segoe UI Variable Text", "Segoe UI", Arial, sans-serif;
            background: #f6f8fb;
        }}
        a {{ color: inherit; text-decoration: none; }}
        .page {{
            max-width: 1480px;
            margin: 0 auto;
            padding: 28px 22px 44px;
        }}
        header {{
            display: grid;
            gap: 14px;
            margin-bottom: 24px;
        }}
        .eyebrow {{
            color: #0f6cbd;
            font-size: 0.78rem;
            font-weight: 700;
            letter-spacing: 0.08em;
            text-transform: uppercase;
        }}
        h1 {{
            margin: 0;
            color: #111827;
            font-size: 2rem;
            font-weight: 700;
        }}
        .summary {{
            margin: 0;
            color: #5b6472;
            line-height: 1.6;
        }}
        .actions {{
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
        }}
        .button {{
            display: inline-flex;
            align-items: center;
            justify-content: center;
            min-height: 40px;
            padding: 0 14px;
            border: 1px solid #d8dee8;
            border-radius: 8px;
            background: #ffffff;
            color: #172033;
            font-weight: 650;
        }}
        .button.primary {{
            border-color: #0f6cbd;
            background: #0f6cbd;
            color: #ffffff;
        }}
        .sheet-panel {{
            margin-bottom: 26px;
            padding: 16px;
            border: 1px solid #d8dee8;
            border-radius: 8px;
            background: #ffffff;
        }}
        .sheet-panel img {{
            display: block;
            width: 100%;
            height: auto;
            border-radius: 6px;
        }}
        .gallery-section {{
            margin-top: 24px;
        }}
        .gallery-section h2 {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 12px;
            margin: 0 0 12px;
            padding: 10px 12px;
            border-radius: 8px;
            background: #162033;
            color: #ffffff;
            font-size: 1rem;
        }}
        .gallery-section h2 span {{
            color: #c8d2e4;
            font-size: 0.9rem;
        }}
        .gallery-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(190px, 1fr));
            gap: 12px;
        }}
        .gallery-card {{
            display: grid;
            gap: 8px;
            min-width: 0;
            padding: 10px;
            border: 1px solid #d8dee8;
            border-radius: 8px;
            background: #ffffff;
        }}
        .gallery-card img {{
            width: 100%;
            aspect-ratio: 1;
            object-fit: contain;
            border-radius: 6px;
            background: #f6f8fb;
        }}
        .gallery-title {{
            color: #111827;
            font-weight: 700;
            line-height: 1.35;
        }}
        .gallery-meta,
        .gallery-ref {{
            color: #5b6472;
            font-size: 0.82rem;
            line-height: 1.4;
            overflow-wrap: anywhere;
        }}
        @media (max-width: 720px) {{
            .page {{ padding: 18px 12px 30px; }}
            h1 {{ font-size: 1.55rem; }}
            .sheet-panel {{ padding: 8px; }}
        }}
    </style>
</head>
<body>
    <main class="page">
        <header>
            <div class="eyebrow">Render overview</div>
            <h1>{html.escape(title)}</h1>
            <p class="summary">{len(entries)} widget screenshots generated from the WASM smoke check. Commit {html.escape(commit)}. Generated {html.escape(generated)}.</p>
            <div class="actions">
                <a class="button primary" href="{html.escape(web_relative(sheet_path, output_dir))}">Open contact sheet</a>
                <a class="button" href="{html.escape(web_relative(index_path, output_dir))}">JSON index</a>
                <a class="button" href="{html.escape(web_relative(markdown_path, output_dir))}">Markdown</a>
                <a class="button" href="../custom.html">Widget catalog</a>
            </div>
        </header>
        <section class="sheet-panel">
            <img src="{html.escape(web_relative(sheet_path, output_dir))}" alt="All widget render outputs">
        </section>
        {''.join(sections)}
    </main>
</body>
</html>
"""
    html_path = output_dir / "index.html"
    html_path.write_text(document, encoding="utf-8")
    return html_path


def main() -> int:
    args = parse_args()
    summary_path = Path(args.summary).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    entries, summary = read_entries(summary_path, output_dir, args)
    if not entries:
        raise RuntimeError("no screenshots found in the smoke summary")

    commit = get_git_commit()
    sheet_path = output_dir / "widget-render-gallery.png"
    build_contact_sheet(entries, sheet_path, args.title, args.columns, args.tile_width, commit)
    index_path = write_index(entries, output_dir, sheet_path, summary_path, summary, commit)
    markdown_path = write_markdown(entries, output_dir, sheet_path, summary_path)
    html_path = write_html(entries, output_dir, sheet_path, index_path, markdown_path, args.title, commit)

    print(f"Render gallery: {html_path}")
    print(f"Contact sheet: {sheet_path}")
    print(f"Index: {index_path}")
    print(f"Widgets: {len(entries)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
