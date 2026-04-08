# EmbeddedGUI Widgets

Standalone repository for `HelloCustomWidgets`.

## SDK dependency model

This repository tracks the EmbeddedGUI SDK as a pinned submodule at `sdk/EmbeddedGUI`.
The default SDK resolution order is:

1. `--sdk-root`
2. `EMBEDDEDGUI_SDK_ROOT`
3. `sdk/EmbeddedGUI`
4. sibling `../EmbeddedGUI`

## Local setup

1. Initialize the SDK submodule:
   `git submodule update --init --recursive`
2. Install Python dependencies:
   `python -m pip install -r requirements.txt`
3. Make sure the local toolchain is available:
   `make` + GCC for `PORT=pc` builds. On Windows, use an environment that provides GNU Make and GCC.
4. Optional for web builds:
   Install Emscripten and make it available via `EMSDK_PATH`/`EMSDK`.

## Quick Start

1. Verify the SDK submodule and the PC toolchain:
   `make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc`
2. Run one local CI sweep for a category:
   `make ci CATEGORY=input`
3. If you prefer a direct Python entrypoint:
   `python scripts/ci_local_check.py --category input`
4. Build the web demos when you need the local site:
   `python scripts/web/wasm_build_demos.py`
5. To match the full widget CI locally, omit `CATEGORY`:
   `make ci`

## CI mapping

- Widget CI equivalent:
  `make ci CATEGORY=input`
- Unit-test only:
  `python scripts/code_compile_check.py --unit-tests-only --bits64`
- GitHub Pages build equivalent:
  `python scripts/web/wasm_build_demos.py`

## Common commands

- Build one widget on PC:
  `make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc`
- Run the local widget CI flow for one category:
  `make ci CATEGORY=input`
- Run the local widget CI flow without Make:
  `python scripts/ci_local_check.py --category input`
- Compile-check one category:
  `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
- Runtime-check one widget:
  `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/xy_pad --timeout 10 --keep-screenshots`
- Runtime-check one category:
  `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --bits64`
- Touch semantics audit:
  `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
- Build the widgets web bundle:
  `python scripts/web/wasm_build_demos.py`
- Preview the local site:
  `python web/start_server.py --port 8080`

## Repository layout

- `example/HelloCustomWidgets/`
  All widget sources, docs, and per-widget test entries.
- `example/HelloUnitTest/`
  Custom-widget unit-test harness.
- `scripts/`
  Widgets-specific compile/runtime/WASM helpers.
- `web/`
  Widgets-only GitHub Pages site.
- `.claude/workflow/`
  Widget workflow and tracker docs.

## Notes

- `iteration_log/` is local-only review evidence and stays ignored.
- The repo root `Makefile` forwards build requests into `sdk/EmbeddedGUI` while keeping outputs in this repository.
