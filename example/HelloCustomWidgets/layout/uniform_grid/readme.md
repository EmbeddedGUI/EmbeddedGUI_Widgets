# uniform_grid 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UniformGrid`
- 对应组件名：`UniformGrid`
- 本次保留语义：`operations / release / density preset`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_uniform_grid`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`uniform_grid` 用来表达“多个等宽等高单元按固定网格均匀排布”的标准布局语义，适合设置入口墙、功能分组页、仪表盘快捷入口和状态卡片矩阵这类需要稳定栅格密度的场景。

## 2. 为什么现有控件不够用

- `settings_panel` 和 `settings_card` 更偏向纵向信息流，不表达均匀栅格排布。
- `data_list_panel` 和 `data_grid` 强调列表或表格语义，不适合做等尺寸 tile 矩阵。
- `items_repeater` 和 `wrap_panel` 更强调模板复用与自适应布局，不保证所有 cell 保持等宽等高。
- 当前仓库需要一个能完整走通 reference、单测、runtime 和 web 发布链路的 `UniformGrid` 页面。

## 3. 目标场景与示例概览

- 主控件保留真实 `UniformGrid` 语义，展示 `Operations wall`、`Release wall`、`Density wall` 三组主 snapshot。
- 主录制轨道只导出主控件的 snapshot 切换，不再让底部 preview 承担叙事职责。
- 底部左侧是 `compact` 静态 preview，只用于对照小尺寸下的 tile 密度。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化层级。
- 页面只保留标题、一个主 `uniform_grid` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_uniform_grid_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改写 `current_snapshot / current_cell`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/uniform_grid/`

## 4. 视觉与布局规格

- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部对照行：`216 x 78`
- `compact` preview：`104 x 78`
- `read only` preview：`104 x 78`
- 页面结构：标题 -> 主 `uniform_grid` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 `badge / title / meta / body` 层级。
  - 主区突出同一 tile shell 在不同 snapshot 下的均匀网格密度，不再叠加旧 preview 交互桥接。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `grid_primary` | `egui_view_uniform_grid_t` | `196 x 124` | `Operations wall` | 主 `UniformGrid` |
| `grid_compact` | `egui_view_uniform_grid_t` | `104 x 78` | `Compact grid` | 紧凑静态对照 |
| `grid_read_only` | `egui_view_uniform_grid_t` | `104 x 78` | `Read only grid` | 只读静态对照 |
| `primary_snapshots` | `egui_view_uniform_grid_snapshot_t[3]` | - | `Operations / Release / Density` | 主控件语义轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Operations wall` | 默认状态，展示稳定的 `3 x 2` tile 入口墙 |
| 主控件 | `Release wall` | 第二组 snapshot，展示更宽松的 `2 x 2` 版本 |
| 主控件 | `Density wall` | 第三组 snapshot，展示紧凑信息密度但仍保持等尺寸格子 |
| `compact` | `Compact grid` | 固定静态对照，验证小尺寸下的 tile 密度 |
| `read only` | `Read only grid` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口

- 主控件保留真实 `UniformGrid` 键盘与触摸语义：
  - `Left / Right / Up / Down`：按网格位置在相邻单元之间移动
  - `Home / End`：跳到首尾单元
  - `Tab`：在当前 snapshot 内前进，必要时切到下一组 snapshot
  - `Enter / Space`：激活当前单元并触发 listener
- 触摸交互保持 same-target release：只在同一单元 `DOWN -> UP` 时提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_cell()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计

`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Operations wall`。
2. 切到 `Release wall`，输出第二组主状态。
3. 切到 `Density wall`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、运行时、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/uniform_grid PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/uniform_grid --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/uniform_grid
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_uniform_grid
```

验收重点：
- 主控件必须能直接看出 `UniformGrid` 在三组 snapshot 下保持等宽等高 tile 的页面结构变化。
- `same-target release / keyboard navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向

- 当前只收口单容器 `UniformGrid` reference，不覆盖自适应换列和真实子控件容器行为。
- 单元内容继续用 snapshot 驱动的静态信息块表示，不接完整嵌套布局树。
- 若后续确认复用价值稳定，再评估是否抽象成更通用的 SDK 栅格容器。

## 11. 与现有控件的边界

- 相比 `items_repeater`：这里强调等宽等高网格，而不是模板重复布局。
- 相比 `wrap_panel`：这里不允许单元因内容或宽度权重改变外框大小。
- 相比 `data_grid`：这里是 tile matrix，而不是表格列语义。

## 12. 本次保留的核心状态与删减项

- 保留的核心状态：
  - `operations wall`
  - `release wall`
  - `density wall`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作

## 13. 当前验收结果（2026-04-17）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/uniform_grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `uniform_grid` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/uniform_grid --track reference --timeout 10 --keep-screenshots`
  - `9` 帧输出到 `runtime_check_output/HelloCustomWidgets_layout_uniform_grid/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/uniform_grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_uniform_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.186 colors=361`
- 截图复核结论：
  - 主区变化边界收敛到 `(60, 99) - (420, 253)`
  - 主区共 `3` 组唯一状态，对应 `Operations wall / Release wall / Density wall`
  - 按 `y >= 253` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
