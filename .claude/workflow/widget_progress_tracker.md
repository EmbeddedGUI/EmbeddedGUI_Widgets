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

- `2026-04-10`
  - 完成 `navigation/annotated_scroll_bar` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `range metrics / value / current part / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保按钮、marker、rail 在 setter、模式切换与禁用 guard 后的渲染稳定。
  - `egui_view_annotated_scroll_bar.c` 新增统一的 `annotated_scroll_bar_clear_pressed_state()`，让 `set_markers()`、`set_content_metrics()`、`set_step_size()`、`set_offset()`、`set_current_part()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_part / pressed_marker / rail_dragging / is_pressed` 清理逻辑；同时把 `compact / read_only / !enable` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交，并修正 `ACTION_UP / ACTION_CANCEL` 的 handled 返回语义。
  - `example/HelloUnitTest/test/test_annotated_scroll_bar.c` 补齐 “setters 清 pressed”“touch cancel 清 pressed”“compact / read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确 `markers / content metrics / step size / offset / current part / compact / read only / view disabled` 共享同一套 pressed 清理语义，模式切换后不能残留 button / marker / rail 的 pressed 渲染。
  - 已通过 `make clean APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc`、`make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc`、`make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并抽查 `runtime_check_output/HelloCustomWidgets_navigation_annotated_scroll_bar/default/frame_0000.png`、`frame_0006.png`、`frame_0010.png` 的输出尺寸与截图完整性，确认主卡和底部 `compact / read only` 预览都没有黑白屏、空白裁切或明显的残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/flip_view` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `items / current index / current part / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保翻页、模式切换与尾页边界切换后的渲染稳定。
  - `egui_view_flip_view.c` 新增统一的 `flip_view_clear_pressed_state()`，让 `set_items()`、`set_current_index()`、`set_current_part()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_part / is_pressed` 清理逻辑；同时把 `compact / read_only / !enable / empty items` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_flip_view.c` 补齐 “setters 清 pressed”“touch cancel 清 pressed”“compact / read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 previous / surface / next 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make clean APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc`、`make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc`、`make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_flip_view/default/frame_0000.png`、`frame_0004.png`、`frame_0010.png` 的输出尺寸与截图完整性，确认主卡、compact/read only 预览都没有黑白屏、空白裁切或明显的残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/pips_pager` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `page metrics / current index / current part / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保分页切换与模式切换后的渲染稳定。
  - `egui_view_pips_pager.c` 新增统一的 `pips_pager_clear_pressed_state()`，让 `set_page_metrics()`、`set_current_index()`、`set_current_part()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_part / pressed_index / is_pressed` 清理逻辑；同时把 `compact / read_only / !enable / empty metrics` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_pips_pager.c` 补齐 “setters 清 pressed”“touch cancel 清 pressed”“compact / read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 previous / next / pip 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_pips_pager/default/frame_0000.png`、`frame_0004.png`、`frame_0009.png` 的输出尺寸与截图完整性，确认主卡、compact/read only 预览都没有黑白屏、空白裁切或明显的残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/nav_panel` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `items / current selection / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保导航切换与模式切换后的渲染稳定。
  - `egui_view_nav_panel.c` 新增统一的 `egui_view_nav_panel_clear_pressed_state()` 返回值语义，让 `set_items()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty items` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_nav_panel.c` 补齐 “set_items clamp 后清 pressed”“same selection 清 pressed”“compact 切换后清理 pressed 且保留 selection”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 row 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_nav_panel/default/frame_0000.png`、`frame_0004.png`、`frame_0010.png` 的输出尺寸与截图完整性，确认主卡、compact/read only 预览都没有黑白屏、空白裁切或明显的残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/tab_strip` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `tabs / current index / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保页签切换与模式切换后的渲染稳定。
  - `egui_view_tab_strip.c` 新增统一的 `egui_view_tab_strip_clear_pressed_state()`，让 `set_tabs()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty tabs` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_tab_strip.c` 补齐 “set_tabs clamp 后清 pressed”“same current 清 pressed”“compact 切换后清理 pressed 且保留 selection”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 tab 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_tab_strip/default/frame_0000.png`、`frame_0004.png`、`frame_0008.png` 的输出尺寸与截图完整性，确认主卡、compact/read only 预览都没有黑白屏或残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/tree_view` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / current selection / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保树选择切换与模式切换后的渲染稳定。
  - `egui_view_tree_view.c` 新增统一的 `egui_view_tree_view_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `snapshot == NULL / read_only / !enable` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_tree_view.c` 补齐 “set_snapshots clamp 后清 pressed”“same snapshot / same selection 清 pressed”“compact 切换后清理 pressed 且保留 selection”“!enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 tree row 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_tree_view/default/frame_0000.png`、`frame_0004.png`、`frame_0010.png` 的输出尺寸与截图完整性，确认主卡、compact/read only 预览都没有黑白屏或残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/tab_view` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / current tab / current part / compact / read only / view disabled` 切换链路里的 pressed 清理、输入抑制与渲染回归，确保页签切换、关闭与恢复后的渲染稳定。
  - `egui_view_tab_view.c` 新增统一的 `egui_view_tab_view_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_current_tab()`、`set_current_part()`、`close_current_tab()`、`restore_tabs()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_tab / pressed_part / is_pressed` 清理逻辑；同时把 `snapshot == NULL / read_only / !enable` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_tab_view.c` 补齐 “same snapshot 清 pressed”“same tab / same part 清 pressed”“close / restore 清 pressed 且回归 action / changed listener”“compact / read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入” 的交互回归；README 同步明确模式切换后不能残留 `tab / close / add` 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并复核 `runtime_check_output/HelloCustomWidgets_navigation_tab_view/default/frame_0000.png`、`frame_0004.png`、`frame_0007.png` 的输出尺寸与截图完整性，确认主卡、close/add、compact/read only 预览都没有黑白屏或残留 pressed 污染。
- `2026-04-10`
  - 完成 `layout/data_list_panel` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `selection / snapshot / compact / read only / view disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_data_list_panel.c` 新增统一的 `egui_view_data_list_panel_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty snapshot` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_data_list_panel.c` 补齐“same snapshot 清 pressed”“same selection 清 pressed”“compact 切换后清理 pressed 且保留 selection”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 row 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并核对 `runtime_check_output/HelloCustomWidgets_layout_data_list_panel/default/frame_0000.png`、`frame_0004.png`、`frame_0008.png` 的帧尺寸与差异区域，确认主卡切换与底部预览都没有空白或残留 pressed 污染。
