# MessageBar 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI InfoBar`
- 补充参考实现：`ModernWpf`
- 对应组件：`MessageBar / InfoBar`
- 当前保留形态：`Updates ready`、`Settings saved`、`Storage almost full`、`Connection lost`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release` 与 `Enter` 提交；`set_snapshots / current_snapshot / font / palette / compact / read_only` setter 统一清理 `pressed`；底部 `compact / read only` preview 继续作为静态 reference 对照
- 当前移除内容：旧 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `message_bar` custom 实现和 `egui_view_message_bar_override_static_preview_api()`，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、静态 preview 语义、README 和单测

## 1. 为什么需要这个控件
`message_bar` 对齐的是 Fluent / WinUI 的 `InfoBar` 语义，用来承载页内常驻但不阻塞的反馈信息。它覆盖 `info / success / warning / error` 四类常见状态，适合设置页、同步页和后台管理页顶部的轻量提示。

## 2. 为什么现有控件不够用
- `toast_stack` 更偏短时通知，不适合作为单条页内反馈条。
- `dialog_sheet` 是阻断式确认层，不承担常驻信息提示。
- `badge_group` 只能表达汇总提醒，不能同时承载标题、正文和动作语义。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `MessageBar / InfoBar` 的示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Message Bar`
- 主区：一个主 `message_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Quota alert`
- 右侧 preview：`read only`，固定显示 `Policy note`
- 页面结构统一收口为：标题 -> 主 `message_bar` -> `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/message_bar/`

## 4. 主区 reference 快照
主控件录制轨道保留 `4` 组主区 reference 语义，最终稳定帧回到默认 `Updates ready`；底部 preview 在整条录制轨道中始终保持不变：

1. 默认态
   `Updates ready`
2. 快照 2
   `Settings saved`
3. 快照 3
   `Storage almost full`
4. 快照 4
   `Connection lost`
5. 最终稳定帧
   回到默认 `Updates ready`

底部 preview 在整条录制轨道中始终固定：
1. `compact`
   `Quota alert`
2. `read only`
   `Policy note`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 220`
- 主 `message_bar` 尺寸：`196 x 96`
- 底部容器尺寸：`216 x 82`
- 单个 preview 尺寸：`104 x 82`
- 页面结构：标题 -> 主 `message_bar` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色 surface、低噪音边框、柔和 severity accent 和明确但克制的标题 / 正文 / action 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Updates ready` | `Quota alert` | `Policy note` |
| `success` | `Settings saved` | 否 | 否 |
| `warning` | `Storage almost full` | `Quota alert` | 否 |
| `error` | `Connection lost` | 否 | 否 |
| 最终稳定帧 | 回到默认 `Updates ready` | 保持不变 | 保持不变 |
| `compact_mode` | 否 | 是 | 否 |
| `read_only_mode` | 否 | 否 | 是 |
| `show_action` | 是 | 是 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_message_bar.c` 当前覆盖 `8` 条用例：

1. `set_snapshots clamps and clears pressed state`。
   覆盖 `set_snapshots()` 的数量钳制、空快照回落与 `pressed` 清理。
2. `snapshot and setters clear pressed state`。
   覆盖 `set_current_snapshot()` 的 guard，以及 `font / palette / compact / read_only` setter 对 `pressed` 的统一清理。
3. `touch same-target release and cancel behavior`。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交、回到 `A` 后 `UP(A)` 才提交，以及 `ACTION_CANCEL` 清理。
4. `keyboard click listener`。
   覆盖主区 `Enter` 提交 click listener。
5. `compact_mode clears pressed and keeps click behavior`。
   覆盖 `compact_mode` 切换后清理 `pressed`，并验证主区点击行为保持可用。
6. `read_only and disabled guards clear pressed state`。
   覆盖 `read_only_mode` 与 `!enable` 下的输入抑制和 `pressed` 清理。
7. `static preview consumes input and keeps state`。
   通过 `message_bar_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`snapshots`、`font`、`on_click_listener`、`api`、全部 palette 字段、`snapshot_count`、`current_snapshot`、`compact_mode`、`read_only_mode`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变，并要求 `click_count == 0`。
8. `internal helpers cover severity and text`。
   覆盖内部 helper、severity glyph、severity 颜色映射与禁用混色逻辑。

说明：
- 主 `message_bar` 仍然是 display-first 的页内反馈控件，但保留标题、正文和 action 层级。
- 触摸继续遵守 `same-target release`：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `compact / read only` preview 统一通过 `egui_view_message_bar_override_static_preview_api()` 吞掉 `touch / key`，不再承担 snapshot 切换或焦点桥接职责。
- `read only` preview 保持 `set_read_only_mode(..., 1)`，只作为静态对照，不参与真实交互。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主控件默认 `Updates ready` 和底部 preview 固定状态，请求首帧截图，等待 `MESSAGE_BAR_RECORD_FRAME_WAIT`。
2. 切到 `Settings saved`，等待 `MESSAGE_BAR_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `MESSAGE_BAR_RECORD_FRAME_WAIT`。
4. 切到 `Storage almost full`，等待 `MESSAGE_BAR_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `MESSAGE_BAR_RECORD_FRAME_WAIT`。
6. 切到 `Connection lost`，等待 `MESSAGE_BAR_RECORD_WAIT`。
7. 请求第四组主区快照，等待 `MESSAGE_BAR_RECORD_FRAME_WAIT`。
8. 回到默认 `Updates ready`，等待 `MESSAGE_BAR_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `MESSAGE_BAR_RECORD_FINAL_WAIT`。

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自四组 severity snapshot 以及最终回落到默认帧。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，首帧和最终稳定帧都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/message_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_message_bar
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Updates ready`、`Settings saved`、`Storage almost full` 与 `Connection lost` 四组 severity 语义必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Updates ready`。
- 主区 `same-target release`、`Enter` 提交以及 setter / guard 清理链路收口后不能残留 `pressed` 污染。
- 底部 `Quota alert / Policy note` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_feedback_message_bar` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_message_bar/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Updates ready / Settings saved / Storage almost full / Connection lost`
  - 主区 RGB 差分边界为 `(45, 103) - (231, 244)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 245` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `toast_stack`：这里是页内常驻提示，不是短时通知。
- 相比 `dialog_sheet`：这里不阻断流程，只表达页内反馈。
- 相比 `badge_group`：这里承载标题、正文和 action，不是汇总提醒。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Updates ready`
  - `Settings saved`
  - `Storage almost full`
  - `Connection lost`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `same-target release`
  - `Enter` 提交
  - `set_snapshots / current_snapshot / font / palette / compact / read_only` 共享 `pressed` 清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 旧 preview snapshot 切换
  - preview 点击清主控件焦点的桥接逻辑
  - 与 reference 页面无关的额外录制动作
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `message_bar` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_message_bar/default`
  - 共捕获 `11` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/message_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_message_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1678 colors=133`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Updates ready / Settings saved / Storage almost full / Connection lost`
  - 主区 RGB 差分边界为 `(45, 103) - (231, 244)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 以 `y >= 245` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区完整覆盖 `MessageBar / InfoBar` 四组 severity 语义，最终稳定帧已回到默认 `Updates ready`，底部 `compact / read only` preview 全程静态
