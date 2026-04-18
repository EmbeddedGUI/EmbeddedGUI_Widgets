# grid 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF Grid`
- 对应组件语义：`Grid`
- 本次保留语义：`Two equal columns / Dense board / Review stack / Stack / Dense / equal-width columns`
- 本次删除内容：行列跨越、共享尺寸组、自适应测量规则、复杂约束求解、拖拽改列，以及旧录制末尾“恢复后立即抓帧”的模板化收尾
- EGUI 适配说明：继续复用 SDK `egui_view_gridlayout` 作为底层布局能力；custom 层只补列数 style helper、静态 preview API、reference 页面结构与验收闭环，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid` 用来表达“同一组信息块按固定列数对齐排布”的基础布局语义，适合摘要卡片、轻量 dashboard、设置总览和审阅面板。当前仓库已有 `uniform_grid`、`grid_view`、`data_grid` 等更高层网格控件，但仍缺一个只负责列布局语义、能稳定承载 reference 页面与静态 preview 的基础 `Grid`。

## 2. 为什么现有控件不够用
- `uniform_grid` 更偏等尺寸 tile 集合，不是基础布局容器。
- `grid_view` 和 `data_grid` 自带更强的数据与交互语义，层级更高。
- `wrap_panel` 更强调流式换行，不表达固定列数的等宽列语义。
- 纯 `linearlayout` 很难直接表达 Fluent / WPF 里的多列对齐布局。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `grid` 面板 -> 底部 `Stack / Dense` 双静态 preview。
- 主控件负责导出三组主区状态：
  - `Two equal columns`
  - `Dense board`
  - `Review stack`
- 底部左侧是 `Stack` 静态 preview，固定展示单列摘要布局。
- 底部右侧是 `Dense` 静态 preview，固定展示三列紧凑布局。
- 两个 preview 统一通过 `hcw_grid_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `col_count / align_type`
  - 不触发布局外的额外交互

目标目录：`example/HelloCustomWidgets/layout/grid/`

## 4. 视觉与布局规格
- 根布局：`224 x 214`
- 主面板：`196 x 118`
- 主 `grid`：`176 x 68`
- 底部对照行：`216 x 74`
- 单个 preview 面板：`104 x 74`
- preview `grid`：`84 x 38`
- 风格约束：
  - 保持浅色 Fluent surface、低噪音色块和轻量信息层级。
  - 主区只保留列数与卡片排布变化，不叠加额外交互 chrome。
  - 底部 preview 全程静态，只承担 reference 对照职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_grid` | `egui_view_gridlayout_t` | `176 x 68` | `Two equal columns` | 主 `Grid` |
| `stack_preview_grid` | `egui_view_gridlayout_t` | `84 x 38` | `Stack` | 单列静态 preview |
| `dense_preview_grid` | `egui_view_gridlayout_t` | `84 x 38` | `Dense` | 三列静态 preview |
| `primary_snapshots` | `grid_snapshot_t[3]` | - | `Two equal / Dense / Review` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Two equal columns` | 默认两列卡片布局 |
| 主控件 | `Dense board` | 三列紧凑排布 |
| 主控件 | `Review stack` | 单列拉伸摘要行 |
| `Stack` preview | `Stack` | 固定静态对照，验证单列语义 |
| `Dense` preview | `Dense` | 固定静态对照，验证三列语义 |

## 7. 交互语义与单测要求
- `apply_standard_style()`、`apply_dense_style()`、`apply_stack_style()`、`set_columns()`、`set_align_type()` 之后都不能残留旧的 `pressed` 高亮。
- `layout_childs()` 必须覆盖：
  - 标准两列布局
  - 三列 dense 布局
  - 单列 stack 布局
- static preview 用例必须验证：
  - `touch / dispatch_key_event()` 输入会被消耗
  - `col_count / align_type` 保持不变
  - cell regions 保持不变
  - `on_click_listener` 不触发
  - `pressed / is_pressed` 被清理
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用 `on_key_event()`

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部双 preview，直接输出默认 `Two equal columns`
2. 切到 `Dense board`
3. 输出第二帧
4. 切到 `Review stack`
5. 输出第三帧
6. 恢复默认主状态
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `Stack / Dense` preview 在整条 reference 轨道里保持静态一致，不再包含额外 preview 切换或收尾刷新动作。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换和最终稳定抓帧使用 `GRID_RECORD_FINAL_WAIT`，中间状态切换仍保留 `GRID_RECORD_WAIT / GRID_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
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
- 主控件三组主状态必须能直接看出列数切换与卡片排布变化。
- `2 -> 3 -> 1` 列切换过程中不能出现重叠、裁切、整块缺失或残留旧布局。
- 单测里的 style helper、布局路径、`dispatch_key_event()` 入口和 static preview 状态保持断言必须全部通过。
- 两个 preview 必须完整可见，不黑白屏、不抖动，并且在所有 runtime 帧里保持静态一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区域

## 12. 与现有控件的边界
- 相比 `uniform_grid`：这里强调基础列布局语义，不是 tile 集合。
- 相比 `grid_view`：这里不承载数据项视图与选择交互，只保留布局本体。
- 相比 `data_grid`：这里没有表格列头、单元格语义和数据行为。
- 相比 `wrap_panel`：这里不依赖流式换行，列数是显式可控的。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Two equal columns`
  - `Dense board`
  - `Review stack`
  - `Stack`
  - `Dense`
  - `equal-width columns`
- 删减的旧桥接与旧口径：
  - 行列跨越与共享尺寸组
  - 自适应测量规则和复杂约束求解
  - 拖拽改列
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `grid` suite `4 / 4`
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
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_grid/default`
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
  - 主区变化边界保持在 `(98, 106) - (379, 233)`
  - 按 `y >= 234` 裁切底部 preview 区域后保持单一哈希，确认 `Stack / Dense` preview 全程静态
  - 结论：主区覆盖默认 `Two equal columns`、`Dense board` 与 `Review stack` 三组 reference 状态，最终稳定帧已显式回到默认快照
