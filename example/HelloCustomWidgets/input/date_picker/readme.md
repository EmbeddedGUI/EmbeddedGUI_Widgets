# date_picker 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI DatePicker`
- 对应组件名：`DatePicker`
- 本次保留状态：`standard`、`opened`、`browse month`、`compact`、`read only`
- 删除效果：页面级 guide、状态文案、`Standard` 标签、分隔线、底部预览标签、标签点击切换、场景化 chrome
- EGUI 适配说明：继续复用仓库内 `date_picker` 基础实现，本轮只收口 `reference` 示例页结构、palette 和录制节奏，不改 SDK

## 1. 为什么需要这个控件

`date_picker` 用来表达“先查看当前日期值，再按需展开月历面板选择某一天”的标准日期输入语义，适合交付日期、截止日期、出行日期、预约日期等页面内设置场景。

## 2. 为什么现有控件不够用

- `calendar_view` 更偏日历浏览，不承担表单字段语义
- `textinput` 需要手动输入日期字符串，不适合低噪音日期选择
- `combobox` / `auto_suggest_box` 强调候选项列表，不适合月历网格选日
- `time_picker` 只覆盖时分，不覆盖年月日与月份浏览

因此这里继续保留 `date_picker`，但示例页必须回到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `DatePicker`，保留字段、helper 和展开月历面板
- 左下 `compact` 预览作为紧凑态对照，只保留字段展示
- 右下 `read only` 预览作为只读对照，不响应 touch / key
- 示例页只保留标题、主 `date_picker` 和底部 `compact / read only` 双预览
- 页面空白区与底部双预览都只承担失焦收口，不再承担状态切换职责
- 录制动作改为程序化 snapshot，不再依赖 guide 或标签点击

目录：

- `example/HelloCustomWidgets/input/date_picker/`

## 4. 视觉与布局规格

- 页面尺寸：`480 x 480`
- 根布局：`224 x 264`
- 页面结构：标题 -> 主 `date_picker` -> `compact / read only` 双预览
- 主控件尺寸：`196 x 180`（展开） / `196 x 82`（收起）
- 底部双预览行：`216 x 48`
- `compact` 预览：`104 x 48`
- `read only` 预览：`104 x 48`
- 视觉约束：
  - 保持浅色 page panel、白色 surface 和低噪音边框
  - 字段、panel、选中日与 today 标记保持标准 `DatePicker` 层级
  - 不再引入页面级引导文案、说明标签和分隔装饰
  - `compact` 与 `read only` 是静态对照，不再承担演示切换逻辑

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 264` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Date Picker` | 页面标题 |
| `picker_primary` | `egui_view_date_picker_t` | `196 x 180` | `2026-03-18` 展开 | 主 `DatePicker` |
| `picker_compact` | `egui_view_date_picker_t` | `104 x 48` | `Mar 18` | 紧凑态预览 |
| `picker_read_only` | `egui_view_date_picker_t` | `104 x 48` | `Apr 05` | 只读态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认展开态 | `2026-03-18`，panel=`Mar 2026` | `Mar 18` | `Apr 05` |
| 浏览态 | 字段仍是 `2026-03-18`，panel 切到 `Apr 2026` | 不响应 | 不响应 |
| 提交态 | 提交为 `2026-04-02`，保持展开 | 不响应 | 不响应 |
| 收起态 | 字段 `2026-11-27`，面板收起 | 不响应 | 不响应 |
| 跨年浏览态 | 字段 `2026-11-27`，panel 浏览到 `Jan 2027` | 不响应 | 不响应 |
| 失焦收口 | 点击空白区或底部预览时关闭主面板 | 静态对照 | 静态对照 |
| 只读弱化 | 不适用 | 不适用 | 保留结果展示但不接受交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主 snapshot、`compact` snapshot 和只读对照
2. 输出默认展开帧
3. 切到浏览态：字段不变，panel 切到 `Apr 2026`
4. 输出浏览帧
5. 切到提交态：主日期改为 `2026-04-02`
6. 输出提交帧
7. 切到收起态：字段为 `2026-11-27`
8. 输出收起帧
9. 切到跨年浏览态：panel 浏览到 `Jan 2027`
10. 输出跨年浏览帧
11. 切换 `compact` 预览到第二个静态日期
12. 输出最终对照帧

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/date_picker PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/date_picker --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主 `DatePicker` 展开态和收起态都必须完整可见
- 月份标题、weekday header、日期网格和选中日必须层级清晰
- `compact` 与 `read only` 对照必须低噪音且明显区分
- 页面中不再出现 guide、状态行、分隔线和 preview label

## 9. 已知限制与后续方向

- 当前仍是页内 inline panel，不是真实浮层
- 暂不做 locale 文案切换，只保留英文月缩写
- 暂不做月份外日期的连续显示
- 如后续继续下沉到框架层，再评估 placeholder、格式化策略和 locale 扩展

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `calendar_view`：这里是标准输入字段 + 月历面板，不是纯浏览型日历
- 相比 `textinput`：这里不做自由文本输入，核心是选日
- 相比 `combobox` / `auto_suggest_box`：这里不做候选列表，核心是月份浏览和日期网格提交
- 相比 `time_picker`：这里处理年月日与月份切换，不处理时分

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 补充参考：`WinUI DatePicker`

## 12. 对应组件名与保留核心状态

- 对应组件名：`DatePicker`
- 本次保留核心状态：
  - `standard`
  - `opened`
  - `browse month`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰

- 不做页面级 guide、状态文案、`Standard` 标签、分隔线和 preview label
- 不做标签点击切换和额外页面层状态桥接
- 不做桌面级浮层定位、Acrylic、复杂阴影和过渡动画
- 不做场景化说明卡片和装饰性 chrome

## 14. EGUI 适配时的简化点与约束

- 直接复用 `date_picker` 基础实现，避免在示例页重复造状态桥
- 用统一浅色 palette 收口到 `Fluent 2 / WPF UI` reference 方向
- 通过示例页高度同步保留主控件展开 / 收起时的合理留白
- 底部双预览固定为静态对照，只负责失焦收口，不再承担演示逻辑
