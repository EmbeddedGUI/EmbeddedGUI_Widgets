# command_bar_flyout 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照控件：`command_bar`、`menu_flyout`
- 对应组件名：`CommandBarFlyout`
- 本次保留状态：`trigger`、`primary commands`、`secondary commands`、`compact`、`disabled`
- 本次删除效果：真实 popup 定位、系统级悬浮层、复杂动画、页面级 guide、叙事化 showcase 外壳
- EGUI 适配说明：在 custom 层新增轻量 `egui_view_command_bar_flyout`，用一套 `snapshot + current_part + pressed_part` 收口触发器、主命令和次级命令，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`command_bar_flyout` 用来表达“由一个触发入口打开的命令面板”。它既不是常驻工具栏，也不是独立菜单栏，而是更贴近 Fluent / WPF UI `CommandBarFlyout` 的轻量命令容器：上部保留主命令，下面承接次级命令。

## 2. 为什么现有控件不够用

- `command_bar` 是常驻命令条，没有 flyout 触发入口和次级命令分层。
- `menu_flyout` 只有纵向菜单面板，不保留主命令轨道。
- `split_button` / `drop_down_button` 只覆盖单一入口，不承载整组主命令和次级命令。

因此这里新增 `command_bar_flyout` reference widget，用来补齐“触发器 + 主命令 + 次级命令”的组合语义。

## 3. 目标场景与示例概览

- 主控件展示一张标准 `CommandBarFlyout`，顶部有触发器，下方展开后包含主命令 rail 和次级命令列表。
- 左下保留 `compact` 静态对照，验证小尺寸收口。
- 右下保留 `disabled` 静态对照，验证禁用态渲染。
- 页面结构统一为：标题 -> 主 `command_bar_flyout` -> `compact / disabled` 双 preview。

目标目录：`example/HelloCustomWidgets/input/command_bar_flyout/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 272`
- 主控件尺寸：`196 x 160`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 尺寸：`104 x 72`
- 风格约束：
  - 保持浅底、白色面板、低噪音边框与阴影。
  - 主命令 rail 复用 `command_bar` 的轻量按钮语言。
  - 次级命令列表复用 `menu_flyout` 的行式 panel 语言。
  - 触摸必须满足 same-target release：
    - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
    - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `flyout_primary` | `egui_view_command_bar_flyout_t` | `196 x 160` | `Page actions` | 主 `CommandBarFlyout` |
| `flyout_compact` | `egui_view_command_bar_flyout_t` | `104 x 72` | `Compact flyout` | 底部 compact static preview |
| `flyout_disabled` | `egui_view_command_bar_flyout_t` | `104 x 72` | `Disabled flyout` | 底部 disabled static preview |
| `primary_snapshots` | `egui_view_command_bar_flyout_snapshot_t[4]` | - | `Edit / Review / Quick / Layout` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_command_bar_flyout_snapshot_t[2]` | - | `Quick / Review` | compact 对照 |
| `disabled_snapshot` | `egui_view_command_bar_flyout_snapshot_t[1]` | - | `Locked` | disabled 固定对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Edit` | 默认展开态，展示主命令和次级命令 |
| 主控件 | `Review` | 次级命令焦点轨道 |
| 主控件 | `Quick` | 关闭态，只保留 trigger |
| 主控件 | `Layout` | 再次展开，用于最终稳定帧 |
| `compact` | `Quick / Review` | 紧凑静态对照 |
| `disabled` | `Locked` | 禁用静态对照 |

- 主控件保留真实 `touch / key` 交互：
  - `Left / Right / Tab / Home / End`
  - `Up / Down`
  - `Enter / Space`
  - `Escape`
- preview 通过 `egui_view_command_bar_flyout_override_static_preview_api()` 统一吞掉 `touch / key` 输入。

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件和 `compact` preview 到默认状态。
2. 输出默认展开帧。
3. 键盘向右移动到主命令。
4. 输出第二帧。
5. 切换到 `Review` snapshot，并向下移动到次级命令。
6. 输出第三帧。
7. 切换到 `Quick`，验证关闭态。
8. 输出第四帧。
9. 切换到 `Layout`，同时切换 `compact` preview。
10. 输出第五帧。
11. 点击 `compact` preview，只做清焦收尾。
12. 输出最终稳定帧。

## 8. 编译、单测、touch、runtime 与 web 验收路径

```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar_flyout PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar_flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64

python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py

python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar_flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar_flyout
```

## 9. 验收重点

- 触发器、主命令 rail、次级命令列表都必须完整可见，不能黑屏或裁切。
- 关闭态只能保留 trigger，展开态必须完整显示主命令和次级命令分层。
- `set_snapshots()`、`set_current_snapshot()`、`set_open()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 都要清掉残留 `pressed_part / is_pressed`。
- 键盘 `Escape` 只能在展开态关闭 flyout，`Enter / Space` 要能激活 trigger 或当前命令。
- `compact` 和 static preview 都不能误触发打开或命令 action。

## 10. 已知限制与后续方向

- 当前不做真实 popup 定位和屏幕边缘避让。
- 当前主命令和次级命令都用固定 `snapshot` 数据驱动，不接业务回调。
- 当前图标继续使用双字母占位，不接入真实资源。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，是否下沉框架层后续再评估。

## 11. 与现有控件的边界

- 相比 `command_bar`：这里有 trigger 和次级命令分层，不是常驻工具栏。
- 相比 `menu_flyout`：这里保留主命令 rail，不是纯纵向菜单。
- 相比 `split_button`：这里承载一组主命令和一组次级命令，不是单个复合按钮。

## 12. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`

## 13. 对应组件名与本次保留的核心状态

- 对应组件名：`CommandBarFlyout`
- 本次保留核心状态：
  - `trigger`
  - `primary commands`
  - `secondary commands`
  - `compact`
  - `disabled`

## 14. 相比参考原型删除的效果与 EGUI 适配约束

- 删除真实 popup 定位、系统级悬浮层、复杂动画和页面级 guide。
- 删除 showcase 式场景包装，只保留 reference 结构。
- 用 `snapshot + current_part + pressed_part + open_state` 收口录制、单测和 preview。
- 不修改 `sdk/EmbeddedGUI`，只在 custom 层实现与验收。
