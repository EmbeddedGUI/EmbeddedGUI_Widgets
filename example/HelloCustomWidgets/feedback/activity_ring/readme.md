# ProgressRing 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressRing`
- 对应组件：`ProgressRing`
- 当前保留形态：`Syncing...`、`86% active`、`compact`、`paused`、`indeterminate`
- 当前保留交互：主区保留 `determinate / indeterminate` 双语义、`set_value()` 退出动画态与 attach / detach timer 生命周期；`style / palette / value` setter 统一清理 `pressed`；底部 `compact / paused` preview 继续作为静态 reference 对照
- 当前移除内容：多环 showcase 语义、页面级 guide、与进度环无关的场景装饰、旧 preview 只验证 consume 的表述，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用 SDK `activity_ring` 的单环绘制能力，在 custom 层补齐 `ProgressRing` 的 `determinate / indeterminate` 语义、timer 生命周期与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`activity_ring` 对齐的是 Fluent / WinUI 里的 `ProgressRing`，适合表达圆形进度或持续进行中的后台任务。仓库里已经有线性的 `progress_bar` 和纯等待态的 `spinner`，但还需要一个同时覆盖单环 determinate / indeterminate 语义的 reference 控件，因此这里继续保留并收口到当前主线工作流。

## 2. 为什么现有控件不够用
- `progress_bar` 面向线性进度条，不适合承担圆形进度语义。
- `spinner` 更偏向纯等待指示，不表达明确的百分比进度。
- SDK 原生 `activity_ring` 只有基础单环绘制能力，缺少当前 reference 页面需要的 `indeterminate` 生命周期和静态 preview 口径。

## 3. 当前页面结构
- 标题：`ProgressRing`
- 主区：一个主 `activity_ring`
- 主区下方：一行状态文案
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `38%`
- 右侧 preview：`paused`，固定显示 `56%`

目录：
- `example/HelloCustomWidgets/feedback/activity_ring/`

## 4. 主区 reference 快照
主控件录制轨道保留 `indeterminate -> determinate -> indeterminate` 的主线语义，最终稳定帧回到默认 `Syncing...`；底部 preview 在整条录制轨道中始终保持不变：

1. 默认态
   `Syncing...`
2. 快照 2
   `Syncing...`
   保持 `indeterminate`，但让环段推进一帧
3. 快照 3
   `86% active`
4. 最终稳定帧
   回到默认 `Syncing...`

底部 preview 在整条录制轨道中始终固定：
1. `compact`
   `38%`
2. `paused`
   `56%`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 188`
- 主 `activity_ring` 尺寸：`88 x 88`
- 状态文案尺寸：`196 x 14`
- 底部行容器尺寸：`104 x 48`
- 单个 preview 尺寸：`48 x 48`
- 页面结构：标题 -> 主 `activity_ring` -> 状态文案 -> 底部 `compact / paused`
- 页面风格：浅灰 page panel、低噪音背景、标准蓝主环、暖色 `paused` 对照

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Paused preview |
| --- | --- | --- | --- |
| 默认显示 | `Syncing...` | `38%` | `56%` |
| `indeterminate` 动画 | 是 | 否 | 否 |
| `86% active` | 是 | 否 | 否 |
| 最终稳定帧 | 回到默认 `Syncing...` | 保持不变 | 保持不变 |
| determinate 百分比 | 是 | 是 | 是 |
| `compact` palette | 否 | 是 | 否 |
| `paused` palette | 否 | 否 | 是 |
| `set_value()` 退出 `indeterminate` | 是 | 否 | 否 |
| attach 后启动 timer | 是 | 否 | 否 |
| detach 后停止 timer | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_activity_ring.c` 当前覆盖 `5` 条用例：

1. `style helpers apply expected geometry and palette`。
   覆盖 `apply_standard_style()`、`apply_compact_style()` 与 `apply_paused_style()` 的几何、palette 和 `pressed` 清理。
2. `value setter clamps and exits indeterminate mode`。
   覆盖 `set_value()` 的数值钳制，以及从 `indeterminate` 回落到 determinate 的行为。
