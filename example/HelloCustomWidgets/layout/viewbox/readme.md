# viewbox 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Viewbox`
- 对应组件名：`Viewbox`
- 计划保留状态：`standard`、`compact`、`read only`、`stretch preset`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_viewbox`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`viewbox` 用来表达“单个内容块在受限容器内按统一缩放规则自适应放大或缩小”的语义。它适合预览卡片、设备缩略图、封面容器、图标组合块和嵌入式仪表摘要这类需要把一整块内容按比例塞进固定边界、但又不希望手工分别处理每个子元素尺寸的场景。

## 2. 为什么现有控件不够用

- `card_control`、`card_action`、`card_panel` 更偏卡片语义本身，不负责对子内容做统一缩放。
- `uniform_grid`、`wrap_panel`、`items_repeater` 关注多项排布，不是单 child 的自适应缩放容器。
- 现有 SDK 基础布局能控制位置和尺寸，但没有当前仓库需要的 `Viewbox` reference 页面、stretch preset、静态 preview 和验收闭环。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `viewbox`：同一组内容在固定边界内按不同 stretch preset 缩放。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的缩放结果。
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态。
- 录制动作优先覆盖 `Right / Left / Home / End / Enter` 与一次真实触摸提交。

目录：
- `example/HelloCustomWidgets/layout/viewbox/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 120` 左右，用于容纳内容框、缩放提示和当前 preset 高亮。
- 主体保持浅色 Fluent 卡片容器，内部是单 child 的缩放 surface + 边界提示框。
- 需要额外保留一层低噪音 viewbox 信息，例如：
  - 当前 snapshot 名称
  - 当前 stretch preset
  - 内容原始尺寸 / 目标容器尺寸

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 244` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Viewbox` | 页面标题 |
| `viewbox_primary` | `egui_view_viewbox_t` | `196 x 120` | `standard` | 主 reference 控件 |
| `viewbox_compact` | `egui_view_viewbox_t` | `104 x 78` | compact | 紧凑静态 preview |
| `viewbox_read_only` | `egui_view_viewbox_t` | `104 x 78` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Uniform fit` | 内容按比例完整落入容器 |
| 主控件 | `Fill preset` | 内容按填满容器的方向扩展 |
| 主控件 | `Downscale only` | 大内容缩小，小内容不再放大 |
| 主控件 | `Action commit` | `Enter / Space` 提交当前 preset |
| `compact` | `Compact preview` | 窄尺寸下的缩放结果对照 |
| `read only` | `Read only viewbox` | 只读弱化对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到当前 preset，保留键盘导航：
  - `Left / Right`：在 stretch preset 之间切换。
  - `Home / End`：跳到首 / 尾 preset。
  - `Tab`：在当前 snapshot 内前进，必要时切到下一组 snapshot。
  - `Enter / Space`：提交当前 preset 并触发 listener。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某个 preset 后，仅在同一 preset `UP` 时提交。
  - `MOVE` 离开原 preset 时取消 pressed，移回原 preset 再恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_preset()`、`set_stretch_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_viewbox_init()`
- `egui_view_viewbox_set_snapshots()/get_current_snapshot()`
- `egui_view_viewbox_set_current_snapshot()`
- `egui_view_viewbox_set_current_preset()/get_current_preset()`
- `egui_view_viewbox_set_stretch_mode()/get_stretch_mode()`
- `egui_view_viewbox_activate_current_preset()`
- `egui_view_viewbox_set_on_action_listener()`
- `egui_view_viewbox_set_font()/set_meta_font()`
- `egui_view_viewbox_set_compact_mode()/set_read_only_mode()`
- `egui_view_viewbox_set_palette()`
- `egui_view_viewbox_get_preset_region()`
- `egui_view_viewbox_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/viewbox PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/viewbox --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/viewbox
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter viewbox
```

## 10. 已知限制与后续方向

- 首版只做 reference 语义，不做真实的通用 child tree 缩放容器。
- 先用 snapshot 数组描述内容边界、目标边界和 stretch preset，不下沉为 SDK 级通用缩放布局。
- 若后续复用价值稳定，再评估是否与 `items_repeater` / `uniform_grid` / `wrap_panel` 抽共享的容器装饰与静态 preview helper。
