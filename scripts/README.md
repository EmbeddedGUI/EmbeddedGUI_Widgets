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
  Release-style manual check entrypoint that chains touch audit, compile, unit tests, runtime verification, WASM build verification, and browser-side web smoke checks.
- `ci_local_check.py`
  Fast local CI wrapper for one widget category or the full widget set.

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

## Principle

- Keep high-frequency developer entrypoints at `scripts/`.
- Keep specialized checks under `scripts/checks/`.
- Keep web publishing logic under `scripts/web/`.
