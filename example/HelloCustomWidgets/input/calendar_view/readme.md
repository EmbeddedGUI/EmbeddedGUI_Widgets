# calendar_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI CalendarView`
- 对应组件名：`CalendarView`
- 本次保留状态：`standard`、`range anchor / preview / commit`、`browse month`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态文案、`Standard` 标签、分隔线、预览标签、标签点击切换与场景化 chrome
- EGUI 适配说明：继续复用仓库内 `calendar_view` 基础实现，本轮只收口 `reference` 示例页结构、配色和录制节奏，不改 SDK

## 1. 为什么需要这个控件
`calendar_view` 用来表达“在一个始终可见的月历面板里浏览月份并选择日期区间”的标准语义，适合排期、冻结窗口、值班表、旅行区间和交付窗口等场景。

## 2. 为什么现有控件不够用
- `date_picker` 更偏字段输入 + 展开面板，不是常驻月历浏览
- `mini_calendar` 更偏展示，不承担区间选择与键盘焦点闭环
- `textinput` 不适合低噪音日期区间输入
- `time_picker` 只覆盖时分，不覆盖月份浏览和日期范围

因此这里继续保留 `calendar_view`，但示例页必须回到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `CalendarView`：保留标签、helper、月份导航、weekday header 和 6x7 day grid
- 左下 `compact` 预览只保留紧凑摘要，用作静态 reference 对照
- 右下 `read only` 预览展示弱化后的只读结果，不响应 touch / key
- 页面只保留标题、主 `calendar_view` 和底部 `compact / read only` 双预览
- 页面空白区和底部预览只承担失焦收口，不再承担 preset 切换职责
- 录制动作改为程序化 snapshot，不再依赖 guide 或预览标签点击

目录：
- `example/HelloCustomWidgets/input/calendar_view/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 232`
- 页面结构：标题 -> 主 `calendar_view` -> `compact / read only` 双预览
- 主控件尺寸：`196 x 144`
- 底部双预览行：`216 x 50`
- `compact` 预览：`104 x 50`
- `read only` 预览：`104 x 50`
- 视觉约束：
  - 保持浅色 page panel、白色 surface 和低噪音边框
  - 主控件保留 `CalendarView` 的标准层级，不再叠加页面级说明文本
  - 区间状态继续通过起止点、高亮桥接和 today marker 表达
  - `compact` 与 `read only` 固定为静态对照，不再承担额外交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Calendar View` | 页面标题 |
| `calendar_primary` | `egui_view_calendar_view_t` | `196 x 144` | `Mar 2026 / 09-13` | 主 `CalendarView` |
| `calendar_compact` | `egui_view_calendar_view_t` | `104 x 50` | `May 05-08` | 紧凑态静态预览 |
| `calendar_read_only` | `egui_view_calendar_view_t` | `104 x 50` | `Jul 18-22` | 只读态静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Mar 2026 / 09-13` | `May 05-08` | `Jul 18-22` |
| anchor 预览 | `Enter` 后向右扩展到 `09-15` | 不响应 | 不响应 |
| browse month | `+` 后浏览到 `Apr 2026` | 不响应 | 不响应 |
| 次级 preset | 切到 `Nov 2026 / 03-07` | 不响应 | 不响应 |
| compact 对照 | 不适用 | 切到 `Jun 12` | 不适用 |
| 失焦收口 | 点击空白区或底部预览时清除主控件焦点 | 静态对照 | 静态对照 |
| 只读弱化 | 不适用 | 不适用 | 保留结果展示但不接受交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主 snapshot、`compact` snapshot 和只读对照
2. 输出默认主状态
3. 主控件进入 `anchor` 状态
4. 键盘向右扩展到 `09-15`
5. 输出区间预览帧
6. 提交区间
7. 浏览到 `Apr 2026`
8. 输出 browse 帧
9. 切到第二组主 snapshot
10. 输出次级 preset 帧
11. 切换 `compact` 预览到第二组静态摘要
12. 输出最终对照帧

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/calendar_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主 `CalendarView` 的 month title、weekday header 和 day grid 必须完整可见
- `anchor` 预览、区间提交和 `browse month` 的层级必须清晰
- `set_range / display month / palette / current part / compact / read only` 切换后不能残留 `pressed_part / pressed_day / is_pressed` 污染
- 底部 `compact / read only` 预览必须统一吞掉 touch / key 输入，并在收到输入后立即清理残留 `pressed` 渲染
- `compact` 与 `read only` 必须是低噪音静态 reference，不再出现页面级标签
- 页面中不再出现 `guide`、状态行、分隔线和 preview label

## 9. 已知限制与后续方向
- 当前仍是单月网格，不做 year / decade 层级切换
- 当前区间仍限定在单月内，不做跨月连续选择
- 暂不接入 locale 文案切换，只保留英文月份缩写和单字母 weekday
- 若后续继续下沉到框架层，再评估跨月范围、年份跳转和更多选择模式

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `date_picker`：这里是常驻月历视图，不是字段 + 展开面板
- 相比 `mini_calendar`：这里强调 range anchor、month browse 和键盘部件切换
- 相比 `textinput`：这里不做自由日期文本输入
- 相比 `time_picker`：这里处理日期网格和月份浏览，不处理时分选择

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 补充参考：`WinUI CalendarView`

## 12. 对应组件名与保留核心状态
- 对应组件名：`CalendarView`
- 本次保留核心状态：
  - `standard`
  - `range anchor / preview / commit`
  - `browse month`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰
- 不做页面级 `guide`、状态文案、`Standard` 标签、分隔线和 preview label
- 不做标签点击切换 preset 和额外页面状态桥接
- 不做系统级浮层、阴影、多层动画和场景化说明卡片
- 不做跨月连续选择和 year / decade 级切换

## 14. EGUI 适配时的简化点与约束
- 直接复用 `calendar_view` 基础实现，避免在示例页重复搭状态桥
- 用统一浅色 palette 收口到 `Fluent 2 / WPF UI` reference 方向
- 主控件保留真实 touch / key 区间选择与月份浏览闭环
- 页面空白区和底部双预览只负责清焦，底部双预览通过统一 static preview API 吞输入，不再承担演示切换逻辑
