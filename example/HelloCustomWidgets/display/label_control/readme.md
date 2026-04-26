# Label 控件说明

## 保留理由

`Label` 对齐 WPF 的控件标题语义，用于为输入框、选择器或局部工具提供稳定的 caption。它和 `TextBlock` 不同：`TextBlock` 只是文本呈现，`Label` 需要表达目标控件、access key、必填状态和只读状态；它也不同于 `Field`，后者是完整表单字段容器，而 `Label` 是更轻量的标题控件。

## 目标场景

- 表单项标题，例如 `Username`、`Mode`、`Locked field`。
- 为目标控件显示 access key 下划线提示。
- 用低噪声标记表达必填或只读状态。
- 在 compact 布局里只保留标题，不显示辅助 hint。

## 视觉与布局规格

- 浅色圆角 surface，左侧保留窄 accent rail。
- 主文本使用 label 字体，辅助 hint 使用更小字号和 muted 颜色。
- `required` 使用小圆点标记，不使用高噪声大面积警示色。
- `target_highlighted` 只增加浅色背景和边框强化，不把 label 做成按钮。
- `read only` 降低文本、边框、required 与 access key 的对比度。

## 状态矩阵

- `standard`：显示 caption、target hint、access key 和 required marker。
- `accent`：强调目标控件已关联或当前聚焦。
- `compact`：压缩高度，只保留 caption。
- `read only`：muted 视觉，保留 caption 与只读语义。
- `static preview`：底部 compact / read only 两个静态对照，touch/key 后状态保持不变。

## 录制动作设计

录制轨道只改变主控件状态：

1. `Caption / access key`
2. `Accent / target highlighted`
3. `Compact / caption only`
4. `Read only / muted`
5. 回到默认稳定帧

底部 preview 全程保持静态，用来对照 compact 与 read only 两种低高度状态。

## API 范围

- `egui_view_label_control_set_text()` 设置 caption。
- `egui_view_label_control_set_target_hint()` 设置目标提示。
- `egui_view_label_control_set_access_key_index()` 设置 access key 下划线位置。
- `egui_view_label_control_set_required()` 设置必填标记。
- `egui_view_label_control_set_target_highlighted()` 设置目标强调态。
- `egui_view_label_control_apply_standard_style()` / `apply_accent_style()` / `apply_compact_style()` / `apply_read_only_style()` 应用预设样式。
- `egui_view_label_control_override_static_preview_api()` 让静态 preview 吞掉输入并保持状态不变。

## 简化点

本控件不实现 WPF `Target` 的真实焦点跳转，也不解析 `_` 访问键标记；EGUI 适配中使用显式 `access_key_index` 控制下划线位置。它不承载任意 content，也不提供表单校验消息，避免和 `Field`、`ContentControl` 产生职责重叠。

## 验收标准

- `make all APP=HelloCustomWidgets APP_SUB=display/label_control PORT=pc`
- `make all APP=HelloUnitTest PORT=pc_test`
- `output\main.exe label_control`
- `python scripts\checks\check_touch_release_semantics.py --scope custom --category display`
- `python scripts\checks\check_docs_encoding.py`
- `python scripts\checks\check_widget_catalog.py`
- `python scripts\sync_widget_catalog.py --check`
- `python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/label_control --track reference --timeout 10 --keep-screenshots`
- `python scripts\code_compile_check.py --custom-widgets --category display --bits64`
- `python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
- `python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/label_control`
- `python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_label_control`

## 参考体系

- Reference system: Fluent 2 / WPF UI
- Reference library: WPF
- Reference component: `Label`
