# ProgressBar 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressBar`
- 对应组件名：`ProgressBar`
- 当前保留形态：`Syncing...`、`92% complete`、`paused`、`error`
- 当前保留交互：主区保留 `indeterminate` 动画推进、`set_value()` 退出 `indeterminate` 回到 determinate 百分比、attach / detach / tick timer 生命周期，以及 palette helper / setter 对 `pressed` 的统一清理；底部 `paused / error` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变
- 当前移除内容：页面级 guide、外部 preview 标签、场景化装饰、与进度条无关的说明壳层、旧 feat 口径文案，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用 SDK `progress_bar` 的 determinate 绘制能力，在 custom 层补齐 `indeterminate` 语义、timer 生命周期和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`progress_bar` 用来表达明确的线性进度，适合安装、同步、下载和后台处理任务。除了常规 determinate 百分比，Fluent / WinUI 主线里同样要求支持 `indeterminate` loading，因此这里需要同时保留线性进度和低噪音等待态。

## 2. 为什么现有控件不够
- `activity_ring` 面向圆形 `ProgressRing`，不适合线性进度条语义。
- `spinner` 更偏持续等待指示，不承担明确百分比语义。
- `skeleton` 是内容占位，不表达当前任务进度。
- SDK 自带 `progress_bar` 只有基础 determinate 绘制，没有当前 reference 页面需要的 `indeterminate` 动画与静态 preview 收口。

## 3. 当前页面结构
- 标题：`ProgressBar`
- 主区：一个主 `progress_bar`
- 主区下方：一行状态文案
- 底部：两个真正静态的 preview
- 左侧 preview：`paused`，固定显示 `46%`
- 右侧 preview：`error`，固定显示 `82%`
- 页面结构统一收口为：标题 -> 主 `progress_bar` -> 状态文案 -> 底部 `paused / error`

目录：
- `example/HelloCustomWidgets/feedback/progress_bar/`

## 4. 主区 reference 快照
主区录制轨道保持 `indeterminate -> determinate -> indeterminate` 的主线语义：

1. `Syncing...`
   主条为 `indeterminate`，用来表达后台同步中的 loading
2. `Syncing...`
   继续保留 `indeterminate`，但让流动条前移，确保录制能看出动画推进
3. `92% complete`
   程序化退出 `indeterminate`，回落到 determinate 百分比完成态
4. `Syncing...`
   回到默认 loading 态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `paused`
   `46%`
   `paused` palette
2. `error`
   `82%`
   `error` palette

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 116`
- 主进度条尺寸：`196 x 18`
- 状态文案尺寸：`196 x 14`
- 底部行容器尺寸：`200 x 12`
- 单个 preview 尺寸：`96 x 12`
- 页面结构：标题 -> 主 `progress_bar` -> 状态文案 -> 底部 `paused / error`
- 页面风格：浅灰 `page panel`、低噪音冷灰 `track`、标准蓝主进度、暖棕 `paused`、红色 `error`

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Paused preview | Error preview |
| --- | --- | --- | --- |
| `indeterminate` 动画 | 是 | 否 | 否 |
| determinate 百分比 | 是 | 是 | 是 |
| `paused` palette | 否 | 是 | 否 |
| `error` palette | 否 | 否 | 是 |
| `set_value()` 退出 `indeterminate` | 是 | 否 | 否 |
| attach 后启动 timer | 是 | 否 | 否 |
| detach 后停止 timer | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_progress_bar.c` 当前覆盖 `5` 条用例：

1. 样式 helper 覆盖 `apply_standard_style()`、`apply_paused_style()`、`apply_error_style()`、`apply_indeterminate_style()` 的 palette 结果和 `pressed` 清理。
2. `set_value()` 覆盖数值钳制、listener 回调，以及从 `indeterminate` 退出回 determinate 百分比的行为。
3. `indeterminate` attach / tick / detach 生命周期覆盖 attach 后启动 timer、tick 推进 phase、detach 后停止 timer。
4. `set_palette()` 覆盖颜色更新与残留 `pressed` 清理。
5. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / on_click_listener / on_progress_changed / api / control_color / bk_color / progress_color / process / is_show_control / timer_started / indeterminate_mode / phase / determinate_value / alpha / enable / is_focused / is_pressed / padding`。

补充说明：
- 预览测试使用 `progress_bar_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `paused / error` preview 统一通过 `hcw_progress_bar_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责，也不会启动 `indeterminate` timer。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Syncing...` 和底部两个静态 preview，请求首帧并等待 `PROGRESS_BAR_RECORD_FRAME_WAIT`。
2. 继续等待 `PROGRESS_BAR_RECORD_ANIM_WAIT`，让 `indeterminate` 条带推进。
3. 请求第二张 loading 帧，并等待 `PROGRESS_BAR_RECORD_FRAME_WAIT`。
4. 程序化切到 `92% complete`，等待 `PROGRESS_BAR_RECORD_WAIT`。
5. 请求 determinate 完成态，并等待 `PROGRESS_BAR_RECORD_FRAME_WAIT`。
6. 回到默认 `Syncing...`，等待 `PROGRESS_BAR_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `PROGRESS_BAR_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- 底部 `paused / error` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化继续由 `indeterminate` 动画推进、`92% complete` 完成态，以及最终回到默认 `Syncing...` 的稳定帧组成。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `PROGRESS_BAR_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0`、默认态恢复和最终稳定帧都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/progress_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/progress_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/progress_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_progress_bar
```

## 10. 验收重点
- 主区 `indeterminate` loading、`92% complete` 完成态和底部 `paused / error` preview 必须完整可见，不能黑屏、白屏或裁切。
- 两张 loading 帧必须能看出 `indeterminate` 条带推进，不能退化成完全静态。
- 最终稳定帧必须显式回到默认 `Syncing...`，不能停在 `92% complete`。
- `set_value()` 退出 `indeterminate`、attach / detach / tick timer 生命周期，以及 palette helper / setter 清理 `pressed` 的行为不能回退。
- 底部 `paused / error` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_progress_bar/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界为 `(44, 190) - (403, 236)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按主区边界裁切后，主区唯一状态数为 `6`
  - 按 `y >= 237` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Syncing...`

## 12. 与现有控件的边界
- 相比 `activity_ring`：这里表达线性进度，不是圆形 `ProgressRing`。
- 相比 `spinner`：这里既保留等待态，也保留 determinate 百分比语义。
- 相比 `skeleton`：这里表达任务进度，不是内容占位。

## 13. 本轮保留与删减
- 保留的主区状态：`Syncing...`、`92% complete`
- 保留的底部对照：`paused`、`error`
- 保留的交互与实现约束：`indeterminate` 动画推进、`set_value()` 退出 `indeterminate`、attach / detach / tick timer 生命周期、static preview 对照
- 删减的旧桥接与旧装饰：页面级 guide、外部 preview 标签、场景化装饰、与进度条无关的说明壳层、旧 feat 口径文案、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/progress_bar PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `progress_bar` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/progress_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_progress_bar/default`
  - 共捕获 `9` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/progress_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_progress_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0885 colors=79`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界为 `(44, 190) - (403, 236)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按主区边界裁切后主区唯一状态数为 `6`
  - 按 `y >= 237` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：主区变化仍严格收敛在 `progress_bar` reference 主体内，最终稳定帧已回到默认 `Syncing...`，底部 `paused / error` preview 全程保持静态
