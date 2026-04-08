# EmbeddedGUI Widgets

Standalone repository for `HelloCustomWidgets`.

Online preview: <https://embeddedgui.github.io/EmbeddedGUI_Widgets/>

HelloCustomWidgets demos now use a default `480 x 480` showcase canvas.
All widget demos follow one presentation rule: title only, standard at top, compact at left, read-only/locked at right.

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
2. Use the setup entrypoint that matches your platform:
   Windows: `setup.bat`
   Linux / macOS: `./setup.sh`
3. If you only need the native PC flow, you can skip Emscripten:
   `python scripts/setup_env.py --skip-emsdk`
4. Manual fallback:
   `python -m pip install -r requirements.txt`

## Quick Start

1. Bootstrap the environment:
   Windows: `setup.bat`
   Linux / macOS: `./setup.sh`
2. Verify the SDK submodule and the PC toolchain:
   `make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc`
3. Run one local CI sweep for a category:
   `make ci CATEGORY=input`
4. If you prefer a direct Python entrypoint:
   `python scripts/ci_local_check.py --category input`
5. Run one release-style manual sweep:
   `python scripts/release_check.py`
6. Build the web demos when you need the local site:
   `python scripts/web/wasm_build_demos.py`
7. To match the full widget CI locally, omit `CATEGORY`:
   `make ci`

## CI mapping

- Widget CI equivalent:
  `make ci CATEGORY=input`
- Unit-test only:
  `python scripts/code_compile_check.py --unit-tests-only --bits64`
- Release-style manual full check:
  `python scripts/release_check.py`
- GitHub Pages build equivalent:
  `python scripts/web/wasm_build_demos.py`

## Common commands

- Bootstrap the local environment:
  Windows: `setup.bat`
  Linux / macOS: `./setup.sh`
- Bootstrap from Python directly:
  `python scripts/setup_env.py --skip-emsdk`
- Build one widget on PC:
  `make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc`
- Default widget showcase canvas:
  `480 x 480`
- Run the local widget CI flow for one category:
  `make ci CATEGORY=input`
- Run the local widget CI flow without Make:
  `python scripts/ci_local_check.py --category input`
- Run the release-style manual check:
  `python scripts/release_check.py`
- Run the release-style manual check for one category:
  `python scripts/release_check.py --category input --skip wasm`
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
- Root convenience entrypoints:
  `setup.bat` / `setup.sh`, `make setup`, `make release-check`
