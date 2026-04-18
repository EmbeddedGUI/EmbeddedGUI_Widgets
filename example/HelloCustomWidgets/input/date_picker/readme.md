# date_picker 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI DatePicker`
- 开源母本：`WPF UI`
- 对应组件：`DatePicker`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留真实日期字段、月份浏览与单日提交的 `touch` 交互，以及 `Left / Right / Up / Down / Home / End / Enter / Space / Escape / Plus` 键盘闭环；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 轮换、preview 清焦桥接、额外收尾态

## 1. 为什么需要这个控件
`date_picker` 用来表达“先查看当前日期，再按需展开月份面板选择某一天”的标准日期输入语义，适合交付日期、预约日期、截止日期、行程日期等表单场景。它同时承担日期字段、月份浏览和单日提交，不需要用户手工输入日期字符串。

## 2. 为什么现有控件不够用
- `calendar_view` 更偏向整月浏览和范围选择，不承担单个字段输入语义。
- `text_box` / `textinput` 需要手工输入日期文本，不适合低噪音的点选式日期提交。
- `combo_box` / `auto_suggest_box` 适合候选项列表，不适合月份网格浏览与单日选取。
- 仓库里当前 `input/date_picker` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Date Picker`
- 主区：一个保留真实日期字段、月份浏览和单日提交闭环的 `date_picker`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Mar 18`
- 右侧 preview：`read only`，固定显示 `Apr 05`

目录：
- `example/HelloCustomWidgets/input/date_picker/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认展开态
   `2026-03-18` / `Mar 2026`
2. 浏览态
   `2026-03-18` / `Apr 2026`
3. 提交态
   `2026-04-02` / `Apr 2026`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Mar 18`
2. `read only`
   `Apr 05`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 264`
- 主控件：`196 x 180`（展开）/ `196 x 82`（收起）
- 底部 preview 行：`216 x 48`
- 单个 preview：`104 x 48`
- 页面结构：标题 -> 主 `date_picker` -> 底部 `compact / read only`
- 风格约束：保持浅色 page panel、白色 surface、低噪音边框和标准日期字段层级；主区只保留 `field + month header + calendar grid + helper` 的最小完整语义；`compact / read only` 只做静态 reference 对照

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `2026-03-18` / `Mar 2026` | `Mar 18` | `Apr 05` |
| 快照 2 | `2026-03-18` / `Apr 2026` | 保持不变 | 保持不变 |
| 快照 3 | `2026-04-02` / `Apr 2026` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到 `2026-03-18` / `Mar 2026` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
录制轨道已经收口到 static preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到浏览态，面板切到 `Apr 2026`
4. 抓取第二组主区快照
5. 切到提交态，字段改为 `2026-04-02`
6. 抓取第三组主区快照
7. 回到默认 `2026-03-18` / `Mar 2026`
8. 抓取最终稳定帧

说明：
- runtime 录制阶段不再轮换 `compact` preview
- 底部 preview 统一通过 `egui_view_date_picker_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担清焦或额外页面状态桥接职责
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `DATE_PICKER_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_snapshot()`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_page()` 与 `focus_primary_widget()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_date_picker.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 setter clamp、`pressed_part / pressed_day / is_pressed` 清理、月份浏览、日期提交、键盘浏览、`compact / read only / disabled` guard 和 focus change 收口。
2. 静态 preview 不变性断言
   通过 `date_picker_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验 `region_screen`、`on_date_changed`、`on_open_changed`、`on_display_month_changed`、`font`、`meta_font`、`label`、`helper`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`today_color`、`year`、`panel_year`、`today_year`、`month`、`panel_month`、`day`、`today_month`、`today_day`、`first_day_of_week`、`compact_mode`、`read_only_mode`、`open_mode`、`preserve_display_month_on_open`、`pressed_part`、`pressed_day`、`alpha` 不变，并要求 `g_date_changed_count == 0`、`g_open_changed_count == 0`、`g_display_changed_count == 0`，且 `is_pressed == false`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/date_picker PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/date_picker --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/date_picker
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_date_picker
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `2026-03-18 / Mar 2026`、`2026-03-18 / Apr 2026` 与 `2026-04-02 / Apr 2026` 三组 reference 状态必须能从截图中稳定区分。
- 主控件的 setter、月份浏览、日期提交与 focus 收口链路不能残留 `pressed_part / pressed_day / is_pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_date_picker/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(164, 99) - (316, 211)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 212` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `calendar_view`：这里是标准日期输入字段，不是范围浏览控件。
- 相比 `text_box`：这里不做自由文本输入，核心是点选式日期提交。
- 相比 `combo_box` / `auto_suggest_box`：这里不是候选列表，而是月份网格浏览。
- 相比 `time_picker`：这里处理年月日与跨月浏览，不处理时分。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `2026-03-18` / `Mar 2026`
  - `2026-03-18` / `Apr 2026`
  - `2026-04-02` / `Apr 2026`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 轮换
  - preview 清焦桥接
  - 额外收尾态

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/date_picker PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `date_picker` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/date_picker --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_date_picker/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/date_picker`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_date_picker`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2013 colors=146`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(164, 99) - (316, 211)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 212` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `2026-03-18 / Mar 2026`、浏览 `2026-03-18 / Apr 2026` 与提交 `2026-04-02 / Apr 2026` 三组 reference 状态，最终稳定帧回到默认态，底部 `compact / read only` preview 全程静态
