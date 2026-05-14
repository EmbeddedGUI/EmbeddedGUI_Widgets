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
   `make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc`
3. Run one local CI sweep for a category:
   `make ci CATEGORY=input`
4. If you prefer a direct Python entrypoint:
   `python scripts/ci_local_check.py --category input`
   This includes widget catalog policy, documentation/stale-reference, and web artifact consistency checks automatically.
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
  `python scripts/web/wasm_build_demos.py`, then `python scripts/web/web_smoke_check.py`, then `python scripts/web/widget_render_gallery.py --summary output/ci_web_smoke/summary.json`

## Common commands

- Bootstrap the local environment:
  Windows: `setup.bat`
  Linux / macOS: `./setup.sh`
- Build from Visual Studio:
  Open `EmbeddedGUI_Widgets.sln` and build `HelloCustomWidgets` with `Debug|x64`, `Release|x64`, or one of the generated `HCW_<category>_<widget>_Debug|x64` / `HCW_<category>_<widget>_Release|x64` configurations. The solution uses the root `Makefile`, so Visual Studio builds follow the same SDK submodule and app source layout as command-line builds.
  This requires the Visual Studio C++ workload plus a working `make`/`gcc` toolchain on `PATH`; set `EGUI_MAKE` if `make` is installed under a custom path. PC simulator logs are routed to the Visual Studio Output window when launched under the debugger.
- Bootstrap from Python directly:
  `python scripts/setup_env.py --skip-emsdk`
- Build one widget on PC:
  `make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc`
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
  `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --timeout 10 --keep-screenshots`
- Runtime-check one category:
  `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --bits64`
- Touch semantics audit:
  `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
- Widget catalog policy audit:
  `python scripts/checks/check_widget_catalog.py`
- Documentation quality and stale-reference audit:
  `python scripts/checks/check_docs_encoding.py`
- Web artifact consistency audit:
  `python scripts/checks/check_web_artifacts.py`
  This also verifies bundled demo README synchronization and render-gallery HTML/Markdown links.
- Web artifact consistency audit for generated release output:
  `python scripts/checks/check_web_artifacts.py --web-root output/release_check_wasm --manifest output/release_check_wasm/demos/demos.json --render-gallery output/release_check_wasm/render-gallery`
- Web artifact consistency audit for one generated category:
  `python scripts/checks/check_web_artifacts.py --web-root output/release_check_wasm --manifest output/release_check_wasm/demos/demos.json --render-gallery output/release_check_wasm/render-gallery --category input`
- Python helper syntax audit:
  `python -m py_compile scripts/ci_local_check.py scripts/code_compile_check.py scripts/code_runtime_check.py scripts/release_check.py scripts/run_app.py scripts/setup_env.py scripts/start_app.py scripts/sync_widget_catalog.py scripts/widget_catalog.py scripts/windows_link_rsp.py scripts/vs/generate_vs_configs.py web/start_server.py example/HelloCustomWidgets/create_custom_widget.py scripts/checks/check_docs_encoding.py scripts/checks/check_web_artifacts.py scripts/checks/check_widget_catalog.py scripts/checks/check_touch_release_semantics.py`
- Web publishing helper syntax audit:
  `python -m py_compile scripts/widget_catalog.py scripts/checks/check_docs_encoding.py scripts/checks/check_web_artifacts.py scripts/web/wasm_build_demos.py scripts/web/web_smoke_check.py scripts/web/widget_render_gallery.py doc/scripts/generate_widget_render_gallery.py`
- Sync widget catalog:
  `python scripts/sync_widget_catalog.py`
- Build the widgets web bundle:
  `python scripts/web/wasm_build_demos.py`
- Generate the widget render overview for docs:
  `python doc/scripts/generate_widget_render_gallery.py --build-wasm --run-smoke`
- Preview the local site:
  `python web/start_server.py --port 8080`

## Repository layout

- `example/HelloCustomWidgets/`
  All widget sources, docs, per-widget test entries, and the app root multi-display entry (`uicode_disp0.c` / `uicode_disp0.h`).
- `example/HelloUnitTest/`
  Custom-widget unit-test harness with the same app root multi-display entry structure.
- `scripts/`
  Widgets-specific compile/runtime/WASM helpers.
- `doc/`
  Repository-local documentation helper scripts and generated doc output locations.
- `web/`
  Widgets-only GitHub Pages site.
- `.claude/workflow/`
  Widget workflow and tracker docs.

## Notes

- `iteration_log/` is local-only review evidence and stays ignored.
- The repo root `Makefile` forwards build requests into `sdk/EmbeddedGUI` while keeping outputs in this repository.
- `HelloCustomWidgets` and `HelloUnitTest` now follow the multi-display SDK directly through `uicode_disp0.c` / `uicode_disp0.h`.
- Root convenience entrypoints:
  `setup.bat` / `setup.sh`, `make setup`, `make release-check`, `make sync-catalog`