3. `indeterminate attach tick and detach lifecycle`。
   覆盖 attach 后启动 timer、tick 推进 phase，以及 detach 后停止 timer。
4. `palette setter clears pressed state`。
   覆盖 `set_palette()` 对颜色更新和残留 `pressed` 的清理。
5. `static preview consumes input and keeps state`。
   通过 `activity_ring_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`on_click_listener`、`api`、`ring_count`、`stroke_width`、`ring_gap`、`start_angle`、`show_round_cap`、`ring_color`、`ring_bg_color`、`values`、`timer_started`、`indeterminate_mode`、`phase`、`determinate_value`、`base_start_angle`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变，并要求 `g_click_count == 0`。

说明：
- 主区继续保留 `determinate / indeterminate` 双语义：默认 loading，`set_value()` 后回落到百分比进度。
- 主控件是 display-first 控件，不承接 click 语义，重点在动画和数值状态切换。
- 底部 `compact / paused` preview 通过 `hcw_activity_ring_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主环默认 `Syncing...` 和底部 preview 固定值，请求首帧截图，等待 `ACTIVITY_RING_RECORD_FRAME_WAIT`。
2. 等待 `ACTIVITY_RING_RECORD_ANIM_WAIT`，让 `indeterminate` 动画推进。
3. 请求第二组主区 loading 快照，等待 `ACTIVITY_RING_RECORD_FRAME_WAIT`。
4. 程序化切到 `86% active`，等待 `ACTIVITY_RING_RECORD_WAIT`。
5. 请求 determinate 完成态快照，等待 `ACTIVITY_RING_RECORD_FRAME_WAIT`。
6. 回到默认 `Syncing...`，等待 `ACTIVITY_RING_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `ACTIVITY_RING_RECORD_FINAL_WAIT`。

说明：
- 录制阶段不再让底部 preview 承担任何状态切换或桥接职责。
- 主区动画继续保留，但底部 preview 通过 `hcw_activity_ring_override_static_preview_api()` 完全收口为静态 reference。
- 状态文案与主环一起作为主区变化的一部分参与 runtime 复核。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，首帧、默认态恢复和最终稳定帧都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/activity_ring --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/activity_ring
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_activity_ring
```

## 10. 验收重点
- 主区和底部 `compact / paused` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `indeterminate` loading、动画推进后的 loading 与 `86% active` determinate 完成态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Syncing...`。
- 主区 `indeterminate` 生命周期、`set_value()` 退出动画态、palette / style setter 清理和 attach / detach timer 链路收口后不能残留 `pressed` 或错误 timer 状态。
- 底部 `38% / 56%` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_feedback_activity_ring` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_activity_ring/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `5` 组唯一状态，对应 `indeterminate` 环段推进、`86% active` 的 determinate 完成态，以及最终回到默认 `Syncing...` 的稳定帧
  - 主区 RGB 差分边界为 `(179, 131) - (301, 287)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 289` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `progress_bar`：这里表达圆形 `ProgressRing`，不是线性进度条。
- 相比 `spinner`：这里既保留等待态，也保留 determinate 百分比语义。
- 相比 `skeleton`：这里表达任务进度，不是内容占位。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Syncing...`
  - `86% active`
- 保留的底部对照：
  - `compact`
  - `paused`
- 保留的交互与实现约束：
  - `determinate / indeterminate` 双语义
  - `set_value()` 退出动画态
  - attach / detach timer 生命周期
  - `style / palette / value` 共享 `pressed` 清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 多环 showcase 语义
  - 页面级 guide
  - 与进度环无关的场景装饰
  - 旧 preview 只验证 consume 的表述
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `activity_ring` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/activity_ring --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_activity_ring/default`
  - 共捕获 `9` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/activity_ring`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_activity_ring`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1434 colors=155`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `5` 组唯一状态，对应 `indeterminate` 环段推进、`86% active` 的 determinate 完成态，以及最终回到默认 `Syncing...` 的稳定帧
  - 主区 RGB 差分边界为 `(179, 131) - (301, 287)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 以 `y >= 289` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区完整覆盖 `ProgressRing` 的 loading / determinate 主线语义，最终稳定帧已回到默认 `Syncing...`，底部 `compact / paused` preview 全程静态
