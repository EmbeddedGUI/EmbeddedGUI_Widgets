# MenuBar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`MenuBar`
- 当前保留形态：`File`、`Edit`、`View`、`Tools`、`compact`、`read only`
- 当前保留交互：主区继续保留顶层 menu 切换、panel row 选择与激活、same-target release、`Left / Right / Home / End / Down / Enter` 键盘导航，以及 `set_snapshots / current_snapshot / current_item / font / meta_font / palette / compact / read_only` setter 统一清理 `pressed_menu / pressed_item / is_pressed`；`read_only / !enable` guard 会先清理 `pressed` 再拒绝后续 `touch / key` 输入；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 selection / activation listener
- 当前移除内容：页面级 `guide`、状态文案、section label、可点击 preview 卡、preview 焦点循环、preview 清焦收尾桥接、重阴影、强描边、重型 focus ring、桌面系统级多级菜单叙事，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `menu_bar` 基础实现，本轮只收口 `reference` 页面说明、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`menu_bar` 用来表达“常驻顶层命令分类 + 当前菜单下拉面板”的标准桌面菜单栏语义，适合 `File / Edit / View / Tools` 这类结构稳定、分组明确的命令入口。

## 2. 为什么现有控件不够
- `menu_flyout` 是局部弹出菜单，不承担常驻顶层菜单栏语义。
- `command_bar` 更接近工具栏，不强调顶层分类菜单与下拉层级。
- `nav_panel` 负责页面导航，不是命令菜单。
- `tab_strip` 负责页面切换，不是命令分组入口。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `MenuBar` 语义的 custom widget。

## 3. 当前页面结构
- 标题：`Menu Bar`
- 主区：一个主 `menu_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Open / Compact`
- 右侧 preview：`read only`，固定显示 `Pinned` 摘要
- 页面结构统一收口为：标题 -> 主 `menu_bar` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/menu_bar/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `File`
   默认主状态，显示 `New workspace / Open recent / Copy link / Properties`
2. `Edit`
   程序化切到编辑态，验证 `Redo / Find in page / Preferences`
3. `View`
   程序化切到视图态，验证 `Density compact / Reading mode / Panels`
4. `Tools`
   程序化切到工具态，验证 `Sync now / Export report / Delete record`
5. `File`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Open / Compact`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Pinned`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 108`
- 底部对照行尺寸：`216 x 74`
- 单个 preview 尺寸：`104 x 74`
- 页面结构：标题 -> 主 `menu_bar` -> 底部 `compact / read only`
- 页面风格：浅底、白色菜单卡、轻边框、低噪音阴影，以及通过轻填充、细 underline、右侧 meta 和克制的 danger 色表达菜单层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `File` | 是 | 否 | 否 |
| `Edit` | 是 | 否 | 否 |
| `View` | 是 | 否 | 否 |
| `Tools` | 是 | 否 | 否 |
| `Open / Compact` | 否 | 是 | 否 |
| `Pinned` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| menu / row same-target release | 是 | 否 | 否 |
| keyboard navigation | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_menu_bar.c` 当前覆盖 `16` 条用例：

1. `set_snapshots()` 覆盖 disabled focus item 收敛、disabled 初始 snapshot 跳过，以及基础状态回落。
2. `set_current_item()` 覆盖 disabled row 跳过，确保不会误落到不可交互项。
3. `set_current_snapshot()` 覆盖 disabled menu snapshot 跳过，并在切换时清理 runtime pressed 状态。
4. `font / meta_font / palette / set_snapshots / current_snapshot / current_item / compact / read_only` setter 全部覆盖 `pressed_menu / pressed_item / is_pressed` 清理。
5. 触摸点击顶层 menu 会切换 `current_snapshot`，并触发 selection listener。
6. 触摸点击 panel row 会先更新 `current_item`，再在 `UP` 时触发 activation listener。
7. `ACTION_CANCEL` 只清理 `pressed`，不会误改当前 snapshot / item，也不会误触发 listener。
8. menu 与 panel row 都继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
9. 键盘导航覆盖 `Down / Left / Right / Home / End / Enter`，验证 menu 切换、row 切换与当前项激活。
10. `read_only / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入；恢复后继续允许正常导航。
11. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_snapshot / current_item / compact_mode` 不变，且不会触发 selection / activation listener。

补充说明：
- 主区顶层 menu 与 panel row 都属于非拖拽点击目标，统一遵守 same-target release。
- 底部 `compact / read only` preview 统一通过 `egui_view_menu_bar_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- preview 输入只清理残留 `pressed`，不改 `current_snapshot / current_item`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `File` 和底部两个静态 preview，请求首帧并等待 `MENU_BAR_RECORD_FRAME_WAIT`。
2. 切到 `Edit`，等待 `MENU_BAR_RECORD_WAIT`。
3. 请求第二帧并等待 `MENU_BAR_RECORD_FRAME_WAIT`。
4. 切到 `View`，等待 `MENU_BAR_RECORD_WAIT`。
5. 请求第三帧并等待 `MENU_BAR_RECORD_FRAME_WAIT`。
6. 切到 `Tools`，等待 `MENU_BAR_RECORD_WAIT`。
7. 请求第四帧并等待 `MENU_BAR_RECORD_FRAME_WAIT`。
8. 回到默认 `File`，同步底部 preview 固定状态并等待 `MENU_BAR_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `MENU_BAR_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `File / Edit / View / Tools` 四组状态，以及最终回到默认 `File` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/menu_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_menu_bar
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 顶层 underline、panel 锚点、separator、当前 row 和 danger row 都必须可辨识，但不能退回高噪音装饰。
- 主区域需要出现 `File -> Edit -> View -> Tools -> File` 的真实变化。
- menu 与 panel row 都必须继续满足 same-target release，不能误提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_snapshot / current_item`。
- `read_only / !enable / static preview` 都不能误触发 selection / activation listener，并且要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `File`，底部 `compact / read only` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_menu_bar/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `File`、`Edit`、`View` 和 `Tools`
  - 主区 RGB 差分边界为 `(50, 109) - (430, 211)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 211` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `File`

## 12. 与现有控件的边界
- 相比 `menu_flyout`：这里是常驻顶层菜单栏，不是局部弹出菜单。
- 相比 `command_bar`：这里强调命令分类和下拉面板，不是工具栏按钮集合。
- 相比 `nav_panel`：这里表达命令结构，不承担页面导航。
- 相比 `tab_strip`：这里不是页面切换，而是命令入口组织。

## 13. 本轮保留与删减
- 保留的主区状态：`File`、`Edit`、`View`、`Tools`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：顶层 menu 切换、panel row 选择与激活、same-target release、`Left / Right / Home / End / Down / Enter` 键盘导航、`set_snapshots / current_snapshot / current_item / font / meta_font / palette / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、状态文案、section label、可点击 preview 卡、preview 焦点循环、preview 清焦收尾桥接、重阴影、强描边、重型 focus ring、桌面系统级多级菜单叙事、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `menu_bar` suite `16 / 16`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_menu_bar/default`
  - 共捕获 `11` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/menu_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_menu_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=174`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `File`、`Edit`、`View` 和 `Tools`
  - 主区 RGB 差分边界为 `(50, 109) - (430, 211)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 211` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `File`，底部 `compact / read only` preview 全程保持静态
