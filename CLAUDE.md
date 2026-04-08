# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository purpose

`EmbeddedGUI_Widgets` is the standalone home for `HelloCustomWidgets`.
The EmbeddedGUI SDK is consumed as a pinned git submodule under `sdk/EmbeddedGUI`.

## Setup

```bash
git submodule update --init --recursive
python -m pip install -r requirements.txt
```

SDK resolution order for local tooling:

1. `--sdk-root`
2. `EMBEDDEDGUI_SDK_ROOT`
3. `sdk/EmbeddedGUI`
4. sibling `../EmbeddedGUI`

## Common commands

```bash
# Build one widget demo on PC
make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc

# Run a compile sweep for one category
python scripts/code_compile_check.py --custom-widgets --category input --bits64

# Run runtime verification for one widget
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/xy_pad --timeout 10 --keep-screenshots

# Run runtime verification for one category
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --bits64

# Audit touch release semantics
python scripts/checks/check_touch_release_semantics.py --scope custom --category input

# Build all widget WASM demos into web/demos/
python scripts/web/wasm_build_demos.py

# Serve the local web site
python web/start_server.py --port 8080
```

## Architecture

- `sdk/EmbeddedGUI/`
  Pinned SDK submodule. Do not edit it for widget-repo-only changes unless the task explicitly requires an SDK change.
- `example/HelloCustomWidgets/`
  All custom widgets, still organized as `category/widget`.
- `example/HelloUnitTest/`
  Minimal unit-test harness for custom widgets that need direct test coverage.
- `scripts/`
  Widgets-focused compile/runtime/touch/WASM helpers.
- `web/`
  Widgets-only GitHub Pages site driven by `web/demos/demos.json`.
- `.claude/workflow/`
  Widget acceptance workflow and progress tracker.

## Workflow rules

- Use simplified Chinese for discussion.
- Keep code in UTF-8 and avoid Chinese in source code.
- `iteration_log/` remains local-only evidence and must not be committed.
- For widget implementation or iteration tasks, read:
  - `.claude/workflow/widget_acceptance_workflow.md`
  - `.claude/workflow/widget_progress_tracker.md`
  - relevant files in `.claude/skills/`

## Build model

The repo root `Makefile` is a thin wrapper around `sdk/EmbeddedGUI/Makefile`.
All builds are executed against the SDK with:

- app sources from this repository's `example/`
- outputs written to this repository's `output/`

Do not assume this repository is itself the SDK root.
