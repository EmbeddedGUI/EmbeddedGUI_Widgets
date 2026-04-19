# Grid 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Grid`
- 对应组件：`Grid`
- 当前保留形态：`Two equal columns`、`Dense board`、`Review stack`、`Stack`、`Dense`
- 当前保留交互：主区保留 `2 / 3 / 1` 列切换与 `equal-width columns` 对齐语义；底部 `Stack / Dense` preview 保持静态 reference 对照
- 当前移除内容：行列跨越、共享尺寸组、自适应测量规则、复杂约束求解、拖拽改列、旧版 finalize README 章节结构，以及录制末尾“恢复后立即抓帧”的模板化收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/grid`，底层仍复用 SDK `egui_view_gridlayout`；custom 层只保留 `hcw_grid_*` style helper、static preview API、reference 页面结构与验收闭环，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid` 用来表达“同一组信息块按固定列数对齐排布”的基础布局语义。它不是 tile 集合或数据视图，而是 Fluent / WPF 里承接摘要卡片、轻量 dashboard 和设置总览的 reference 布局控件。

## 2. 为什么现有控件不够用
- `uniform_grid` 更偏等尺寸 tile 集合，不是基础布局容器。
- `grid_view` 和 `data_grid` 自带更强的数据与交互语义，层级更高。
- `wrap_panel` 更强调流式换行，不表达固定列数的等宽列语义。
- 纯 `linearlayout` 很难直接表达 Fluent / WPF 里的多列对齐布局。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `grid` -> 底部 `Stack / Dense` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Two equal columns`
  - `Dense board`
  - `Review stack`
- 录制最终稳定帧显式回到默认 `Two equal columns`。
- 底部左侧是 `Stack` 静态 preview，固定对照单列摘要布局，保持 `col_count = 1`。
- 底部右侧是 `Dense` 静态 preview，固定对照三列紧凑布局，保持 `col_count = 3`。
- 两个 preview 统一通过 `hcw_grid_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 清理残留 `is_pressed`
  - 不改 `col_count / align_type`
  - 不改子项 `region`
  - 不触发 `on_click_listener`

目标目录：
- `example/HelloCustomWidgets/layout/grid/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Two equal columns`
2. 快照 2
   `Dense board`
3. 快照 3
   `Review stack`
4. 最终稳定帧
   回到默认 `Two equal columns`

底部 preview 在整条轨道中固定为：
1. `Stack`
2. `Dense`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主面板：`196 x 118`
- 主 `grid`：`176 x 68`
- 底部 preview 行：`216 x 74`
- 单个 preview：`104 x 74`
- preview `grid`：`84 x 38`
- 页面结构：标题 -> 主 `grid` -> 底部 `Stack / Dense`
- 风格约束：保持浅色 Fluent surface、低噪音色块、轻量 heading 与 note 层级，以及稳定的列数和卡片排布差异；底部 preview 固定为静态 reference 对照，不回退到旧 demo 的额外交互 chrome 与模板化收尾。

## 6. 状态矩阵
| 状态 | 主控件 | Stack preview | Dense preview |
| --- | --- | --- | --- |
| 默认显示 | `Two equal columns` | `Stack` | `Dense` |
| 快照 2 | `Dense board` | 保持不变 | 保持不变 |
| 快照 3 | `Review stack` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Two equal columns` | 保持不变 | 保持不变 |
| `set_columns()` / `set_align_type()` / `layout_childs()` 列布局 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_grid.c` 当前覆盖 `4` 条用例：

1. `hcw_grid_apply_standard_style()`、`hcw_grid_apply_dense_style()`、`hcw_grid_apply_stack_style()`、`hcw_grid_set_columns()`、`hcw_grid_set_align_type()` 的 `pressed` 清理与状态更新。
2. `hcw_grid_layout_childs()` 在标准两列、dense 三列和 stack 单列三组布局下的放置结果。
3. `hcw_grid_layout_childs()` 本身也会清理残留 `pressed`。
4. static preview 吞掉 `touch / key` 后保持 `col_count / align_type / cell regions / on_click_listener` 不变，并清理 `is_pressed` 与点击计数。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- style helper、`set_columns()`、`set_align_type()`、`layout_childs()` 与 static preview 路径都统一要求先清理残留 `is_pressed`，再处理后续状态。
- 当前 preview 用例继续显式校验 `g_click_count == 0`、preview `col_count == 3`、preview `align_type == EGUI_ALIGN_LEFT`、preview `is_pressed == false`，以及全部 preview cell `region` 保持不变。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Two equal columns`，同时重放底部 `Stack / Dense` preview 固定状态并抓取首帧，等待 `GRID_RECORD_FRAME_WAIT`。
2. 切到 `Dense board`，等待 `GRID_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照，等待 `GRID_RECORD_FRAME_WAIT`。
4. 切到 `Review stack`，等待 `GRID_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `GRID_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Two equal columns`，同时重放底部 preview 固定状态，等待 `GRID_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `GRID_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Stack / Dense` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- `apply_primary_default_state()` 与 `apply_preview_states()` 只在 `ui_ready` 后触发布局，避免挂载前后的布局口径分叉。
- README 这里按当前 `test.c` 如实保留首轮切换、默认回落与最终抓帧使用 `GRID_RECORD_FINAL_WAIT`，中间状态切换使用 `GRID_RECORD_WAIT`，抓帧使用 `GRID_RECORD_FRAME_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Two equal columns`、`Dense board`、`Review stack` 三组可识别状态，最终稳定帧必须回到默认 `Two equal columns`。
- 主区真实布局仍需保留 `2 -> 3 -> 1` 列切换和卡片排布差异，不能出现重叠、裁切或残留旧布局。
- 底部 `Stack / Dense` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key`，不能改写 `col_count / align_type / region / on_click_listener / is_pressed`。
- style helper、`set_columns()`、`set_align_type()`、`layout_childs()` 与 static preview 路径都必须先清理残留 `is_pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_grid` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(98, 106) - (379, 233)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 234` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Two equal columns`

## 12. 与现有控件的边界
- 相比 `uniform_grid`：这里强调基础列布局语义，不是 tile 集合。
- 相比 `grid_view`：这里不承载数据项视图与选择交互，只保留布局本体。
- 相比 `data_grid`：这里没有表格列头、单元格语义和数据行为。
- 相比 `wrap_panel`：这里不依赖流式换行，列数是显式可控的。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Two equal columns`
  - `Dense board`
  - `Review stack`
- 保留的底部对照：
  - `Stack`
  - `Dense`
- 保留的交互：
  - `equal-width columns`
  - `2 / 3 / 1` 列切换
  - static preview 输入吞掉
- 删减的旧桥接与旧轨道：
  - 行列跨越与共享尺寸组
  - 自适应测量规则和复杂约束求解
  - 拖拽改列
  - 旧版 finalize README 章节结构
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `grid` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_grid/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1672 colors=171`
- 截图复核结论：
  - 主区覆盖 `Two equal columns / Dense board / Review stack` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Two equal columns`
  - 主区 RGB 差分边界收敛到 `(98, 106) - (379, 233)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Stack / Dense` preview 以 `y >= 234` 裁切后全程保持单哈希静态
