# rich_edit_box 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 平台语义参考：`RichEditBox`
- 补充对照控件：`text_box`、`rich_text_block`
- 对应组件名：`RichEditBox`
- 计划保留状态：`standard`、`compact`、`read only`、`format preset`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_rich_edit_box`，优先收口单文档富文本编辑 surface、段落样式 preset 和静态 preview，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`rich_edit_box` 用来表达“同一个输入面板里既能编辑文字，又能保留轻量格式层级”的语义。它适合备注编辑、发布说明、问题复盘、设备诊断摘要这类需要一小段标题、正文、强调句或清单项共存的场景。

## 2. 为什么现有控件不够用

- `text_box` 只覆盖纯文本输入，不负责段落样式、强调块和格式 preset。
- `rich_text_block` 只负责展示，不承担真实编辑、焦点和键盘输入链路。
- `token_input`、`shortcut_recorder` 这类输入控件关注结构化录入，不适合连续富文本编辑。
- 当前仓库还没有一个符合 Fluent / WinUI `RichEditBox` 语义的 reference 页面、单测和 web 验证闭环。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `rich_edit_box`：在固定编辑面板内同时表达标题、正文和轻量强调段落。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的格式层级。
- 底部右侧展示 `read only` 静态 preview，用于对照只读弱化后的展示结果。
- 首轮录制动作优先覆盖 `focus -> preset switch -> apply -> preview dismiss` 的主链路。

目录：
- `example/HelloCustomWidgets/input/rich_edit_box/`

## 4. 视觉与布局规格

- 页面结构延续 input 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 146` 左右，用于容纳格式摘要、编辑面板和底部 preset row。
- 主体保持浅色 Fluent 卡片容器，内部保留：
  - 顶部 document title / helper
  - 中部编辑 surface
  - 底部 format preset pills
- 需要额外保留一层低噪音 rich edit 信息，例如：
  - 当前 snapshot 名称
  - 当前 preset
  - 段落数量 / 只读状态

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 270` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Rich Edit Box` | 页面标题 |
| `rich_edit_primary` | `egui_view_rich_edit_box_t` | `196 x 146` | `standard` | 主 reference 控件 |
| `rich_edit_compact` | `egui_view_rich_edit_box_t` | `104 x 82` | compact | 紧凑静态 preview |
| `rich_edit_read_only` | `egui_view_rich_edit_box_t` | `104 x 82` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Draft note` | 默认正文编辑态 |
| 主控件 | `Callout preset` | 当前段落切到强调样式 |
| 主控件 | `Checklist preset` | 当前段落切到清单样式 |
| 主控件 | `Action commit` | `Enter / Space` 提交当前 preset 或动作 |
| `compact` | `Compact preview` | 窄尺寸下的格式层级对照 |
| `read only` | `Read only rich edit` | 只读弱化对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到编辑 surface，保留真实键盘编辑链路。
- 首版聚焦以下键盘语义：
  - `Left / Right`：在 format preset 之间切换。
  - `Home / End`：跳到首 / 尾 preset。
  - `Tab`：在编辑 surface 与 preset row 之间移动。
  - `Enter / Space`：应用当前 preset，或提交当前动作。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某个 preset 后，仅在同一 preset `UP` 时提交。
  - `MOVE` 离开原 preset 时取消 pressed，移回原 preset 再恢复。
- `set_documents()`、`set_current_document()`、`set_current_preset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_rich_edit_box_init()`
- `egui_view_rich_edit_box_set_documents()/get_current_document()`
- `egui_view_rich_edit_box_set_current_document()`
- `egui_view_rich_edit_box_set_current_preset()/get_current_preset()`
- `egui_view_rich_edit_box_apply_current_preset()`
- `egui_view_rich_edit_box_set_on_action_listener()`
- `egui_view_rich_edit_box_set_font()/set_meta_font()`
- `egui_view_rich_edit_box_set_compact_mode()/set_read_only_mode()`
- `egui_view_rich_edit_box_set_palette()`
- `egui_view_rich_edit_box_get_part_region()`
- `egui_view_rich_edit_box_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=input/rich_edit_box PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rich_edit_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/rich_edit_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter rich_edit_box
```

## 10. 已知限制与后续方向

- 首版只做 reference 语义，不做完整 inline object tree、图片嵌入或复杂 IME 组合输入。
- 先用 document snapshot 数组描述段落内容、preset 和只读对照，不下沉为 SDK 级通用富文本编辑器。
- 若后续复用价值稳定，再评估是否与 `text_box`、`rich_text_block` 抽共享的文本测量、段落排布和静态 preview helper。
