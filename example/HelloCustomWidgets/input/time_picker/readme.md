# time_picker 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI TimePicker`
- 对应组件名：`TimePicker`
- 本次保留状态：`standard`、`opened`、`12h / 24h`、`compact`、`read only`
- 删除效果：页面级 guide、状态文案、`Standard` 标签、分隔线、底部预览标签、标签点击切换、场景化 chrome
- EGUI 适配说明：继续复用仓库内 `time_picker` 基础实现，本轮只收口 `reference` 示例页结构、palette 和录制节奏，不改 SDK

## 1. 为什么需要这个控件

`time_picker` 用来表达“先查看当前时间值，再按需展开选择小时、分钟和上下午”的标准时间输入语义，适合会议时间、静默时段、提醒时间和同步窗口等页面内设置场景。

## 2. 为什么现有控件不够用

- `number_picker` 只覆盖单列数字滚动，不承担时间字段语义
- `combobox` / `auto_suggest_box` 强调候选项列表，不适合时间分段调节
- `textinput` 需要手动输入时间字符串，不适合低噪音设置页
- `settings_panel` 只负责承载设置项，不负责时间字段本体交互

因此这里继续保留 `time_picker`，但示例页必须回到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `TimePicker`，保留字段、helper 和展开选择面板
- 左下 `compact` 预览作为紧凑态对照，使用 `24h` 显示
- 右下 `read only` 预览作为只读对照，不响应 touch / key
- 示例页只保留标题、主 `time_picker` 和底部 `compact / read only` 双预览
- 页面空白区与底部双预览只承担 dismiss，不再承担状态切换职责
- 录制动作改为程序化 snapshot，不再依赖 guide 或标签点击

目录：

- `example/HelloCustomWidgets/input/time_picker/`

## 4. 视觉与布局规格

- 页面尺寸：`480 x 480`
- 根布局：`224 x 220`
- 页面结构：标题 -> 主 `time_picker` -> `compact / read only` 双预览
- 主控件尺寸：`196 x 126`（展开） / `196 x 68`（收起）
- 底部双预览行：`216 x 58`
- `compact` 预览：`104 x 58`
- `read only` 预览：`104 x 58`
- 视觉约束：
  - 保持浅色 page panel、白色 surface 和低噪音边框
  - 字段内部保留 `hour / minute / period` 的标准层级
  - 展开面板保留三列调节区域，不再增加页面级说明壳
  - `compact` 与 `read only` 固定为静态对照，不再承担点击切换逻辑

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 220` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Time Picker` | 页面标题 |
| `picker_primary` | `egui_view_time_picker_t` | `196 x 126` | `08:30 AM` 展开 | 主 `TimePicker` |
| `picker_compact` | `egui_view_time_picker_t` | `104 x 58` | `13:30` | `24h` 紧凑态预览 |
| `picker_read_only` | `egui_view_time_picker_t` | `104 x 58` | `07:15 AM` | 只读态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认展开态 | `08:30 AM`，焦点在 minute | `13:30` | `07:15 AM` |
| 浏览态 | `10:45 AM`，焦点切到 hour | 不响应 | 不响应 |
| 收起态 | `06:00 PM`，面板收起 | 不响应 | 不响应 |
| 夜间态 | `09:15 PM`，焦点切到 period | 不响应 | 不响应 |
| `24h` 对照 | 不适用 | `13:30` -> `18:00` | 不适用 |
| dismiss | 点击空白区或底部预览时关闭主面板 | 静态对照 | 静态对照 |
| 只读弱化 | 不适用 | 不适用 | 保留结果展示但不接受交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主 snapshot、`compact` snapshot 和只读对照
2. 输出默认展开帧
3. 切到 `10:45 AM` 展开态
4. 输出浏览帧
5. 切到 `06:00 PM` 收起态
6. 输出收起帧
7. 切到 `09:15 PM` 展开态，焦点停在 period
8. 输出夜间态截图
9. 切换 `compact` 预览到 `18:00`
10. 输出最终对照帧

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/time_picker PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/time_picker --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主 `TimePicker` 展开态和收起态都必须完整可见
- `hour / minute / period` 的层级、焦点和中心值必须清晰可辨
- `compact` 与 `read only` 对照必须低噪音且明显区分
- 页面中不再出现 guide、状态行、分隔线和 preview label

## 9. 已知限制与后续方向

- 当前仍是页内 inline panel，不是真实浮层
- 暂不做真实滚轮惯性和复杂动画
- 暂不接入 locale 文案，只覆盖 `12h / 24h`
- 如后续继续下沉到框架层，再评估 placeholder、step 和 locale 扩展

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `number_picker`：这里是标准时间字段 + 分段调节，不是单列数字选择
- 相比 `combobox` / `auto_suggest_box`：这里不做候选列表，核心是时间语义
- 相比 `settings_panel`：这里提供时间输入主体，而不是设置容器
- 相比 `split_button` / `menu_flyout`：这里不承担命令触发，而是值选择

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 补充参考：`WinUI TimePicker`

## 12. 对应组件名与保留核心状态

- 对应组件名：`TimePicker`
- 本次保留核心状态：
  - `standard`
  - `opened`
  - `12h / 24h`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰

- 不做页面级 guide、状态文案、`Standard` 标签、分隔线和 preview label
- 不做标签点击切换和额外页面层状态桥接
- 不做桌面级浮层定位、Acrylic、复杂阴影和过渡动画
- 不做场景化说明卡片和装饰性 chrome

## 14. EGUI 适配时的简化点与约束

- 直接复用 `time_picker` 基础实现，避免在示例页重复造状态桥
- 用统一浅色 palette 收口到 `Fluent 2 / WPF UI` reference 方向
- 通过示例页高度同步保留主控件展开 / 收起时的合理留白
- 页面空白区和底部双预览只负责 dismiss，不再承担演示逻辑
