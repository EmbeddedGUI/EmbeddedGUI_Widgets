# flyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Flyout`
- 补充对照控件：`tool_tip`、`teaching_tip`
- 对应组件名：`Flyout`
- 本次保留状态：`top / bottom placement`、`open / close`、`primary / secondary action`、`compact`、`disabled`
- 本次删除效果：preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与目标操作面板无关的额外交互录制
- EGUI 适配说明：沿用仓库内 `flyout` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Flyout` 用来表达围绕单一目标展开的轻量操作面板。它比 `tool_tip` 更重，因为可以承载标题、正文和动作；又比 `teaching_tip` 更轻，因为不承担教学卡片和大面积叙事任务。

## 2. 为什么现有控件不够用
- `tool_tip` 只有提示，不承载动作按钮。
- `teaching_tip` 更偏教学和 coachmark，视觉与语义都更重。
- `menu_flyout` 偏菜单行列表，不强调 target + bubble 的锚定关系。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `Flyout` 的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `flyout`，通过录制轨道覆盖默认 `bottom placement`、secondary action 选中、`top placement`、warning 态、target dismiss 与最终稳定帧。
- 底部左侧展示 `compact` 静态对照，保留小尺寸下的 target 与 bubble 布局。
- 底部右侧展示 `disabled` 静态对照，保留禁用后的弱化展示态。
- 页面结构统一收口为：标题 -> 主 `flyout` -> `compact / disabled` 双 preview。
- 两个 preview 统一通过 `egui_view_flyout_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/feedback/flyout/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 132`
- 底部对照行尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`

视觉约束：
- 页面保持浅灰 page panel、白色 surface 和低噪音边框。
- target 与 bubble 的锚定关系必须清晰，但不能回到 showcase 式重装饰。
- `compact / disabled` 都必须是静态 preview，只负责 reference 对照，不再承接任何录制桥接逻辑。
- 触摸继续遵循 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `flyout_primary` | `egui_view_flyout_t` | `196 x 132` | `Review / open` | 主控件 |
| `flyout_compact` | `egui_view_flyout_t` | `104 x 80` | `Filter / static` | 紧凑静态对照 |
| `flyout_disabled` | `egui_view_flyout_t` | `104 x 80` | `Locked / static` | 禁用静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Review` | 默认 `bottom placement / open` |
| 主控件 | `Review secondary` | secondary action 选中态 |
| 主控件 | `Search` | `top placement / open` |
| 主控件 | `Sync` | `warning tone / open` |
| 主控件 | `closed` | target dismiss 后的关闭态 |
| `compact` | `Filter` | 紧凑静态 preview |
| `disabled` | `Locked` | 禁用静态 preview |

## 7. 交互与状态语义
- 主 `flyout` 保留 target、bubble、primary / secondary action、`top / bottom placement`、`Escape` 关闭与 target dismiss 语义。
- `compact / disabled` preview 都通过 static preview API 吞掉 `touch / key`，作为真正静态的 reference 对照。
- `disabled` preview 继续使用 `set_disabled_mode(..., 1)`，保证不承接真实交互。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/feedback/flyout/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview snapshot 切换
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件的 action 选中、placement 切换、warning、dismiss 与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot 和 open 状态”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到默认 `Review / open`，并同步两个静态 preview 状态。
2. 请求默认打开态截图。
3. 键盘向右移动到 secondary action，请求选中态截图。
4. 程序化切换到 `top placement`，请求布局变化截图。
5. 发送 `Escape` 关闭主控件并请求关闭态截图。
6. 程序化切换到 warning 态并请求打开截图。
7. 点击主控件 target 触发 dismiss，再请求关闭态截图。
8. 恢复默认 `Review / open`，再请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_flyout
```

验收重点：
- target、bubble 和两个 action 都必须完整可见。
- `top / bottom placement` 必须能从关键截图直接辨认。
- target dismiss、action 激活与 `Escape` 都必须稳定关闭，不残留 `pressed_part / is_pressed`。
- `compact / disabled` preview 必须在所有 runtime 帧中保持静态，不得因点击或 snapshot 切换产生额外变化。
- preview 不响应触摸或键盘输入，也不触发 action 回调。

## 11. 已知限制
- 当前版本是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做复杂避让、边缘翻转和额外动画。
- 当前动作由固定 snapshot 驱动，不接业务动态数据源。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部提示容器。
