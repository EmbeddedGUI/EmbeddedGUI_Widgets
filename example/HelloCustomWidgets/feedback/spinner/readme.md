# Spinner 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI Spinner`
- 对应组件：`Spinner`
- 当前保留形态：`Syncing files`、`Publishing docs`、`Refreshing cache`、`compact`、`muted`
- 当前保留交互：主区保留真实 `indeterminate` 旋转；`palette / stroke_width / arc_length / rotation_angle / set_spinning` setter 统一清理 `pressed`；底部 `compact / muted` preview 继续作为静态 reference 对照
- 当前移除内容：preview panel、preview heading、与等待指示无关的场景卡片和说明文案、旧 preview 输入只验证 consume 的单测口径，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用 SDK `egui_view_spinner` 的圆弧绘制和旋转能力，在 custom 层维持样式 helper、固定相位静态 preview API 与 `rotation_angle` setter，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`spinner` 用来表达“后台任务正在进行，但当前没有明确百分比”的等待态，适合附着在局部区域或轻量状态文案旁边。仓库里已经有偏进度语义的 `activity_ring` 和 `progress_bar`，但还需要一个更纯粹的 indeterminate loading 指示器，因此 `Spinner` 仍然需要保留在当前 `reference` 主线中。

## 2. 为什么现有控件不够用
- `activity_ring` 对齐的是 `ProgressRing`，仍保留 determinate 百分比语义。
- `progress_bar` 强调线性推进，不适合小尺寸等待指示。
- `skeleton` 更偏内容占位，不适合表达独立后台动作。
- SDK 自带 `spinner` 只有基础绘制和旋转能力，缺少当前仓库要求的统一 reference 页面结构与静态 preview 语义。

## 3. 当前页面结构
- 标题：`Spinner`
- 主区：一个主 `spinner`
- 主区下方：一行状态文案
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`muted`
- 页面结构统一收口为：标题 -> 主 `spinner` -> 状态文案 -> `compact / muted`

目录：
- `example/HelloCustomWidgets/feedback/spinner/`

## 4. 主区 reference 快照
主控件录制轨道保留 `3` 组主区 reference 语义，最终稳定帧回到默认 `Syncing files`；底部 preview 在整条录制轨道中始终保持不变：

1. 默认态
   `Syncing files`
2. 快照 2
   `Publishing docs`
3. 快照 3
   `Refreshing cache`
4. 最终稳定帧
   回到默认 `Syncing files`

底部 preview 在整条录制轨道中始终固定：
1. `compact`
   固定相位 `300°`
2. `muted`
   固定相位 `24°`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 152`
- 主 `spinner` 尺寸：`44 x 44`
- 状态文案尺寸：`196 x 14`
- 底部行容器尺寸：`88 x 40`
- 单个 preview 尺寸：`40 x 40`
- 页面结构：标题 -> 主 `spinner` -> 状态文案 -> 底部 `compact / muted`
- 页面风格：浅灰 page panel、低噪音浅色背景、品牌蓝主态和暖色 / 青绿色辅助态

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Muted preview |
| --- | --- | --- | --- |
| 默认显示 | `Syncing files` | 固定 `300°` | 固定 `24°` |
| `Publishing docs` | 是 | 否 | 否 |
| `Refreshing cache` | 是 | 否 | 否 |
| 最终稳定帧 | 回到默认 `Syncing files` | 保持不变 | 保持不变 |
| `is_spinning = 1` | 是 | 否 | 否 |
| `is_spinning = 0` | 否 | 是 | 是 |
| 固定 `rotation_angle` | 否 | 是 | 是 |
| `compact` style | 否 | 是 | 否 |
| `muted` style | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_spinner.c` 当前覆盖 `4` 条用例：

1. `style helpers apply expected geometry and palette`。
   覆盖 `apply_standard_style()`、`apply_compact_style()` 与 `apply_muted_style()` 的几何参数、配色和 `pressed` 清理。
2. `setters clear pressed state and clamp`。
   覆盖 `set_palette()`、`set_stroke_width()`、`set_arc_length()`、`set_rotation_angle()` 的钳制与 `pressed` 清理。
3. `set_spinning starts and stops animation`。
   覆盖 `set_spinning()` 在 attach 后启停 timer 的生命周期，以及 detach 后停止动画。
4. `static preview consumes input and keeps state`。
   通过 `spinner_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`on_click_listener`、`api`、`arc_length`、`rotation_angle`、`stroke_width`、`color`、`is_spinning`、`timer_started`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变，并要求 `g_click_count == 0`。

说明：
- 主 `spinner` 是 display-only 控件，不承接真实点击或键盘导航职责。
- 主控件保持真实 `indeterminate` 旋转，不回退到假静态截图。
- `compact / muted` preview 统一通过 `hcw_spinner_set_spinning(..., 0)` 停止旋转，并用 `hcw_spinner_set_rotation_angle()` 固定弧段相位。
- `compact / muted` preview 继续通过 `hcw_spinner_override_static_preview_api()` 吞掉 `touch / key`，只做静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主控件默认 `Syncing files` 和底部 preview 固定状态，请求首帧截图，等待 `SPINNER_RECORD_FRAME_WAIT`。
2. 切到 `Publishing docs`，等待 `SPINNER_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `SPINNER_RECORD_FRAME_WAIT`。
4. 切到 `Refreshing cache`，等待 `SPINNER_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `SPINNER_RECORD_FRAME_WAIT`。
6. 回到默认 `Syncing files`，等待 `SPINNER_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `SPINNER_RECORD_FINAL_WAIT`。

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化来自三组主 spinner 配色 / 弧段参数切换，以及录制期间的持续旋转推进。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，首帧和最终稳定帧都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/spinner --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/spinner
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_spinner
```

## 10. 验收重点
- 主区和底部 `compact / muted` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Syncing files`、`Publishing docs` 与 `Refreshing cache` 三组 reference 语义必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Syncing files`。
- 主区真实旋转、style helper、setter 钳制和 `set_spinning()` timer 生命周期收口后不能残留 `pressed` 或错误的 timer 状态。
- 底部 `compact / muted` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_feedback_spinner` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_spinner/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `7` 组唯一状态，对应 `Syncing files / Publishing docs / Refreshing cache` 三组主区语义与其间的旋转推进
  - 主区 RGB 差分边界为 `(195, 186) - (285, 248)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 249` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `activity_ring`：这里不表达 determinate 百分比，只表达等待态。
- 相比 `progress_bar`：这里不是线性进度条。
- 相比 `skeleton`：这里不表达内容占位，只表达后台动作正在进行。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Syncing files`
  - `Publishing docs`
  - `Refreshing cache`
- 保留的底部对照：
  - `compact`
  - `muted`
- 保留的交互与实现约束：
  - 主区真实 `indeterminate` 旋转
  - `palette / stroke_width / arc_length / rotation_angle / set_spinning` 共享 `pressed` 清理
  - attach / detach timer 生命周期
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - preview panel 与 preview heading
  - 与等待指示无关的场景卡片和说明文案
  - 旧 preview 输入只验证 consume 的单测口径
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `spinner` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/spinner --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_spinner/default`
  - 共捕获 `9` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/spinner`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_spinner`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1159 colors=105`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `7` 组唯一状态，对应 `Syncing files / Publishing docs / Refreshing cache` 三组主区语义与其间的旋转推进
  - 主区 RGB 差分边界为 `(195, 186) - (285, 248)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 以 `y >= 249` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区完整覆盖 `Spinner` 三组 reference 语义并保留真实旋转推进，最终稳定帧已回到默认 `Syncing files`，底部 `compact / muted` preview 全程静态
