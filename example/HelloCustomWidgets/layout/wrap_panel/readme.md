# wrap_panel 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`WrapPanel`
- 计划保留状态：`standard`、`compact`、`read only`、`keyboard navigation`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_wrap_panel`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`wrap_panel` 用来表达“项目按流式顺序排布，遇到边界后自动换行”的标准布局语义。它适合标签集合、过滤项组、轻量操作卡片和状态 chips 这类需要保持顺序但又不能固定成等宽矩阵的场景。

## 2. 为什么现有控件不够用

- `uniform_grid` 强调固定列数和等尺寸单元，不适合长短不一的条目。
- `data_list_panel` 偏纵向列表，不表达流式换行。
- `settings_panel`、`settings_card` 偏设置页信息流，不适合做轻量标签面板。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `wrap_panel`，包含不同宽度的 tag / chip / pill 项。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的换行密度。
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态。
- 录制动作优先覆盖 `Left / Right / Up / Down / Tab / Enter` 与一次真实触摸提交。

目录：

- `example/HelloCustomWidgets/layout/wrap_panel/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 124` 左右。
- 单项保留 `badge / title / meta` 的轻量层级，避免回到 showcase 风格。
- Fluent 方向：
  - 浅底、圆角、低噪音边框。
  - 条目允许不同宽度，但行高统一。
  - 换行后保持清晰的行距与列距，不引入过强装饰。

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 240` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Wrap Panel` | 页面标题 |
| `wrap_primary` | `egui_view_wrap_panel_t` | `196 x 124` | `standard` | 主 reference 控件 |
| `wrap_compact` | `egui_view_wrap_panel_t` | `104 x 78` | compact | 紧凑静态 preview |
| `wrap_read_only` | `egui_view_wrap_panel_t` | `104 x 78` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Tag flow` | 展示流式排布和自动换行 |
| 主控件 | `Focus move` | 键盘在条目间移动当前项 |
| 主控件 | `Wrap continue` | 跨行后仍保持顺序导航 |
| 主控件 | `Action commit` | `Enter / Space` 激活当前项 |
| `compact` | `Compact wrap` | 窄尺寸下对照 |
| `read only` | `Read only wrap` | 只读弱化对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到当前项，保留键盘导航：
  - `Left / Right` 在同一行的邻项间移动。
  - `Up / Down` 按视觉行关系移动到最近项。
  - `Home / End` 跳到首尾项。
  - `Tab` 在当前 snapshot 内循环，必要时切到下一组 snapshot。
  - `Enter / Space` 激活当前项并触发 listener。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某项后，仅在同一项 `UP` 时提交。
  - `MOVE` 离开原项时取消 pressed，移回原项再恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_wrap_panel_init()`
- `egui_view_wrap_panel_set_snapshots()/get_current_snapshot()`
- `egui_view_wrap_panel_set_current_snapshot()`
- `egui_view_wrap_panel_set_current_item()/get_current_item()`
- `egui_view_wrap_panel_activate_current_item()`
- `egui_view_wrap_panel_set_on_action_listener()`
- `egui_view_wrap_panel_set_font()/set_meta_font()`
- `egui_view_wrap_panel_set_compact_mode()/set_read_only_mode()`
- `egui_view_wrap_panel_set_palette()`
- `egui_view_wrap_panel_get_item_region()`
- `egui_view_wrap_panel_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/wrap_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/wrap_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/wrap_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_wrap_panel
```

## 10. 已知限制与后续方向

- 首版只做 reference 语义，不实现真实子控件容器。
- 先用 snapshot 描述条目，不下沉为通用布局容器。
- 若后续复用价值稳定，再考虑是否抽象为更通用的 flow layout 基础控件。
