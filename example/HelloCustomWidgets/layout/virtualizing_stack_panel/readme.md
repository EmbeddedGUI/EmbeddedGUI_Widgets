# virtualizing_stack_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`VirtualizingStackPanel`
- 计划保留状态：`standard`、`compact`、`read only`、`keyboard navigation`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_virtualizing_stack_panel`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`virtualizing_stack_panel` 用来表达“项目按纵向栈式顺序排布，但视口内只绘制当前窗口内容”的布局语义。它适合设置列表、搜索结果、消息摘要、属性面板这类数据量较大、阅读方向稳定、又不希望一次性把全部条目都铺到页面里的场景。

## 2. 为什么现有控件不够用

- `data_list_panel` 更偏带语义的 `ListView` 页面，不是可嵌入的基础虚拟堆叠容器。
- `wrap_panel` 与 `virtualizing_wrap_panel` 强调多列换行密度，不适合单轴纵向窗口化列表。
- `uniform_grid` 强调等尺寸网格，不适合高低不一或主次文本明显的纵向条目。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `virtualizing_stack_panel`：项目保持纵向 stack 排布，但只渲染当前可见窗口。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的行密度。
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态。
- 录制动作优先覆盖 `Down / Ctrl+Down(page jump) / Home / End / Enter` 与一次真实触摸提交。

目录：
- `example/HelloCustomWidgets/layout/virtualizing_stack_panel/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 132` 左右，用于容纳可见行、窗口提示与尾部溢出提示。
- 主体保持浅色 Fluent 卡片容器，内部是单列 stack shell + 轻量 row item。
- 需要额外保留一层低噪音窗口信息，例如：
  - 当前可见区间
  - 总条目数
  - 是否还有上/下文未渲染

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 248` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Virtualizing Stack Panel` | 页面标题 |
| `virtualizing_stack_panel_primary` | `egui_view_virtualizing_stack_panel_t` | `196 x 132` | `standard` | 主 reference 控件 |
| `virtualizing_stack_panel_compact` | `egui_view_virtualizing_stack_panel_t` | `104 x 82` | compact | 紧凑静态 preview |
| `virtualizing_stack_panel_read_only` | `egui_view_virtualizing_stack_panel_t` | `104 x 82` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Windowed stack` | 仅绘制视口窗口内的项目 |
| 主控件 | `Selection keep` | 当前项移动时自动更新可见窗口 |
| 主控件 | `Page jump` | `Ctrl+Up / Ctrl+Down` 代替 `PageUp / PageDown` 成批移动窗口 |
| 主控件 | `Action commit` | `Enter / Space` 激活当前项 |
| `compact` | `Compact density` | 窄尺寸下的行密度对照 |
| `read only` | `Read only stack` | 只读弱化对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到当前项，保留键盘导航：
  - `Up / Down`：在上下相邻项之间移动。
  - `Ctrl+Up / Ctrl+Down`：在当前 SDK 无独立 `PageUp / PageDown` key code 的前提下，按一屏窗口大小向前 / 向后跳转。
  - `Home / End`：跳到首 / 尾项并更新窗口。
  - `Tab`：在当前 snapshot 内前进，必要时切到下一组 snapshot。
  - `Enter / Space`：激活当前项并触发 listener。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某项后，仅在同一项 `UP` 时提交。
  - `MOVE` 离开原项时取消 pressed，移回原项再恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_window_anchor()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_virtualizing_stack_panel_init()`
- `egui_view_virtualizing_stack_panel_set_snapshots()/get_current_snapshot()`
- `egui_view_virtualizing_stack_panel_set_current_snapshot()`
- `egui_view_virtualizing_stack_panel_set_current_item()/get_current_item()`
- `egui_view_virtualizing_stack_panel_set_window_anchor()/get_window_anchor()`
- `egui_view_virtualizing_stack_panel_activate_current_item()`
- `egui_view_virtualizing_stack_panel_set_on_action_listener()`
- `egui_view_virtualizing_stack_panel_set_font()/set_meta_font()`
- `egui_view_virtualizing_stack_panel_set_compact_mode()/set_read_only_mode()`
- `egui_view_virtualizing_stack_panel_set_palette()`
- `egui_view_virtualizing_stack_panel_get_item_region()`
- `egui_view_virtualizing_stack_panel_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/virtualizing_stack_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_virtualizing_stack_panel
```

## 10. 已知限制与后续方向

- 首版只做 reference 语义，不做真正的数据源虚拟化容器。
- 先用 snapshot 数组描述总数据和窗口锚点，不下沉为通用列表基础设施。
- 若后续复用价值稳定，再评估是否与 `data_list_panel` / `virtualizing_wrap_panel` 抽共享的纵向 metrics / windowing helper。