- `2026-04-10`
  - 完成 `layout/master_detail` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `selection / compact / read only / view disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_master_detail.c` 新增统一的 `egui_view_master_detail_clear_pressed_state()`，让 `set_items()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty items` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_master_detail.c` 补齐“same selection 清 pressed”“compact 切换后清理 pressed 且保留 selection”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 master row 的 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/master_detail --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对 `runtime_check_output/HelloCustomWidgets_layout_master_detail/default/frame_0000.png`、`frame_0004.png`、`frame_0008.png`，确认主卡切换与底部预览都没有残留 pressed 污染。
- `2026-04-10`
  - 完成 `layout/expander` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `current / expanded / compact / read only / view disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_expander.c` 新增统一的 `expander_clear_pressed_state()`，让 `set_items()`、`set_current_index()`、`set_expanded_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty items` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_expander.c` 补齐“same current 清 pressed”“same expanded 清 pressed”“compact 切换后清理 pressed 且保留 toggle”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 header 的 `pressed` 高亮或 disclosure chevron 下压位移。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对 `runtime_check_output/HelloCustomWidgets_layout_expander/default/frame_0000.png`、`frame_0004.png`、`frame_0008.png`，确认主卡切换与底部预览都没有残留 pressed 污染。
- `2026-04-10`
  - 完成 `layout/split_view` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `item / pane / compact / read only / view disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_split_view.c` 新增统一的 `sv_clear_pressed_state()`，让 `set_items()`、`set_current_index()`、`set_pane_expanded()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index / pressed_toggle / is_pressed` 清理逻辑；同时把 `read_only / !enable / empty items` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_split_view.c` 补齐“same current index 清 pressed”“same pane / pane 切换清 pressed”“compact 切换后清理 pressed 且保留 selection”“read_only / !enable 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 rail row 或 toggle 的 `pressed` 高亮与下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对 `runtime_check_output/HelloCustomWidgets_layout_split_view/default/frame_0000.png`、`frame_0004.png`、`frame_0008.png`，确认主卡切换与底部预览都没有残留 pressed 污染。
- `2026-04-10`
  - 完成 `navigation/menu_flyout` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / disabled / view disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_menu_flyout.c` 新增统一的 `egui_view_menu_flyout_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_disabled_mode()` 共用同一套 pressed 清理逻辑；同时新增自定义 touch / key guard，把 `disabled_mode / !enable` 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_menu_flyout.c` 补齐“same snapshot 清 pressed”“compact 切换后清理 pressed 且保留 click”“disabled_mode / !enable 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `navigation/breadcrumb_bar` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_breadcrumb_bar.c` 新增统一的 `egui_view_breadcrumb_bar_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 pressed 清理逻辑；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_breadcrumb_bar.c` 补齐“same snapshot 清 pressed”“compact 切换后清理 pressed 且保留 click”“read only / disabled 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `layout/settings_panel` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_settings_panel.c` 新增统一的 `egui_view_settings_panel_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 pressed 清理逻辑；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_settings_panel.c` 补齐“same snapshot 清 pressed”“compact 切换后清理 pressed 且保留 click”“read only / disabled 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `input/drop_down_button` 二次收口：在既有 `reference` 页面结构不再调整的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制，确保交互后的渲染稳定。
  - `egui_view_drop_down_button.c` 新增统一的 `egui_view_drop_down_button_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 pressed 清理逻辑；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，收到新输入时会先清理残留 pressed 再拒绝提交。
  - `example/HelloUnitTest/test/test_drop_down_button.c` 补齐“same snapshot 清 pressed”“compact 切换后清理 pressed 且保留 click”“read only / disabled 清理残留 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确模式切换后不能残留 `pressed` 高亮或下压位移渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category input`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `feedback/teaching_tip` 二次收口：在既有 `reference` 页面结构保持不变的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制。
  - `egui_view_teaching_tip.c` 新增统一的 `egui_view_teaching_tip_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_current_part()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_part + is_pressed` 清理逻辑；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，确保收到新输入时先清理旧高亮，再拒绝提交。
  - `example/HelloUnitTest/test/test_teaching_tip.c` 新增“same snapshot / same part 清 pressed”“compact 切换后恢复点击”“read only / disabled 清理残留 pressed 并忽略后续输入”的交互回归；README 同步明确模式切换后不能残留 target 或 action 的 pressed 高亮。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `display/persona_group` 二次收口：在既有 `reference` 页面结构保持不变的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制。
  - `egui_view_persona_group.c` 新增统一的 `egui_view_persona_group_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 共用同一套 `pressed_index + is_pressed` 清理逻辑；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，确保收到新输入时先清理旧高亮，再拒绝提交。
  - `example/HelloUnitTest/test/test_persona_group.c` 新增“same index / same snapshot 清 pressed”“compact 切换后恢复点击”“read only / disabled 清理残留 pressed 并忽略后续输入”的交互回归；README 同步明确模式切换后不能残留 avatar pressed 高亮。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category display`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `display/badge_group` 二次收口：在既有 `reference` 结构不再调整页面骨架的前提下，把工作重点收回到交互行为，补齐 `snapshot / compact / read only / disabled` 切换链路里的 pressed 清理与输入抑制。
  - `egui_view_badge_group.c` 新增统一的 `egui_view_badge_group_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 都能清掉残留 pressed；同时把 `read only / disabled` 的 touch 与 key guard 收口到事件入口，保证收到新输入时先清理旧高亮，再拒绝提交。
  - `example/HelloUnitTest/test/test_badge_group.c` 新增“same snapshot 清 pressed”“compact 切换后恢复点击”“disabled 清理残留 pressed 并在恢复后再次点击”的交互回归；README 同步明确 `read only` 除了弱化 tone 和抑制输入外，还要求模式切换后不残留 pressed 渲染。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category display`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `display/card_panel` 二次收口：在已落到 `reference` 结构的基础上，补齐卡片在快照切换、`read only`、disabled 和 compact 模式切换下的 pressed 清理语义，并把 `read only / disabled` 的输入抑制真正收口到控件实现里。
  - `egui_view_card_panel.c` 新增统一的 `egui_view_card_panel_clear_pressed_state()`，让 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 都能清掉残留 pressed；同时补上自定义 touch / key guard，让 `read only` 与 disabled 状态在新输入到来时清理渲染并拒绝提交，compact 态则保留卡片 click 语义。
  - `example/HelloUnitTest/test/test_card_panel.c` 新增“切快照清 pressed”“compact 切换后恢复点击”“`read only` 清 pressed 并忽略 `touch / key`”“disabled 清理残留 pressed 并在恢复后再次点击”的交互回归；README 同步明确 `read only` 需要同时满足弱化 tone、隐藏 action pill、输入抑制和 pressed 清理。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category display`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `layout/parallax_view` 二次收口：在已落到 `reference` 结构的基础上，清理 `egui_view_parallax_view.c` 文件尾部残留的旧 `locked_mode` 死代码，并继续把 pressed 清理语义统一到 `compact / read only / disabled` 交互链路里。
  - `egui_view_parallax_view.c` 新增统一的 `parallax_view_clear_pressed_state()`，让 `set_rows()`、`set_compact_mode()`、`set_read_only_mode()` 与 touch 入口都通过同一 helper 清空 `pressed_row / is_pressed`；`set_rows()` 同步重算 offset、触发 `on_changed` 并刷新渲染，避免交互切换后残留旧高亮。
  - `example/HelloUnitTest/test/test_parallax_view.c` 新增“异目标 release 不提交”“`ACTION_CANCEL` 清理 pressed”“切入 `compact` 清 pressed 并忽略输入”“disabled 新输入清理残留 pressed”与独立的 `read only` 输入抑制回归，确保交互后的渲染状态稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧归档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `input/number_box` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 field、border、文字和 accent 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和主态 / `compact` / `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_number_box.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `pressed_part / is_pressed`；同时补上只读态和 disabled 态对 touch / key 的输入抑制。
  - `example/HelloUnitTest/test/test_number_box.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“异目标 release 不提交”“切到只读时清空 pressed 并忽略后续 touch / key 输入”和 disabled 抑制回归；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和渲染稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/number_box PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category input`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/number_box --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `feedback/skeleton` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 block、border、accent 和 footer 文案的对比度。
  - `test.c` 把 `apply_read_only_snapshot()` 收口为 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和主态 / `compact` / `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_skeleton.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `is_pressed`；同时补上只读态和 disabled 态对默认 touch / key click 链路的输入抑制，并统一只读态 timer 的停止 / 恢复语义。
  - `example/HelloUnitTest/test/test_skeleton.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed、停掉 timer 并忽略后续 touch / key 输入”的交互回归，拆出 disabled 抑制测试，并补充 `is_pressed` 清理断言；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和渲染稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `feedback/toast_stack` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 severity strip、glyph、后卡、action pill、meta pill 和底部分隔线的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距、只读预览文案和主态 / `compact` / `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_toast_stack.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `is_pressed`；同时补上只读态和 disabled 态对默认 touch / key click 链路的输入抑制。
  - `example/HelloUnitTest/test/test_toast_stack.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归，拆出 disabled 抑制测试，并补充 `is_pressed` 清理断言；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和渲染稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧补档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `feedback/message_bar` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 severity strip、glyph circle、pin chip 和底部分隔线的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距、只读预览文案和主态 / `compact` / `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_message_bar.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `is_pressed`；同时补上只读态和 disabled 态对默认 touch / key click 链路的输入抑制。
  - `example/HelloUnitTest/test/test_message_bar.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归，拆出 disabled 抑制测试，并补充 `is_pressed` 清理断言；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和渲染稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧补档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `feedback/dialog_sheet` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 hero glyph、footer summary、tag 和 action button 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和主态 / `compact` / `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_dialog_sheet.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_snapshots()`、`set_current_snapshot()`、`set_current_action_inner()`、`set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `pressed_action / is_pressed`；同时补上只读态对 touch / key 的输入抑制，并收紧 `ACTION_UP` 的提交语义。
  - `example/HelloUnitTest/test/test_dialog_sheet.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归，拆出 disabled 抑制测试，并补充 `is_pressed` 清理断言；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和渲染稳定。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并把关键帧补档到本地 `iteration_log/` 供验收复核。
- `2026-04-10`
  - 完成 `navigation/menu_bar` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 summary cap、cue、meta chip、anchor、当前 menu fill 和 underline 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和 `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_menu_bar.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增统一的 pressed 清理辅助函数，并在 `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_compact_mode()` 和 `set_read_only_mode()` 中同步清理 `pressed_item / pressed_menu / is_pressed`；同时补上只读态对 touch / key 的输入抑制。
  - `example/HelloUnitTest/test/test_menu_bar.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归，并把 disabled 抑制单测拆分独立；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和静态展示语义。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `navigation/nav_panel` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 selected row、indicator、badge、footer 分隔和 compact rail chrome 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和 `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_nav_panel.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，并在 `set_items()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 中统一清理 pressed / `pressed_index`；同时补上 `Up / Down / Home / End` 键盘导航，以及只读态对 touch / key 的输入抑制。
  - `example/HelloUnitTest/test/test_nav_panel.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增键盘导航测试和“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和静态展示语义。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `navigation/breadcrumb_bar` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻只读态 fill、border、text、accent、current pill、separator chevron 和 underline 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和 `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_breadcrumb_bar.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，并在 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 中统一清理 pressed；同时补上只读态对 touch / key 的输入抑制，确保交互切换后不会残留按下渲染。
  - `example/HelloUnitTest/test/test_breadcrumb_bar.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确 `read only` 需要同时满足 pressed 清理、输入抑制和静态展示语义。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `navigation/tab_strip` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻容器边框、active fill、divider、underline 和只读态 chrome 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和 `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_tab_strip.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，新增 `INDEX_NONE` 常量，并在 `set_tabs()`、`set_current_index()`、`set_compact_mode()` 和 `set_read_only_mode()` 中统一清理 pressed 状态；同时补上 `Left / Right / Home / End` 键盘导航，以及只读态对 touch / key 的输入抑制。
  - `example/HelloUnitTest/test/test_tab_strip.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增键盘导航测试和“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确 `read only` 需要同时满足视觉弱化和输入抑制。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `navigation/tree_view` 二次收口：在原有 `reference` 页面结构基础上，把控件内部的旧 `locked_mode` 语义统一为 `read_only_mode`，并继续压轻 tree card、list border、selected row、indicator、guide、glyph、caption、footer 和 meta pill 的对比度。
  - `test.c` 新增 `apply_read_only_state()`，让 `read only` 预览在初始化和录制 case `0` 重置时都显式回到只读态；同步微调标题间距和 `read only` palette，继续保持底部两个 preview 只承担 reference 对照职责。
  - `egui_view_tree_view.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，在 `set_snapshots()`、`set_current_snapshot()`、`set_compact_mode()` 和 `set_read_only_mode()` 中统一清理 pressed 状态，并补齐只读态的 touch / key 输入抑制。
  - `example/HelloUnitTest/test/test_tree_view.c` 把旧 `locked_mode` 单测改成 `read_only_mode`，新增“切到只读时清空 pressed 并忽略后续 touch / key 输入”的交互回归；README 同步明确 `read only` 需要同时满足视觉弱化和输入抑制。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `layout/parallax_view` 实现级样式收口：页面统一为标题、主 `parallax_view` 与 `compact / read only` 静态对照，保留 `ParallaxView` 的 hero layer depth、active row 和 footer summary 语义，同时继续压轻 hero strips、row fill/border、meta chip、progress pill 和 footer chrome。
  - `test.c` 删除旧双列包裹壳，把 `parallax_locked / locked_rows` 收口为 `parallax_read_only / read_only_rows`，补上 `apply_read_only_state()` 与 `consume_preview_touch()`，并在录制 case `0` 显式重置主控件、`compact` 和 `read only` 对照；底部两个 preview 统一禁用 `touch / focus`，只承担 reference 对照职责。
  - `egui_view_parallax_view.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，补齐只读态的 touch / key 输入抑制，并在切到只读时清掉 pressed；同步压轻默认浅灰蓝 palette、hero layers、row fill/border、meta chip、progress pill 和 footer summary 的对比度。
  - `example/HelloUnitTest/test/test_parallax_view.c` 同步改用 `set_read_only_mode()`，新增只读态清空 pressed 并忽略 touch / key 输入的交互单测；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-10`
  - 完成 `layout/settings_panel` 实现级样式收口：页面统一为标题、主 `settings_panel` 与 `compact / read only` 静态对照，保留 `SettingCard / SettingsGroup` 的分组卡片、focus row 和尾部 `value / switch / chevron` 语义，同时继续压轻 section row、value badge 和 footer meta chrome。
  - `test.c` 删除底部旧双列包裹壳，把 `panel_locked / locked_snapshots` 收口为 `panel_read_only / read_only_snapshots`，补上 `apply_read_only_state()`，并在录制 case `0` 显式重置主卡、`compact` 和 `read only` 对照；底部两个 preview 统一禁用 `touch / focus`，只承担 reference 对照职责。
  - `egui_view_settings_panel.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，补齐只读态的 touch / key 输入抑制，并在切到只读时清掉 pressed；同步压轻默认浅灰蓝 palette、section row、switch track、value badge、eyebrow、footer meta 和 focus pill 的对比度。
  - `example/HelloUnitTest/test/test_settings_panel.c` 同步改用 `set_read_only_mode()`，新增只读态清空 pressed 并忽略 touch / key 输入的交互单测；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认切换后的渲染稳定。
