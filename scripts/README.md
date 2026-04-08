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
  Release-style manual check entrypoint that chains touch audit, compile, unit tests, runtime verification, and WASM build verification.
- `ci_local_check.py`
  Fast local CI wrapper for one widget category or the full widget set.

## Subdirectories

- `checks/`
  Focused validation helpers such as touch release semantics auditing.
- `web/`
  WASM build and GitHub Pages packaging helpers for the widgets-only site.

## Principle

- Keep high-frequency developer entrypoints at `scripts/`.
- Keep specialized checks under `scripts/checks/`.
- Keep web publishing logic under `scripts/web/`.
