# ToastStack 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Toast / Snackbar`
- 对应组件名：`Toast / Snackbar`
- 当前保留形态：`Backup ready`、`Draft`、`Storage low`、`Upload failed`、`compact`、`read only`
- 当前保留交互：主区保留四类 severity 切换、same-target release、`current_snapshot / font / meta_font / palette / compact / read_only` setter 统一清理 `pressed`、`read_only / !enable` guard；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变
- 当前移除内容：旧版 preview snapshot 切换、preview 桥接点击 / 焦点清理、与 reference 页面无关的额外录制动作，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `toast_stack` custom 实现，本轮只收口 `reference` 页面结构、静态 preview、README、单测与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`toast_stack` 对齐的是 Fluent / WinUI 里的轻量级 `Toast / Snackbar` 语义，用来承载短时、非阻塞、带状态层级的通知反馈。它适合设置页、同步页、工作台首页这类需要同时展示最近两到三条提醒的场景，比单条横幅更能表达“前卡 + 背卡”的层级关系。

## 2. 为什么现有控件不够
- `message_bar` 更偏常驻页内反馈，不强调多条 toast 的叠卡层级。
- `dialog_sheet` 是阻塞式确认层，不适合承载轻量通知。
- `badge_group` 只能做聚合提醒，不能同时承载正文、动作和时间信息。
- 仓库仍需要一版贴近 Fluent / WPF UI `Toast / Snackbar` 的 `reference` 示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Toast Stack`
- 主区：一个主 `toast_stack`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Quota alert`
- 右侧 preview：`read only`，固定显示 `Policy note`
- 页面结构统一收口为：标题 -> 主 `toast_stack` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/toast_stack/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Backup ready`
   默认 `info`，带 action
2. `Draft`
   `success`，带 action
3. `Storage low`
   `warning`，带 action
4. `Upload failed`
   `error`，带 action
5. `Backup ready`
   回到默认 `info`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Quota alert`
   紧凑布局、`warning` 语义、静态 action
2. `read only`
   `Policy note`
   只读弱化配色、无 action

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 232`
- 主 `toast_stack` 尺寸：`196 x 108`
- 底部容器尺寸：`216 x 83`
- 单个 preview 尺寸：`104 x 83`
- 页面结构：标题 -> 主 `toast_stack` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 `surface`、低噪音边框、柔和的 `severity accent`，以及克制的标题 / 正文 / action 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `info` | 是 | 否 | 是 |
| `success` | 是 | 否 | 否 |
| `warning` | 是 | 是 | 否 |
| `error` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `action` | 是 | 是 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_toast_stack.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 覆盖 snapshot 数量钳制、空数据重置，以及 `current_snapshot / pressed` 清理。
2. `current_snapshot` guard 与 `font / meta_font / palette / compact / read_only` setter 覆盖默认字体回落、无效 snapshot 保护和 `pressed` 清理。
3. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 click；`ACTION_CANCEL` 只清理 `pressed`。
4. 键盘 `Enter` 继续触发 click listener。
5. `compact_mode` 切换后会清理残留 `pressed`，但保留点击语义。
6. `read_only_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入，恢复后继续允许正常点击。
7. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / snapshots / font / meta_font / on_click_listener / api / palette / snapshot_count / current_snapshot / compact_mode / read_only_mode / alpha / enable / is_focused / is_pressed / padding`。
8. 内部 helper 覆盖 `clamp_snapshot_count`、`text_len`、severity glyph 与 severity 颜色映射，以及 disabled mix。

补充说明：
- 预览测试使用 `toast_stack_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `compact / read only` preview 统一通过 `egui_view_toast_stack_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Backup ready` 和底部两个静态 preview，请求首帧并等待 `TOAST_STACK_RECORD_FRAME_WAIT`。
2. 切到 `Draft`，等待 `TOAST_STACK_RECORD_WAIT`。
3. 请求第二帧并等待 `TOAST_STACK_RECORD_FRAME_WAIT`。
4. 切到 `Storage low`，等待 `TOAST_STACK_RECORD_WAIT`。
5. 请求第三帧并等待 `TOAST_STACK_RECORD_FRAME_WAIT`。
6. 切到 `Upload failed`，等待 `TOAST_STACK_RECORD_WAIT`。
7. 请求第四帧并等待 `TOAST_STACK_RECORD_FRAME_WAIT`。
8. 回到默认 `Backup ready`，等待 `TOAST_STACK_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `TOAST_STACK_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Backup ready / Draft / Storage low / Upload failed` 四组 severity snapshot，以及最终回到默认 `Backup ready` 的稳定帧。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `TOAST_STACK_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 与最终稳定帧入口统一回到默认 `Backup ready`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/toast_stack
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_toast_stack
```

## 10. 验收重点
- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Backup ready / Draft / Storage low / Upload failed` 四组状态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Backup ready`，不能停在 `Upload failed`。
- same-target release、`current_snapshot / font / meta_font / palette / compact / read_only` setter 清理 `pressed`，以及 `read_only / !enable` guard 需要与单测口径一致。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_toast_stack/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Backup ready`、`Draft`、`Storage low` 和 `Upload failed`
  - 主区 RGB 差分边界为 `(45, 94) - (405, 254)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 254` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Backup ready`

## 12. 与现有控件的边界
- 相比 `message_bar`：这里是短时叠卡通知，不是常驻页内反馈。
- 相比 `dialog_sheet`：这里不阻断流程，只表达轻量反馈。
- 相比 `badge_group`：这里承载标题、正文、action 和时间元信息，不是汇总提醒。

## 13. 本轮保留与删减
- 保留的主区状态：`Backup ready`、`Draft`、`Storage low`、`Upload failed`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：same-target release、`current_snapshot / font / meta_font / palette / compact / read_only` setter 清理 `pressed`、`read_only / !enable` guard、static preview 对照
- 删减的旧桥接与旧装饰：旧版 preview snapshot 切换、preview 桥接点击 / 焦点清理、与 reference 页面无关的额外录制动作、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `toast_stack` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_toast_stack/default`
  - 共捕获 `11` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/toast_stack`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_toast_stack`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1769 colors=209`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Backup ready`、`Draft`、`Storage low` 和 `Upload failed`
  - 主区 RGB 差分边界为 `(45, 94) - (405, 254)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 254` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Backup ready`，底部 `compact / read only` preview 全程保持静态
