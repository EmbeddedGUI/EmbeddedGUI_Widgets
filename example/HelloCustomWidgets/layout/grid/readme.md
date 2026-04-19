# Grid 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Grid`
- 对应组件：`Grid`
- 当前保留形态：`Two equal columns`、`Dense board`、`Review stack`、`Stack`、`Dense`
- 当前保留交互：主区保留 `2 / 3 / 1` 列切换、`equal-width columns` 对齐语义与静态 preview 对照
- 当前移除内容：行列跨越、共享尺寸组、自适应测量规则、复杂约束求解、拖拽改列、旧版 finalize README 章节结构，以及录制末尾“恢复后立即抓帧”的模板化收尾
- EGUI 适配说明：继续复用 SDK `egui_view_gridlayout` 作为底层布局能力，custom 层只保留 `hcw_grid_*` style helper、static preview API、reference 页面结构与验收闭环，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid` 用来表达“同一组信息块按固定列数对齐排布”的基础布局语义，适合摘要卡片、轻量 dashboard、设置总览和审阅面板。

仓库里已有 `uniform_grid`、`grid_view`、`data_grid` 和 `wrap_panel`，但仍缺一个能稳定承接 `Grid` 基础列布局语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `uniform_grid` 更偏等尺寸 tile 集合，不是基础布局容器。
- `grid_view` 和 `data_grid` 自带更强的数据与交互语义，层级更高。
- `wrap_panel` 更强调流式换行，不表达固定列数的等宽列语义。
- 纯 `linearlayout` 很难直接表达 Fluent / WPF 里的多列对齐布局。

## 3. 当前页面结构
- 标题：`Grid`
- 主区：1 个承接基础列布局语义的 `grid`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Stack`，固定显示单列摘要布局
- 右侧 preview：`Dense`，固定显示三列紧凑布局

目录：
- `example/HelloCustomWidgets/layout/grid/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Two equal columns`
2. 快照 2
   `Dense board`
3. 快照 3
   `Review stack`
4. 最终稳定帧
   回到默认 `Two equal columns`

底部 preview 在整条轨道中始终固定：

1. `Stack`
   `Static preview.`
2. `Dense`
   `Three columns.`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主面板：`196 x 118`
- 主 `grid`：`176 x 68`
- 底部 preview 行：`216 x 74`
- 单个 preview：`104 x 74`
- preview `grid`：`84 x 38`
- 页面结构：标题 -> 主 `grid` -> 底部 `Stack / Dense`
- 风格约束：浅色 Fluent surface、低噪音色块、轻量 heading 与 note 层级，以及稳定的列数和卡片排布差异，不回退到旧 demo 的额外交互 chrome 与模板化收尾

## 6. 状态矩阵
| 状态 | 主控件 | Stack preview | Dense preview |
| --- | --- | --- | --- |
| 默认显示 | `Two equal columns` | `Stack` | `Dense` |
| 快照 2 | `Dense board` | 保持不变 | 保持不变 |
| 快照 3 | `Review stack` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Two equal columns` | 保持不变 | 保持不变 |
| `set_columns()` / `set_align_type()` / `layout_childs()` 列布局 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Dense board`
4. 抓取第二组主区快照
5. 切到 `Review stack`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `set_columns()`、`set_align_type()` 与 `layout_childs()` 列布局语义，供运行时手动复核。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `hcw_grid_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，主区首轮切换和最终稳定抓帧使用 `GRID_RECORD_FINAL_WAIT`，中间状态切换保留 `GRID_RECORD_WAIT / GRID_RECORD_FRAME_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_grid.c` 当前覆盖 `4` 条用例，分为两部分：

1. 主控件布局与状态守卫
   覆盖 `apply_standard_style()`、`apply_dense_style()`、`apply_stack_style()`、`set_columns()`、`set_align_type()` 清理 `pressed`，以及标准两列、dense 三列、stack 单列三组布局结果。
2. 静态 preview 输入抑制
   通过独立 preview `grid` 固定校验 `touch / dispatch_key_event()` 输入被吞掉后，`col_count / align_type / cell regions / on_click_listener` 结果保持不变，且 `is_pressed` 和点击计数会被清理。

同时要求：
- `g_click_count == 0`
- preview `col_count == 3`
- preview `align_type == EGUI_ALIGN_LEFT`
- preview `is_pressed == false`
- 所有 preview cell `region` 保持不变

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区录制只允许出现 `Two equal columns`、`Dense board`、`Review stack` 3 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实布局仍需保留 `2 -> 3 -> 1` 列切换和卡片排布差异，不能出现重叠、裁切或残留旧布局。
- 底部 `Stack / Dense` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `col_count / align_type / region / on_click_listener / is_pressed`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(98, 106) - (379, 233)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 234` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `uniform_grid`：这里强调基础列布局语义，不是 tile 集合。
- 相比 `grid_view`：这里不承载数据项视图与选择交互，只保留布局本体。
- 相比 `data_grid`：这里没有表格列头、单元格语义和数据行为。
- 相比 `wrap_panel`：这里不依赖流式换行，列数是显式可控的。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Two equal columns`
  - `Dense board`
  - `Review stack`
  - `Stack`
  - `Dense`
- 本次保留交互：
  - `equal-width columns`
  - static preview consumes input
- 删减的装饰或桥接：
  - 行列跨越与共享尺寸组
  - 自适应测量规则和复杂约束求解
  - 拖拽改列
  - 旧版 finalize README 章节结构
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `grid` suite `4 / 4`
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
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_grid/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1672 colors=171`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(98, 106) - (379, 233)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 234` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Two equal columns`、`Dense board` 与 `Review stack` 3 组 reference 快照，最终稳定帧显式回到默认 `Two equal columns`，底部 `Stack / Dense` preview 全程静态
