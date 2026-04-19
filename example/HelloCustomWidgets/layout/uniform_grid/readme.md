# UniformGrid 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF / UniformGrid`
- 对应组件：`UniformGrid`
- 当前保留形态：`Operations wall`、`Release wall`、`Density wall`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `same-target release`、`Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_uniform_grid`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`uniform_grid` 用来表达“多个等宽等高单元按固定网格均匀排布”的标准布局语义，适合设置入口墙、功能分组页、仪表盘快捷入口和状态卡片矩阵这类需要稳定栅格密度的场景。

## 2. 为什么现有控件不够用
- `settings_panel` 和 `settings_card` 更偏纵向信息流，不表达均匀栅格排布。
- `data_list_panel` 和 `data_grid` 强调列表或表格语义，不适合做等尺寸 tile 矩阵。
- `items_repeater` 和 `wrap_panel` 更强调模板复用与自适应布局，不保证所有 cell 保持等宽等高。
- `virtualizing_wrap_panel` 与 `virtualizing_stack_panel` 更偏窗口化容器，不替代固定网格的轻量 reference 页面。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `uniform_grid` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Operations wall`
  - `Release wall`
  - `Density wall`
- 录制最终稳定帧显式回到默认 `Operations wall`。
- 底部左侧是 `Compact` 静态 preview，只负责对照小尺寸下的 tile 密度。
- 底部右侧是 `Read only` 静态 preview，只负责对照冻结后的弱化视觉。
- 两个 preview 统一通过 `egui_view_uniform_grid_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_cell`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/uniform_grid/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Operations wall`
2. 快照 2
   `Release wall`
3. 快照 3
   `Density wall`
4. 最终稳定帧
   回到默认 `Operations wall`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部 preview 行：`216 x 78`
- 单个 preview：`104 x 78`
- 页面结构：标题 -> 主 `uniform_grid` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 `badge / title / meta / body` 层级；主区突出同一 tile shell 在不同 snapshot 下的均匀网格密度；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Operations wall` | `Compact grid` | `Read only grid` |
| 快照 2 | `Release wall` | 保持不变 | 保持不变 |
| 快照 3 | `Density wall` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Operations wall` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_uniform_grid.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认单元回落、空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()`、`set_current_cell()` 的 pressed 清理与状态更新。
3. `get_cell_region()`、`activate_current_cell()` 与 listener 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
5. 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter` 行为，含跨 snapshot 的 `Tab` 切换。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_cell / compact_mode / read_only_mode` 不变并清理 pressed；恢复后继续验证 `Home / End + Enter`。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_cell / compact_mode / read_only_mode` 不变，同时不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Operations wall`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Release wall`，等待 `UNIFORM_GRID_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Density wall`，等待 `UNIFORM_GRID_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Operations wall`，同时重放底部 preview 固定状态，等待 `UNIFORM_GRID_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `UNIFORM_GRID_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 same-target release 与键盘导航语义，供手动复核和单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/uniform_grid PORT=pc

# 在 X:\ 短路径下执行
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

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Operations wall`、`Release wall`、`Density wall` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、键盘导航与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_cell / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_uniform_grid` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_uniform_grid/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(60, 99) - (419, 252)`
  - 以 `y >= 253` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Operations wall`

## 12. 与现有控件的边界
- 相比 `items_repeater`：这里强调等宽等高网格，而不是模板重复布局。
- 相比 `wrap_panel`：这里不允许单元因内容或宽度权重改变外框大小。
- 相比 `data_grid`：这里是 tile matrix，而不是表格列语义。
- 相比 `virtualizing_wrap_panel`：这里保留固定网格页面语义，不引入窗口化虚拟化轨道。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Operations wall`
  - `Release wall`
  - `Density wall`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/uniform_grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_uniform_grid/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/uniform_grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_uniform_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.186 colors=361`
- 截图复核结论：
  - 主区覆盖 `Operations wall / Release wall / Density wall` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Operations wall`
  - 主区差分边界收敛到 `(60, 99) - (419, 252)`
  - 以 `y >= 253` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
