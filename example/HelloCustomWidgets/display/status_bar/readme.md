# StatusBar 控件说明

## 保留理由

`StatusBar` 对齐 WPF 的底部状态栏语义，用于在应用窗口底部展示当前同步、编辑、运行或只读状态。它和 `TextBlock`、`Badge`、`InfoBar` 的职责不同：`TextBlock` 只负责文本，`Badge` 只表达短状态，`InfoBar` 是消息通知；`StatusBar` 需要在一个低高度容器里同时承载多个状态项，并保持稳定、低噪声的底部信息层。

## 目标场景

- 编辑器或配置页底部显示 `Ready / Line / Mode`。
- 构建、同步、连接状态同时展示。
- 只读或锁定页面保留状态可见性，但不提供交互入口。

## 视觉与布局

- 浅色圆角容器，1px 边框，内部按 item `weight` 横向分配区域。
- 每个 item 包含状态点、label 和 value；宽度不足时只保留 value。
- `normal / info / ok / warn` 状态通过低饱和颜色点表达，不使用大面积警示色。
- `emphasized` item 只增加浅色 pill 背景，不改变状态栏整体层级。
- 锁定、只读或禁用等场景由 APP 侧通过 palette、enable 与静态 preview 配置表达，控件本体不保存专用模式。

## 控件清单与状态矩阵

- `standard`：三段状态项，显示同步、行号和模式。
- `accent`：强调运行状态和 warning 状态。
- `telemetry`：APP 侧使用较窄尺寸和绿色 accent 表达密集状态摘要。
- `locked`：APP 侧使用 muted palette 或 disabled 状态表达锁定信息。
- `static preview`：底部两个静态对照由 APP 配置不同 palette、尺寸和 enable，touch/key 输入后状态保持不变。

## API 范围

- `egui_view_status_bar_set_items()` / `egui_view_status_bar_set_item()` 设置状态项。
- `egui_view_status_bar_set_fonts()` 设置 label/value 字体。
- `egui_view_status_bar_set_palette()` 设置颜色；不同业务状态由 APP 侧直接配置 palette、字体、尺寸和 enable。
- `egui_view_status_bar_get_item_region()` 供单测和后续命中区域验证使用。
- `egui_view_status_bar_override_static_preview_api()` 让 preview 吞掉输入并保持静态。

## 简化点

本实现不做 WPF `StatusBarItem` 的任意子控件托管，不提供菜单、进度条或交互按钮。当前控件只保留 reference 主线需要的分段状态呈现能力，避免把状态栏变成场景化 dashboard。

## 验收标准

- `make all APP=HelloCustomWidgets APP_SUB=display/status_bar PORT=pc`
- `make all APP=HelloUnitTest PORT=pc_test`
- `output\main.exe status_bar`
- `python scripts\checks\check_touch_release_semantics.py --scope custom --category display`
- `python scripts\checks\check_docs_encoding.py`
- `python scripts\checks\check_widget_catalog.py`
- `python scripts\sync_widget_catalog.py --check`
- `python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/status_bar --track reference --timeout 10 --keep-screenshots`
- `python scripts\code_compile_check.py --custom-widgets --category display --bits64`
- `python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
- `python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/status_bar`
- `python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_status_bar`

## 参考体系

- Reference system: Fluent 2 / WPF UI
- Reference library: WPF
- Reference component: `StatusBar`
