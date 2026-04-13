# scroll_presenter 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 平台语义参考：`ScrollPresenter`
- 补充对照控件：`scroll_viewer`、`scroll_bar`、`data_list_panel`
- 对应组件名：`ScrollPresenter`
- 计划保留状态：`standard`、`compact`、`read only`、`both-axis pan`
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_scroll_presenter`，优先收口无外层 chrome 的可平移 viewport、offset 指标与静态 preview，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`scroll_presenter` 用来表达“内容 surface 自身支持平移浏览，但默认不露出完整滚动条 chrome”的标准语义。它适合大画布摘要、缩略时间线、自由排布面板、图像检查区和轻量地图这类需要保留 viewport / extent / offset 语义、但又不希望滚动条抢占视觉层级的场景。

## 2. 为什么现有控件不够用

- `scroll_viewer` 包含明显的滚动条轨道与 thumb，更适合带 chrome 的通用滚动容器，不适合表达纯内容 viewport。
- `scroll_bar` 只覆盖单独的轴向滚动部件，不承载内容裁切、双轴 offset 与拖拽平移语义。
- `data_list_panel` 偏向列表数据容器，不适合自由内容 surface 或双轴平移场景。
- 当前仓库还没有一个符合 `ScrollPresenter` 语义的 reference 页面、单测与 web 验证路径。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `scroll_presenter`：固定 viewport 内承载一张大于视口的布局画布，保留节点、路径和说明卡片。
- 底部左侧展示 `compact` 静态 preview，用于对照窄尺寸下的 viewport 密度。
- 底部右侧展示 `read only` 静态 preview，用于对照静态 surface 的弱化表现。
- 首轮录制动作优先覆盖 `focus -> vertical pan -> horizontal pan -> far corner -> preview dismiss` 主链路。

目录：
- `example/HelloCustomWidgets/layout/scroll_presenter/`

## 4. 视觉与布局规格

- 页面结构延续 layout 类 reference：标题 + 主控件 + 双 preview。
- 主控件建议尺寸：`196 x 160` 左右，用于同时容纳 viewport、内容画布与低噪音 offset 信息。
- 主体保持浅色 Fluent 卡片容器，但默认不绘制外露滚动条轨道；仅保留：
  - 顶部 snapshot 标题 / 辅助信息
  - 中部裁切 viewport 与内容 surface
  - 角落的 offset / extent 小标签，提示当前浏览位置
- 需要通过边缘渐隐、裁切和内容位移明确传达“可平移内容”的 affordance，而不是依赖显式 scrollbar。

## 5. 控件清单

| 变量名 | 类型 | 建议尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 284` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Scroll Presenter` | 页面标题 |
| `scroll_presenter_primary` | `egui_view_scroll_presenter_t` | `196 x 160` | `standard` | 主 reference 控件 |
| `scroll_presenter_compact` | `egui_view_scroll_presenter_t` | `104 x 88` | compact | 紧凑静态 preview |
| `scroll_presenter_read_only` | `egui_view_scroll_presenter_t` | `104 x 88` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Origin view` | 初始停留在左上原点区域 |
| 主控件 | `Mid canvas` | 平移到中部节点区 |
| 主控件 | `Far corner` | 跳到右下末端区域 |
| 主控件 | `Surface drag` | 直接拖拽 surface 的连续交互态 |
| `compact` | `Compact preview` | 窄尺寸 viewport 对照 |
| `read only` | `Read only preview` | 只读弱化后的静态对照 |

## 7. 交互与状态语义

- 主控件默认聚焦到内容 surface，所有滚动 / 平移语义都由同一个 focus target 承担。
- 首版聚焦以下键盘语义：
  - `Up / Down`：按行平移垂直 offset。
  - `Left / Right`：按列平移水平 offset。
  - `Home / End`：跳到左上原点 / 右下末端。
  - `+ / -`：按页平移垂直 offset。
  - `Enter / Space`：切换到下一组预设 viewport。
- 触摸语义收口为：
  - 在 surface 内按下并拖拽时连续更新 offset，但不改写按下目标。
  - `ACTION_UP` 仅在最初命中的 surface 上收尾，不引入额外 allowlist。
- `set_snapshots()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要清理旧的 pressed / drag 状态。
- `read_only` 与 static preview 需要吞掉新的 `touch / key` 输入，并先清旧状态。

## 8. 计划 API

- `egui_view_scroll_presenter_init()`
- `egui_view_scroll_presenter_set_snapshots()/get_current_snapshot()`
- `egui_view_scroll_presenter_set_current_snapshot()`
- `egui_view_scroll_presenter_set_vertical_offset()/get_vertical_offset()/get_max_vertical_offset()`
- `egui_view_scroll_presenter_set_horizontal_offset()/get_horizontal_offset()/get_max_horizontal_offset()`
- `egui_view_scroll_presenter_scroll_line()/scroll_page()`
- `egui_view_scroll_presenter_activate_current_snapshot()`
- `egui_view_scroll_presenter_set_on_view_changed_listener()`
- `egui_view_scroll_presenter_set_font()/set_meta_font()`
- `egui_view_scroll_presenter_set_compact_mode()/set_read_only_mode()`
- `egui_view_scroll_presenter_set_palette()`
- `egui_view_scroll_presenter_handle_navigation_key()`
- `egui_view_scroll_presenter_get_surface_region()`
- `egui_view_scroll_presenter_override_static_preview_api()`

## 9. 验收目标

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/scroll_presenter PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_presenter --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_presenter
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter scroll_presenter
```

## 10. 已知限制与后续方向

- 首版只做轻量 `ScrollPresenter` reference，不接入缩放因子、嵌套滚动链、滚轮惯性或系统级输入桥接。
- 先用 snapshot 数组描述大画布内容、viewport 和 offset，不下沉为 SDK 级通用平移容器。
- 若后续复用价值稳定，再评估是否与 `scroll_viewer`、`scroll_bar` 抽共享的 offset metrics 与 surface drag helper。
