# menu_flyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`MenuFlyout / ContextMenu`
- 本次保留状态：`standard`、`submenu`、`shortcut`、`danger`、`compact`、`disabled`
- 删除效果：页面级 `guide`、状态文案、section label、可点击预览卡、Acrylic、桌面级大阴影、复杂级联子菜单动画
- EGUI 适配说明：继续复用仓库内 `menu_flyout` 基础实现，本轮只收口 `reference` 页结构、示例命令内容和绘制强度，不改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`menu_flyout` 用来表达“在局部区域弹出一组短命令”的标准菜单语义。它适合卡片操作、列表项上下文菜单、工具栏溢出菜单这类需要短路径操作的场景。

## 2. 为什么现有控件不够用
- `menu_bar` 是常驻顶层菜单栏，不是局部弹出菜单。
- `nav_panel` 是页面导航容器，不承担命令弹出职责。
- `command_bar` 更接近工具栏，不强调 `submenu / shortcut / danger / disabled` 这类菜单行语义。

因此这里继续保留 `menu_flyout`，但示例页要回到统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主状态：展示一张标准 `MenuFlyout` 面板，保留图标占位、标题、右侧快捷键、危险项和 submenu 入口。
- `compact` 预览：保留同一语义，但缩短行高和内容宽度，用于验证小尺寸收口。
- `disabled` 预览：保留命令结构，但统一弱化交互状态，用于验证禁用边界。
- 页面只保留标题、主 `menu_flyout` 和底部 `compact / disabled` 双预览，不再保留 guide、状态栏、section label 或可点击预览卡。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`188 x 104`
- 底部对照行尺寸：`214 x 78`
- `compact` 预览：`104 x 78`
- `disabled` 预览：`104 x 78`
- 页面结构：标题 -> 主 `menu_flyout` -> `compact / disabled`
- 样式约束：
  - 外层继续保持浅底、轻边框、弱阴影的菜单卡片语义
  - 行内状态只通过轻填充、高亮行和右侧 meta 文本表达，不回退到 showcase 式大色块
  - `danger` 必须可辨识，但不能压过整体的中性菜单结构
  - 底部两个预览固定为静态 reference 对照，不再承担切换职责

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `flyout_primary` | `egui_view_menu_flyout_t` | `188 x 104` | `Review history` | 主 `MenuFlyout` |
| `flyout_compact` | `egui_view_menu_flyout_t` | `104 x 78` | `compact open/copy/more` | 紧凑静态对照 |
| `flyout_disabled` | `egui_view_menu_flyout_t` | `104 x 78` | `disabled rename/export/delete` | 禁用静态对照 |
| `primary_snapshots` | `egui_view_menu_flyout_snapshot_t[4]` | - | `open/review/copy/rename` | 主面板轨道切换数据 |
| `compact_snapshots` | `egui_view_menu_flyout_snapshot_t[2]` | - | `compact action` | 紧凑预览录制切换数据 |
| `disabled_snapshots` | `egui_view_menu_flyout_snapshot_t[1]` | - | `disabled action` | 禁用预览固定数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `Review history` | 默认态 | 保留 submenu 入口和 separator |
| 主控件 | `Pin summary` | 状态命令 | 验证 `On` 右对齐和高亮行 |
| 主控件 | `Delete record` | 危险命令 | 验证 danger 语义但保持低噪音 |
| 主控件 | `Move to group` | 布局命令 | 验证第二组 submenu 入口 |
| `compact` | `Open / Copy / More` | 紧凑对照 | 行高收紧、层级仍可辨识 |
| `disabled` | `Rename / Export / Delete` | 禁用对照 | 保留结构但统一弱化 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主 snapshot 与 `compact` snapshot 后输出默认帧。
2. 切换到状态命令 snapshot，验证 `meta` 和 separator 的稳定对齐。
3. 切换到危险命令 snapshot，验证 danger 行的弱强调表达。
4. 切换 `compact` snapshot，验证底部静态对照更新。
5. 切换到布局命令 snapshot，输出最终帧。

## 8. 编译、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主面板和底部 `compact / disabled` 预览都必须完整可见
- 行内图标占位、标题、右侧快捷键和 submenu 箭头需要稳定对齐
- `danger` 项和 `disabled` 预览必须可辨识，但不能变成高噪音装饰
- 底部预览不再响应点击，只保留静态 reference 对照

## 9. 已知限制与后续方向
- 当前仍是固定尺寸 `reference` 示例，不做真实 popup 定位与级联子菜单展开。
- 当前图标使用双字母占位，不接入真实图标资源。
- 当前录制仍采用程序化切换 snapshot，不依赖真实命中操作。

## 10. 与现有控件的边界
- 相比 `menu_bar`：这里是局部弹出菜单，不是常驻顶层菜单栏。
- 相比 `command_bar`：这里强调菜单行语义，不是工具栏按钮集合。
- 相比 `nav_panel`：这里不承担页面导航，只承担局部命令入口。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`MenuFlyout / ContextMenu`
- 本次保留核心状态：
  - `standard`
  - `submenu`
  - `shortcut`
  - `danger`
  - `compact`
  - `disabled`

## 13. 相比参考原型删掉的效果或装饰
- 删掉 Acrylic、桌面级大阴影和复杂级联子菜单动画。
- 删掉可点击底部预览卡和页面级 guide / 状态文案 / section label。
- 删掉真实图标、checkbox/radio 菜单项和完整 hover/pressed/focus ring 体系。

## 14. EGUI 适配时的简化点与约束
- 用固定 snapshot + item 数组驱动菜单内容，优先保证示例稳定。
- 用统一的行高、separator 和右对齐 meta 文本保证 `480 x 480` 下的可审阅性。
- `compact` 和 `disabled` 预览固定放在底部双列，只保留 reference 对照，不再承担交互职责。
- 当前先作为 `HelloCustomWidgets` 示例推进，后续如需下沉到框架层，再单独评估与通用 `menu` 抽象的边界。
