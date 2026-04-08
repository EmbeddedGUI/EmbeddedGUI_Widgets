# Scripts Layout

This repository keeps only the scripts needed for standalone custom-widget work.

## Top-level entrypoints

- `code_compile_check.py`
  Batch compile-check for widget demos and the custom-widget unit-test harness.
- `code_runtime_check.py`
  Runtime screenshot verification for widget demos.

## Subdirectories

- `checks/`
  Focused validation helpers such as touch release semantics auditing.
- `web/`
  WASM build and GitHub Pages packaging helpers for the widgets-only site.

## Principle

- Keep high-frequency developer entrypoints at `scripts/`.
- Keep specialized checks under `scripts/checks/`.
- Keep web publishing logic under `scripts/web/`.
