# uniform_grid 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`UniformGrid`
- 计划保留状态：`standard`、`compact`、`read only`、`keyboard navigation`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_uniform_grid`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`uniform_grid` 用来表达“多个等宽等高单元按固定网格均匀排布”的标准布局语义。它适合设置入口墙、功能分组页、仪表盘快捷入口和状态卡片矩阵这类需要稳定栅格密度的场景。

## 2. 为什么现有控件不够用
- `settings_panel` 和 `settings_card` 更偏向纵向信息流，不表达均匀栅格排布。
- `data_list_panel` 和 `data_grid` 强调列表/表格语义，不适合做等尺寸 tile 矩阵。
- 基础线性布局虽然能拼出网格，但当前仓库里没有可直接审阅的 `UniformGrid` reference 控件。

## 3. 目标场景与示例概览
- 主区域展示一个 `2 x 3` 的标准 `uniform_grid`，每个单元统一尺寸，体现 equal cell sizing。
- 底部左侧展示 `compact` 静态 preview，用于对照小尺寸下的 tile 密度。
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态。
- 录制轨道优先覆盖 `Right / Left / Up / Down / Tab / Enter` 的键盘闭环与 preview 收尾。

目录：
- `example/HelloCustomWidgets/layout/uniform_grid/`

## 4. 视觉与布局规格
- 根容器：延续 layout 类 reference 页面竖向标题 + 主控件 + 双 preview 结构。
- 主控件：建议 `196 x 124` 左右，容纳 `2 x 3` 等尺寸 tile。
- 单元结构：`badge / title / meta / body` 四层以内，避免回退到 showcase 风格。
- Fluent 方向：
  - 浅底、低噪音、圆角卡片和细边框。
  - 所有 cell 保持统一尺寸，不因文案长度改变外框大小。
  - 通过 tone 或 emphasis 做少量层级区分，但不引入重阴影和强叙事背景。

## 5. 控件清单
| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 240` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Uniform Grid` | 页面标题 |
| `grid_primary` | `egui_view_uniform_grid_t` | `196 x 124` | `standard` | 主 reference 控件 |
| `grid_compact` | `egui_view_uniform_grid_t` | `104 x 78` | compact | 紧凑静态 preview |
| `grid_read_only` | `egui_view_uniform_grid_t` | `104 x 78` | compact + read only | 只读静态 preview |

## 6. 状态覆盖矩阵
| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Operations grid` | 默认 snapshot，展示均匀 tile 栅格 |
| 主控件 | `Focus move` | 方向键在单元间移动焦点 |
| 主控件 | `Tab wrap` | `Tab` 循环聚焦 cell 并允许跨 snapshot |
| 主控件 | `Action commit` | `Enter / Space` 激活当前 cell |
| `compact` | `Compact grid` | 小尺寸对照 |
| `read only` | `Read only grid` | 只读对照 |

## 7. 交互与状态语义
- 主控件默认聚焦到当前 cell，保留键盘导航：
  - `Left / Right / Up / Down` 在网格内移动当前单元。
  - `Home / End` 跳到首尾单元。
  - `Tab` 在当前 snapshot 内循环，必要时切到下一组 snapshot。
  - `Enter / Space` 激活当前单元并触发 listener。
- 触摸语义优先保持 same-target release：
  - `ACTION_DOWN` 命中某个 cell 后，仅在同一 cell `UP` 时提交。
  - `MOVE` 离开 cell 时取消视觉 pressed，回到原 cell 才恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_cell()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都需要清理残留 `pressed`。
- `read_only_mode`、`!enable` 和 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API
- `egui_view_uniform_grid_init()`
- `egui_view_uniform_grid_set_snapshots()/get_current_snapshot()`
- `egui_view_uniform_grid_set_current_snapshot()`
- `egui_view_uniform_grid_set_current_cell()/get_current_cell()`
- `egui_view_uniform_grid_activate_current_cell()`
- `egui_view_uniform_grid_set_on_action_listener()`
- `egui_view_uniform_grid_set_font()/set_meta_font()`
- `egui_view_uniform_grid_set_compact_mode()/set_read_only_mode()`
- `egui_view_uniform_grid_set_palette()`
- `egui_view_uniform_grid_get_cell_region()`
- `egui_view_uniform_grid_override_static_preview_api()`

## 9. 编译 / runtime / web 验收目标
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/uniform_grid PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/uniform_grid --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/uniform_grid
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_uniform_grid
```

## 10. 已知限制与后续方向
- 首版只做固定行列的 reference 语义，不覆盖自适应换列和真实子控件容器行为。
- 单元内容先用 snapshot 驱动的静态信息块表示，不接完整嵌套布局树。
- 若后续确认复用价值足够，再讨论是否下沉成更通用的 SDK 容器控件。
