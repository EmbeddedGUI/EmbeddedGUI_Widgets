# DataGrid 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI DataGrid`
- 对应组件：`DataGrid`
- 当前保留形态：`Rollout board`、`Audit board`、`Release board`、`Compact grid`、`Read only grid`
- 当前保留交互：主区保留行选择、same-target release 与键盘行导航/激活闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：旧 preview 清主区焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/data_grid`，底层仍复用仓库内现有 `egui_view_data_grid` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`data_grid` 用来表达“列头 + 多行数据 + 当前行选择 + 行激活”的标准表格语义。它不是设置列表或卡片集合，而是 Fluent 里承接发布检查、审阅列表和运营看板的 reference 表格控件。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏单列摘要列表，不强调显式列头和表格结构。
- `settings_panel / settings_expander` 面向设置页，不适合承载多列业务数据。
- `card_control / card_action / card_expander` 都是卡片摘要语义，不是标准表格。
- SDK 原生 `table` 只提供基础单元格绘制，不包含当前仓库需要的 `DataGrid` reference 页面、静态 preview 和验收闭环。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `data_grid` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Rollout board`
  - `Audit board`
  - `Release board`
- 录制最终稳定帧显式回到默认 `Rollout board`。
- 底部左侧是 `Compact` 静态 preview，固定对照 `Compact grid`，保持 `compact_mode = 1`。
- 底部右侧是 `Read only` 静态 preview，固定对照 `Read only grid`，保持 `compact_mode = 1`、`read_only_mode = 1`。
- 两个 preview 统一通过 `egui_view_data_grid_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 清理残留 `pressed_row` 与 `is_pressed`
  - 不改 `current_snapshot / current_row / compact_mode / read_only_mode`
  - 不触发 `on_action_listener`

目标目录：
- `example/HelloCustomWidgets/layout/data_grid/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Rollout board`
2. 快照 2
   `Audit board`
3. 快照 3
   `Release board`
4. 最终稳定帧
   回到默认 `Rollout board`

底部 preview 在整条轨道中固定为：
1. `Compact grid`
2. `Read only grid`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 248`
- 主控件：`196 x 126`
- 底部 preview 行：`216 x 80`
- 单个 preview：`104 x 80`
- 页面结构：标题 -> 主 `data_grid` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量分隔线；当前行通过左侧 selection strip、行底色和轻量 tone 差异表达，不依赖重阴影；底部 preview 固定为静态 reference 对照，不再承担焦点桥接、额外轨道切换或录制收尾职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Rollout board` | `Compact grid` | `Read only grid` |
| 快照 2 | `Audit board` | 保持不变 | 保持不变 |
| 快照 3 | `Release board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Rollout board` | 保持不变 | 保持不变 |
| 行选择、same-target release、键盘导航与激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_data_grid.c` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认态回退，以及 `current_snapshot / current_row` 重置。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()` 的 `pressed` 清理与状态更新；`set_current_snapshot()`、`set_current_row()` 的状态同步。
3. 行 region 命中、`activate_current_row()` 与 `on_action_listener` 触发。
4. 触摸 same-target release 与 `ACTION_CANCEL` 语义。
5. 键盘 `Home / End / Up / Down / Tab / Enter` 导航与激活闭环。
6. `read_only / !enable` 守卫清理残留 `pressed` 并屏蔽 `touch / key / navigation`；恢复后重新允许 `HOME / END + ENTER`。
7. static preview 吞掉 `touch / key / navigation`，并保持 `current_snapshot / current_row / compact_mode / read_only_mode` 不变，同时不触发 `on_action_listener`。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- setter、guard 与 static preview 路径都统一要求先清理残留 `pressed_row / is_pressed`，再处理后续状态。
- 当前 preview 用例继续显式校验 `g_action_count == 0`、`pressed_row == EGUI_VIEW_DATA_GRID_ROW_NONE`、`is_pressed == false`，以及 compact preview 的 `current_snapshot / current_row / compact_mode / read_only_mode` 保持不变。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Rollout board`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `DATA_GRID_RECORD_FRAME_WAIT`。
2. 切到 `Audit board`，等待 `DATA_GRID_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照，等待 `DATA_GRID_RECORD_FRAME_WAIT`。
4. 切到 `Release board`，等待 `DATA_GRID_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `DATA_GRID_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Rollout board`，同时重放底部 preview 固定状态并把焦点回到主区，等待 `DATA_GRID_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `DATA_GRID_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- `apply_primary_default_state()` 与 `apply_preview_states()` 只在 `ui_ready` 后触发布局，避免挂载前后的布局口径分叉。
- README 这里按当前 `test.c` 如实保留首轮切换使用 `DATA_GRID_RECORD_FINAL_WAIT`、第二轮切换与默认回落使用 `DATA_GRID_RECORD_WAIT`、抓帧使用 `DATA_GRID_RECORD_FRAME_WAIT`、最终抓帧使用 `DATA_GRID_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Rollout board`、`Audit board`、`Release board` 三组可识别状态，最终稳定帧必须回到默认 `Rollout board`。
- 主区真实交互仍需保留行选择、same-target release、`read_only / !enable` 守卫和键盘行激活语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key / navigation`，且不改 `current_snapshot / current_row / compact_mode / read_only_mode`。
- static preview、setter 和 guard 路径都必须先清理残留 `pressed_row / is_pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_data_grid` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_data_grid/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(60, 107) - (420, 243)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 243` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Rollout board`

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里保留显式列头和多列 cell，不是单列摘要列表。
- 相比 `settings_panel`：这里承载的是表格行，不是设置项组合。
- 相比卡片容器：这里强调标准表格外壳、行选择和激活，不是摘要卡片集合。
- 相比 SDK `table`：这里补的是 Fluent `DataGrid` 的 reference 语义、交互和验收闭环。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Rollout board`
  - `Audit board`
  - `Release board`
- 保留的底部对照：
  - `Compact grid`
  - `Read only grid`
- 保留的交互：
  - `row selection`
  - same-target touch release
  - 键盘行导航与激活
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss / preview click` 收尾动作
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `data_grid` suite `7 / 7`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_data_grid/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1891 colors=186`
- 截图复核结论：
  - 主区覆盖 `Rollout board / Audit board / Release board` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Rollout board`
  - 主区 RGB 差分边界收敛到 `(60, 107) - (420, 243)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact grid / Read only grid` preview 以 `y >= 243` 裁切后全程保持单哈希静态
