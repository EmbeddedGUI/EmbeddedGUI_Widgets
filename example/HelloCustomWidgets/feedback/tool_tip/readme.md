# tool_tip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ToolTip`
- 补充对照控件：`teaching_tip`
- 对应组件名：`ToolTip`
- 本次保留状态：`top / bottom placement`、`delay show`、`open / close`、`compact`、`read only`
- 本次删除效果：preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与目标提示无关的额外交互录制
- EGUI 适配说明：沿用仓库内 `tool_tip` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`ToolTip` 用来表达围绕单个目标的轻量提示语义，适合解释按钮、快捷操作、同步状态或悬停提示。它不是教学式大卡片，也不是全局浮层，而是更接近 Fluent / WPF UI 的短提示气泡。

## 2. 为什么现有控件不够用
- `teaching_tip` 更偏教学与引导，信息量和动作层级都更重。
- `message_bar` 是页内横向反馈条，不围绕目标锚定。
- `toast_stack` 是短时通知，不承担目标提示语义。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `ToolTip` 的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `tool_tip`，通过录制轨道覆盖默认延时打开、`top placement`、`warning` 与 `Esc` 关闭。
- 底部左侧展示 `compact` 静态对照，保留紧凑尺寸下的 target 与 bubble 布局。
- 底部右侧展示 `read only` 静态对照，保留只读后的弱化展示态。
- 页面结构统一收口为：标题 -> 主 `tool_tip` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_tool_tip_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/feedback/tool_tip/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 238`
- 主控件尺寸：`196 x 118`
- 底部对照行尺寸：`216 x 82`
- `compact` 预览：`104 x 82`
- `read only` 预览：`104 x 82`

视觉约束：
- 页面保持浅灰 page panel、白色气泡面板和低噪音描边。
- 主控件仍需清晰表达 target 与 bubble 的锚定关系，但不能回到 showcase 风格的大投影和重装饰。
- `compact / read only` 都必须是静态 preview，只负责 reference 对照，不再承接任何录制桥接逻辑。
- 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 238` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ToolTip` | 页面标题 |
| `tool_tip_primary` | `egui_view_tool_tip_t` | `196 x 118` | `Save / closed` | 主控件 |
| `tool_tip_compact` | `egui_view_tool_tip_t` | `104 x 82` | `Filter / static` | 紧凑静态对照 |
| `tool_tip_read_only` | `egui_view_tool_tip_t` | `104 x 82` | `Preview / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Save` | 默认 `accent / bottom placement / delayed open` |
| 主控件 | `Search` | `accent / top placement / open` |
| 主控件 | `Publish` | `warning / bottom placement / delayed open` |
| 主控件 | `closed` | 默认态与 `Esc` 关闭后的稳定帧 |
| `compact` | `Filter` | 紧凑静态 preview |
| `read only` | `Preview` | 只读静态 preview |

## 7. 交互与状态语义
- 主 `tool_tip` 保留真实 target 点击延时打开、二次点击关闭、`Enter / Space` 延时打开与 `Esc` 关闭语义。
- `compact / read only` preview 都通过 static preview API 吞掉 `touch / key`，作为真正静态的 reference 对照。
- `read only` preview 继续使用 `set_read_only_mode(..., 1)`，保证不承接真实交互。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/feedback/tool_tip/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview snapshot 切换
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件的 `delay open / top placement / warning / Esc close` 变化与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot 和 open 状态”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到默认关闭态，并同步两个静态 preview 状态。
2. 请求默认关闭态截图。
3. 点击主控件 target，等待延时后请求默认打开态截图。
4. 程序化切换到 `top placement` 并保持打开，请求截图。
5. 程序化切换到 `warning`，重新点击 target，等待延时后请求截图。
6. 发送 `Esc` 关闭主控件并请求关闭态截图。
7. 恢复默认 `Save / closed`，再请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

验收重点：
- 主控件和底部 `compact / read only` preview 都必须完整可见。
- `top / bottom placement` 必须能从关键截图中直接辨认。
- 点击 target 只负责启动延时显示，延时结束前不能提前打开。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或 snapshot 切换产生额外变化。
- `Esc` 关闭后的收尾帧与最终稳定帧都不能出现黑白屏、裁切、整屏污染或残留 `pressed / focus / timer` 脏态。
- preview 不响应触摸或键盘输入。

## 11. 已知限制
- 当前版本是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做自动避让、屏幕边缘翻转和复杂动画。
- 当前以固定 snapshot 数据保证录制稳定，不接业务动态文本来源。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部提示容器。
