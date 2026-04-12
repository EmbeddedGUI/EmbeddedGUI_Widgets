# flyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Flyout`
- 补充对照控件：`ToolTip`、`TeachingTip`
- 对应组件名：`Flyout`
- 本次保留状态：`top / bottom placement`、`open / close`、`primary / secondary action`、`compact`、`disabled`
- 本次删除效果：系统级 popup、复杂避让、重投影、额外 close button、页面级 guide 包装
- EGUI 适配说明：当前仅在 `HelloCustomWidgets` custom 层实现 anchored `flyout`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`flyout` 用来表达“围绕单一目标展开的轻量操作面板”。它比 `tool_tip` 更重，因为可以承载标题、正文和动作；又比 `teaching_tip` 更轻，因为不承担教学卡片和大面积叙事任务。

## 2. 为什么现有控件不够用
- `tool_tip` 只有提示，不承载动作按钮。
- `teaching_tip` 更偏教学和 coachmark，视觉与语义都更重。
- `menu_flyout` 偏菜单行列表，不强调 target + bubble 的锚定关系。

## 3. 目标场景与示例概览
- 主控件展示标准 `flyout`，保留 target、bubble、primary / secondary action、`top / bottom placement` 和 `Escape` 关闭。
- 底部左侧展示 `compact` 静态对照，验证小尺寸收口。
- 底部右侧展示 `disabled` 静态对照，验证禁用语义和非交互预览。
- 主控件保留真实 touch / key 交互；两个 preview 通过 `egui_view_flyout_override_static_preview_api()` 吞掉输入。

目标目录：`example/HelloCustomWidgets/feedback/flyout/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 132`
- 底部对照行尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 + 主 `flyout` + 底部 `compact / disabled`
- 视觉约束：
  - 使用浅灰 page panel、白色 surface 和低噪音边框。
  - target 与 bubble 的锚定关系必须清晰，但不回到 showcase 式重装饰。
  - `compact / disabled` 直接通过控件模式表达，不增加外部标签。
  - 触摸遵循 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `flyout_primary` | `egui_view_flyout_t` | `196 x 132` | `Review` | 主控件 |
| `flyout_compact` | `egui_view_flyout_t` | `104 x 80` | `Filter` | `compact` 静态对照 |
| `flyout_disabled` | `egui_view_flyout_t` | `104 x 80` | `Locked` | `disabled` 静态对照 |
| `primary_snapshots` | `egui_view_flyout_snapshot_t[4]` | - | `Review / Search / Sync / Pinned` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_flyout_snapshot_t[2]` | - | `Filter / Share` | 紧凑对照 |
| `disabled_snapshot` | `egui_view_flyout_snapshot_t[1]` | - | `Locked` | 禁用对照 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Review` | `bottom placement`，默认 primary focus |
| 主控件 | `Search` | `top placement`，默认 secondary focus |
| 主控件 | `Sync` | `warning tone`，验证 target 再次点击 dismiss |
| 主控件 | `Pinned` | 关闭态，仅保留 target |
| `compact` | `Filter / Share` | 两组紧凑静态对照 |
| `disabled` | `Locked` | 非交互禁用对照 |

## 7. 录制动作设计
1. 重置主控件与 `compact` preview。
2. 输出默认 `bottom placement` 打开态。
3. 键盘向右移动到 secondary action 并输出焦点帧。
4. 切换到 `top placement` 并输出布局帧。
5. 发送 `Escape` 关闭并输出关闭帧。
6. 切换到 warning snapshot 并输出打开帧。
7. 点击 target 触发 same-target dismiss 并输出关闭帧。
8. 切换 `compact` preview 到第二组快照，并记录最终对照片。

## 8. 编译、单测、touch、runtime 与 web 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64

python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py

python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_flyout
```

## 9. 验收重点
- target、bubble 和两个 action 都必须完整可见。
- `top / bottom placement` 必须能从截图直接辨认。
- target 再次点击、action 激活和 `Escape` 都必须稳定 dismiss。
- `set_snapshots()`、`set_current_snapshot()`、`set_open()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 都要清掉残留 `pressed_part / is_pressed`。
- static preview 必须吞掉 `touch / key`，保持既有 open 状态，不触发 action 回调。

## 10. 已知限制与后续方向
- 当前版本是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做复杂避让、边缘翻转和额外动画。
- 当前动作由固定 snapshot 驱动，不接业务动态数据源。

## 11. 与现有控件的边界
- 相比 `tool_tip`：这里保留动作按钮，不再只是提示气泡。
- 相比 `teaching_tip`：这里压缩了教学卡片语义，保持更轻的操作 surface。
- 相比 `menu_flyout`：这里强调锚点与 bubble 关系，不是纯菜单列表。
