# RepeatButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI RepeatButton`
- 参考开源库：`WPF UI`
- 对应组件：`RepeatButton`
- 当前保留形态：`Volume 12`、`Volume 15`、`Volume 18`、`compact`、`disabled`
- 当前保留交互：主区保留 immediate click、press-and-hold repeat、移出/移回恢复与 `Space / Enter` 键盘 repeat；底部 `compact / disabled` preview 统一收口为静态 reference 对照
- 当前移除内容：旧主面板说明文案、preview 说明标签、preview 快照轮换、录制阶段真实 `touch / key` 长按、额外收尾态，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用当前目录下的 `egui_view_repeat_button` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 README、reference 录制说明、static preview 语义与验收记录

## 1. 为什么需要这个控件
`repeat_button` 用于表达“按下立即触发一次，继续按住则持续重复触发”的标准命令语义，适合音量步进、数值加减、滚动微调和列表连续翻页这类离散但连续可重复的操作入口。

仓库里已有普通 `button`，但还缺少一个明确承接 press-and-hold repeat 语义、带独立 reference 页面、README、单测和 web 链路的 `RepeatButton`。

## 2. 为什么现有控件不够用
- `button` 只表达单次点击，不表达按住连发。
- `toggle_button` 表达状态切换，不适合连续步进。
- `slider` 适合连续拖拽，不适合离散重复命令。
- 如果继续在 demo 页用普通 `button` 手工拼 repeat timer，会把触摸移出/移回、键盘长按、静态 preview 输入吞掉和 attach / detach timer 恢复语义分散在页面层，难以复用和验证。

## 3. 当前页面结构
- 标题：`RepeatButton`
- 主区：1 个保留真实 repeat 语义的 `repeat_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Fast`
- 右侧 preview：`disabled`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/repeat_button/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Volume 12`
2. 快照 2
   `Volume 15`
3. 快照 3 / 最终稳定帧
   `Volume 18`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Fast`
2. `disabled`
   `Locked`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主控件：`160 x 40`
- 底部 preview 行：`200 x 32`
- 单个 preview：`96 x 32`
- 页面结构：标题 -> 主 `repeat_button` -> 底部 `compact / disabled`
- 风格约束：浅色圆角 page panel、低噪音布局、标准命令按钮比例和轻量 focus ring，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Volume 12` | `Fast` | `Locked` |
| 快照 2 | `Volume 15` | 保持不变 | 保持不变 |
| 快照 3 / 最终稳定帧 | `Volume 18` | 保持不变 | 保持不变 |
| 立即 click + repeat | 是 | 否 | 否 |
| 移出 / 移回命中区恢复 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_repeat_button.c` 当前覆盖 `9` 条用例：

1. init 默认 timing / style。
   覆盖默认 `initial_delay_ms = 360`、`repeat_interval_ms = 90`、standard 背景、默认文字色、`icon_text_gap = 6` 与初始 timer 停止。
2. setters 与 style helpers 清理 `pressed`。
   覆盖 `apply_compact_style()`、`apply_disabled_style()`、`set_text()`、`set_icon()`、`set_font()`、`set_icon_font()`、`set_icon_text_gap()` 与 `set_repeat_timing()`，要求先清理 `is_pressed / touch_active / key_active / timer_started`，再更新内容与 timing。
3. `touch down` 立即 click + repeat。
   覆盖按下即触发一次 click，随后通过 timer 持续 repeat；`UP` 后不再额外 click，且状态完全清理。
4. move outside 停 timer，move back 恢复且不额外 click。
   覆盖 `DOWN(A) -> MOVE(B)` 时主按钮退出 pressed 并停 timer，`MOVE(A)` 返回命中区后恢复 pressed 与 timer，但不补发 click。
5. `Space / Enter` keyboard repeat。
   覆盖 `Space` 按下即 click 并启动 repeat，`Enter` 走完整按下/抬起链路后也能完成一次 repeat-button 提交。
6. 未处理 key 清理 `pressed` 并停 timer。
   覆盖 `Tab` 这类非提交按键在已有键盘 repeat 状态下会清理残留 pressed 与 timer。
7. disabled guard 阻止 click 并清状态。
   覆盖 `!enable` 后的 `Space` 与 `touch` 路径，不触发新 click，且不保留旧 pressed / timer 状态。
