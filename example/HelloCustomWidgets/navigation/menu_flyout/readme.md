# menu_flyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`MenuFlyout / ContextMenu`
- 本次保留状态：`standard`、`submenu`、`shortcut`、`danger`、`compact`、`disabled`
- 本次删除效果：页面级 `guide`、状态说明、可切换 preview 标签、过重阴影、Acrylic、叙事化 showcase 外壳
- EGUI 适配说明：继续复用仓库内 `menu_flyout` 基础实现，本轮只收口 `reference` 页面结构、交互语义和 runtime 收尾，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`menu_flyout` 用来表达“在局部区域弹出一组短命令”的标准菜单语义，适合卡片操作、工具栏溢出菜单、列表项上下文菜单这类需要快速触达命令的场景。它不是常驻导航，也不是工具栏按钮集合，而是更贴近 Fluent / WPF UI 的轻量命令面板。

## 2. 为什么现有控件不够用？
- `menu_bar` 是常驻顶层菜单栏，不承担局部弹出菜单语义。
- `command_bar` 更接近工具栏，不强调 `submenu / shortcut / danger / disabled` 这类菜单行结构。
- `nav_panel` 负责页面导航，不承载局部命令弹层。

因此这里继续保留 `menu_flyout`，但示例页面只保留符合 Fluent / WPF UI 主线的 `reference` 结构。

## 3. 目标场景与示例概览
- 主控件展示一张标准 `MenuFlyout` 面板，保留图标占位、标题、右侧快捷键、危险项和子菜单入口。
- 底部左侧展示 `compact` 静态对照，用于验证小尺寸下的菜单行收口。
- 底部右侧展示 `disabled` 静态对照，用于验证禁用态渲染。
- 页面结构统一收口为：标题 -> 主 `menu_flyout` -> `compact / disabled` 双 preview。
- 两个 preview 统一通过 `egui_view_menu_flyout_override_static_preview_api()` 收口：
  - preview 吞掉 `touch / key` 输入；
  - preview 只负责清理残留 `pressed`；
  - preview 不会修改 `current_snapshot`，也不会触发 click；
  - preview 点击只保留一个最小收尾逻辑：清主控件 focus。
- 旧的 guide、状态栏、额外 preview 说明文字和场景化包装全部删除。

目标目录：`example/HelloCustomWidgets/navigation/menu_flyout/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`188 x 104`
- 底部对照行尺寸：`214 x 78`
- `compact` preview：`104 x 78`
- `disabled` preview：`104 x 78`
- 页面结构：标题 + 主控件 + 底部双 preview
- 样式约束：
  - 保持浅底、白色菜单卡、轻边框、低噪音阴影的 Fluent 菜单语义。
  - 行内状态只通过轻填充、强调色和右侧 `meta` 文本表达，不回退到高噪音 showcase 风格。
  - `danger` 和 `disabled` 必须可辨识，但不能压过整体中性菜单层级。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
  - `snapshot / compact / disabled / !enable` 切换后都不能残留旧的 `pressed` 高亮。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Menu Flyout` | 页面标题 |
| `flyout_primary` | `egui_view_menu_flyout_t` | `188 x 104` | `Review history` | 标准主控件 |
| `flyout_compact` | `egui_view_menu_flyout_t` | `104 x 78` | `Open / Copy / More` | `compact` 静态对照 |
| `flyout_disabled` | `egui_view_menu_flyout_t` | `104 x 78` | `Rename / Export / Delete` | `disabled` 静态对照 |
| `primary_snapshots` | `egui_view_menu_flyout_snapshot_t[4]` | - | `Open / Sort / Export / Layout` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_menu_flyout_snapshot_t[2]` | - | `Open / Pin` | `compact` 程序化切换轨道 |
| `disabled_snapshots` | `egui_view_menu_flyout_snapshot_t[1]` | - | `Rename / Export / Delete` | `disabled` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `Open panel` | 默认菜单态 | 保留 `shortcut / submenu / separator` |
| 主控件 | `Name ascending` | 排序态 | 验证 `meta` 与强调行对齐 |
| 主控件 | `Export report` | 危险命令态 | 验证 `danger` 行的低噪音强调 |
| 主控件 | `Layout wide` | 最终主态 | 作为交互后稳定态 |
| `compact` | `Open / Copy / More` | 默认紧凑态 | 验证小尺寸结构 |
| `compact` | `Pin / Density / Delete` | 第二组紧凑态 | 作为 runtime 关键帧 |
| `disabled` | `Rename / Export / Delete` | 固定禁用态 | static preview，点击只清主控件 focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认状态。
2. 请求初始帧。
3. 切到第二组主菜单命令。
4. 请求第二帧。
5. 切到危险命令组。
6. 请求第三帧。
7. 切到第四组主菜单命令。
8. 请求第四帧。
9. 切换 `compact` 到第二组 snapshot，并主动给主控件请求 focus。
10. 请求 `compact` 第二帧。
11. 点击 `compact` preview，只执行 preview 的静态收尾逻辑。
12. 请求 preview 点击后的收尾帧。
13. 再请求最终稳定帧，确认没有残留 `pressed / focus`、黑白屏、裁切或整屏污染。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make clean APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

## 9. 验收重点
- 主控件和底部 `compact / disabled` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 行内图标占位、标题、右侧快捷键和子菜单箭头要保持稳定对齐。
- `danger` 与 `disabled` 要一眼可辨，但不能退回高噪音装饰。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不提交，回到原目标后释放才提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 都要先清掉残留 `pressed`。
- `disabled_mode`、`!enable` 和 static preview 都不能误触发 click。
- runtime 需要重点复核初始帧、危险态、`compact` 第二帧、preview 点击后的收尾帧和最终稳定帧，确认交互后渲染干净。

## 10. 已知限制与后续方向
- 当前仍是固定尺寸 `reference` 示例，不做真实 popup 定位和级联子菜单展开。
- 图标继续使用双字母占位，不接入真实图标资源。
- 录制仍以程序化切换 `snapshot` 为主，不依赖真实业务命令回调。

## 11. 与现有控件的边界
- 相比 `menu_bar`：这里是局部弹出菜单，不是常驻顶层菜单栏。
- 相比 `command_bar`：这里强调菜单行语义，不是工具栏按钮集合。
- 相比 `nav_panel`：这里不承担页面导航，只承载局部命令入口。

## 12. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 13. 对应组件名与本次保留的核心状态
- 对应组件名：`MenuFlyout / ContextMenu`
- 本次保留核心状态：
  - `standard`
  - `submenu`
  - `shortcut`
  - `danger`
  - `compact`
  - `disabled`

## 14. 相比参考原型删除的效果与 EGUI 适配约束
- 删除页面级 `guide`、状态说明、preview 标签和场景化外壳。
- 删除 preview 参与页面切换和额外交互闭环的职责，只保留静态对照与主控件 focus 收尾。
- 删除 Acrylic、过重阴影、复杂级联子菜单动画和桌面系统级菜单接管。
- 继续复用仓库内 `menu_flyout` 基础实现，不修改 `sdk/EmbeddedGUI`。
- 通过统一的 `clear_pressed_state()` 和 static preview API，把 setter、guard、preview 的状态清理收口到同一套语义。
