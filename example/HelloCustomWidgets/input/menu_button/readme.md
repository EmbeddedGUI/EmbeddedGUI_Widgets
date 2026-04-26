# Menu Button 控件说明

## 控件定位

`menu_button` 对齐 Fluent 2 / Fluent UI React 的 `MenuButton` 语义：按钮本身不执行主动作，而是打开一组命令菜单，用户再从菜单中选择具体命令。它补齐了 `Button`、`DropDownButton`、`SplitButton` 之间的语义空位，适用于页面操作、更多操作和命令集合入口。

现有控件不足之处：

- `Button` 只表达单一立即动作，不适合承载多个互斥或相关命令。
- `DropDownButton` 更偏向打开选项集合或下拉选择，不强调菜单项的命令、快捷键和勾选状态。
- `SplitButton` 同时提供主动作和菜单动作，而 `MenuButton` 只有菜单入口，没有主动作区域。

## 目标场景

示例页面保留三个层级：

- 主 `MenuButton`：显示 `Page actions` 触发按钮，点击后打开 `Page menu` 菜单。
- 菜单项：`New page`、`Duplicate`、`Export`、`Archive`，覆盖默认、强调、成功和警告语义色。
- 底部静态 preview：`compact` 与 `read only` 两个状态，验证非主交互区域在录制过程中保持稳定。

## 视觉与布局规格

- 整体使用浅色 Fluent 面板、低噪声边框和 9px 左右圆角。
- 触发按钮包含 leading icon、label 和 chevron，打开时 chevron 朝上。
- 菜单面板在触发按钮下方展开，包含标题、图标、文本、快捷键和 selected check mark。
- `compact` 模式只保留短触发按钮，不展开菜单。
- `read only` 模式保留外观但忽略触控和键盘输入。

## 状态矩阵

| 状态 | 行为 |
| --- | --- |
| Closed | 只显示触发按钮和当前选中项摘要 |
| Open | 显示菜单面板与焦点菜单项 |
| Pressed | same-target release 成立时才提交 |
| Selected | 当前命令显示 check mark，并同步 closed 摘要 |
| Disabled item | 可见但不可提交 |
| Compact | 静态短按钮展示 |
| Read only | 可见但不响应输入 |

## 录制动作

录制轨道覆盖：

1. 初始 closed 状态。
2. 触控点击触发按钮，打开菜单。
3. 键盘 `Down + Enter` 选择下一项。
4. 再次触控打开菜单并点击 `Export`。
5. 回到默认 closed 状态。

## 验收标准

- `make all APP=HelloCustomWidgets APP_SUB=input/menu_button PORT=pc` 通过。
- `make all APP=HelloUnitTest PORT=pc_test` 通过，且 `output\main.exe menu_button` 全部通过。
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category input` 通过。
- 单控件 runtime、input 分类 compile/runtime、WASM build、web smoke 全部通过。
- 截图中主按钮、菜单面板、快捷键、选中标记和底部 preview 均完整可见，无黑屏、白屏、裁切或文字重叠。

## 参考体系

- Reference system：Fluent 2
- Reference library：Fluent UI React
- Reference component：MenuButton

## EGUI 适配说明

- 当前实现停留在 `HelloCustomWidgets` reference 层，不下沉到 SDK。
- 菜单弹层绘制在同一 view 内，适合当前录制与静态验收流程。
- 键盘覆盖 `Down / Up / Home / End / Enter / Space / Escape`。
- 触控实现遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，`DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交。
