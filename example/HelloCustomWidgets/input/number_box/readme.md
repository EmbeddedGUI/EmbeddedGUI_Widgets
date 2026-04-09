# number_box 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`NumberBox`
- 本次保留状态：`standard`、`compact`、`read only`、`step up`、`step down`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、键盘输入校验、错误提示气泡、图标前后缀、Acrylic 与复杂焦点动画
- EGUI 适配说明：使用固定范围、轻量步进按钮和静态单位后缀，在 `480 x 480` 页面里优先保证数字输入框的可读性与对照关系稳定

## 1. 为什么需要这个控件

`number_box` 用来在表单、设置页和属性面板里输入离散数字，比如边距、延迟、字号和数量限制。它应该比 `slider` 更精确，比 `number_picker` 更贴近表单控件，也比纯 `textinput` 更适合低风险的整数步进输入。

## 2. 为什么现有控件不够用

- `number_picker` 是滚轮式选择，适合列表值滚动，不适合标准表单里的轻量数字输入
- `slider` 更偏连续拖动，难以表达精确步进
- `textinput` 是通用文本编辑，不自带数值范围与加减步进语义
- 当前主线需要一版接近 `Fluent 2 / WPF UI` 的标准 `NumberBox` reference

因此这里继续保留 `number_box`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `number_box`，用于调节 `Spacing`
- 左下 `compact` 预览展示窄宽度下的轻量数字输入
- 右下 `read only` 预览展示只读弱化版数字框
- 示例页只保留标题、主 `number_box` 和底部 `compact / read only` 双预览，不再保留外部 guide、状态回显和标签点击

目录：

- `example/HelloCustomWidgets/input/number_box/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 154`
- 页面结构：标题 -> 主 `number_box` -> `compact / read only` 双预览
- 主数字框：`196 x 70`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`104 x 44`
- `read only` 预览：`104 x 44`
- 视觉规则：
  - 使用浅灰白 page panel + 白底轻边框容器
  - 主数字框保留 label、helper、value field 与 `- / +` 按钮语义
  - `compact` 预览压缩为更轻量的 field + stepper 结构
  - 只读态移除步进按钮，只保留弱化数值展示

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 154` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Number Box` | 页面标题 |
| `box_primary` | `egui_view_number_box_t` | `196 x 70` | `Spacing = 24 px` | 标准数字框 |
| `box_compact` | `egui_view_number_box_t` | `104 x 44` | `12 ms` | 紧凑预览 |
| `box_read_only` | `egui_view_number_box_t` | `104 x 44` | `16 px` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主数字框 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `24 px` | `12 ms` | `16 px` |
| 轮换 1 | `28 px` | 保持 | 保持 |
| 轮换 2 | `32 px` | 保持 | 保持 |
| 轮换 3 | `28 px` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `14 ms` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 移除步进按钮，仅保留弱化数值展示 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主值与 `compact` 值
2. 请求第一页截图
3. 程序化切换主数字框到 `28 px`
4. 请求第二页截图
5. 程序化切换主数字框到 `32 px`
6. 请求第三页截图
7. 程序化切换主数字框回 `28 px`
8. 请求第四页截图
9. 程序化切换 `compact` 到 `14 ms`
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/number_box PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/number_box --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主数字框和底部双预览必须完整可见，不能被裁切
- 主数字框必须看起来像标准表单数字输入，而不是滚轮或 slider
- 数值与单位后缀要保持居中，不能贴边
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签

## 9. 已知限制与后续方向

- 当前版本只覆盖整数步进，不做键盘文本编辑
- 当前不做错误校验提示、placeholder 和复杂焦点环
- 当前 `compact` 与 `read only` 仅作为静态对照，不承担交互职责
- 若后续要沉入框架层，再单独评估与文本输入、校验态和表单系统的联动

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `number_picker`：本控件是标准表单数字输入，不是滚轮选择器
- 相比 `slider`：本控件表达离散步进，不是连续拖动
- 相比 `textinput`：本控件有明确范围、步长与加减按钮语义
- 相比 `stepper`：本控件编辑数字值，不表达流程步骤

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`NumberBox`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `step up`
  - `step down`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做键盘输入、选中文本和光标编辑
- 不做错误态图标、浮层提示和复杂校验动画
- 不做鼠标滚轮输入、hover 光效和系统级焦点环

## 14. EGUI 适配时的简化点与约束

- 使用固定整数范围和步进，优先保证 `480 x 480` 页面里的可审阅性
- 通过轻量 `- / +` 按钮承载交互，不引入复杂文本输入状态机
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 先完成示例级数字框，再决定是否上升到框架公共控件