- `2026-04-10`
  - 完成 `display/badge_group` 实现级样式收口：页面统一为标题、主 `badge_group` 与 `compact / read only` 静态对照，保留 `BadgeGroup` 的多 badge 组合、focus badge 驱动摘要和 mixed row 语义，同时继续压轻 badge fill、focus badge fill、mixed row 对比和 footer summary chrome。
  - `test.c` 删除底部旧列容器壳，把 `group_locked / locked_snapshots` 收口为 `group_read_only / read_only_snapshots`，补上 `apply_read_only_state()`，并在录制 case `0` 显式重置主卡、`compact` 和 `read only` 对照；底部两个 preview 统一禁用 `touch / focus`，只承担 reference 对照职责。
  - `egui_view_badge_group.h/.c` 把 `locked_mode` API 收口为 `read_only_mode`，补齐只读态的 touch / key 输入抑制，并在切到只读时清掉 pressed；同步压轻默认浅灰蓝 palette、badge meta chip、card border、eyebrow pill、focus pill 和 footer summary 的对比度。
  - `example/HelloUnitTest/test/test_badge_group.c` 同步改用 `set_read_only_mode()`，新增只读态清空 pressed 并忽略 touch / key 输入的交互单测；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc`、`make all APP=HelloUnitTest PORT=pc_test`、`python scripts/checks/check_touch_release_semantics.py --scope custom --category display`、`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots`、`python scripts/checks/check_docs_encoding.py`，并人工核对关键帧确认交互切换后的渲染稳定。
- `2026-04-09`
  - 完成 `display/card_panel` 实现级样式收口：移除底部 preview 的旧列容器壳，页面统一为标题、主 `card_panel` 与 `compact / read only` 静态对照，继续保留 `Card` 的 badge、summary slot、detail strip 和 action pill 语义。
  - `test.c` 将 `panel_locked / locked_snapshots` 收口为 `panel_read_only / read_only_snapshots`，底部两个 preview 统一禁用 touch / focus，并在录制起点显式重置主轨道、`compact` 与 `read only` 对照；`egui_view_card_panel.c` 进一步压轻 header strap、metric chip、detail strip、footer line 和 action chrome，并把 `read_only_mode` 再灰蓝化；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc`、`check_touch_release_semantics.py --scope custom --category display`、`code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `display/persona_group` 实现级样式收口：移除底部 preview 的旧列容器壳，页面统一为标题、主 `persona_group` 与 `compact / read only` 静态对照，继续保留 `AvatarGroup` 的成员重叠、焦点成员、presence 和 overflow 气泡语义。
  - `test.c` 将 `group_readonly / readonly_snapshots` 收口为 `group_read_only / read_only_snapshots`，底部两个 preview 统一禁用 touch / focus，并在录制起点显式重置主轨道、`compact` 与 `read only` 对照；`egui_view_persona_group.c` 进一步压轻 avatar ring、presence badge、overflow bubble、card border 和 footer summary，并把 `read_only_mode` 再灰蓝化；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc`、`check_touch_release_semantics.py --scope custom --category display`、`code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `feedback/dialog_sheet` 实现级样式收口：移除底部 preview 的旧列容器壳，页面统一为标题、主 `dialog_sheet` 与 `compact / read only` 静态对照，继续保留 `ContentDialog` 的 `warning / error / accent / success` 四态、footer summary、tag 和 primary / secondary action 语义。
  - `test.c` 将 `sheet_locked / locked_snapshots` 收口为 `sheet_read_only / read_only_snapshots`，底部两个 preview 统一禁用 touch / focus，并在录制起点显式重置主轨道、`compact` 与 `read only` 对照；`egui_view_dialog_sheet.c` 进一步压轻 overlay、sheet surface、hero、footer summary、tag 和 action chrome，并把 `locked_mode` 再灰蓝化；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc`、`check_touch_release_semantics.py --scope custom --category feedback`、`code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `feedback/teaching_tip` 实现级样式收口：移除底部 preview 的旧列容器壳与页面桥接逻辑，页面统一为标题、主 `teaching_tip` 与 `compact / read only` 静态对照，继续保留 `TeachingTip` 的 target 锚点、`top / bottom placement`、close / action 和 closed / reopen 语义。
  - `test.c` 将 `tip_locked` 收口为 `tip_read_only`，底部两个 preview 统一禁用 touch / focus，并在录制起点显式重置主轨道和 `read only` 对照；`egui_view_teaching_tip.c` 进一步压轻 callout surface、pointer tail、target halo、action button、close pill 和 closed hint；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc`、`check_touch_release_semantics.py --scope custom --category feedback`、`code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `feedback/skeleton` 实现级样式收口：移除底部 preview 的旧列容器壳与页面桥接逻辑，页面统一为标题、主 `skeleton` 与 `compact / read only` 静态对照，继续保留 `Skeleton` 的 `wave` 主态、`compact` 脉冲占位和 `read only` 静态占位语义。
  - `test.c` 将 `skeleton_locked` 收口为 `skeleton_read_only`，底部两个 preview 统一禁用 touch / focus 并补上程序化 `read only` 重置；`egui_view_skeleton.c` 进一步压轻容器 fill / border、placeholder emphasis、wave band、pulse 对比和 read-only chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc`、`check_touch_release_semantics.py --scope custom --category feedback`、`code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `feedback/toast_stack` 实现级样式收口：移除底部 preview 的旧列容器壳与页面桥接逻辑，页面统一为标题、主 `toast_stack` 与 `compact / read only` 静态对照，继续保留 `Toast / Snackbar` 的前卡、后两层叠卡、正文、action 和 meta 层级语义。
  - `test.c` 将 `stack_locked` 收口为 `stack_read_only`，底部两个 preview 统一禁用 touch / focus 并补上程序化 `read only` 重置；`egui_view_toast_stack.c` 进一步压轻 back card、front card、severity strip、action pill、meta pill 和 read-only chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc`、`check_touch_release_semantics.py --scope custom --category feedback`、`code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `feedback/message_bar` 实现级样式收口：移除底部 preview 的旧列容器壳与页面桥接逻辑，页面统一为标题、主 `message_bar` 与 `compact / read only` 静态对照，继续保留 `MessageBar / InfoBar` 的四类 severity、正文与 action 层级语义。
  - `test.c` 将 `bar_locked` 收口为 `bar_read_only`，底部两个 preview 统一禁用 touch / focus 并补上程序化 `read only` 重置；`egui_view_message_bar.c` 进一步压轻容器 fill / border、severity strip、glyph circle、action button 和 pin chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category feedback`、`code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。

- `2026-04-09`
  - 完成 `navigation/nav_panel` 实现级样式收口：移除底部 preview 的旧列容器壳与 preview 交互桥接逻辑，页面统一为标题、主 `nav_panel` 与 `compact / read only` 静态对照，继续保留 `NavigationView` 的 selected row、compact rail 和 footer 语义。
  - `test.c` 将 `panel_locked` 收口为 `panel_read_only`，底部两个 preview 统一禁用 touch / focus 并改为程序化切换；`egui_view_nav_panel.c` 进一步压轻 selected row、indicator、badge 和 footer chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/tab_strip` 实现级样式收口：移除底部 preview 的旧列容器壳与 preview 交互桥接逻辑，页面统一为标题、主 `tab_strip` 与 `compact / read only` 静态对照，继续保留 variable-width tab、current tab 和 compact 语义。
  - `test.c` 将 `bar_locked` 收口为 `bar_read_only`，底部两个 preview 统一禁用 touch / focus 并改为程序化切换；`egui_view_tab_strip.c` 进一步压轻容器边框、active fill、divider、underline 和 locked chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/breadcrumb_bar` 实现级样式收口：移除底部 preview 的旧列容器壳与页面级点击轮换逻辑，页面统一为标题、主 `breadcrumb_bar` 与 `compact / read only` 静态对照，继续保留 `BreadcrumbBar` 的层级路径、省略折叠和 current item 语义。
  - `test.c` 将 `bar_locked` 收口为 `bar_read_only`，底部两个 preview 统一禁用 touch / focus 并改为程序化切换；`egui_view_breadcrumb_bar.c` 进一步压轻容器边框、current item pill、separator chevron 和底部 underline；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/pips_pager` 实现级样式收口：移除底部 preview 的旧双列容器壳，页面统一为标题、主 `pips_pager` 与 `compact / read only` 静态对照，继续保留 `PipsPager` 的 previous / next、当前页和离散 pips 语义。
  - `test.c` 将 `pager_locked` 收口为 `pager_read_only`，底部两个 preview 统一禁用 touch / focus，并为 read-only 预览补上更弱的灰蓝 palette；`egui_view_pips_pager.c` 进一步压轻 previous/next 按钮、inactive dots、current pill、focus ring 和 read-only chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/annotated_scroll_bar` 实现级样式收口：移除底部 preview 的旧双列容器壳，页面统一为标题、主 `annotated_scroll_bar` 与 `compact / read only` 静态对照，继续保留 `AnnotatedScrollBar` 的键盘步进、marker jump、rail drag 和只读对照语义。
  - `test.c` 将 `annotated_scroll_bar_locked` 收口为 `annotated_scroll_bar_read_only`，底部两个 preview 统一禁用 touch / focus，并为 read-only 预览补上更弱的灰蓝 palette；`egui_view_annotated_scroll_bar.c` 进一步压轻 summary、count pill、bubble、connector、rail、indicator、button、focus ring 和 read-only chrome；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/tree_view` 实现级样式收口：移除底部 preview 的双列包裹结构，把页面统一为标题、主控件与 `compact / read only` 静态对照，继续保留 `TreeView` 的层级缩进、展开分支和选中语义。
  - `test.c` 把 `tree_locked` 收口为 `tree_read_only`，两个 preview 统一禁用 touch / focus；`egui_view_tree_view.c` 进一步压轻 tree card、list border、selected row、indicator、guide、glyph、caption、footer 和 meta pill；README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/menu_flyout` 实现级样式收口：继续保留 `MenuFlyout / ContextMenu` 主线语义，删掉底部可点击预览卡职责，把页面统一收口为标题、主控件和 `compact / disabled` 双预览。
  - 主面板命令内容统一改成更中性的业务摘要命令，`egui_view_menu_flyout.c` 进一步压轻圆角、阴影、边框、separator 和行内高亮强度，README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_flyout PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_flyout --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/flip_view` 实现级样式收口：继续保留 `FlipView` 主线语义，删掉旧示例里的故事化文案和 read-only 锁定叙事，把页面统一收口为标题、主控件和 `compact / read only` 双预览。
  - 主控件与预览轨道统一改成低噪音业务摘要内容，`egui_view_flip_view.c` 进一步压轻圆角、阴影、边框和按钮强调度，README 重写为当前 `reference` 模板。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/calendar_view` 实现级样式收口：删除页面级 guide、状态文案、`Standard` 标签、分隔线、预览标签与 label-click 场景切换逻辑，把页面结构统一为标题、主 `calendar_view` 与 `compact / read only` 双预览。
  - 主控件保留真实 `CalendarView` 区间选择、月份浏览与焦点闭环，页面空白区和底部预览统一改为清焦收口；`compact` 与 `read only` 预览收敛为静态 reference 对照，README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/calendar_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_view --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
  - 完成 `input/rating_control` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `rating_control` 与 `compact / read only` 双预览。
  - 主控件保留真实触摸评分、`Clear` 和键盘闭环，底部两个预览收敛为静态 reference 对照；README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/rating_control --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/swipe_control` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `swipe_control` 与 `compact / read only` 双预览。
  - 主控件保留真实 reveal 键盘闭环与底层 touch 语义，底部两个预览收敛为静态 reference 对照；README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/swipe_control --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/scroll_bar` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `scroll_bar` 与 `compact / read only` 双预览。
  - 主控件保留真实 `ScrollBar` 键盘闭环与底层触摸语义，底部两个预览收敛为静态 reference 对照；README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/scroll_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/command_bar` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和标签点击切换逻辑，把页面结构统一为标题、主 `command_bar` 与 `compact / disabled` 双预览。
  - 主控件保留真实命令焦点切换与键盘闭环，底部两个预览收敛为静态 reference 对照；README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/drop_down_button` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `drop_down_button` 与 `compact / read only` 双预览。
  - 主控件保留真实触摸点击与 `Enter` 键切换闭环，底部两个预览收敛为静态 reference 对照；README 已重写为当前 `reference` 结构。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/shortcut_recorder` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `shortcut_recorder` 与 `compact / read only` 双预览。
  - 主控件保留字段监听、组合键捕获、preset 应用和 clear 的真实触摸 / 键盘闭环，底部两个预览收敛为静态 reference 对照，不再承担额外交互职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/shortcut_recorder --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/segmented_control` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `segmented_control` 与 `compact / read only` 双预览。
  - 主控件保留真实触摸切换、`Left / Right / Up / Down / Home / End / Tab` 键盘闭环与焦点 ring，底部两个预览收敛为静态 reference 对照，不再承担额外交互职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/segmented_control PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/segmented_control --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/toggle_split_button` 实现级样式收口：删除 guide、状态回显、section divider、外部 preview 标签和 label-click 场景切换逻辑，把页面结构统一为标题、主 `toggle_split_button` 与 `compact / read only` 双预览。
  - 主控件保留 checked/unchecked、主段/菜单段切换与键盘录制闭环，底部两个预览收敛为静态 reference 对照，不再承担额外交互职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/toggle_split_button PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_split_button --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/split_button` 实现级样式收口：删除 guide、状态回显、section divider 和外部 preview 标签，把页面结构统一为标题、主 `split_button` 与 `compact / disabled` 双预览。
  - 主控件保留主段 / 菜单段键盘切换与 reference snapshot，底部两个预览收敛为静态对照，不再承担场景切换或标签点击职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/split_button PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/split_button --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/toggle_button` 实现级样式收口：删除 guide、状态回显、section divider 和外部 preview 标签，把页面结构统一为标题、主 `toggle_button` 与 `compact / read only` 双预览。
  - 主按钮保留 `Enter / Space` 键盘闭环与 on/off reference snapshot，底部两个预览收敛为静态对照，不再承担场景切换或标签点击职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/toggle_button PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_button --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/time_picker` 实现级样式收口：移除 guide / 状态文案 / standard label / section divider / preview label 等旧页面 chrome，页面结构统一为标题、主 `time_picker` 与 `compact / read only` 双预览。
  - 统一 `time_picker` reference palette、主控件展开/收起尺寸、底部双预览与录制动作；保留标准时间字段、`12h / 24h` 对照与只读对照，同时把页面空白区和底部预览收敛为最小 dismiss 逻辑，不再承担标签切换职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/time_picker PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/time_picker --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/date_picker` 实现级样式收口：移除 guide / 状态文案 / standard label / section divider / preview label 等旧页面 chrome，页面结构统一为标题、主 `date_picker` 与 `compact / read only` 双预览。
  - 统一 `date_picker` reference palette、主控件展开/收起尺寸、底部双预览与录制动作；保留标准日期字段、月份浏览、跨年浏览与只读对照，同时把底部预览收敛为静态对照，只承担失焦收口。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/date_picker PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/date_picker --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/token_input` 实现级样式收口：移除 guide / 状态文案 / standard label / section divider / preview label，页面结构统一为标题、主 `token_input` 与 `compact / read only` 双预览。
  - 统一 `token_input` reference palette、主控件尺寸、底部双预览和录制动作；保留真实 token 输入、提交与 remove 交互，同时把底部预览改为静态对照，不再承担标签切换职责。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/token_input PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/token_input --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/auto_suggest_box` 实现级样式收口：移除 guide / 状态文案 / standard label / section divider / preview label，页面结构统一为标题、主 `auto_suggest_box` 与 `compact / read only` 双预览。
  - 统一 `auto_suggest_box` palette、主控件尺寸与底部双预览布局；录制动作改为程序化 `expand/collapse` 与键盘提交，并在展开帧中重排底部预览，避免与 suggestions 列表重叠。
  - README 重写为当前 `reference` 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/color_picker` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `color_picker` 与 `compact / read only` 双预览。
  - 统一 `color_picker` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与页面状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/color_picker PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/color_picker --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/password_box` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `password_box` 与 `compact / read only` 双预览。
  - 统一 `password_box` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与页面状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/password_box PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/password_box --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `input/number_box` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `number_box` 与 `compact / read only` 双预览。
  - 统一 `number_box` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与页面状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=input/number_box PORT=pc`、`check_touch_release_semantics.py --scope custom --category input`、`code_runtime_check.py --app HelloCustomWidgets --app-sub input/number_box --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `layout/expander` 实现级样式收口：删除 guide / 状态文案 / standard label / section divider / preview label 等页面级 chrome，页面结构收敛为标题、主 `expander` 与 `compact / read only` 双预览。
  - 统一 `expander` 主卡与双预览尺寸、palette 和录制动作；底部预览改为禁用交互的静态对照，不再承担标签点击与页面状态桥接职责。
  - README 重写为当前 reference 结构；已通过 `make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc`、`check_touch_release_semantics.py --scope custom --category layout`、`code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
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
  - 完成 `navigation/menu_bar` 实现级样式收口：移除底部 preview 的点击与焦点循环职责，把页面统一为标题、主控件与 `compact / read only` 静态对照，继续压轻 `egui_view_menu_bar.c` 的顶栏当前态、focus ring、panel shadow、separator、row highlight 与 summary strip。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
- `2026-04-09`
  - 完成 `navigation/tab_view` 实现级样式收口：移除底部 preview 的双列包裹结构，把页面统一为标题、主控件与 `compact / read only` 静态对照，继续压轻 `egui_view_tab_view.c` 的 tab shell、active tab、close/add、body panel、badge 与 footer pill。
  - 已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc`、`check_touch_release_semantics.py --scope custom --category navigation`、`code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots`、`make all APP=HelloUnitTest PORT=pc_test`、`output\main.exe`、`check_docs_encoding.py`。
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
