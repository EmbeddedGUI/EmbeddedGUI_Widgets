# teaching_tip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TeachingTip`
- 补充对照实现：`ModernWpf`
- 对应组件名：`TeachingTip`
- 本次保留状态：`accent`、`warning`、`top / bottom placement`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态桥接、外部 preview 标签、旧双列预览壳层、过重 callout shadow、过亮 pointer tail、过强 action / close chrome
- EGUI 适配说明：沿用仓库内 `teaching_tip` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`teaching_tip` 用来表达围绕具体目标的上下文引导语义，适合首次引导、快捷键提示、功能解释和发布前提醒。它不是页内横幅，也不是阻塞式弹层，而是更接近 Fluent / WPF UI 的轻量 anchored callout。

## 2. 为什么现有控件不够用？
- `message_bar` 是页内横向反馈条，不围绕目标锚定。
- `dialog_sheet` 是收口式对话层，不是贴近控件的上下文提示。
- `toast_stack` 偏短时通知，不承担教学引导语义。
- `menu_flyout` 是命令面板，不是带目标锚点的提示卡片。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI 的 `TeachingTip` 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `teaching_tip`，通过录制动作覆盖 `bottom placement`、`top placement`、`warning`、关闭态与重开态。
- 底部左侧展示 `compact` 静态对照，验证缩小尺寸下的 coachmark、target 和单动作布局。
- 底部右侧展示 `read only` 静态对照，验证禁用交互后的弱化 callout。
- 页面结构统一收口为：标题 -> 主 `teaching_tip` -> `compact / read only`。
- 两个 preview 统一通过 `egui_view_teaching_tip_override_static_preview_api()` 收口：
  - preview 自身吞掉 `touch / key`，不改 `current_snapshot / current_part`，也不触发 click。
  - preview 点击时只负责清主控件 focus，用于 runtime 复核交互后的收尾渲染。
- 旧的 preview 列容器、外部标签和页面桥接逻辑全部移除。

