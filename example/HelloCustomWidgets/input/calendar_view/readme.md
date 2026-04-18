# CalendarView 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`CalendarView`
- 当前保留形态：`standard`、`range preview`、`browse month`、`compact`、`read only`
- 当前保留交互：主区保留真实区间预览、月份浏览和快照切换；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态文案、`Standard` 标签、preview 标签点击桥接、录制阶段 `compact` 快照切换

## 1. 为什么需要这个控件
`calendar_view` 用来表达“在常驻月历面板里浏览月份并选择日期区间”的标准语义，适合排期、冻结窗口、值班表、预订区间和交付窗口等场景。

## 2. 为什么现有控件不够用
- `date_picker` 更偏字段输入加弹出面板，不是常驻月历浏览。
- `time_picker` 只覆盖时间，不覆盖日期网格和月份切换。
- 普通文本字段不适合低噪音日期区间输入。
- 仓库里当前 `input/calendar_view` 的 README 仍保留旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Calendar View`
- 主区：一个保留真实区间预览与月份浏览链路的主 `calendar_view`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `May 05-08`
- 右侧 preview：`read only`，固定显示 `Jul 18-22`

目录：
- `example/HelloCustomWidgets/input/calendar_view/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认区间
   `Booking window / Mar 2026 / 09-13`
2. 区间预览态
   `Mar 2026 / 09-15`
3. 月份浏览态
   `Apr 2026 / 09-15`
4. 第二组主快照
   `Release freeze / Nov 2026 / 03-07`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `May 05-08`
2. `read only`
   `Jul 18-22`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主控件：`196 x 144`
- 底部 preview 行：`216 x 50`
- 单个 preview：`104 x 50`
- 页面结构：标题 -> 主 `calendar_view` -> 底部 `compact / read only`
- 风格约束：使用浅色 page panel、白色 surface 和轻边框；主控件保留标题、weekday header 和 6x7 日期网格层次；区间通过起止日、高亮桥接和 today marker 表达；底部 `compact / read only` preview 保持低噪音静态对照

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Booking window / Mar 2026 / 09-13` | `May 05-08` | `Jul 18-22` |
| 快照 2 | `Mar 2026 / 09-15` | 保持不变 | 保持不变 |
| 快照 3 | `Apr 2026 / 09-15` | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Release freeze / Nov 2026 / 03-07` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件键盘闭环，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Booking window`，同时恢复底部 `compact / read only` 固定状态
2. 抓取首帧
3. 发送 `Enter + Right + Right`，把主区区间预览扩展到 `09-15`
4. 抓取第二组主区快照
5. 发送 `Enter + Plus`，提交区间并浏览到 `Apr 2026`
6. 抓取第三组主区快照
7. 切换到第二组主快照 `Release freeze`
8. 抓取第四组主区快照
9. 保持 `Release freeze` 不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `egui_view_calendar_view_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立即清理残留 `pressed_part / pressed_day / is_pressed`
- preview 不改 `selection / display month / current_part / region_screen / palette / font`

当前 `test.c` 已保持统一 finalize 模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，录制键盘入口先通过 `focus_primary_calendar_view()` 收敛焦点，再进入显式布局后的稳定抓帧路径，确保四组主区状态与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_calendar_view.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `touch` 双击提交区间、`Tab` 部件切换、方向键浏览、`Enter / Space` 区间编辑、`+ / -` 月份切换、`Escape` 收口，以及 `set_range()`、`set_display_month()`、`set_palette()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()` 的 `pressed` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `selection year / month / start / end`、`display year / month / current_part / compact_mode / read_only_mode`、`region_screen / palette / font / meta_font` 不变，并要求 `pressed_part / pressed_day / is_pressed` 被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/calendar_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/calendar_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_calendar_view
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `默认 / 区间预览 / 月份浏览 / Release freeze` 四组 reference 状态必须能从截图中稳定区分。
- 主控件区间预览、月份浏览以及 setter 状态清理链路收口后不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_calendar_view/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(44, 93) - (435, 308)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 309` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `date_picker`：这里是常驻月历视图，不是字段加弹出面板。
- 相比 `time_picker`：这里处理日期网格和月份浏览，不处理时分选择。
- 相比普通文本字段：这里不做自由日期文本输入。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Booking window / Mar 2026 / 09-13`
  - `Mar 2026 / 09-15`
  - `Apr 2026 / 09-15`
  - `Release freeze / Nov 2026 / 03-07`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态文案
  - `Standard` 标签与 preview 标签点击桥接
  - 录制阶段 `compact` 快照切换
  - 非必要的跨月范围、年份跳转和 showcase 化页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/calendar_view PORT=pc`
- `HelloUnitTest`：`BUILD PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 当前 shell 直接执行 `X:\output\main.exe` 超时，未能在本轮重新抓取完整摘要
  - 最近一次成功执行口径：总计 `845 / 845`，其中 `calendar_view` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_calendar_view/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/calendar_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_calendar_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1769 colors=178`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(44, 93) - (435, 308)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 309` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Booking window`、`09-15` 区间预览、`Apr 2026` 月份浏览与 `Release freeze` 四组 reference 状态，最终稳定帧保持 `Release freeze`，底部 `compact / read only` preview 全程静态
