# menu_flyout 设计说明

## 1. 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`MenuFlyout / ContextMenu`

## 2. 为什么需要这个控件
`menu_flyout` 用来表达“在局部区域弹出一组短命令”的标准菜单语义，适合卡片操作、工具栏溢出菜单、列表项上下文菜单这类需要快速触达命令的场景。

## 3. 为什么现有控件不够用
- `menu_bar` 是常驻顶层菜单栏，不适合局部弹出。
- `nav_panel` 负责页面导航，不承载命令弹层语义。
- `command_bar` 更接近工具栏，不强调 `submenu / shortcut / danger / disabled` 这类菜单行结构。

因此这里继续保留 `menu_flyout`，但示例页只保留符合 Fluent / WPF UI 主线的 reference 结构。

## 4. 目标场景与示例概览
- 主控件：展示一张标准 `MenuFlyout` 面板，保留图标占位、标题、右侧快捷键、危险项和子菜单入口。
- `compact` 预览：保留相同命令语义，但压缩行高与宽度，验证小尺寸下的渲染收口。
- `disabled` 预览：保留命令结构，但统一弱化交互状态，用于验证禁用态渲染。
- 页面结构只保留标题、主 `menu_flyout` 和底部 `compact / disabled` 双预览，不再保留 guide、状态栏或可切换预览卡。

## 5. 视觉与布局规格
- 根容器：`224 x 224`
- 主控件：`188 x 104`
- 底部双预览容器：`214 x 78`
- 单个预览卡：`104 x 78`
- 页面层级：标题 -> 主 `menu_flyout` -> `compact / disabled`
- 视觉约束：
  - 保持浅底、轻边框、弱阴影的 Fluent 菜单面板语义。
  - 行内状态只通过轻填充、强调色和右侧 meta 文本表达，不回退到高噪音 showcase 风格。
  - `danger` 必须可辨识，但不能压过整体的中性菜单结构。
  - 底部两个预览固定为静态 reference 对照，不承担真实命令交互。

## 6. 控件清单与状态矩阵

| 变量名 | 类型 | 尺寸 | 初始状态 | 用途 |
| --- | --- | --- | --- | --- |
| `flyout_primary` | `egui_view_menu_flyout_t` | `188 x 104` | `Review history` | 主 `MenuFlyout` |
| `flyout_compact` | `egui_view_menu_flyout_t` | `104 x 78` | `Open / Copy / More` | 紧凑静态对照 |
| `flyout_disabled` | `egui_view_menu_flyout_t` | `104 x 78` | `Rename / Export / Delete` | 禁用静态对照 |
| `primary_snapshots` | `egui_view_menu_flyout_snapshot_t[4]` | - | 主面板四组命令 | 主轨道切换数据 |
| `compact_snapshots` | `egui_view_menu_flyout_snapshot_t[2]` | - | 两组 compact 对照 | 预览录制数据 |
| `disabled_snapshots` | `egui_view_menu_flyout_snapshot_t[1]` | - | 一组 disabled 对照 | 静态禁用数据 |

## 7. 交互与状态语义
- 主控件保留真实点击与 `Enter` 调用链路，点击后切换到下一组 `snapshot`。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 都会先清理残留 `pressed`，避免切换后留下旧高亮或下压位移。
- `disabled_mode` 或 `!enable` 时，新的 touch / key 输入会先清掉残留 `pressed`，然后直接拒绝提交。
- `ACTION_CANCEL` 仍复用底层 `egui_view_on_touch_event()` 的 same-target release 语义，只负责清 pressed，不触发 click。
- 底部预览统一通过 `egui_view_menu_flyout_override_static_preview_api()` 收口：
  - 预览吞掉 touch / key 输入。
  - 预览只负责清残留 `pressed`。
  - 预览不会改 `snapshot`，也不会触发 click。
- preview 点击只保留一个最小收尾逻辑：清主控件 focus，不承担任何额外切换职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 按以下顺序录制关键帧：
1. 重置主控件与 `compact` 预览，输出初始帧。
2. 切到第二组主面板命令，验证 `meta` 与 `separator` 对齐。
3. 切到危险命令组，验证 `danger` 行的弱强调渲染。
4. 切换 `compact` 预览的 `snapshot`，验证小尺寸收口。
5. 切回主控件第四组命令，验证最终主态。
6. 点击 `compact` 预览，确认静态 preview 只做清焦收尾、不污染渲染。
7. 输出 preview 点击后的最终稳定帧。

## 9. 编译、单测与 runtime 验收命令
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

## 10. 验收重点
- 主面板和底部 `compact / disabled` 预览都必须完整可见，不能黑屏、白屏或裁切。
- 行内图标占位、标题、右侧快捷键和子菜单箭头要保持稳定对齐。
- `danger` 与 `disabled` 要可辨识，但不能退回高噪音装饰。
- `snapshot / compact / disabled / !enable` 切换后不能残留 `pressed` 高亮或下压位移。
- `ACTION_CANCEL`、`disabled_mode`、`!enable` 和 static preview 都不能误触发 click。
- 需要抽查 runtime 输出中的以下关键帧：
  - 初始帧
  - 主控件切换帧
  - `compact` 切换帧
  - preview 点击后的收尾帧

## 11. 已知限制
- 当前仍是固定尺寸 `reference` 示例，不做真实 popup 定位和级联子菜单展开。
- 图标继续使用双字母占位，不接入真实图标资源。
- 录制仍以程序化切换 `snapshot` 为主，不依赖真实命中操作驱动主控件切换。

## 12. 与现有控件的边界
- 相比 `menu_bar`：这里是局部弹出菜单，不是常驻顶层菜单栏。
- 相比 `command_bar`：这里强调菜单行语义，不是工具栏按钮集合。
- 相比 `nav_panel`：这里不承担页面导航，只承载局部命令入口。

## 13. 本轮保留与删除的语义
- 保留：`standard`、`submenu`、`shortcut`、`danger`、`compact`、`disabled`
- 删除：页面级 guide、状态文案、可切换 preview 卡、Acrylic、桌面级大阴影、复杂级联子菜单动画

## 14. EGUI 适配说明
- 继续复用仓库内 `menu_flyout` 基础实现，不修改 `sdk/EmbeddedGUI`。
- 通过统一的 `clear_pressed_state()` 与 static preview API，把 setter、guard、preview 的状态清理收口到同一套语义。
- 当前先作为 `HelloCustomWidgets` reference 示例维护，后续是否下沉到框架层再单独评估。
