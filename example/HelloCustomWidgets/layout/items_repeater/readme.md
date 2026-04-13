# items_repeater 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 ItemsRepeater`
- 对应组件名：`ItemsRepeater`
- 计划保留状态：`standard`、`compact`、`read only`、`keyboard navigation`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_items_repeater`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`items_repeater` 用来表达“同一批数据按统一 item template 重复排布，但布局本身保持轻量、可嵌入、可按场景切换”的语义。它适合最近项摘要、标签集合、轻量资源卡片和多状态条目带这类需要重复呈现同构内容、但又不希望直接上完整 `ListView` 或页面级列表控件的场景。

## 2. 为什么现有控件不够用

- `data_list_panel` 更偏带容器语义的 `ListView` 页面，不是轻量的 item repeater 宿主。
- `wrap_panel`、`uniform_grid`、`virtualizing_wrap_panel` 更偏布局容器，不负责统一 item template、当前项和提交语义。
- `virtualizing_stack_panel` 解决的是纵向窗口化列表，不覆盖非虚拟化 repeater 的模板化重复布局。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `items_repeater`：同组 snapshot 数据按 preset layout 重复排布，并保留当前项与激活语义。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的重复项密度。
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态。
- 录制动作优先覆盖 `Right / Down / Home / End / Enter` 与一次真实触摸提交。

目录：
- `example/HelloCustomWidgets/layout/items_repeater/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 120` 左右，用于容纳重复项、布局提示和当前项高亮。
- 主体保持浅色 Fluent 卡片容器，内部是轻量 repeater surface + 可复用 item template。
- 需要额外保留一层低噪音 repeater 信息，例如：
  - 当前 snapshot 名称
  - 当前 layout preset
  - 当前项序号 / 总项数

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 248` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Items Repeater` | 页面标题 |
| `items_repeater_primary` | `egui_view_items_repeater_t` | `196 x 120` | `standard` | 主 reference 控件 |
| `items_repeater_compact` | `egui_view_items_repeater_t` | `104 x 82` | compact | 紧凑静态 preview |
| `items_repeater_read_only` | `egui_view_items_repeater_t` | `104 x 82` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Template repeat` | 同构 item 按统一模板重复排布 |
| 主控件 | `Layout preset` | snapshot 可切换 `stack / wrap / strip` 预设布局 |
| 主控件 | `Selection keep` | 当前项在布局切换后仍保持可追踪 |
| 主控件 | `Action commit` | `Enter / Space` 激活当前项 |
| `compact` | `Compact density` | 窄尺寸下的重复项密度对照 |
| `read only` | `Read only repeater` | 只读弱化对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到当前项，保留键盘导航：
  - `Left / Right / Up / Down`：按当前布局顺序在相邻项之间移动。
  - `Home / End`：跳到首 / 尾项。
  - `Tab`：在当前 snapshot 内前进，必要时切到下一组 snapshot。
  - `Enter / Space`：激活当前项并触发 listener。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某项后，仅在同一项 `UP` 时提交。
  - `MOVE` 离开原项时取消 pressed，移回原项再恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_layout_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_items_repeater_init()`
- `egui_view_items_repeater_set_snapshots()/get_current_snapshot()`
- `egui_view_items_repeater_set_current_snapshot()`
- `egui_view_items_repeater_set_current_item()/get_current_item()`
- `egui_view_items_repeater_set_layout_mode()/get_layout_mode()`
- `egui_view_items_repeater_activate_current_item()`
- `egui_view_items_repeater_set_on_action_listener()`
- `egui_view_items_repeater_set_font()/set_meta_font()`
- `egui_view_items_repeater_set_compact_mode()/set_read_only_mode()`
- `egui_view_items_repeater_set_palette()`
- `egui_view_items_repeater_get_item_region()`
- `egui_view_items_repeater_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/items_repeater PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_repeater --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/items_repeater
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_items_repeater
```

## 10. 已知限制与后续方向

- 首版只做 reference 语义，不做真正的数据绑定管线和通用模板系统。
- 先用 snapshot 数组描述 item 集合和 layout preset，不下沉为 SDK 级通用 repeater。
- 若后续复用价值稳定，再评估是否与 `wrap_panel` / `uniform_grid` / `virtualizing_stack_panel` 抽共享的 item metrics 与布局 helper。
