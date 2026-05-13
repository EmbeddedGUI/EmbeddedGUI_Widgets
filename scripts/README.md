# Scripts Layout

This repository keeps only the scripts needed for standalone custom-widget work.

## Top-level entrypoints

- `setup_env.py`
  Local environment bootstrap for Python dependencies, native build tools, and optional Emscripten setup.
- `code_compile_check.py`
  Batch compile-check for widget demos and the custom-widget unit-test harness.
- `code_runtime_check.py`
  Runtime screenshot verification for widget demos.
- `release_check.py`
  Release-style manual check entrypoint that chains catalog/docs audits, touch audit, compile, unit tests, runtime verification, WASM build verification, browser-side web smoke checks, and render gallery generation.
- `ci_local_check.py`
  Fast local CI wrapper for one widget category or the full widget set, including catalog and documentation encoding checks.

## App entry note

- `HelloCustomWidgets` and `HelloUnitTest` use `uicode_disp0.c` / `uicode_disp0.h` directly for the multi-display SDK entry.
- New widget demos, runtime recording hooks, and shared entry wiring should continue to go through `uicode_disp0.h`.

## Subdirectories

- `checks/`
  Focused validation helpers such as touch release semantics auditing and documentation encoding checks.
- `web/`
  WASM build and GitHub Pages packaging helpers for the widgets-only site.

## Useful checks

- `python scripts/checks/check_touch_release_semantics.py --scope custom`
  Audit widget touch release behavior.
- `python scripts/checks/check_docs_encoding.py`
  Validate documentation files decode as UTF-8 and catch obvious README corruption such as repeated `????`.
- `python scripts/checks/check_widget_catalog.py`
  Validate `widget_catalog.json` coverage, track-to-visibility policy, reference metadata completeness, and replacement targets.
- `python scripts/sync_widget_catalog.py`
  Rewrite `widget_catalog.json` into canonical order and sync it with actual `HelloCustomWidgets` directories.
- `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing`
  Refresh `web/demos/demos.json` and bundled README files from existing web demo artifacts without rebuilding WASM.
- `python scripts/web/web_smoke_check.py`
  Run headless browser smoke checks for `web/demos/`, capture one screenshot per demo, and generate `summary.json`, `summary.md`, and a contact sheet for manual visual review. The script auto-adds Linux CI-friendly browser flags on GitHub Actions and also accepts repeatable `--browser-arg`.
- `python scripts/web/widget_render_gallery.py --summary output/ci_web_smoke/summary.json`
  Compose all smoke-check screenshots into a widget render gallery under `web/render-gallery/`, including a contact sheet, JSON index, Markdown index, and HTML page for GitHub Pages.
- `python scripts/release_check.py --category input --skip wasm,web_smoke,render_gallery`
  Run the native release checks for one category while explicitly skipping the heavier web publishing stages.
- `python doc/scripts/generate_widget_render_gallery.py --build-wasm --run-smoke`
  Build HelloCustomWidgets WASM demos, capture one browser screenshot per widget, and generate the doc-side contact sheet under `doc/source/images/`.
- `python scripts/web/widget_render_gallery.py --summary output/ci_web_smoke/summary.json --doc-output-dir doc/source/images`
  Reuse an existing smoke summary and generate both the web gallery and the doc-side render overview.

## Principle

- Keep high-frequency developer entrypoints at `scripts/`.
- Keep specialized checks under `scripts/checks/`.
- Keep web publishing logic under `scripts/web/`.
