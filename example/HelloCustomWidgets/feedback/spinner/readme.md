# Spinner 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI Spinner`
- 对应组件名：`Spinner`
- 当前保留状态：`syncing`、`publishing`、`refreshing`、`compact`、`muted`
- 当前移除内容：preview panel、preview heading、与等待指示无关的场景卡片和说明文案、旧 preview 输入只验证 consume 的单测口径
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

当前目录：`example/HelloCustomWidgets/feedback/spinner/`

## 4. 主区 reference 轨道
主控件录制轨道只保留三组主区 snapshot 和最终稳定帧：

1. `Syncing files`
   标准蓝色 spinner
2. `Publishing docs`
   暖色 spinner
3. `Refreshing cache`
   青绿色 spinner
4. `Syncing files`
   回到默认蓝色 spinner，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   固定相位 `300°`
   更短弧段、更轻笔画
2. `muted`
   固定相位 `24°`
   弱化灰蓝配色

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
| `Syncing files` | 是 | 否 | 否 |
| `Publishing docs` | 是 | 否 | 否 |
| `Refreshing cache` | 是 | 否 | 否 |
| `is_spinning = 1` | 是 | 否 | 否 |
| `is_spinning = 0` | 否 | 是 | 是 |
| 固定 `rotation_angle` | 否 | 是 | 是 |
| `compact` style | 否 | 是 | 否 |
| `muted` style | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `spinner` 仍是 display-only 控件，不承接真实点击或导航职责。
- 主控件保持真实 indeterminate 旋转，不回退到假静态截图。
- `compact / muted` preview 统一通过 `hcw_spinner_set_spinning(..., 0)` 停止旋转，并用 `hcw_spinner_set_rotation_angle()` 固定弧段相位。
- `compact / muted` preview 继续通过 `hcw_spinner_override_static_preview_api()` 吞掉 `touch / key`，只做静态 reference 对照。
- 本轮不修改 SDK 动画逻辑，只在 custom 层维持更明确的 preview 控制能力。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主控件默认 `Syncing files` 和底部 preview 固定状态
2. 请求首帧
3. 切到 `Publishing docs`
4. 请求第二帧
5. 切到 `Refreshing cache`
6. 请求第三帧
7. 回到默认 `Syncing files` 并请求最终稳定帧

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化来自三组主 spinner 配色 / 弧段参数切换，以及录制期间的持续旋转推进。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_spinner.c` 当前覆盖四部分：

1. 样式 helper 几何与 palette
2. `palette / stroke_width / arc_length / rotation_angle` setter 的钳制与 `pressed` 清理
3. `set_spinning()` 启停 timer 的生命周期
4. 静态 preview 不变性断言

其中静态 preview 用例通过 `spinner_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / on_click_listener / api / arc_length / rotation_angle / stroke_width / color / is_spinning / timer_started / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

## 11. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc`
- `HelloUnitTest`：`PASS`，在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `spinner` suite `4 / 4`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_spinner/default`
- feedback 分类 compile/runtime 回归：`PASS`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_spinner`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1159 colors=105`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_spinner/default`

- 总帧数：`9`
- 主区 RGB 差分边界：`(195, 186) - (285, 248)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`7`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `spinner` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区出现 `7` 组唯一状态，对应 `Syncing files / Publishing docs / Refreshing cache` 三组主区语义与其间的旋转推进，最终稳定帧回到默认 `Syncing files`。
- 按 `y >= 249` 裁剪底部 preview 区域后保持单哈希，确认 `compact / muted` preview 在整条录制轨道中始终静态一致。

## 13. 已知限制
- 当前只维护单个 `Spinner` 指示器，不实现遮罩、页面级 loading 或批量任务容器。
- 当前不显示百分比，不承担 determinate progress 语义。
- 当前固定相位 preview 只服务于 reference 截图稳定性，不追求更复杂动画节奏。

## 14. 与现有控件的边界
- 相比 `activity_ring`：这里不表达 determinate 百分比，只表达等待态。
- 相比 `progress_bar`：这里不是线性进度条。
- 相比 `skeleton`：这里不表达内容占位，只表达后台动作正在进行。

## 15. EGUI 适配说明
- 主区继续直接复用 SDK `egui_view_spinner` 的圆弧绘制和旋转 timer。
- `rotation_angle` 通过 custom setter 做归一化，便于固定静态 preview 相位。
- preview 统一走静态 API，不参与真实交互，也不启动旋转 timer。
- README、demo、单测和 runtime 验收口径已经对齐到当前 reference workflow。
