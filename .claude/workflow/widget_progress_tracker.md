# Widget Progress Tracker

## 用途说明

- 本文件用于跟踪 `HelloCustomWidgets` 当前仍保留的主线控件。
- 每次继续工作前，先读 `.claude/workflow/widget_acceptance_workflow.md` 和本文件。
- 当前仓库已经清退全部 `showcase` / `deprecated` 控件目录，只保留 `reference` 主线。
- 后续如果某个控件无法继续证明其 `Fluent 2 / WPF UI` 主线价值，应在这里明确记录删除结论，而不是继续堆积。

## 当前快照

- 截至 `2026-04-09`，`example/HelloCustomWidgets/` 当前保留 `42` 个控件目录。
- 所有保留控件均来自 `reference` 主线：
  - `input = 18`
  - `layout = 6`
  - `navigation = 10`
  - `display = 3`
  - `feedback = 5`
- 当前 `widget_catalog.json`、`web/catalog-policy.json` 与默认 web 入口已同步到 `reference-only` 状态。
- 已清退轨道：
  - 全部 `deprecated`
  - 全部 `showcase`
  - 非主线历史分类 `chart/*`、`media/*` 及其它不再保留的场景化控件

## 当前固定规则

- 第一优先级：统一到 `Fluent 2 / WPF UI` 主线。
- 一次只处理一个控件或一组边界非常明确的清理动作。
- 当前控件未完成验收前，不启动下一个控件。
- 文档、README、workflow 和 web 文案必须保持 UTF-8。
- 任何“看起来像 HMI / showcase / 场景演示页”的实现都不是默认保留理由。

## 当前进行中

| 状态 | 控件名 | 分类 | 开始日期 | 当前阶段 | 目标 |
| --- | --- | --- | --- | --- | --- |
| 暂无 | - | - | - | - | - |

## 当前保留的 Reference 主线控件

### Input（18）

- `auto_suggest_box` -> `AutoSuggestBox`
- `calendar_view` -> `CalendarView`
- `color_picker` -> `ColorPicker`
- `command_bar` -> `CommandBar`
- `date_picker` -> `DatePicker`
- `drop_down_button` -> `DropDownButton`
- `number_box` -> `NumberBox`
- `password_box` -> `PasswordBox`
- `rating_control` -> `RatingControl`
- `scroll_bar` -> `ScrollBar`
- `segmented_control` -> `SegmentedControl`
- `shortcut_recorder` -> `KeyboardAcceleratorRecorder`
- `split_button` -> `SplitButton`
- `swipe_control` -> `SwipeControl`
- `time_picker` -> `TimePicker`
- `toggle_button` -> `ToggleButton`
- `toggle_split_button` -> `ToggleSplitButton`
- `token_input` -> `TokenInput`

### Layout（6）

- `data_list_panel` -> `ListView`
- `expander` -> `Expander`
- `master_detail` -> `MasterDetail`
- `parallax_view` -> `ParallaxView`
- `settings_panel` -> `SettingCardGroup`
- `split_view` -> `SplitView`

### Navigation（10）

- `annotated_scroll_bar` -> `AnnotatedScrollBar`
- `breadcrumb_bar` -> `BreadcrumbBar`
- `flip_view` -> `FlipView`
- `menu_bar` -> `MenuBar`
- `menu_flyout` -> `MenuFlyout`
- `nav_panel` -> `NavigationView`
- `pips_pager` -> `PipsPager`
- `tab_strip` -> `TabStrip`
- `tab_view` -> `TabView`
- `tree_view` -> `TreeView`

### Display（3）

- `badge_group` -> `BadgeGroup`
- `card_panel` -> `Card`
- `persona_group` -> `AvatarGroup`

### Feedback（5）

- `dialog_sheet` -> `ContentDialog`
- `message_bar` -> `MessageBar`
- `skeleton` -> `Skeleton`
- `teaching_tip` -> `TeachingTip`
- `toast_stack` -> `Toast`

