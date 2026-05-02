# EmbeddedGUI Widgets Doc Outputs

This directory contains repository-local documentation helpers and generated doc assets.

## Widget Render Gallery

Generate a contact sheet for all HelloCustomWidgets render outputs:

```bash
python doc/scripts/generate_widget_render_gallery.py --build-wasm --run-smoke
```

For a faster local check against already captured smoke screenshots:

```bash
python doc/scripts/generate_widget_render_gallery.py --summary output/ci_web_smoke/summary.json
```

The generated files are written to `doc/source/images/`:

- `hello_custom_widgets_render_gallery.png`
- `hello_custom_widgets_render_gallery.md`

These files are generated artifacts and are intentionally ignored by git.
