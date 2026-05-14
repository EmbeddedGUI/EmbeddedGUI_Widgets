---
name: github-pages-web
description: Use when publishing HelloCustomWidgets demos to GitHub Pages, refreshing web artifacts, or checking the render gallery
---

# GitHub Pages Web Publishing Skill

EmbeddedGUI Widgets 的在线 demo 站点通过 GitHub Pages 发布，源码和已提交产物在 `web/` 目录。当前站点围绕 `HelloCustomWidgets` catalog 和 Fluent 2 / WPF UI reference 主线组织。

## 架构概览

```
web/
├── index.html                 # 站点入口，展示 catalog/policy 摘要
├── custom.html                # HelloCustomWidgets 交互式目录与 iframe 预览页
├── custom-page.js             # custom.html 的 manifest 加载、筛选和预览逻辑
├── index-page.js              # index.html 的统计和入口卡片逻辑
├── i18n.js                    # 中英文文本工具
├── style.css                  # 站点样式
├── doc-render.js              # README Markdown 渲染工具
├── catalog-policy.json        # 从 widget catalog 同步出的发布策略摘要
├── demos/
│   ├── demos.json             # web 目录 manifest
│   └── HelloCustomWidgets_<category>_<widget>/
│       ├── HelloCustomWidgets.html
│       ├── HelloCustomWidgets.js
│       ├── HelloCustomWidgets.wasm
│       └── README.md
└── render-gallery/
    ├── index.html
    ├── README.md
    ├── widget-render-gallery.json
    ├── widget-render-gallery.png
    └── thumbs/
```

`custom.html` 运行时读取 `demos/demos.json`，按 `track`、搜索关键词和 hash 选择 demo。一般不要手动编辑 manifest；优先通过脚本刷新。

## demos.json 字段

每个条目由 `scripts/web/wasm_build_demos.py` 从 build 结果和 widget catalog 生成，典型结构：

```json
{
  "name": "HelloCustomWidgets_input_button",
  "app": "HelloCustomWidgets",
  "category": "HelloCustomWidgets",
  "width": 480,
  "height": 480,
  "appSub": "input/button",
  "doc": "demos/HelloCustomWidgets_input_button/README.md",
  "widgetId": "input/button",
  "track": "reference",
  "visibility": "public",
  "referenceSystem": "Fluent 2",
  "referenceLibrary": "WPF UI",
  "referenceComponent": "Button",
  "replacement": null
}
```

关键约定：

- `name` 是 `web/demos/` 下的目录名，也是 `custom.html#<name>` 的 hash 目标。
- `appSub` / `widgetId` 使用 `category/widget` 形式。
- 默认网页目录只发布 catalog 策略允许的 `reference` 条目。
- bundled `README.md` 必须与 `example/HelloCustomWidgets/<category>/<widget>/readme.md` 保持同步。

## 刷新 Web Demo

```bash
# 全量构建默认 reference web demo
python scripts/web/wasm_build_demos.py

# 只构建一个 widget
python scripts/web/wasm_build_demos.py --app-sub input/auto_suggest_box

# 构建一个显式 track
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --track reference

# 不重建 WASM，只用现有 web/demos 产物刷新 demos.json 和 bundled README
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --track reference --refresh-existing
```

脚本会：

- 调用 SDK 的 Emscripten 构建。
- 把产物复制到 `web/demos/HelloCustomWidgets_<category>_<widget>/`。
- 复制源 README 到 bundled `README.md`。
- 生成或合并 `web/demos/demos.json`。
- 默认全站构建时清理不应保留的旧 `HelloCustomWidgets_*` demo 目录。

## Web Smoke 和 Render Gallery

```bash
# 对已提交 web/demos 运行浏览器 smoke 检查
python scripts/web/web_smoke_check.py

# 只检查一个 demo
python scripts/web/web_smoke_check.py --demo HelloCustomWidgets_input_auto_suggest_box

# 从 smoke summary 生成 web/render-gallery
python scripts/web/widget_render_gallery.py --summary output/ci_web_smoke/summary.json --output-dir web/render-gallery

# 构建 WASM、运行 smoke，并生成 gallery
python scripts/web/widget_render_gallery.py --build-wasm --run-smoke
```

`web_smoke_check.py` 会启动本地静态服务器，逐个打开 manifest 中的 demo，输出截图、`summary.json`、`summary.md` 和 contact sheet。`widget_render_gallery.py` 使用 smoke 截图生成 `render-gallery` 的 HTML、Markdown、JSON、缩略图和总览图。

## Artifact 一致性检查

```bash
# 检查已提交 web/demos 和 web/render-gallery
python scripts/checks/check_web_artifacts.py

# 检查 release_check.py 生成的输出目录
python scripts/checks/check_web_artifacts.py --web-root output/release_check_wasm --manifest output/release_check_wasm/demos/demos.json --render-gallery output/release_check_wasm/render-gallery

# 检查 category-scoped release 输出
python scripts/checks/check_web_artifacts.py --web-root output/release_check_wasm --manifest output/release_check_wasm/demos/demos.json --render-gallery output/release_check_wasm/render-gallery --category input
```

这个检查会验证 manifest、demo 目录、WASM/JS/HTML 文件、bundled README、render-gallery JSON、缩略图、HTML/Markdown 链接和分类计数是否与 catalog 同步。

## 发布工作流

`.github/workflows/wasm-deploy.yml` 只在 `workflow_dispatch` 手动触发时部署 GitHub Pages。流程为：

```
checkout + submodules
  └─ setup emsdk / Python / Chrome
       ├─ python -m py_compile ...
       ├─ python scripts/checks/check_docs_encoding.py
       ├─ python scripts/web/wasm_build_demos.py
       ├─ python scripts/web/web_smoke_check.py --capture-mode cdp
       ├─ python scripts/web/widget_render_gallery.py --summary ...
       ├─ python scripts/checks/check_web_artifacts.py
       └─ actions/deploy-pages
```

本地预览：

```bash
python web/start_server.py --port 8080
```

浏览器访问 `http://localhost:8080`。

## 修改 Web 页面时

- `index.html` 和 `custom.html` 保持数据驱动，新增字段优先从 `demos.json` 或 `catalog-policy.json` 读取。
- 页面文案在 `index-page.js`、`custom-page.js` 和 `i18n.js` 中维护，保持中英文同时可用。
- 如果新增 manifest 字段，要同步更新 `scripts/checks/check_web_artifacts.py` 的必要校验。
- 修改 gallery 输出结构时，同步检查 `scripts/web/widget_render_gallery.py`、`doc/scripts/generate_widget_render_gallery.py` 和 `check_web_artifacts.py`。

## WASM Demo 注意事项

- `HelloCustomWidgets` / `HelloUnitTest` 的 app 根入口直接使用 `uicode_disp0.c` / `uicode_disp0.h`。
- 录制动作可以放在 demo 的 `test.c` 中，并用 `#if EGUI_CONFIG_RECORDING_TEST` 包裹；WASM demo 不依赖录制自动播放。
- 有 `resource/` 目录的 widget 由构建系统自动生成和打包资源，demo 代码通常不需要为 WASM 单独处理。
- 默认 web 目录应保持 reference 主线；历史轨道只在显式 `--track showcase` / `--track deprecated` 构建时回看。
