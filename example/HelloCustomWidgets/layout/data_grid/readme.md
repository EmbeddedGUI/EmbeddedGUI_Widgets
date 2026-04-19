# DataGrid 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI DataGrid`
- 对应组件：`DataGrid`
- 当前保留形态：`Rollout board`、`Audit board`、`Release board`、`compact`、`read only`
- 当前保留交互：主区保留行选择、same-target release、`Up / Down / Home / End / Tab / Enter / Space` 键盘导航与静态 preview 对照
- 当前移除内容：旧 preview 清焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾，以及旧版 finalize README 章节结构
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_data_grid` 封装；本轮只收口 reference 页面结构、录制轨道、静态 preview 语义和 README，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`data_grid` 用来表达“列头 + 多行数据 + 当前行选择 + 行激活”的标准表格语义，适合发布检查、审阅列表、任务排期和轻量运营看板这类需要在有限空间里并排比较多列字段的场景。

仓库里已有 `data_list_panel`、`settings_panel`、`settings_expander` 和多种卡片容器，但仍缺一个能稳定承接 `DataGrid` 表头、多列行数据和当前行激活语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏单列摘要列表，不强调显式列头和表格结构。
- `settings_panel / settings_expander` 面向设置页，不适合承载多列业务数据。
- `card_control / card_action / card_expander` 都是卡片摘要语义，不是标准表格。
- SDK 原生 `table` 只提供基础单元格绘制，不包含当前仓库需要的 `DataGrid` reference 页面、静态 preview 和验收闭环。

## 3. 当前页面结构
- 标题：`Data Grid`
- 主区：1 个保留真实表格导航语义的 `data_grid`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact grid`
- 右侧 preview：`read only`，固定显示 `Read only grid`

目录：
- `example/HelloCustomWidgets/layout/data_grid/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Rollout board`
2. 快照 2
   `Audit board`
3. 快照 3
   `Release board`
4. 最终稳定帧
   回到默认 `Rollout board`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact grid`
2. `read only`
   `Read only grid`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 248`
- 主控件：`196 x 126`
- 底部 preview 行：`216 x 80`
- 单个 preview：`104 x 80`
- 页面结构：标题 -> 主 `data_grid` -> 底部 `compact / read only`
- 风格约束：浅色 Fluent 容器、低噪音边框和轻量分隔线；当前行通过左侧 selection strip、行底色和轻量 tone 差异表达，不依赖重阴影；底部 preview 全程静态，不再承担焦点回收、额外轨道切换或录制收尾职责

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Rollout board` | `Compact grid` | `Read only grid` |
| 快照 2 | `Audit board` | 保持不变 | 保持不变 |
| 快照 3 | `Release board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Rollout board` | 保持不变 | 保持不变 |
| same-target release / 键盘行导航与激活 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Audit board`
4. 抓取第二组主区快照
5. 切到 `Release board`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `current_snapshot / current_row`、same-target release 和键盘行导航语义，供运行时手动复核。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `egui_view_data_grid_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，主区首轮切换与最终稳定抓帧使用 `DATA_GRID_RECORD_FINAL_WAIT`，中间状态切换保留 `DATA_GRID_RECORD_WAIT / DATA_GRID_RECORD_FRAME_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_data_grid.c` 当前覆盖 `7` 条用例，分为两部分：

1. 主控件交互与状态守卫
   覆盖 `set_snapshots()` clamp/default、setter 清理 `pressed`、区域激活 listener、same-target touch release、`Home / End / Up / Down / Tab / Enter` 键盘导航，以及 `read only / !enable` guard。
2. 静态 preview 输入抑制
   通过单独 preview widget 固定校验 `touch / dispatch_key_event()` 输入被吞掉后，`current_snapshot / current_row / compact_mode / read_only_mode` 保持不变，且 `g_action_count == 0`、`pressed_row` 与 `is_pressed` 会被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区录制只允许出现 `Rollout board`、`Audit board`、`Release board` 3 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留行选择、same-target release、键盘导航和当前行激活语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `current_snapshot / current_row / compact_mode / read_only_mode / on_action / is_pressed`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_data_grid/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(60, 107) - (420, 243)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 243` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里保留显式列头和多列 cell，不是单列摘要列表。
- 相比 `settings_panel`：这里承载的是表格行，不是设置项组合。
- 相比卡片容器：这里强调标准表格外壳、行选择和激活，不是摘要卡片集合。
- 相比 SDK `table`：这里补的是 Fluent `DataGrid` 的 reference 语义、交互和验收闭环。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Rollout board`
  - `Audit board`
  - `Release board`
  - `compact`
  - `read only`
- 本次保留交互：
  - `row selection`
  - same-target touch release
  - 键盘 `Up / Down / Home / End / Tab / Enter / Space`
- 删减的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss / preview click` 收尾动作
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `data_grid` suite `7 / 7`
  - 无关 warning：`test_split_view.c:186:13: warning: 'get_view_center' defined but not used`
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
  - 主区 RGB 差分边界为 `(60, 107) - (420, 243)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 243` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Rollout board`、`Audit board` 与 `Release board` 3 组 reference 快照，最终稳定帧显式回到默认 `Rollout board`，底部 `compact / read only` preview 全程静态
