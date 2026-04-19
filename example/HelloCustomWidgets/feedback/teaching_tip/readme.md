# TeachingTip 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TeachingTip`
- 补充对照实现：`ModernWpf`
- 对应组件名：`TeachingTip`
- 当前保留形态：`Quick filters`、`Cmd palette`、`Sync draft`、`Tip hidden`、`compact`、`read only`
- 当前保留交互：主区保留 `target / primary / secondary / close` 的真实交互语义、`top / bottom placement`、`closed / reopen`、same-target release，以及 `current_snapshot / current_part / font / meta_font / compact / read_only / palette` setter 统一清理 `pressed`；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变
- 当前移除内容：旧版强交互录制轨道、preview 桥接点击、与主区 reference 语义无关的额外切换动作，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `teaching_tip` custom 实现，本轮只收口 `reference` 页面结构、静态 preview、README、单测与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`teaching_tip` 用于围绕具体目标表达带上下文的引导信息，适合首次引导、快捷操作提示、功能解释和风险提醒。它不是页内横幅，也不是阻塞式弹层，而是更接近 Fluent / WPF UI 的 anchored callout 语义。

## 2. 为什么现有控件不够
- `tool_tip` 更偏轻量悬浮提示，不承担多动作引导卡片语义。
- `message_bar` 是页内横向反馈，不围绕具体目标锚定。
- `dialog_sheet` 是收口式对话层，不适合贴近目标的上下文提示。
- `toast_stack` 偏瞬时通知，不负责教学式引导。
- 仓库仍需要一版贴近 Fluent / WPF UI `TeachingTip` 的 `reference` 页面，和其他 feedback 控件划清边界。

## 3. 当前页面结构
- 标题：`Teaching Tip`
- 主区：一个主 `teaching_tip`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Quick tip`
- 右侧 preview：`read only`，固定显示 `Preview`
- 页面结构统一收口为：标题 -> 主 `teaching_tip` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/teaching_tip/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Quick filters / bottom open`
   默认 `accent / bottom placement`
2. `Cmd palette / top open`
   程序化切到顶部气泡
3. `Sync draft / warning open`
   程序化切到 warning 语义
4. `Tip hidden / closed`
   使用同步后的 closed snapshot，保留主区关闭态
5. `Quick filters / bottom open`
   回到默认态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：
1. `compact`
   `Quick tip`
2. `read only`
   `Preview`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 252`
- 主 `teaching_tip` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 -> 主 `teaching_tip` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 `surface`、低噪音边框、克制的 `accent / warning` 色带

## 6. 状态矩阵
| 状态 / 区域 | 主 `teaching_tip` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `bottom placement` | 是 | 是 | 是 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `closed / reopen` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_teaching_tip.c` 当前覆盖 `10` 条用例：

1. `set_snapshots()` 的 clamp、默认 part 解析，以及 `secondary-only / close-only / closed` 场景下的默认 part 保护。
2. `current_snapshot / current_part` guard 与 `pressed` 清理，包括 invalid snapshot、invalid part 和 closed snapshot 的 target-only 口径。
3. `font / meta_font / compact / read_only / palette` setter 与内部 helper 覆盖默认字体回落、tone 颜色、part 收集、text helper 和 disabled mix。
4. metrics 计算、`top / bottom placement` 变化与 hit testing，覆盖 target / primary / secondary / close 区域。
5. 触摸交互覆盖 same-target release、`ACTION_CANCEL`、close 提交和 closed 状态下 target reopen。
6. keyboard navigation 覆盖 `Left / Right / Home / End / Tab / Escape / Enter / Space` 与 read-only / disabled guard。
7. `compact_mode` 切换后会清理 `pressed`，但保留 target 输入行为。
8. `read_only_mode` 会清理残留 `pressed`，忽略后续输入，并在恢复后继续允许 part 切换。
9. `!enable` 状态下忽略输入并清理 `pressed`，恢复后允许 target 重新触发状态切换。
10. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / snapshots / font / meta_font / on_part_changed / api / palette / snapshot_count / current_snapshot / current_part / compact_mode / read_only_mode / pressed_part / alpha / enable / is_focused / is_pressed / padding`。

补充说明：
- 预览测试使用 `teaching_tip_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `compact / read only` preview 统一通过 `egui_view_teaching_tip_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Quick filters / bottom open`，同步底部两个静态 preview，请求首帧并等待 `TEACHING_TIP_RECORD_FRAME_WAIT`。
2. 切到 `Cmd palette / top open`，等待 `TEACHING_TIP_RECORD_WAIT`。
3. 请求第二帧并等待 `TEACHING_TIP_RECORD_FRAME_WAIT`。
4. 切到 `Sync draft / warning open`，等待 `TEACHING_TIP_RECORD_WAIT`。
5. 请求第三帧并等待 `TEACHING_TIP_RECORD_FRAME_WAIT`。
6. 通过 `sync_closed_snapshot_from()` 同步 closed snapshot 后进入 `Tip hidden / closed`，等待 `TEACHING_TIP_RECORD_WAIT`。
7. 请求第四帧并等待 `TEACHING_TIP_RECORD_FRAME_WAIT`。
8. 回到默认 `Quick filters / bottom open`，同步 preview 固定状态并请求最终稳定帧，继续等待 `TEACHING_TIP_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 reference snapshot 切换与 closed snapshot 同步。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 与最终稳定帧入口统一回到 `apply_primary_default_state()`，确保录制首尾都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/teaching_tip
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_teaching_tip
```

## 10. 验收重点
- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Quick filters / Cmd palette / Sync draft / Tip hidden` 四组状态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Quick filters / bottom open`，不能停在 closed 状态。
- `target / primary / secondary / close` 语义、same-target release、close / reopen 和键盘 guard 需要与单测口径一致。
- `current_snapshot / current_part / font / meta_font / compact / read_only / palette` setter 清理 `pressed` 的行为不能回退。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_teaching_tip/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Quick filters / bottom open`、`Cmd palette / top open`、`Sync draft / warning open` 和 `Tip hidden / closed`
  - 主区 RGB 差分边界为 `(68, 81) - (413, 207)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 207` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Quick filters / bottom open`

## 12. 与现有控件的边界
- 相比 `tool_tip`：这里是带动作的引导卡片，不是轻量悬浮提示。
- 相比 `message_bar`：这里是围绕目标的锚定 callout，不是页内横向反馈条。
- 相比 `toast_stack`：这里不承担瞬时通知队列，只表达目标引导和功能提醒。

## 13. 本轮保留与删减
- 保留的主区状态：`Quick filters`、`Cmd palette`、`Sync draft`、`Tip hidden`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`target / primary / secondary / close` 语义、`top / bottom placement`、`closed / reopen`、same-target release、static preview 对照
- 删减的旧桥接与旧装饰：旧版强交互录制轨道、preview 桥接点击、与主区 reference 语义无关的额外切换动作、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `teaching_tip` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_teaching_tip/default`
  - 共捕获 `10` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/teaching_tip`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_teaching_tip`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1921 colors=162`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Quick filters / bottom open`、`Cmd palette / top open`、`Sync draft / warning open` 和 `Tip hidden / closed`
  - 主区 RGB 差分边界为 `(68, 81) - (413, 207)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 207` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Quick filters / bottom open`，底部 `compact / read only` preview 全程保持静态
