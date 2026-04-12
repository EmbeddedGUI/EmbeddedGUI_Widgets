# tool_tip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ToolTip`
- 补充对照控件：`TeachingTip`
- 对应组件名：`ToolTip`
- 本次保留状态：`top / bottom placement`、`delay show`、`open / close`、`compact`、`read only`
- 本次删除效果：系统级 popup、复杂避让、重阴影、过强箭头装饰、外部 guide 文案壳层
- EGUI 适配说明：当前仅在 `HelloCustomWidgets` custom 层实现 anchored hint，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`tool_tip` 用来表达围绕单个目标的轻量提示语义，适合解释按钮、快捷操作、同步状态或悬停提示。它不是教学式大卡片，也不是全局浮层，而是更接近 Fluent / WPF UI 的短提示气泡。

## 2. 为什么现有控件不够用？
- `teaching_tip` 更偏教学与引导，信息量更重，动作层级也更强。
- `message_bar` 是页内横向反馈条，不围绕目标锚定。
- `toast_stack` 是短时通知，不承担目标提示语义。
- 参考主线仍需要一版贴近 Fluent / WPF UI `ToolTip` 的 anchored hint 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `tool_tip`，通过点击目标后延时打开，覆盖 `bottom placement`、`top placement` 和 `warning` 三组快照。
- 底部左侧展示 `compact` 静态对照，验证紧凑尺寸下的提示气泡与目标布局。
- 底部右侧展示 `read only` 静态对照，验证弱化后的只读展示态。
- 主控件保留真实交互：点击目标启动延时显示，再次点击关闭；`Enter / Space` 也走延时打开；`Esc` 关闭。
- 两个 preview 通过 `egui_view_tool_tip_override_static_preview_api()` 吞掉 `touch / key`，只作为静态对照，不参与打开或关闭。

目标目录：`example/HelloCustomWidgets/feedback/tool_tip/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 238`
- 主控件尺寸：`196 x 118`
- 底部对照行尺寸：`216 x 82`
- `compact` 预览：`104 x 82`
- `read only` 预览：`104 x 82`
- 页面结构：标题 + 主 `tool_tip` + 底部 `compact / read only`
- 风格约束：
  - 使用浅灰 page panel、白色气泡面板和低噪音描边。
  - target 与 bubble 的锚定关系要清晰，但不能回到 showcase 风格的大投影和重装饰。
  - `compact / read only` 直接通过控件模式表达，不再依赖外部标签。
  - 主控件必须遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 238` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ToolTip` | 页面标题 |
| `tool_tip_primary` | `egui_view_tool_tip_t` | `196 x 118` | `Save` | 主控件 |
| `tool_tip_compact` | `egui_view_tool_tip_t` | `104 x 82` | `Filter` | `compact` 静态对照 |
| `tool_tip_read_only` | `egui_view_tool_tip_t` | `104 x 82` | `Preview` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_tool_tip_snapshot_t[3]` | - | `Save / Search / Publish` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_tool_tip_snapshot_t[2]` | - | `Filter / Search` | 紧凑对照数据 |
| `read_only_snapshots` | `egui_view_tool_tip_snapshot_t[1]` | - | `Preview` | 只读对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Save` | 默认 `accent / bottom placement / delayed open` |
| 主控件 | `Search` | `accent / top placement` |
| 主控件 | `Publish` | `warning / bottom placement` |
| 主控件 | `closed` | 点击目标后等待、`Esc` 关闭与再次打开 |
| `compact` | `Filter` | 默认紧凑态对照 |
| `compact` | `Search` | 第二组紧凑态对照 |
| `read only` | `Preview` | 固定只读对照，始终打开，吞掉 `touch / key` |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认关闭态截图。
3. 点击主控件 target，等待延时后请求展开截图。
4. 程序化切换到 `top placement` 快照并保持打开，再请求截图。
5. 程序化切换到 `warning` 快照并关闭，重新点击 target，等待延时后请求截图。
6. 发送 `Esc` 关闭主控件并请求截图。
7. 切换 `compact` 到第二组快照，让主控件主动请求 focus，再请求对照截图。
8. 点击 `compact` preview，仅清理主控件 focus，并分别记录收尾帧与最终稳定帧。

## 8. 编译、单测、touch、runtime 与 web 验收
```bash
make clean APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc
make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
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

## 9. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见。
- `top / bottom placement` 必须能从截图中直接辨认。
- 点击 target 只负责启动延时显示，延时结束前不能提前打开。
- `DOWN(A) -> MOVE(B) -> UP(B)` 必须取消延时并清掉 `current_part`，回到 `A` 后再释放才允许提交。
- 二次点击 target、按下 `Esc` 都必须稳定关闭，不残留 `pressed / current_part / timer_started`。
- `read only / disabled` 必须同时抑制输入并清理残留交互状态。
- static preview 必须吞掉 `touch / key`，保持现有打开状态，不触发 click。
- preview 点击后的收尾帧和最终稳定帧都不能出现黑白屏、裁切、整屏污染或残留 focus ring。

## 10. 已知限制与后续方向
- 当前版本是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做自动避让、屏幕边缘翻转和复杂动画。
- 当前以固定 snapshot 数据保证录制稳定，不接业务动态文本来源。

## 11. 与现有控件的边界
- 相比 `teaching_tip`：这里强调轻量提示，不承载教学动作卡片语义。
- 相比 `message_bar`：这里是目标锚定提示，不是页内横幅反馈。
- 相比 `toast_stack`：这里强调局部上下文，不是全局通知。

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`ToolTip`
- 本次保留核心状态：
  - `top placement`
  - `bottom placement`
  - `delay show`
  - `open / close`
  - `compact`
  - `read only`
  - `accent / warning / neutral`

## 13. 相比参考原型删除的效果或装饰
- 删除系统级 popup、复杂避让与多目标联动。
- 删除过重阴影、夸张箭头、外部 guide 标题和叙事性壳层。
- 删除 preview 参与实际交互切换的职责，只保留静态对照。

## 14. EGUI 适配时的简化点与约束
- 在 custom 层实现 `target + bubble` 一体绘制，不改 SDK。
- 通过 `pending_show / timer_started / open / touch_active / key_active / toggle_on_release` 管理延时显示生命周期。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 所有 setter、静态 preview 和输入 guard 共用统一的状态清理语义，避免残留 `pressed / current_part / timer`。
