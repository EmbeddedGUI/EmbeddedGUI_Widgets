# calendar_date_picker 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI CalendarDatePicker`
- 开源母本：`WPF UI`
- 对应组件：`CalendarDatePicker`
- 当前保留形态：`2026-03-18 / Mar 2026`、`2026-03-18 / Apr 2026`、`2026-04-02 / Apr 2026`、`compact`、`read only`
- EGUI 适配说明：目录和 demo 使用 `input/calendar_date_picker`，实现文件为 `egui_view_calendar_date_picker.*`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`calendar_date_picker` 表达“一个日期输入按钮，按需展开日历弹层选择单日”的标准表单语义，适合交付日期、预约日期、截止日期、行程日期等低噪声输入场景。它把日期字段、弹层月份浏览、今日标记、已选日期和单日提交收在一个控件内，避免用户手工输入日期字符串。

## 2. 为什么现有控件不够用
- `date_picker` 保留日期字段与日期选择语义，本控件按 `CalendarDatePicker` 命名补齐 WPF UI 主线中的弹层日期选择入口。
- `calendar_view` 偏向整月浏览和范围选择，不承担单个表单字段的打开/关闭状态。
- `text_box` 需要手工输入日期文本，不适合点选式日期提交。
- `combo_box` / `auto_suggest_box` 适合候选项列表，不适合月份网格浏览。

## 3. 当前页面结构
- 标题：`Calendar Date Picker`
- 主区：一个保留真实日期字段、月份浏览和单日提交闭环的 `calendar_date_picker`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Mar 18`
- 右侧 preview：`read only`，固定显示 `Apr 05`

目录：
- `example/HelloCustomWidgets/input/calendar_date_picker/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：
1. 默认展开态：`2026-03-18` / `Mar 2026`
2. 浏览态：`2026-03-18` / `Apr 2026`
3. 提交态：`2026-04-02` / `Apr 2026`

底部 preview 固定为：
1. `compact`：`Mar 18`
2. `read only`：`Apr 05`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 264`
- 主控件：`196 x 180`（展开）/ `196 x 82`（收起）
- 底部 preview 行：`216 x 48`
- 单个 preview：`104 x 48`
- 页面结构：标题 -> 主 `calendar_date_picker` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、白色 surface、低噪声边框、标准日期字段层级；主区只保留 `field + month header + calendar grid + helper` 的最小完整语义

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `2026-03-18` / `Mar 2026` | `Mar 18` | `Apr 05` |
| 快照 2 | `2026-03-18` / `Apr 2026` | 保持不变 | 保持不变 |
| 快照 3 | `2026-04-02` / `Apr 2026` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到 `2026-03-18` / `Mar 2026` | 保持不变 | 保持不变 |
| 静态 preview 吞掉输入且不改状态 | 是 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_calendar_date_picker.c` 覆盖 `8` 条用例：
1. setter 与 listener guard：日期 clamp、today clamp、first day 边界、display month、opened 与 preserve display month。
2. setter 清理按压态：覆盖 `set_date()`、`set_display_month()`、`set_opened()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`。
3. 字体、palette 与内部 helper：日期格式化、月份标题、起始单元格、锚点日期与 disabled tone。
4. metrics 与 hit-testing：字段区、上一月、下一月、日期格命中，以及 compact/read-only 布局分支。
5. touch 开关、浏览与日期提交：字段开合、月份前后浏览、日期 same-target release、跨目标 move 不提交与 cancel 清理。
6. 键盘导航、浏览与提交：`Left / Right / Up / Down / Home / End / Enter / Space / Escape / Plus`。
7. compact、read only、disabled 与 focus guard：输入拒绝、残留按压态清理和 focus 收口。
8. static preview 吞掉输入并保持状态不变。

## 8. 录制动作设计
录制轨道采用 static preview 工作流：
1. 应用默认主区快照和底部 preview 固定状态并抓取首帧，等待 `CALENDAR_DATE_PICKER_RECORD_FRAME_WAIT`。
2. 切到浏览态，面板切到 `Apr 2026`，等待 `CALENDAR_DATE_PICKER_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `CALENDAR_DATE_PICKER_RECORD_FRAME_WAIT`。
4. 切到提交态，字段改为 `2026-04-02`，等待 `CALENDAR_DATE_PICKER_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `CALENDAR_DATE_PICKER_RECORD_FRAME_WAIT`。
6. 回到默认 `2026-03-18 / Mar 2026`，等待 `CALENDAR_DATE_PICKER_RECORD_WAIT`。
7. 抓取最终稳定帧，等待 `CALENDAR_DATE_PICKER_RECORD_FINAL_WAIT`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/calendar_date_picker PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe calendar_date_picker
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_date_picker --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/calendar_date_picker
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_calendar_date_picker
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区三组 reference 状态必须能从截图中稳定区分。
- setter、月份浏览、日期提交和 focus 收口链路不能残留 `pressed_part / pressed_day / is_pressed` 污染。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_calendar_date_picker` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_calendar_date_picker/default`
- 复核内容：帧数、主区状态分组、主区 RGB 差分边界、主区外稳定性、底部 preview 静态一致性。

## 12. 与现有控件的边界
- 相比 `calendar_view`：这里是单日期字段的弹层选择器，不是范围浏览控件。
- 相比 `date_picker`：这里补齐 `CalendarDatePicker` 命名和下拉日历字段入口，继续保持单日选择口径。
- 相比 `text_box`：这里不做自由文本输入，核心是点选式日期提交。
- 相比 `time_picker`：这里处理年月日与跨月浏览，不处理时分。

## 13. 本轮保留与删减
保留：
- 日期字段开合
- 月份浏览与单日提交
- 今日与已选日期视觉状态
- 键盘闭环
- `compact / read only` 静态 preview

删减：
- 页面级 guide 与状态说明
- preview 轮换
- preview 交互桥接
- 额外收尾状态

## 14. 当前验收结果
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/calendar_date_picker PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - Windows 长链接 `Error 87` 继续由 response-file fallback 成功处理
  - `output\main.exe calendar_date_picker`：`calendar_date_picker 8/8`
- catalog / 文档 / 触控语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触控语义结果：`custom_audited=29 custom_skipped_allowlist=5`
  - 文档编码结果：`136 files`
  - widget catalog 结果：`108 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_date_picker --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_calendar_date_picker/default`
  - 共捕获 `9` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `34 / 34` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/calendar_date_picker`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_calendar_date_picker`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2013 colors=146`
- 截图复核结论：
  - 主区覆盖默认 `2026-03-18 / Mar 2026`、浏览 `2026-03-18 / Apr 2026` 与提交 `2026-04-02 / Apr 2026` 三组 reference 状态
  - 最终稳定帧回到默认 `2026-03-18 / Mar 2026`
  - 全帧分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(164, 100) - (317, 213)`
  - 按 `y >= 250` 裁切底部 preview 后，preview 区全程保持单一哈希
