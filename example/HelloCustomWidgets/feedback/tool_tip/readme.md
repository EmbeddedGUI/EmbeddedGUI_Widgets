# ToolTip 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ToolTip`
- 对应组件：`ToolTip`
- 当前保留形态：`Save / closed`、`Save / delayed open`、`Search / top placement`、`Publish / warning open`、`Publish / Esc close`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release`、`Enter / Space` 延时打开与 `Esc` 关闭；`show_delay / snapshot / compact / read_only / open / font / meta_font / palette` setter 统一清理 pending 交互状态；底部 `compact / read only` preview 继续作为静态 reference 对照
- 当前移除内容：旧版 preview snapshot 切换、preview click / focus 桥接、与主区 reference 语义无关的额外录制动作、系统级 popup 跟随与避让、复杂动画，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `tool_tip` custom 实现和 `egui_view_tool_tip_override_static_preview_api()`，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、静态 preview、README、单测和录制轨道

## 1. 为什么需要这个控件
`tool_tip` 对齐的是 Fluent / WinUI 里的轻量目标提示语义，用来围绕单个按钮或热点区域提供短提示、快捷说明和轻量风险提醒。它不是 `teaching_tip` 那种引导卡片，也不是 `message_bar` 那种页内横幅，而是更贴近悬停提示气泡的 reference 对照。

## 2. 为什么现有控件不够
- `teaching_tip` 更偏引导和教学，信息密度、动作层级和占位都更重。
- `message_bar` 是页内横向反馈，不围绕单个目标锚定。
- `toast_stack` 是瞬时通知，不承担目标解释和悬停提示语义。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `ToolTip` 的 reference 页面，用来和其他 feedback 控件划清边界。

## 3. 当前页面结构
- 标题：`ToolTip`
- 主区：一个主 `tool_tip`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Filter`
- 右侧 preview：`read only`，固定显示 `Preview`
- 页面结构统一收口为：标题 -> 主 `tool_tip` -> `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/tool_tip/`

## 4. 主区 reference 快照
主区录制轨道当前保留 `5` 组 reference 语义，最终稳定帧回到默认 `Save / closed`；底部 preview 在整条录制轨道中始终保持不变：

1. 默认态
   `Save / closed`
2. 快照 2
   `Save / delayed open`
3. 快照 3
   `Search / top placement`
4. 快照 4
   `Publish / warning open`
5. 快照 5
   `Publish / Esc close`
6. 最终稳定帧
   回到默认 `Save / closed`

底部 preview 在整条录制轨道中始终固定：
1. `compact`
   `Filter`
2. `read only`
   `Preview`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 238`
- 主 `tool_tip` 尺寸：`196 x 118`
- 底部容器尺寸：`216 x 82`
- 单个 preview 尺寸：`104 x 82`
- 页面结构：标题 -> 主 `tool_tip` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 `surface`、低噪音边框、轻量阴影与克制的 accent / warning 色带

## 6. 状态矩阵
| 状态 / 区域 | 主 `tool_tip` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Save / closed` | `Filter` | `Preview` |
| `delayed open` | 是 | 否 | 否 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `Esc close` | 是 | 否 | 否 |
| 最终稳定帧 | 回到默认 `Save / closed` | 保持不变 | 保持不变 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_tool_tip.c` 当前覆盖 `9` 条用例：

1. `init uses default state`。
   覆盖默认 `show_delay`、`open`、`compact / read_only`、默认字体、`focusable / clickable` 与 `current_part` 初始值。
2. `setters clear pending state`。
   覆盖 `show_delay / current_snapshot / compact / read_only / open / font / meta_font / palette` setter 对 `pending_show / timer / pressed` 的统一清理，以及 `show_delay` 的最小值归一化。
3. `touch click arms delay and second click closes`。
   覆盖主区触摸后启动 delay、timer tick 打开气泡，以及再次点击关闭。
4. `touch move outside cancels delay`。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不打开气泡，并清掉残留交互状态。
5. `key delay and escape close`。
   覆盖 `Enter` 延时打开与 `Esc` 关闭。
6. `unhandled key clears pressed state`。
   覆盖无关键盘输入对 `pressed / key_active` 的清理。
7. `disabled and read_only guard prevent open`。
   覆盖 `!enable` 与 `read_only_mode` 下的输入抑制与状态复位。
8. `static preview consumes input and keeps state`。
   通过 `tool_tip_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`snapshots`、`font`、`meta_font`、`on_click_listener`、`api`、全部 palette 字段、`show_delay_ms`、`snapshot_count`、`current_snapshot`、`current_part`、`compact_mode`、`read_only_mode`、`open`、`timer_started`、`timer_running`、`pending_show`、`touch_active`、`key_active`、`toggle_on_release`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变，并要求 `g_click_count == 0`。
