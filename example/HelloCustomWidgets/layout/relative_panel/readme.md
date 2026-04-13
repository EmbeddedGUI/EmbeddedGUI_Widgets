# relative_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 平台语义参考：`RelativePanel`
- 补充对照控件：`grid_splitter`、`card_control`、`scroll_presenter`
- 对应组件名：`RelativePanel`
- 计划保留状态：`standard`、`compact`、`read only`、`rule highlight`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_relative_panel`，优先收口相对定位规则、锚点关系与静态 preview，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`relative_panel` 用来表达“多个子块按照彼此的相对关系完成布局，而不是固定网格或线性栈”的标准语义。它适合信息卡编排、欢迎页摘要、关系图面板、设置总览和多区块摘要页这类需要强调 `align / above / below / right of / align edge` 规则的场景。

## 2. 为什么现有控件不够用

- `card_control`、`master_detail` 更偏固定槽位，不负责表达通用的相对定位规则。
- `grid_splitter` 与 `split_view` 强调分栏或分区，不适合做多块自由关系布局。
- `scroll_presenter` 关注 viewport 平移，不承担布局规则可视化与关系语义。
- 当前仓库还没有一个符合 `RelativePanel` 语义的 reference 页面、单测与 web 验证路径。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `relative_panel`：多个信息卡按照“顶部摘要 -> 右侧状态 -> 下方详情 -> 底部辅助块”的规则排布。
- 底部左侧展示 `compact` 静态 preview，用于对照小尺寸下的规则压缩表现。
- 底部右侧展示 `read only` 静态 preview，用于对照静态 reference 的弱化层级。
- 首轮录制动作优先覆盖 `focus rule -> move relation -> activate current card -> preview dismiss` 主链路。

目录：
- `example/HelloCustomWidgets/layout/relative_panel/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 156` 左右，用于完整展示 4 到 5 个相对定位块。
- 主体保持浅色 Fluent 卡片容器，布局区内保留：
  - 当前 rule badge / 标题
  - 多个带关系提示的卡片块
  - 低噪音辅助线或 anchor 提示
- 需要通过对齐线、关系标签和块间距离明确表达“相对规则正在生效”，而不是退化成普通 stack/grid。

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 280` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Relative Panel` | 页面标题 |
| `relative_panel_primary` | `egui_view_relative_panel_t` | `196 x 156` | `standard` | 主 reference 控件 |
| `relative_panel_compact` | `egui_view_relative_panel_t` | `104 x 86` | compact | 紧凑静态 preview |
| `relative_panel_read_only` | `egui_view_relative_panel_t` | `104 x 86` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Summary anchor` | 默认聚焦主摘要卡 |
| 主控件 | `Right rail relation` | 聚焦右侧关系块 |
| 主控件 | `Bottom detail` | 聚焦下方详情块 |
| 主控件 | `Rule highlight` | 当前规则高亮与辅助线可见 |
| `compact` | `Compact preview` | 小尺寸关系排布对照 |
| `read only` | `Read only preview` | 只读静态对照 |

## 7. 交互与状态语义

- 主控件默认聚焦当前规则对应的卡片块，并保持显式关系高亮。
- 首版聚焦以下键盘语义：
  - `Left / Right / Up / Down`：在可见关系块之间移动焦点。
  - `Home / End`：跳到首块 / 末块。
  - `Tab`：在当前规则和卡片块之间切换。
  - `Enter / Space`：激活当前块或切换到下一组布局规则。
- 触摸语义保持 same-target release：
  - 点击某个块时只在同一块 `UP` 时提交。
  - 辅助线和 rule badge 不单独成为提交目标。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理旧的 pressed 状态。
- `read_only` 与 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_relative_panel_init()`
- `egui_view_relative_panel_set_snapshots()/get_current_snapshot()`
- `egui_view_relative_panel_set_current_snapshot()`
- `egui_view_relative_panel_set_current_item()/get_current_item()`
- `egui_view_relative_panel_activate_current_item()`
- `egui_view_relative_panel_set_on_action_listener()`
- `egui_view_relative_panel_set_font()/set_meta_font()`
- `egui_view_relative_panel_set_compact_mode()/set_read_only_mode()`
- `egui_view_relative_panel_set_palette()`
- `egui_view_relative_panel_get_item_region()`
- `egui_view_relative_panel_handle_navigation_key()`
- `egui_view_relative_panel_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/relative_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/relative_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/relative_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter relative_panel
```

## 10. 已知限制与后续方向

- 首版只做轻量 `RelativePanel` reference，不接入真实自动布局求解器或任意约束组合。
- 先用 snapshot 数组固化一组代表性的相对关系，不下沉为 SDK 级通用布局引擎。
- 若后续复用价值稳定，再评估是否与 `card_control`、`master_detail` 抽共享的布局块与关系高亮 helper。
