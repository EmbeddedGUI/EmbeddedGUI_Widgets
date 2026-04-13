# scroll_viewer 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`、`WPF UI`
- 平台语义参考：`ScrollViewer`
- 补充对照控件：`scroll_bar`、`data_list_panel`、`card_control`
- 对应组件名：`ScrollViewer`
- 计划保留状态：`standard`、`compact`、`disabled`、`scrollbars visible`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_scroll_viewer`，优先收口单容器滚动 surface、viewport 指标与静态 preview，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`scroll_viewer` 用来表达“内容区域本身可滚动，而不是只摆一个滚动条”的标准语义。它适合长设置页、诊断摘要、富文本说明、卡片列表等内容超过单屏高度、但又不想绑定到特定数据容器的场景。

## 2. 为什么现有控件不够用

- `scroll_bar` 只覆盖滚动条本身，不承担 viewport、内容裁切与滚动 surface 的完整语义。
- `data_list_panel` 更偏向列表数据容器，不适合表达任意内容块的通用滚动区域。
- `split_view`、`card_panel` 这类控件能承载内容，但不直接提供 Fluent / WinUI `ScrollViewer` 的 reference 验证闭环。
- 当前仓库还没有一个符合 `ScrollViewer` 语义的 reference 页面、单测与 web 验证路径。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `scroll_viewer`：固定 viewport 内承载多段说明卡片、状态标签和长文本。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的滚动容器密度。
- 底部右侧展示 `disabled` 静态 preview，用于对照禁用后滚动条与内容层级的弱化表现。
- 首轮录制动作优先覆盖 `focus -> vertical scroll -> page jump -> preview dismiss` 主链路。

目录：
- `example/HelloCustomWidgets/layout/scroll_viewer/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 160` 左右，用于同时容纳 viewport、内容块和滚动条轨道。
- 主体保持浅色 Fluent 卡片容器，内部保留：
  - 顶部 viewport 标题 / 辅助信息
  - 中部滚动内容 surface
  - 右侧垂直滚动条与可选底部水平滚动指示
- 需要额外保留一层低噪音 scroll 信息，例如：
  - 当前 snapshot 名称
  - 垂直 / 水平 offset
  - viewport 与 content extent

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 284` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Scroll Viewer` | 页面标题 |
| `scroll_viewer_primary` | `egui_view_scroll_viewer_t` | `196 x 160` | `standard` | 主 reference 控件 |
| `scroll_viewer_compact` | `egui_view_scroll_viewer_t` | `104 x 88` | compact | 紧凑静态 preview |
| `scroll_viewer_disabled` | `egui_view_scroll_viewer_t` | `104 x 88` | compact + disabled | 禁用静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview top` | 初始停留在顶部摘要区 |
| 主控件 | `Mid content` | 滚动到中部卡片区 |
| 主控件 | `Bottom summary` | 跳到尾部总结区 |
| 主控件 | `Scrollbar drag` | 滚动 thumb 拖拽中的连续交互态 |
| `compact` | `Compact preview` | 窄尺寸滚动容器对照 |
| `disabled` | `Disabled preview` | 禁用弱化后的静态对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到滚动 surface，保留真实 viewport 滚动链路。
- 首版聚焦以下键盘语义：
  - `Up / Down`：按行滚动。
  - `PageUp / PageDown`：按页滚动。
  - `Home / End`：跳到顶部 / 底部。
  - `Left / Right`：在存在水平 overflow 时调整水平 offset。
  - `Tab`：在 surface 与滚动条可聚焦部件之间移动。
- 触摸语义区分两类：
  - 点击滚动条按钮或轨道时保持 same-target release。
  - 拖拽 thumb 属于连续交互，需要按例外路径记录 release allowlist。
- `set_snapshots()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()` 都要清理旧的 pressed / drag 状态。
- `disabled` 与 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_scroll_viewer_init()`
- `egui_view_scroll_viewer_set_snapshots()/get_current_snapshot()`
- `egui_view_scroll_viewer_set_current_snapshot()`
- `egui_view_scroll_viewer_set_vertical_offset()/get_vertical_offset()`
- `egui_view_scroll_viewer_set_horizontal_offset()/get_horizontal_offset()`
- `egui_view_scroll_viewer_scroll_line()/scroll_page()`
- `egui_view_scroll_viewer_set_scrollbar_visibility()`
- `egui_view_scroll_viewer_set_on_view_changed_listener()`
- `egui_view_scroll_viewer_set_font()/set_meta_font()`
- `egui_view_scroll_viewer_set_compact_mode()`
- `egui_view_scroll_viewer_set_palette()`
- `egui_view_scroll_viewer_get_part_region()`
- `egui_view_scroll_viewer_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/scroll_viewer PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_viewer --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_viewer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter scroll_viewer
```

## 10. 已知限制与后续方向

- 首版只做单容器 `ScrollViewer` reference，不接入真实虚拟化、大文档分块或嵌套惯性滚动。
- 先用 snapshot 数组描述内容段落、viewport 和 offset，不下沉为 SDK 级通用滚动容器。
- 若后续复用价值稳定，再评估是否与 `scroll_bar`、`data_list_panel` 抽共享的轨道、thumb 与 viewport helper。