9. `attach and detach restore pending timer`。
   覆盖 attach / detach 后 pending timer 的恢复与 delay 继续生效。

说明：
- 主区触摸继续遵循 `same-target release`：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到原命中目标释放才生效。
- `Enter / Space` 会走同一套 delay 打开语义，`Esc` 负责关闭已打开的气泡。
- `compact / read only` preview 统一通过 `egui_view_tool_tip_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板，并通过 `focus_primary_tool_tip()` 保证 `Esc` 键录制入口的焦点一致性。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 恢复主区默认 `Save / closed`，同步底部双静态 preview，请求首帧截图，等待 `TOOL_TIP_RECORD_FRAME_WAIT`。
2. 对主区调用 `begin_show_delay()`，等待 `560ms`，让默认态进入稳定的 `Save / delayed open`。
3. 请求第二组主区快照，等待 `TOOL_TIP_RECORD_FRAME_WAIT`。
4. 程序化切到 `Search / top placement` 并显式打开，等待 `TOOL_TIP_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `TOOL_TIP_RECORD_FRAME_WAIT`。
6. 程序化切到 `Publish / warning open` 并显式打开，等待 `TOOL_TIP_RECORD_WAIT`。
7. 请求第四组主区快照，等待 `TOOL_TIP_RECORD_FRAME_WAIT`。
8. 对主区发送 `Esc`，收口到 `Publish / Esc close`，等待 `TOOL_TIP_RECORD_WAIT`。
9. 请求第五组主区快照，等待 `TOOL_TIP_RECORD_FRAME_WAIT`。
10. 回到默认 `Save / closed`，同步底部 preview，请求最终稳定帧，并继续等待 `TOOL_TIP_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，在请求前统一布局和刷新。
- 底部 preview 在整条轨道中不承接任何状态切换职责。
- 主区变化只来自 reference snapshot 切换、delay 打开和 `Esc` 关闭。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/tool_tip --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/tool_tip
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_tool_tip
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Save / closed`、`Save / delayed open`、`Search / top placement`、`Publish / warning open` 与 `Publish / Esc close` 五组 reference 状态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Save / closed`。
- 主区 `same-target release`、delay 打开、`Enter / Space`、`Esc` 关闭以及 setter / guard 清理链路收口后不能残留 `pressed`、`pending_show` 或 timer 脏态。
- 底部 `Filter / Preview` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_feedback_tool_tip` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_tool_tip/default`
- 本轮复核结果：
  - 共捕获 `12` 帧
  - 主区共出现 `5` 组唯一状态，对应 `Save / closed`、`Save / delayed open`、`Search / top placement`、`Publish / warning open` 与 `Publish / Esc close`
  - 主区 RGB 差分边界为 `(54, 97) - (427, 203)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 203` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `teaching_tip`：这里是轻量目标提示，不承担引导式布局和多动作卡片职责。
- 相比 `message_bar`：这里是围绕目标的锚定气泡，不是页内横向反馈条。
- 相比 `toast_stack`：这里不承担瞬时通知队列，只表达目标提示和快捷说明。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Save / closed`
  - `Save / delayed open`
  - `Search / top placement`
  - `Publish / warning open`
  - `Publish / Esc close`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `same-target release`
  - `Enter / Space` 延时打开
  - `Esc` 关闭
  - `show_delay / snapshot / compact / read_only / open / font / meta_font / palette` 共享 pending 交互状态清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 旧版 preview snapshot 切换
  - preview click / focus 桥接
  - 与主区 reference 语义无关的额外录制动作
  - 系统级 popup 跟随、屏幕边缘避让和复杂动画
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `tool_tip` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/tool_tip --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_tool_tip/default`
  - 共捕获 `12` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/tool_tip`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_tool_tip`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1815 colors=132`
- 截图复核结论：
  - 共捕获 `12` 帧
  - 主区共出现 `5` 组唯一状态，对应 `Save / closed`、`Save / delayed open`、`Search / top placement`、`Publish / warning open` 与 `Publish / Esc close`
  - 主区 RGB 差分边界为 `(54, 97) - (427, 203)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 以 `y >= 203` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区完整覆盖 `ToolTip` 五组 reference 语义，最终稳定帧已回到默认 `Save / closed`，底部 `compact / read only` preview 全程静态