目标目录：`example/HelloCustomWidgets/feedback/teaching_tip/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 132`
- 底部对照行尺寸：`216 x 80`
- `compact` 预览：`104 x 80`
- `read only` 预览：`104 x 80`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白底 callout surface 和低噪音浅边框。
  - target 与 bubble 的锚定关系必须清楚，但整体不能回到旧 showcase 风格。
  - `primary / secondary / close` 仍保留清晰的动作层级，但 chrome 必须更轻。
  - 底部两个 preview 不承接真实交互；`touch / key` 统一被 static preview API 吞掉，点击仅用于清主控件 focus。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
  - `snapshot / compact / read only` 切换后不能残留 target 或 action 的 `pressed` 高亮，避免 callout 在交互收尾后停留在旧态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Teaching Tip` | 页面标题 |
| `tip_primary` | `egui_view_teaching_tip_t` | `196 x 132` | `Quick filters` | 标准主控件 |
| `tip_compact` | `egui_view_teaching_tip_t` | `104 x 80` | `Quick tip` | `compact` 静态对照 |
| `tip_read_only` | `egui_view_teaching_tip_t` | `104 x 80` | `Preview` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_teaching_tip_snapshot_t[4]` | - | `bottom / top / warning / closed` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_teaching_tip_snapshot_t[2]` | - | `Quick tip / Search` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_teaching_tip_snapshot_t[1]` | - | `Preview` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Quick filters` | 默认 `accent / bottom placement` |
| 主控件 | `Cmd palette` | `accent / top placement` |
| 主控件 | `Sync draft` | `warning / bottom placement` |
| 主控件 | `Tip hidden` | 关闭态与 target 重开轨道 |
| `compact` | `Quick tip` | 默认紧凑对照 |
| `compact` | `Search` | 第二组紧凑对照 |
| `read only` | `Preview` | 固定只读对照；static preview 吞掉 `touch / key`，点击只清主控件 focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认 `bottom placement` 截图。
3. 点击主控件 target。
4. 请求 target 聚焦截图。
5. 点击主控件 primary。
6. 请求 primary 聚焦截图。
7. 程序化切换主控件到 `top placement`。
8. 请求 `top placement` 截图。
9. 点击主控件 secondary。
10. 请求 secondary 聚焦截图。
11. 程序化切换主控件到 `warning`。
12. 请求 warning 截图。
13. 点击主控件 primary。
14. 请求 warning primary 聚焦截图。
15. 点击 close 进入关闭态。
16. 请求关闭态截图。
17. 点击 target 重新展开。
18. 请求重开截图。
19. 用 `Right / Right / Escape` 回放键盘焦点迁移。
20. 请求键盘轨道截图。
21. 程序化切换 `compact` 到第二组 snapshot。
22. 让主控件主动请求 focus。
23. 请求 `compact` 第二组 snapshot 截图。
24. 点击 `compact` preview，只清主控件 focus。
25. 请求 preview 点击后的收尾截图。
26. 再请求最终稳定帧，确认没有残留 focus ring、pressed 或整屏污染。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make clean APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc
make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` preview 都必须完整可见。
- target 与 bubble 的锚定关系必须清楚。
- `top / bottom placement` 必须能从截图中直接辨认。
- 关闭态与重开态必须稳定，不依赖外部状态桥接。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不能误提交，回到原目标后释放才提交。
- `read only / disabled` 不仅要忽略后续 `touch / key` 输入，还要在新输入或模式切换时清掉残留 `pressed` 渲染。
- static preview 必须吞掉 `touch / key`，且不能改 `current_snapshot / current_part`，也不能触发 click。
- preview 点击后的收尾帧和最终稳定帧都必须回稳，不能残留 focus ring、pressed、黑白屏、裁切或整屏污染。
- 页面中不再出现 `guide`、状态桥接、外部 preview 标签和旧双列包裹壳层。
- 单测必须覆盖 setter 清理 pressed、same-target release / cancel、`read only / disabled` guard 清理残留 pressed，以及 static preview 吞掉 `touch / key` 且不改 `current_snapshot / current_part`。

## 9. 已知限制与后续方向
- 当前版本是页内固定锚点 reference，不做真实 popup 跟随。
- 当前不做自动避让和屏幕边缘翻转策略。
- 当前不做真实图标资源和入场动画。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不扩展成多目标引导系统。

## 10. 与现有控件的边界
- 相比 `message_bar`：这里强调目标锚定的上下文提示，不是横向反馈条。
- 相比 `dialog_sheet`：这里是 anchored callout，不是居中确认层。
- 相比 `toast_stack`：这里强调教学引导，不是短时通知。
- 相比 `menu_flyout`：这里不呈现命令列表，核心是目标提示与动作收口。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TeachingTip`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`TeachingTip`
- 本次保留核心状态：
  - `accent`
  - `warning`
  - `top placement`
  - `bottom placement`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、状态桥接、preview 标签和旧双列预览壳层。
- 删除 preview 参与点击切换、页面桥接和焦点承接的职责。
- 删除过重的 callout shadow、过亮的 pointer tail、过强的 action pill 和 close chrome。
- 删除系统级 popup、Acrylic、入场动画和多目标链式引导。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 主控件遵守 same-target release，并在 `read only / disabled` 输入守卫与各类 setter 里共用同一套残留 `pressed` 清理语义。
- `compact / read only` 统一通过 `egui_view_teaching_tip_override_static_preview_api()` 收口，吞掉 `touch / key`，不改 `current_snapshot / current_part`，点击仅用于清主控件 focus。
- 通过程序化切换 snapshot 与控件内导航 helper 保证 runtime 稳定抓取状态变化。
- `snapshot / compact / read only / disabled` 共用同一套 `pressed` 清理语义，确保 target、action pill 和 close affordance 在交互收尾后不残留旧高亮。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
