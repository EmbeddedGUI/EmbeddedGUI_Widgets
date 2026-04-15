# date_picker 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI DatePicker`
- 对应组件：`DatePicker`
- 当前保留形态：`standard`、`opened`、`browse month`、`compact`、`read only`
- 当前移除内容：页面级 guide、状态说明、preview 轮换、preview 清焦桥接、额外收尾态

## 1. 为什么需要这个控件
`date_picker` 用来表达“先查看当前日期，再按需展开月份面板选择某一天”的标准日期输入语义，适合交付日期、预约日期、截止日期、行程日期等表单场景。它同时承担日期字段、月份浏览和单日提交，不需要用户手工输入日期字符串。

## 2. 为什么现有控件不够用
- `calendar_view` 更偏向整月浏览和范围选择，不承担单个字段输入语义。
- `text_box` / `textinput` 需要手工输入日期文本，不适合低噪音的点选式日期提交。
- `combo_box` / `auto_suggest_box` 适合候选项列表，不适合月份网格浏览与单日选取。
- `time_picker` 只覆盖时分，不覆盖年月日与跨月浏览。

## 3. 当前页面结构
- 标题：`Date Picker`
- 主区：一个可真实交互的 `date_picker`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Mar 18`
- 右侧 preview：`read only`，固定显示 `Apr 05`

目录：`example/HelloCustomWidgets/input/date_picker/`

## 4. 主区参考快照
主控件在录制轨道里只保留 3 组 reference 状态：

1. 默认展开态
   字段：`2026-03-18`
   面板：`Mar 2026`
2. 浏览态
   字段仍为：`2026-03-18`
   面板切到：`Apr 2026`
3. 提交态
   字段更新为：`2026-04-02`
   面板保持：`Apr 2026`

底部 preview 始终固定：

1. `compact`
   字段：`Mar 18`
2. `read only`
   字段：`Apr 05`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 264`
- 主控件尺寸：`196 x 180`（展开）/ `196 x 82`（收起）
- 底部 preview 行：`216 x 48`
- 单个 preview：`104 x 48`
- 风格约束：浅色 page panel、白色 surface、低噪音边框、标准日期字段和月份网格层级

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认展开态 | `2026-03-18`，`Mar 2026` | `Mar 18` | `Apr 05` |
| 浏览态 | 字段不变，面板切到 `Apr 2026` | 不响应输入 | 不响应输入 |
| 提交态 | 字段提交为 `2026-04-02` | 不响应输入 | 不响应输入 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
录制轨道已经收口到 static preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到浏览态，面板切到 `Apr 2026`
4. 抓取第二组主区快照
5. 切到提交态，字段改为 `2026-04-02`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- runtime 录制阶段不再轮换 `compact` preview。
- 底部 preview 统一通过 `egui_view_date_picker_override_static_preview_api()` 吞掉 touch / key。
- 页面空白区仍保留最小失焦逻辑，但 preview 不再承担额外交互职责。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_date_picker.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 setter clamp、`pressed_part / pressed_day / is_pressed` 清理、月份浏览、日期提交、键盘浏览、`compact / read only / disabled` guard 和 focus change 收口。
2. 静态 preview 不变性断言
   通过 `date_picker_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`on_date_changed`、`on_open_changed`、`on_display_month_changed`、`font`、`meta_font`、`label`、`helper`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`today_color`、`year`、`panel_year`、`today_year`、`month`、`panel_month`、`day`、`today_month`、`today_day`、`first_day_of_week`、`compact_mode`、`read_only_mode`、`open_mode`、`preserve_display_month_on_open`、`pressed_part`、`pressed_day`、`alpha`

同时要求：
- `g_date_changed_count == 0`
- `g_open_changed_count == 0`
- `g_display_changed_count == 0`
- `is_pressed == false`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/date_picker PORT=pc

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

## 10. 当前结果
- `HelloUnitTest`：`845 / 845` 通过，`date_picker` suite `8 / 8`
- widget 编译：PASS
- widget catalog 同步：PASS（`106` entries）
- `touch release semantics`：PASS（`custom_audited=28`，`custom_skipped_allowlist=5`）
- `docs encoding`：PASS（`134 files`）
- `widget catalog check`：PASS（`106 widgets: reference=106, showcase=0, deprecated=0`）
- widget 级 runtime：PASS
- input 分类 compile/runtime 回归：PASS
- wasm 构建：PASS
- web smoke：`PASS status=Running canvas=480x480 ratio=0.2013 colors=146`

## 11. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_input_date_picker/default`

- 总帧数：`8`
- 主区 RGB 差分边界：`(164, 99) - (317, 212)`
- 遮罩主区差分边界后，边界外唯一哈希数：`1`
- 按主区差分边界裁剪后，主区唯一状态数：`3`
- 按 `y >= 300` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 变化只发生在主区月份标题、日期网格与字段文本，符合 `Mar 18` 默认态、`Apr 2026` 浏览态和 `2026-04-02` 提交态三组 reference 轨道。
- 主区差分边界外全程静态，没有黑屏、白屏、错位或 preview 污染。
- 底部 `compact / read only` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前仍是页面内 inline panel，不是浮层式日期面板。
- 暂不做 locale 文案切换，月份与 weekday 保持英文缩写。
- 暂不扩展跨月连续范围选择，只保留单日浏览与提交。

## 13. 与现有控件的边界
- 相比 `calendar_view`：这里是标准日期输入字段，不是范围浏览控件。
- 相比 `text_box`：这里不做自由文本输入，核心是点选式日期提交。
- 相比 `combo_box` / `auto_suggest_box`：这里不是候选列表，而是月份网格浏览。
- 相比 `time_picker`：这里处理年月日与跨月浏览，不处理时分。

## 14. EGUI 适配说明
- 继续复用仓库内既有 `date_picker` 基础实现，不修改 SDK。
- 示例页只负责 reference 页面结构、静态 preview 和录制轨道收口。
- 底部 preview 统一通过 `egui_view_date_picker_override_static_preview_api()` 吞掉输入并清理残留 `pressed`。
- 当前优先保证 setter 清理、same-target release、静态 preview 不变性和 runtime 渲染稳定，再评估是否继续上升到框架层公共控件。
