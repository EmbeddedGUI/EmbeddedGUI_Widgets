# Widget Progress Tracker

## 用途说明

- 本文件用于跟踪 `HelloCustomWidgets` 当前仍保留的 `reference` 主线控件。
- 每次继续工作前，先读 `.claude/workflow/widget_acceptance_workflow.md` 和本文件。
- 当前仓库已经清退全部 `showcase` / `deprecated` 目录，只保留 `reference` 主线。
- 如果某个控件无法继续证明其 `Fluent 2 / WPF UI` 主线价值，应在这里明确记录删除结论，而不是静默消失。

## 当前快照

- 截至 `2026-04-09`，`example/HelloCustomWidgets/` 当前保留 `42` 个控件目录。
- 所有保留控件均来自 `reference` 主线：
  - `input = 18`
  - `layout = 6`
  - `navigation = 10`
  - `display = 3`
  - `feedback = 5`
- `widget_catalog.json`、`web/catalog-policy.json` 与默认 web 入口已同步到 `reference-only` 状态。
- 已清退轨道：
  - 全部 `deprecated`
  - 全部 `showcase`
  - 非主线历史分类 `chart/*`、`media/*` 及其它不再保留的场景化控件

## 当前固定规则

- 第一优先级：统一到 `Fluent 2 / WPF UI` 主线。
- 一次只处理一个控件，完成验收和提交后再切下一个。
- 未完成当前控件前，不启动下一个控件。
- README、workflow、web 文档必须保持 UTF-8，不允许乱码留仓。
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
  - 完成 `layout/master_detail` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `master_detail` 与 `compact / read only` 双预览。
  - 统一 `master_detail` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与页面状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc`、`check_touch_release_semantics.py --scope custom --category layout`、`code_runtime_check.py --app HelloCustomWidgets --app-sub layout/master_detail --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `layout/data_list_panel` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `data_list_panel` 与 `compact / read only` 双预览。
  - 统一 `data_list_panel` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc`、`check_touch_release_semantics.py --scope custom --category layout`、`code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `layout/settings_panel` 实现级样式收口：删除 guide / 状态文案 / section label / preview label 等页面级 chrome，页面结构收敛为标题、主 `settings_panel` 与 `compact / read only` 双预览。
  - 统一 `settings_panel` palette、底部双预览尺寸与录制节奏；底部预览改为禁用交互的静态对照，不再承担点击轮换职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc`、`check_touch_release_semantics.py --scope custom --category layout`、`code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `layout/split_view` 实现级样式收口：删除 guide / 状态文案 / standard label / section label / preview label 等页面级 chrome，示例页收敛为标题、主 `split_view` 与 `compact / read only` 双预览。
  - 统一 `split_view` 主卡与双预览尺寸、palette 和录制动作；底部预览改为静态对照，不再承担交互职责。
- `2026-04-09`
  - 完成 `display/persona_group` 实现级样式收口，页面结构统一到标题 + 主控件 + 双预览。
- `2026-04-09`
  - 完成 `display/card_panel` 实现级样式收口，移除旧版状态桥接和页面壳。
- `2026-04-09`
  - 完成 `display/badge_group` 实现级样式收口，统一到 Fluent / WPF UI 低噪音浅色 reference。
- `2026-04-09`
  - 完成 `feedback/teaching_tip` 实现级样式收口。
- `2026-04-09`
  - 完成 `feedback/dialog_sheet` 实现级样式收口。
- `2026-04-09`
  - 完成 `feedback/skeleton` 实现级样式收口。
- `2026-04-09`
  - 完成 `feedback/message_bar` 实现级样式收口。
- `2026-04-09`
  - 完成 `feedback/toast_stack` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/annotated_scroll_bar` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/tree_view` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/pips_pager` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/breadcrumb_bar` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/tab_strip` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/nav_panel` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/menu_bar` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/tab_view` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/menu_flyout` 实现级样式收口。
- `2026-04-09`
  - 完成 `navigation/flip_view` 实现级样式收口。
- `2026-04-08`
  - 完成 `layout/parallax_view` 实现级样式收口。
- `2026-04-08`
  - 完成全部 `deprecated` 目录清退。
  - 完成全部 `showcase` 目录清退。
  - `widget_catalog.json` 收口到 `reference=42 / showcase=0 / deprecated=0`。
  - web 入口、构建脚本、默认 `APP_SUB` 与 catalog policy 全部对齐到 `reference-only`。
- `2026-04-08`
  - 修复 `HelloCustomWidgets_plan.md`、web 入口文档与脚本文案中的历史残留描述。
  - 当前默认维护入口统一为 `input/auto_suggest_box`。

## 已搁置 / 待恢复

| 控件名 | 分类 | 日期 | 原因 | 恢复前提 |
| --- | --- | --- | --- | --- |
| 暂无 | - | - | - | - |

## 下一控件选择前检查清单

1. “当前进行中”是否为空。
2. 拟处理控件是否仍在 `reference` 主线清单内。
3. 目标是否明确指向 Fluent / WPF UI 语义收口，而不是继续保留场景化装饰。
4. README、实现、测试与 runtime 验证路径是否都能闭环。
5. 如果控件价值不足，是否已经优先考虑“删除而不是继续修补”。
6. 是否已经准备好在完成后更新 tracker、跑校验并单独提交。