## 最近完成的收口动作

- `2026-04-09`
  - 完成 `navigation/menu_bar` 实现级样式收口：删除 guide / 状态栏 / section label / 预览标签等页面级 chrome，主页面收敛为标题、主 `menu_bar` 与 compact / read-only 双预览。
  - 压缩 `menu_bar` 示例页根布局与底部留白，统一主态、compact 和 read-only palette 到 Fluent / WPF UI 中性浅色菜单栏语法。
  - README 重写为当前 reference 结构，删除历史迭代残留的冗长视觉规则堆积。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/tab_view` 实现级样式收口：删除 guide / 状态栏 / section label / 预览标签等页面级 chrome，示例页压缩为标题、主 `tab_view` 与 compact / read-only 双预览。
  - 收紧 `tab_view` 控件语义：移除 tabs 上方额外的 workspace/helper 说明条，压缩根布局和主卡留白，统一 palette、tab shell、close/add 按钮与内容面板到 Fluent / WPF UI 低噪音工作区风格。
  - README 同步到 `224 x 224` 根布局与 `198 x 112` 主控件尺寸，compact snapshot 切换改为录制态程序化触发。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/menu_flyout` 实现级样式收口：删除 guide / 状态文案 / section label / 预览标签等页面级 chrome，示例页收敛为标题、主 flyout 与 compact / disabled 双预览。
  - 收紧 `menu_flyout` 视觉表达：压缩根布局与主卡留白，统一 palette、分隔线、边框与弱阴影语法到 Fluent / WPF UI 中性轻量弹出菜单风格。
  - 同步修正文档尺寸与结构说明，compact snapshot 文案缩短以避免标题挤压。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/flip_view` 实现级样式收口：示例页删除 guide / 状态栏 / section label 等页面级 chrome，只保留标题、标准主卡和 compact / read-only 双预览。
  - 收紧 `flip_view` 视觉表达：改回浅底、轻边框、弱阴影、低饱和 accent 的 Fluent / WPF UI 单卡轮播语义，overlay 按钮回归中性 shell。
  - 录制动作切换为程序化切轨，不再依赖点击隐藏标签；README 与尺寸说明同步到 `224 x 228` / `196 x 122` / `216 x 64`。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots`、`check_docs_encoding.py`。
- `2026-04-08`
  - 完成 `layout/parallax_view` 实现级样式收口：移除示例页冗余 chrome / 状态文案，保留标题 + 主卡 + 双预览结构。
  - 收紧 `parallax_view` 视觉层次：降低外层色偏与装饰噪音，保留 hero shift / active row / compact / read-only 核心语义。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category layout`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots`。
- `2026-04-08`
  - 完成全部 `deprecated` 目录清退。
  - 完成全部 `showcase` 目录清退。
  - `widget_catalog.json` 收口为 `reference=42 / showcase=0 / deprecated=0`。
  - web 入口、构建脚本、默认 `APP_SUB` 和 catalog policy 已全部对齐到 `reference-only`。
- `2026-04-08`
  - 修复 `HelloCustomWidgets_plan.md`、web 入口文案与脚本帮助文本的历史残留描述。
  - 当前默认维护入口统一为 `input/auto_suggest_box`。

## 已搁置 / 待恢复

| 控件名 | 分类 | 日期 | 原因 | 恢复前提 |
| --- | --- | --- | --- | --- |
| 暂无 | - | - | - | - |

## 下一个控件选择前检查清单

1. “当前进行中” 是否为空。
2. 拟处理控件是否仍在 `reference` 主线清单内。
3. 目标是否明确指向 Fluent / WPF UI 语义收口，而不是继续保留场景化装饰。
4. README、实现、测试与 runtime 验证路径是否都能闭环。
5. 如果控件价值不足，是否已经优先考虑“删除而不是继续修补”。
6. 是否已经准备好在完成后更新 tracker、跑校验并单独提交。
