# data_grid 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI DataGrid`
- 对应组件名：`DataGrid`
- 本次保留语义：`rollout board`、`audit board`、`release board`、`compact`、`read only`、`row selection`
- 本次删除内容：旧 preview 清焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_data_grid` 封装；本轮只收口 `reference` 页面结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`data_grid` 用来表达“列头 + 多行数据 + 当前行选择 + 行激活”的标准表格语义，适合发布检查、审阅列表、任务排期和轻量运营看板这类需要在有限空间里并排比较多列字段的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏单列摘要列表，不强调显式列头和表格结构。
- `settings_panel / settings_expander` 面向设置页，不适合承载多列业务数据。
- `card_control / card_action / card_expander` 都是卡片摘要语义，不是标准表格。
- SDK 原生 `table` 只提供基础单元格绘制，不包含当前仓库需要的 `DataGrid` reference 页面、静态 preview 和验收闭环。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `data_grid` -> 底部 `compact / read only` 双静态 preview。
- 主控件负责展示三组主区状态：
  - `Rollout board`
  - `Audit board`
  - `Release board`
- 底部左侧是 `compact` 静态 preview，固定展示紧凑密度表格。
- 底部右侧是 `read only` 静态 preview，固定展示只读弱化表格。
- 两个 preview 统一通过 `egui_view_data_grid_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `current_snapshot / current_row / compact_mode / read_only_mode`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/data_grid/`

## 4. 视觉与布局规格
- 根布局：`224 x 248`
- 主控件：`196 x 126`
- 底部对照行：`216 x 80`
- `compact` preview：`104 x 80`
- `read only` preview：`104 x 80`
- 页面结构：标题 -> 主 `data_grid` -> `compact / read only`
- 风格约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量分隔线。
  - 当前行通过左侧 selection strip、行底色和轻量 tone 差异表达，不依赖重阴影。
  - 底部 preview 全程静态，不再承担焦点回收、额外轨道切换或录制收尾职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `grid_primary` | `egui_view_data_grid_t` | `196 x 126` | `Rollout board` | 主 `DataGrid` |
| `grid_compact` | `egui_view_data_grid_t` | `104 x 80` | `Compact grid` | 紧凑静态 preview |
| `grid_read_only` | `egui_view_data_grid_t` | `104 x 80` | `Read only grid` | 只读静态 preview |
| `primary_snapshots` | `egui_view_data_grid_snapshot_t[3]` | - | `Rollout / Audit / Release` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Rollout board` | 默认状态，展示标准 `DataGrid` 外壳和当前行 |
| 主控件 | `Audit board` | 第二组 snapshot，验证同一表格壳体下的数据切换 |
| 主控件 | `Release board` | 第三组 snapshot，验证 warning 语义下的行选择 |
| `compact` preview | `Compact grid` | 固定静态对照，只验证紧凑密度下的列头和行 |
| `read only` preview | `Read only grid` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实输入闭环：
  - `Up / Down / Home / End / Tab` 负责行导航
  - `Enter / Space` 激活当前行并触发 listener
  - 触摸保持 same-target release：只有同一行上的 `DOWN -> UP` 才提交
- `set_snapshots()`、`set_current_snapshot()`、`set_current_row()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 之后都不能残留旧的 `pressed` 状态。
- `read only` 和 `!enable` 期间：
  - `touch / dispatch_key_event()` 都不能改状态
  - `current_snapshot / current_row` 保持不变
  - 不触发 listener
- static preview 用例必须验证：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / current_row / compact_mode / read_only_mode` 不变
  - 不触发 listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，直接输出默认 `Rollout board`
2. 切到 `Audit board`
3. 输出第二组主区状态
4. 切到 `Release board`
5. 输出第三组主区状态
6. 恢复默认主状态
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 `reference` 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview dismiss / preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `DATA_GRID_RECORD_FINAL_WAIT`，中间状态切换仍保留 `DATA_GRID_RECORD_WAIT / DATA_GRID_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_grid --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_grid
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_grid
```

## 10. 验收重点
- 主控件三组主状态必须能直接看出表格标题、行内容和当前行变化。
- `same-target release / keyboard activation / read only / !enable / static preview keeps state` 必须全部通过单测。
- 两个 preview 必须完整可见、不抖动，并且在所有 runtime 帧里保持静态一致。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_data_grid/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里保留显式列头和多列 cell，不是单列摘要列表。
- 相比 `settings_panel`：这里承载的是表格行，不是设置项组合。
- 相比 SDK `table`：这里补的是 Fluent `DataGrid` 的 reference 语义、交互和验收闭环。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `rollout board`
  - `audit board`
  - `release board`
  - `compact`
  - `read only`
  - `row selection`
- 保留的交互：
  - same-target touch release
  - 键盘 `Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧口径：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss / preview click` 收尾动作

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `data_grid` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_grid --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_data_grid/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1891 colors=186`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(60, 107) - (420, 243)`
  - 按 `y >= 243` 裁切底部 preview 区域后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Rollout board`、`Audit board` 与 `Release board` 三组 reference 状态，最终稳定帧已显式回到默认快照
