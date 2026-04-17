# ProgressRing 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressRing`
- 对应组件名：`ProgressRing`
- 当前保留状态：`standard`、`compact`、`paused`、`indeterminate`
- 当前移除内容：多环 showcase 语义、页面级 guide、与进度环无关的场景装饰、旧 preview 只验证 consume 的表述
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

当前目录：`example/HelloCustomWidgets/feedback/activity_ring/`

## 4. 主区 reference 轨道
主控件录制轨道保持 `indeterminate -> determinate -> indeterminate` 的主线语义：

1. `Syncing...`
   主环处于 `indeterminate`，用于表达后台同步中的 loading。
2. `Syncing...`
   继续保留 `indeterminate`，但让环段推进一帧，确保 runtime 截图能看出动画前进。
3. `86% active`
   程序化退出 `indeterminate`，回落到 determinate 完成态。
4. `Syncing...`
   回到默认 loading 态，作为最终稳定帧。

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `38%`
   `compact` palette
2. `paused`
   `56%`
   `paused` palette

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
| `indeterminate` 动画 | 是 | 否 | 否 |
| determinate 百分比 | 是 | 是 | 是 |
| `compact` palette | 否 | 是 | 否 |
| `paused` palette | 否 | 否 | 是 |
| `set_value()` 退出 `indeterminate` | 是 | 否 | 否 |
| attach 后启动 timer | 是 | 否 | 否 |
| detach 后停止 timer | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主环 `indeterminate` 初始态和底部 preview 固定值
2. 抓取首帧
3. 等待动画推进
4. 抓取第二帧 loading
5. 程序化切到 `86% active`
6. 抓取 determinate 完成态
7. 回到默认 `Syncing...`
8. 再抓取一帧最终稳定帧

说明：
- 录制阶段不再让底部 preview 承担任何状态切换或桥接职责。
- 主区动画继续保留，但底部 preview 通过 `hcw_activity_ring_override_static_preview_api()` 完全收口为静态 reference。
- 状态文案与主环一起作为主区变化的一部分参与 runtime 复核。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `ACTIVITY_RING_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0`、默认态恢复和最终稳定帧都统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_activity_ring.c` 当前覆盖五部分：

1. 样式 helper
   覆盖 `apply_standard_style()`、`apply_compact_style()`、`apply_paused_style()` 的几何、palette 和 `pressed` 清理。
2. `set_value()` clamp 与退出动画态
   覆盖数值钳制，以及从 `indeterminate` 回落到 determinate 的行为。
3. `indeterminate` attach / tick / detach 生命周期
   覆盖 attach 后启动 timer、tick 推进 phase，以及 detach 后停止 timer。
4. palette setter
   覆盖 `set_palette()` 对颜色更新和残留 `pressed` 的清理。
5. 静态 preview 不变性断言
   通过 `activity_ring_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / on_click_listener / api / ring_count / stroke_width / ring_gap / start_angle / show_round_cap / ring_color / ring_bg_color / values / timer_started / indeterminate_mode / phase / determinate_value / base_start_angle / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为 “consumes input and keeps state”。
- 事件分发已统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc`
- `HelloUnitTest`：`PASS`，已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `activity_ring` suite `5 / 5`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_activity_ring/default`
- feedback 分类 compile/runtime 回归：`PASS`
  compile `10 / 10`，runtime `10 / 10`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_activity_ring`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1434 colors=155`

## 11. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_activity_ring/default`

- 总帧数：`9`
- 主区 RGB 差分边界：`(179, 131) - (301, 287)`
- 遮罩主区变化边界后，主区外唯一哈希数：`1`
- 按主区边界裁切后，主区唯一状态数：`5`
- 按 `y >= 289` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `activity_ring` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区出现 `5` 组唯一状态，对应 `indeterminate` 环段推进、`86% active` 的 determinate 完成态，以及最终回到默认 `Syncing...` loading 轨道后的稳定帧。
- 按 `y >= 289` 裁切底部 preview 区域后保持单哈希，确认 `compact / paused` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前不叠加剩余时间、步骤说明或任务图标。
- 当前 `indeterminate` 仍是 custom 层的轻量 reference 动画，不追求完整 WinUI easing。
- 当前只维护单环 `ProgressRing`，不恢复旧的多环 activity demo。
- 底部 preview 只承担 reference 对照，不承担真实交互职责。

## 13. 与现有控件的边界
- 相比 `progress_bar`：这里表达圆形 `ProgressRing`，不是线性进度条。
- 相比 `spinner`：这里既保留等待态，也保留 determinate 百分比语义。
- 相比 `skeleton`：这里表达任务进度，不是内容占位。

## 14. EGUI 适配说明
- determinate 单环继续使用 SDK `activity_ring` 的原始绘制，不修改底层 widget。
- `indeterminate` 通过 custom 层附加状态表、attach / detach timer 与 `start_angle + arc length` 的轻量动画补齐。
- `set_value()` 会退出 `indeterminate`，恢复 determinate 百分比语义。
- preview 统一走静态 API，不参与真实交互，也不启动动画 timer。
