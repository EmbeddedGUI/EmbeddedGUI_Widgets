# ContentControl

`ContentControl` 是 WPF / WinUI 布局体系里的单内容宿主，用来承载一个 child，并通过 `HorizontalContentAlignment`、`VerticalContentAlignment`、`Padding` 等属性决定内容在宿主中的位置。它不承担边框装饰的全部语义，也不表达业务卡片，只提供“一个内容放到一个容器槽位里”的基础能力。

## 1. 为什么需要这个控件

已有 `Border` 更偏向背景、边框、圆角和 padding；`ContentControl` 关注的是单内容承载与对齐。很多复杂控件都需要先把内容放进一个稳定宿主，再由外层决定是否加边框、卡片、弹层或列表语义。

## 2. 为什么现有控件不够用

`StackPanel`、`Grid`、`Canvas` 负责多子元素布局；`Border` 负责可见边界；`CardControl` 带有卡片语义。它们都不能单独表达 WPF / WinUI 的 `ContentControl`：只有一个 content，支持水平 / 垂直内容对齐，并保留轻量 content slot。

## 3. 目标场景与示例概览

本示例保留四组主区状态：

- `Center / padding 16`
- `Left / vertical center`
- `Compact / top left`
- `Read only / muted content`

底部保留 `compact` 与 `read only` 两个静态 preview，验证 preview 消费输入但不改变状态。

## 4. 视觉与布局规格

宿主使用浅色 surface、低对比边框、细 accent slot guide 和克制圆角。默认尺寸为 `168 x 94`，主区 child 使用 `116 x 24` 文本内容。compact 模式收紧到 `padding 8 / 6` 与 `radius 6`，read only 模式降低边框、accent 和文字对比。

## 5. 控件清单与状态矩阵

- `egui_view_content_control_t`
- 单一 `child`
- `surface_color / border_color / accent_color`
- `corner_radius / border_width`
- `content_align_type`
- `compact_mode`
- `read_only_mode`

## 6. 录制动作设计

录制轨道只切换主区快照：默认 center、leading、compact、read only，并最终回到默认 center。底部 preview 全程静态，用于观察 compact / read only reference 对照。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/content_control PORT=pc`
- `make all APP=HelloUnitTest PORT=pc_test`
- `output\main.exe content_control`
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
- `python scripts/checks/check_docs_encoding.py`
- `python scripts/checks/check_widget_catalog.py`
- `python scripts/sync_widget_catalog.py --check`
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/content_control --track reference --timeout 10 --keep-screenshots`
- `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
- `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/content_control`
- `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_content_control`

截图必须确认主区四态可见、底部 preview 静态、没有黑屏 / 白屏 / 主体缺失 / 文本重叠 / 裁切。

## 8. 参考设计体系与开源母本

参考 WPF / WinUI `ContentControl` 的核心语义：单一 `Content`、`Padding`、`HorizontalContentAlignment`、`VerticalContentAlignment` 与只读/禁用视觉弱化。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. catalog 元数据

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF / WinUI`
- `reference_component`: `ContentControl`

## 10. 保留与删除

保留单内容宿主、内容对齐、padding、compact、read only 和静态 preview。删除业务卡片、复杂模板选择、资源绑定、动画切换和多 child panel 行为，避免与 `Grid`、`StackPanel`、`Border` 或 `CardControl` 混淆。

## 11. EGUI 适配简化点

当前实现只在 custom widget 层提供轻量 `egui_view_group_t` 派生版本，不下沉到 SDK。内容模板简化为一个 child 指针；对齐通过 `content_align_type` 映射到现有 `EGUI_ALIGN_*`；静态 preview 通过覆盖 API 消费 touch / key 并保持状态。
