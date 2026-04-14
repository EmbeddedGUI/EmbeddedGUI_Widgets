# teaching_tip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TeachingTip`
- 补充对照实现：`ModernWpf`
- 对应组件名：`TeachingTip`
- 本次保留状态：`accent`、`warning`、`top / bottom placement`、`closed / reopen`、`compact`、`read only`
- 本次删除效果：preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与 anchored callout 无关的额外交互录制
- EGUI 适配说明：沿用仓库内 `teaching_tip` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`TeachingTip` 用来表达围绕具体目标的上下文引导语义，适合首次引导、快捷键提示、功能解释和发布前提醒。它不是页内横幅，也不是阻塞式弹层，而是更接近 Fluent / WPF UI 的轻量 anchored callout。

## 2. 为什么现有控件不够用
- `message_bar` 是页内横向反馈条，不围绕目标锚定。
- `dialog_sheet` 是收口式对话层，不是贴近控件的上下文提示。
- `toast_stack` 偏短时通知，不承担教学引导语义。
- `menu_flyout` 是命令面板，不是带目标锚点的提示卡片。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `TeachingTip` 的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `teaching_tip`，通过录制轨道覆盖 `bottom placement`、`top placement`、warning、关闭态、重开态和键盘导航。
- 底部左侧展示 `compact` 静态对照，保留缩小尺寸下的 coachmark、target 和单动作布局。
- 底部右侧展示 `read only` 静态对照，保留禁用交互后的弱化 callout。
- 页面结构统一收口为：标题 -> 主 `teaching_tip` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_teaching_tip_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/feedback/teaching_tip/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 132`
- 底部对照行尺寸：`216 x 80`
- `compact` 预览：`104 x 80`
- `read only` 预览：`104 x 80`

视觉约束：
- 页面保持浅灰 page panel、白底 callout surface 和低噪音浅边框。
- target 与 bubble 的锚定关系必须清楚，但整体不能回到旧 showcase 风格。
- `primary / secondary / close` 仍保留清晰的动作层级，但 chrome 必须更轻。
- 底部两个 preview 不承接真实交互，只负责静态 reference 对照。
- 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Teaching Tip` | 页面标题 |
| `tip_primary` | `egui_view_teaching_tip_t` | `196 x 132` | `Quick filters` | 标准主控件 |
| `tip_compact` | `egui_view_teaching_tip_t` | `104 x 80` | `Quick tip / static` | 紧凑静态对照 |
| `tip_read_only` | `egui_view_teaching_tip_t` | `104 x 80` | `Preview / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Quick filters` | 默认 `accent / bottom placement` |
| 主控件 | `Cmd palette` | `accent / top placement` |
| 主控件 | `Sync draft` | `warning / bottom placement` |
| 主控件 | `Tip hidden` | close 后的关闭态与 target 重开态 |
| 主控件 | `keyboard nav` | `Right / Right / Escape` 的主控件状态变化 |
| `compact` | `Quick tip` | 紧凑静态 preview |
| `read only` | `Preview` | 只读静态 preview |

## 7. 交互与状态语义
- 主 `teaching_tip` 保留 target、bubble、`primary / secondary / close`、`top / bottom placement`、关闭态与重开态语义。
- `compact / read only` preview 都通过 static preview API 吞掉 `touch / key`，作为真正静态的 reference 对照。
- `read only` preview 继续使用 `set_read_only_mode(..., 1)`，保证不承接真实交互。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/feedback/teaching_tip/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview snapshot 切换
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 保留主控件的 target / primary / secondary / close / reopen / keyboard 录制轨道
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot 和 current_part”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到默认 `bottom placement`，并同步两个静态 preview 状态。
2. 请求默认截图。
3. 点击 target 并请求截图。
4. 点击 primary 并请求截图。
5. 程序化切换到 `top placement`，请求截图。
6. 点击 secondary 并请求截图。
7. 程序化切换到 warning，点击 primary 并请求截图。
8. 点击 close 进入关闭态，再请求截图。
9. 点击 target 重开并请求截图。
10. 回放 `Right / Right / Escape` 键盘导航并请求截图。
11. 恢复默认 `bottom placement` 与两个静态 preview，再请求最终稳定帧。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

验收重点：
- 主控件和底部 `compact / read only` preview 都必须完整可见。
- target 与 bubble 的锚定关系必须清楚，`top / bottom placement` 需要能直接辨认。
- 关闭态与重开态必须稳定，不依赖外部桥接。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或 snapshot 切换产生额外变化。
- preview 不响应触摸或键盘输入，也不改变 `current_snapshot / current_part`。

## 11. 已知限制
- 当前版本是页内固定锚点 reference，不做真实 popup 跟随。
- 当前不做自动避让和屏幕边缘翻转策略。
- 当前不做真实图标资源和入场动画。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不扩展成多目标引导系统。
