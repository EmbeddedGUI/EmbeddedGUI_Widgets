# grid_view 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 平台语义参考：`GridView`
- 补充对照控件：`data_list_panel`、`items_repeater`、`scroll_presenter`
- 对应组件名：`GridView`
- 计划保留状态：`standard`、`compact`、`read only`、`selection focus`
- EGUI 适配说明：在 custom 层实现轻量 `hcw_grid_view` 语义封装，优先收口平铺集合、焦点选择与静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`grid_view` 用来表达“同一批内容按多列磁贴平铺浏览，并保留当前项选中与激活语义”的标准集合视图。它适合模板库、素材画廊、头像墙、设备分组和卡片式资源挑选这类需要快速扫视多项内容的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏单列 `ListView` 摘要面板，不适合表达多列磁贴集合。
- `items_repeater` 更偏模板重复与布局切换，本身不承担 `GridView` 的选中项、焦点迁移和集合浏览语义。
- `wrap_panel`、`uniform_grid`、`virtualizing_wrap_panel` 更偏布局容器，不负责标准集合项的当前态与激活闭环。
- 当前仓库还没有一个符合 `GridView` 语义的 reference 页面、单测与 web 验证路径。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `grid_view`：以 2 到 3 列磁贴承载同构内容，覆盖 `assets`、`templates`、`team board` 三组 snapshot。
- 底部左侧展示 `compact` 静态 preview，用于对照更小尺寸下的平铺密度。
- 底部右侧展示 `read only` 静态 preview，用于对照只读弱化后的选中态与磁贴层级。
- 首轮录制动作优先覆盖 `Right / Down / Home / End / Enter` 与一次真实触摸提交。

目录：
- `example/HelloCustomWidgets/layout/grid_view/`

## 4. 视觉与布局规格
- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 148` 左右，用于完整展示多列磁贴与当前项高亮。
- 主体保持浅色 Fluent 卡片容器，内部保留：
  - 当前 snapshot 标题与摘要
  - 多列 tile surface
  - 当前项 focus / selection ring
  - 低噪音 footer 与密度提示
- `compact` 与 `read only` 直接通过控件自身模式表达，不额外引入页面壳层。

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 288` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Grid View` | 页面标题 |
| `grid_view_primary` | `hcw_grid_view_t` | `196 x 148` | `standard` | 主 reference 控件 |
| `grid_view_compact` | `hcw_grid_view_t` | `104 x 86` | compact | 紧凑静态 preview |
| `grid_view_read_only` | `hcw_grid_view_t` | `104 x 86` | compact + read only | 只读静态 preview |

## 6. 状态矩阵
| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Assets gallery` | 默认磁贴集合，验证标准 GridView 结构 |
| 主控件 | `Template board` | 切换到第二组 snapshot，验证多列密度变化 |
| 主控件 | `Team board` | 切换到第三组 snapshot，验证不同 tile meta |
| 主控件 | `Selection focus` | 当前磁贴保持 focus/selection 可辨识 |
| `compact` | `Compact density` | 窄尺寸下的多列平铺对照 |
| `read only` | `Read only grid` | 只读弱化对照 |

## 7. 交互与状态语义
- 主控件默认聚焦到当前磁贴，保留键盘导航：
  - `Left / Right / Up / Down`：按几何方向在可见磁贴间移动焦点。
  - `Home / End`：跳到首项 / 末项。
  - `Tab`：在主 grid 与说明性 header 区域间最小切换。
  - `Enter / Space`：激活当前磁贴并触发 listener。
- 触摸语义保持 same-target release：
  - `ACTION_DOWN` 命中某项后，仅在同一项 `UP` 时提交。
  - `MOVE` 离开原项时取消 pressed，移回原项后才恢复。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理旧的 pressed 状态。
- `read_only` 与 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `hcw_grid_view_init()`
- `hcw_grid_view_set_snapshots()/get_current_snapshot()`
- `hcw_grid_view_set_current_snapshot()`
- `hcw_grid_view_set_current_item()/get_current_item()`
- `hcw_grid_view_activate_current_item()`
- `hcw_grid_view_set_on_action_listener()`
- `hcw_grid_view_set_font()/set_meta_font()`
- `hcw_grid_view_set_compact_mode()/set_read_only_mode()`
- `hcw_grid_view_set_palette()`
- `hcw_grid_view_get_item_region()`
- `hcw_grid_view_handle_navigation_key()`
- `hcw_grid_view_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter grid_view
```

## 10. 已知限制与后续方向
- 首版只做轻量 `GridView` reference，不接入真实数据绑定、滚动虚拟化或复杂容器模板。
- 先用 snapshot 数组固化多组典型磁贴集合，不下沉成 SDK 级通用集合控件。
- 若后续复用价值稳定，再评估是否与 `items_repeater`、`wrap_panel` 抽共享的 tile metrics 与布局 helper。
