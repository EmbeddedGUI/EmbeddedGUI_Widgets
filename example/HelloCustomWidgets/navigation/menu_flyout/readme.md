# MenuFlyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`MenuFlyout / ContextMenu`
- 当前保留形态：`Open panel`、`Name ascending`、`Export report`、`Layout wide`、`compact`、`disabled`
- 当前保留交互：主区保留 same-target release、`Enter` 键盘点击、`set_snapshots / current_snapshot / font / meta_font / palette / compact / disabled` setter 统一清理 `pressed`，以及 `disabled_mode / !enable` guard 先清理 `pressed` 再拒绝提交；底部 `compact / disabled` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 click
- 当前移除内容：页面级 `guide`、状态说明、可切换 preview 标签、preview 参与切换的桥接逻辑、过重阴影、Acrylic、叙事化 showcase 外壳，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `menu_flyout` 基础实现，本轮只收口 `reference` 页面结构、静态 preview、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`menu_flyout` 用来表达“在局部区域弹出一组短命令”的标准菜单语义，适合卡片操作、工具栏溢出菜单、列表项上下文菜单这类需要快速触达命令的场景。它不是常驻导航，也不是工具栏按钮集合，而是更贴近 Fluent / WPF UI 的轻量命令面板。

## 2. 为什么现有控件不够
- `menu_bar` 是常驻顶层菜单栏，不承担局部弹出菜单语义。
- `command_bar` 更接近工具栏，不强调 `submenu / shortcut / danger / disabled` 这类菜单行结构。
- `nav_panel` 负责页面导航，不承载局部命令弹层。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI 的 `MenuFlyout / ContextMenu` 示例。

## 3. 当前页面结构
- 标题：`Menu Flyout`
- 主区：一个主 `menu_flyout`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Open / Copy / More`
- 右侧 preview：`disabled`，固定显示 `Rename / Export / Delete`
- 页面结构统一收口为：标题 -> 主 `menu_flyout` -> 底部 `compact / disabled`

目录：
- `example/HelloCustomWidgets/navigation/menu_flyout/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Open panel`
   默认主状态，显示 `shortcut / submenu / separator`
2. `Name ascending`
   程序化切到排序态，验证 `meta` 与强调行
3. `Export report`
   程序化切到危险命令态，验证 `danger` 行低噪音强调
4. `Layout wide`
   程序化切到布局态，验证最终主态
5. `Open panel`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Open / Copy / More`
   紧凑静态对照，固定 `compact_mode`
2. `disabled`
   `Rename / Export / Delete`
   禁用静态对照，固定 `compact_mode + disabled_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`188 x 104`
- 底部对照行尺寸：`214 x 78`
- 单个 preview 尺寸：`104 x 78`
- 页面结构：标题 -> 主 `menu_flyout` -> 底部 `compact / disabled`
- 页面风格：浅底、白色菜单卡、轻边框、低噪音阴影，以及通过轻填充、强调色和右侧 `meta` 文本表达菜单层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| `Open panel` | 是 | 否 | 否 |
| `Name ascending` | 是 | 否 | 否 |
| `Export report` | 是 | 否 | 否 |
| `Layout wide` | 是 | 否 | 否 |
| `Open / Copy / More` | 否 | 是 | 否 |
| `Rename / Export / Delete` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `disabled_mode` | 否 | 否 | 是 |
| `same-target release` | 是 | 否 | 否 |
| `keyboard click` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_menu_flyout.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 覆盖 snapshot 数量钳制、当前 snapshot 重置，以及 `pressed` 清理。
2. `set_current_snapshot()` 覆盖越界 guard、空 snapshot guard，以及 `pressed` 清理。
3. `font / meta_font / palette / set_snapshots / current_snapshot / compact / disabled` setter 全部覆盖 `pressed` 清理。
4. 触摸点击与 `Enter` 键盘点击都会驱动 click listener。
5. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 click；`ACTION_CANCEL` 只清理 `pressed`。
6. `disabled_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入。
7. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_snapshot / compact_mode` 不变，且不会触发 click。
8. 内部 helper 用例覆盖 snapshot/item 数量钳制、焦点项收敛与 `meta` 宽度计算。

补充说明：
- 主区 same-target release 继续遵守非拖拽控件口径：只有回到初始命中区域再 `UP` 才能提交。
- 底部 `compact / disabled` preview 统一通过 `egui_view_menu_flyout_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- 预览输入只清理残留 `pressed`，不改 `current_snapshot`，也不触发 click。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Open panel` 和底部两个静态 preview，请求首帧并等待 `MENU_FLYOUT_RECORD_FRAME_WAIT`。
2. 切到 `Name ascending`，等待 `MENU_FLYOUT_RECORD_WAIT`。
3. 请求第二帧并等待 `MENU_FLYOUT_RECORD_FRAME_WAIT`。
4. 切到 `Export report`，等待 `MENU_FLYOUT_RECORD_WAIT`。
5. 请求第三帧并等待 `MENU_FLYOUT_RECORD_FRAME_WAIT`。
6. 切到 `Layout wide`，等待 `MENU_FLYOUT_RECORD_WAIT`。
7. 请求第四帧并等待 `MENU_FLYOUT_RECORD_FRAME_WAIT`。
8. 回到默认 `Open panel`，同步底部 preview 固定状态并等待 `MENU_FLYOUT_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `MENU_FLYOUT_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / disabled` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Open panel / Name ascending / Export report / Layout wide` 四组状态，以及最终回到默认 `Open panel` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/menu_flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_menu_flyout
```

## 10. 验收重点
- 主控件和底部 `compact / disabled` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 行内图标占位、标题、右侧快捷键和子菜单箭头要保持稳定对齐。
- `danger` 与 `disabled` 要一眼可辨，但不能退回高噪音装饰。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能误提交，只有回到 `A` 才能提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_snapshot` 或误触发 click。
- `disabled_mode / !enable / static preview` 都不能误触发 click，并且要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `Open panel`，底部 `compact / disabled` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_menu_flyout/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Open panel`、`Name ascending`、`Export report` 和 `Layout wide`
  - 主区 RGB 差分边界为 `(66, 111) - (412, 184)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 184` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Open panel`

## 12. 与现有控件的边界
- 相比 `menu_bar`：这里是局部弹出菜单，不是常驻顶层菜单栏。
- 相比 `command_bar`：这里强调菜单行语义，不是工具栏按钮集合。
- 相比 `nav_panel`：这里不承担页面导航，只承载局部命令入口。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和 preview 桥接逻辑。

## 13. 本轮保留与删减
- 保留的主区状态：`Open panel`、`Name ascending`、`Export report`、`Layout wide`
- 保留的底部对照：`compact`、`disabled`
- 保留的交互与实现约束：same-target release、`Enter` 键盘点击、`set_snapshots / current_snapshot / font / meta_font / palette / compact / disabled` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、状态说明、可切换 preview 标签、preview 参与切换的桥接逻辑、过重阴影、Acrylic、叙事化 showcase 外壳、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `menu_flyout` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_menu_flyout/default`
  - 共捕获 `11` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/menu_flyout`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_menu_flyout`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=161`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Open panel`、`Name ascending`、`Export report` 和 `Layout wide`
  - 主区 RGB 差分边界为 `(66, 111) - (412, 184)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 184` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Open panel`，底部 `compact / disabled` preview 全程保持静态