8. static preview 吞输入且保持状态不变。
   通过 `repeat_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`text`、`icon`、`font`、`icon_font`、`background`、`color`、`alpha`、`icon_text_gap`、`on_click_listener`、`initial_delay_ms`、`repeat_interval_ms` 不变，并要求 `g_click_count == 0`。
9. attach / detach 恢复 repeat timer。
   覆盖主按钮在保持触摸按住时 detach 会停 timer，re-attach 后恢复 repeat timer，确保窗口生命周期切换不破坏长按语义。

说明：
- 主区真实交互继续保留 immediate click、press-and-hold repeat、移出/移回命中区恢复与 `Space / Enter` 键盘 repeat。
- 所有 setter / 样式 helper、未处理键与 disabled guard 都统一要求先清理残留 `pressed` 与 repeat timer，再进入后续状态。
- 底部 `compact / disabled` preview 统一通过 `egui_view_repeat_button_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不触发 click。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 恢复主区默认 `Volume 12`，同时恢复底部 `compact / disabled` 固定状态，聚焦主按钮并抓取首帧，等待 `REPEAT_BUTTON_RECORD_FRAME_WAIT`。
2. 切到 `Volume 15`，等待 `REPEAT_BUTTON_RECORD_WAIT`。
3. 抓取 `Volume 15` 主区快照，等待 `REPEAT_BUTTON_RECORD_FRAME_WAIT`。
4. 切到 `Volume 18`，等待 `REPEAT_BUTTON_RECORD_WAIT`。
5. 抓取 `Volume 18` 主区快照，等待 `REPEAT_BUTTON_RECORD_FRAME_WAIT`。
6. 保持最终状态不变并等待 `REPEAT_BUTTON_RECORD_FINAL_WAIT`。
7. 抓取最终稳定帧，继续等待 `REPEAT_BUTTON_RECORD_FINAL_WAIT`。

说明：
- 主区仍然保留真实 `touch` 和 keyboard repeat 行为，供运行时手动交互。
- runtime 录制阶段不再真实执行触摸长按或 `Space / Enter` 长按，reference 轨道改为程序化主区快照切换。
- 底部 preview 统一通过 `egui_view_repeat_button_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。
- `REPEAT_BUTTON_DEFAULT_SNAPSHOT` 继续作为默认态入口，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/repeat_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/repeat_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_repeat_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Volume 12`、`Volume 15`、`Volume 18` `3` 组可识别状态。
- 主区真实交互仍需保留 immediate click、press-and-hold repeat、移出/移回命中区恢复与 `Space / Enter` repeat 语义。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持静态一致，收到输入后不能改写 `region_screen / text / icon / font / icon_font / background / color / alpha / icon_text_gap / on_click_listener / initial_delay_ms / repeat_interval_ms`。
- WASM demo 必须能以 `HelloCustomWidgets_input_repeat_button` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_repeat_button/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主按钮实际占位边界收敛到 `(82, 173) - (397, 228)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 245` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `button`：这里强调按下立即触发并支持按住重复，而不是一次性点击。
- 相比 `toggle_button`：这里没有持久选中态。
- 相比 `slider`：这里不承担连续拖拽输入。
- 相比 `number_box`：这里不负责文本输入和解析。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Volume 12`
  - `Volume 15`
  - `Volume 18`
- 保留的底部对照：
  - `compact`
  - `disabled`
- 保留的交互：
  - immediate click + repeat
  - 移出 / 移回命中区恢复
  - `Space / Enter` 键盘 repeat
  - setter / 样式 helper 状态清理
  - static preview 对照
  - attach / detach timer restore
- 删减的旧桥接与旧装饰：
  - 旧主面板说明文案
  - preview 说明标签
  - preview 快照轮换
  - 录制阶段真实 `touch / key` 长按
  - 额外收尾态与旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `repeat_button` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/repeat_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_repeat_button/default`
  - 共捕获 `8` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/repeat_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_repeat_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0977 colors=139`
- 截图复核结论：
  - 主区覆盖默认 `Volume 12`、`Volume 15` 与 `Volume 18` 三组 reference 状态
  - 最终稳定帧保持 `Volume 18`
  - 主按钮实际占位边界收敛到 `(82, 173) - (397, 228)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / disabled` preview 以 `y >= 245` 裁切后全程保持单哈希静态
