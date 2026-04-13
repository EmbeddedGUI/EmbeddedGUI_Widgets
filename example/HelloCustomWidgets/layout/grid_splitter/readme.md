# grid_splitter 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`GridSplitter`
- 本次保留状态：`standard`、`compact`、`read only`、`drag resize`、`keyboard resize`
- 本次删除效果：页面级 `guide`、额外场景标签、过重装饰、与布局无关的叙事外壳
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_grid_splitter`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid_splitter` 用来表达“左右两个 pane 共享一条可拖拽分隔柄”的标准布局语义。它适合资源管理器、属性检查器、工作台和设置页这类需要在同一屏内动态调节两栏宽度的场景。

## 2. 为什么现有控件不够用
- `split_view` 更偏向“可展开导航 pane + detail”的语义，不等同于持续调整列宽的 `GridSplitter`
- `data_list_panel` 和 `master_detail` 更偏向选择与阅读，不提供独立的 resize handle
- SDK 基础布局虽然能做双栏，但没有一个可直接审阅的 `GridSplitter` reference 控件

## 3. 目标场景与示例概览
- 主区域展示标准 `GridSplitter`：左侧 pane、右侧 pane 和中间 drag handle 始终同屏可见
- 底部左侧展示 `compact` 静态 preview，用于对照小尺寸下的分栏结构
- 底部右侧展示 `read only` 静态 preview，用于对照禁交互后的弱化状态
- 录制轨道覆盖键盘步进、snapshot 切换、真实拖拽和 preview 点击后的焦点收尾

目录：
- `example/HelloCustomWidgets/layout/grid_splitter/`

## 4. 视觉与布局规格
- 根容器：`224 x 236`
- 主控件：`196 x 118`
- 底部对照行：`216 x 74`
- `compact` preview：`104 x 74`
- `read only` preview：`104 x 74`
- 页面结构：标题 -> 主 `grid_splitter` -> `compact / read only` 双 preview
- 视觉约束：
  - 保持浅底、低噪音、圆角卡片和细边框的 Fluent 主线
  - 两个 pane 通过轻量色带和内容块表达层级，不回退到 showcase 风格
  - handle 始终作为唯一 resize affordance，不能被其它视觉噪音淹没

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 236` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Grid Splitter` | 页面标题 |
| `splitter_primary` | `egui_view_grid_splitter_t` | `196 x 118` | `Canvas split` | 标准主控件 |
| `splitter_compact` | `egui_view_grid_splitter_t` | `104 x 74` | compact | 紧凑静态 preview |
| `splitter_read_only` | `egui_view_grid_splitter_t` | `104 x 74` | compact + read only | 只读静态 preview |

## 6. 状态覆盖矩阵
| 区域 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Canvas split` | 默认主态 |
| 主控件 | `Audit split` | 左侧 pane 更宽的对照态 |
| 主控件 | `Inspector split` | 右侧 pane 更宽的对照态 |
| 主控件 | `keyboard resize` | `Left / Right / Home / End` 步进 |
| 主控件 | `drag resize` | 真实拖拽 handle 调整比例 |
| `compact` | `Compact split` | 默认紧凑对照 |
| `compact` | `Compact audit` | 第二组静态对照 |
| `read only` | `Read only split` | 固定只读对照 |

## 7. 交互与状态语义
- 主控件只保留一个交互目标：中间 `handle`
- `ACTION_DOWN` 命中 handle 后进入拖拽，`ACTION_MOVE` 连续更新分栏比例
- 这是连续拖拽控件，触摸语义检查通过 allowlist 登记，不复用普通 same-target release 规则
- 键盘语义：
  - `Left / Right`：按固定步长调整分栏比例
  - `Home / End`：跳到最小/最大比例
  - `Tab`：切到下一组 snapshot，并恢复该 snapshot 默认比例
  - `Enter / Space`：回到当前 snapshot 默认比例
- `set_snapshots()`、`set_current_snapshot()`、`set_split_ratio()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都会清理残留 `pressed`
- `read_only_mode`、`!enable` 和 static preview 都会吞掉新的 `touch / key` 输入，并先清理旧的 pressed 状态

## 8. API 约定
- `egui_view_grid_splitter_init()`
- `egui_view_grid_splitter_set_snapshots()/get_current_snapshot()`
- `egui_view_grid_splitter_set_current_snapshot()`
- `egui_view_grid_splitter_set_split_ratio()/get_split_ratio()`
- `egui_view_grid_splitter_set_on_ratio_changed_listener()`
- `egui_view_grid_splitter_set_font()/set_meta_font()`
- `egui_view_grid_splitter_set_compact_mode()/set_read_only_mode()`
- `egui_view_grid_splitter_set_palette()`
- `egui_view_grid_splitter_get_handle_region()`
- `egui_view_grid_splitter_override_static_preview_api()`

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认 snapshot
2. 抓取初始帧
3. 通过 `Right` 展示键盘步进
4. 抓取键盘步进后的帧
5. 切到 `Audit split`
6. 抓取第二组 snapshot
7. 对主控件执行一次真实拖拽
8. 抓取拖拽后的比例变化
9. 切到 `Inspector split`，同时切换 `compact` preview
10. 抓取最终对照帧
11. 点击 `compact` preview，只清主控件 focus
12. 抓取最终收尾帧

## 10. 编译、touch、runtime 与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_splitter PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_splitter --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_splitter
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_splitter
```

## 11. 验收重点
- 主控件和底部两个 preview 都必须完整可见，不能黑屏、白屏或裁切
- handle 在拖拽和键盘步进后都要保持清晰，不出现残留 pressed 高亮
- 左右 pane 在极值比例下仍然保留最小可见宽度，不能塌缩成不可读块
- static preview 的 `touch / key` 不能改动比例，也不能触发 listener
- `read_only_mode` 和 `!enable` 进入后，必须先清理旧 pressed 再拒绝输入

## 12. 已知限制与后续方向
- 当前只做横向双栏 `GridSplitter` reference，不覆盖纵向版本
- 当前 pane 内容仍是静态快照，不接真实布局树和嵌套子控件
- 当前不做多分隔柄、多列联动和复杂容器约束求解

## 13. 与现有控件的边界
- 相比 `split_view`：这里强调连续调整列宽，而不是 pane 展开/收起
- 相比 `master_detail`：这里的核心交互是 handle drag，不是列表选中
- 相比基础线性布局：这里提供可直接审阅的 `GridSplitter` 标准语义和 preview 收口

## 14. 对应组件名与本次保留状态
- 对应组件名：`GridSplitter`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `drag resize`
  - `keyboard resize`

## 15. 删掉的装饰与 EGUI 适配约束
- 不保留旧版页面标签、guide 文案和额外场景包装
- 不做 hover-only 放大、复杂动画和多余阴影
- preview 只承担静态对照与主控件 focus 收尾，不承担新的交互职责
- 通过统一的 pressed 清理语义，把 setter、guard、preview 和拖拽收口到同一套状态生命周期
