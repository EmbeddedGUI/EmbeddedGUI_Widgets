# text_box 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`TextBox`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`、`submitted`
- 删除效果：页面级 guide、额外 section label、外部 preview 标签、虚拟键盘浮层、showcase 式装饰块
- EGUI 适配说明：复用 SDK `textinput` 的文本编辑、光标和键盘提交链路，只在 custom 层补齐 Fluent 风格外观绘制与静态 preview 语义

## 1. 为什么需要这个控件？
`text_box` 是最基础的表单文本输入控件，用于名称、备注、搜索词和轻量配置项录入。当前 `HelloCustomWidgets` 还没有一个对齐 `Fluent 2 / WPF UI` 主线的 `TextBox` reference 页面，因此需要补齐。

## 2. 为什么现有控件不够用？
- `password_box` 会隐藏字符，语义不同。
- `number_box` 只覆盖数值步进，不适合普通文本录入。
- `auto_suggest_box` 自带建议列表，不是最小输入框语义。
- SDK 已有基础 `textinput`，但当前仓库还没有 reference 主线下的 `input/text_box` 包装页、单测和 catalog 闭环。

## 3. 目标场景与示例概览
- 主区域展示一个可交互的标准 `TextBox`，保留真实焦点、光标和键盘编辑链路。
- 左下 `compact` preview 展示紧凑输入框尺寸。
- 右下 `read only` preview 展示只读弱化视觉，但作为静态 preview 不承担真实编辑职责。
- 页面只保留标题、主输入框和底部双 preview。

目录：
- `example/HelloCustomWidgets/input/text_box/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 128`
- 页面结构：标题 -> 主 `text_box` -> `compact / read only` 双 preview
- 主输入框尺寸：`196 x 40`
- 底部 preview 行：`216 x 32`
- 单个 preview：`104 x 32`

视觉约束：
- 标准态使用白底轻边框，与 Fluent 表单输入框一致。
- `compact` 保留输入语义，仅压缩尺寸和 padding。
- `read only` 使用更弱的填充与边框，不承担真实编辑。
- 焦点态保留轻量 accent ring，不引入厚阴影或额外 chrome。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 128` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Text Box` | 页面标题 |
| `box_primary` | `egui_view_textinput_t` | `196 x 40` | focused | 主输入框 |
| `box_compact` | `egui_view_textinput_t` | `104 x 32` | static preview | 紧凑预览 |
| `box_read_only` | `egui_view_textinput_t` | `104 x 32` | static preview | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主输入框 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 焦点 / 光标 | 是 | 否 | 否 |
| 键盘编辑 | 是 | 否 | 否 |
| `Enter` submit | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| preview 点击清焦点 | 否 | 是 | 是 |

## 7. 交互语义
- 主输入框继续复用 SDK `textinput` 的 `touch -> focus -> key edit` 链路，不改 `sdk/EmbeddedGUI`。
- `Backspace / Delete / Left / Right / Home / End / Enter` 继续走 SDK 原生行为。
- 自定义层只覆盖 `on_draw`，改为仓内统一的 Fluent 风格输入框外观。
- 底部两个 preview 通过 `hcw_text_box_override_static_preview_api()` 统一吞掉 `touch / key`。
- 页面层只在 preview `ACTION_DOWN` 时清主输入框焦点，不在 preview 上追加业务交互。

## 8. 本轮收口内容
- 新增 `egui_view_text_box.h/.c`，作为 SDK `textinput` 的 reference 包装层。
- 在包装层补齐：
  - `standard / compact / read only` 三套样式 helper
  - 复用 view background 系统的 Fluent 风格输入框外观
  - 静态 preview 输入吞掉
- 新增 `test.c` reference 页面，只保留主输入框与双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、键盘编辑、提交回调、只读保护与静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 链路：
1. 还原主输入框、`compact` 和 `read only` 默认状态，并给主输入框 request focus。
2. 截默认态。
3. 发送 `Backspace`，删除末尾数字。
4. 发送 `2`，把主输入框文本改成 `Node 02`。
5. 截编辑后状态。
6. 发送 `Enter`，将主输入框内容同步到 `compact` preview。
7. 点击 `compact` preview 清理主输入框焦点。
8. 截最终收尾帧。

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/text_box PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/text_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/text_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_text_box
```

验收重点：
- 主输入框与底部双 preview 必须完整可见，不黑屏、不白屏、不裁切。
- 主输入框必须保持可读的表单输入框语义，而不是 button 或 label。
- `Backspace / Enter` 编辑链路必须稳定。
- `compact / read only` preview 必须保持静态 reference，对输入只做吞掉和焦点收尾。

## 11. 已知限制
- 当前只覆盖单行 `TextBox` reference，不扩展多行编辑器或搜索框内置图标。
- 不做虚拟键盘页面联动与系统 IME 浮层。
- 页面不额外承载帮助文案和标签栏，避免偏离 reference 主线。

## 12. 与现有控件的差异边界
- 相比 `password_box`：这里展示明文输入，不承担 reveal / hide。
- 相比 `number_box`：这里不做数值范围、步进和单位后缀。
- 相比 `auto_suggest_box`：这里不做候选列表和展开态。

## 13. 对应组件名，以及本次保留的核心状态
- 对应组件名：`TextBox`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `focused`
  - `submitted`

## 14. EGUI 适配时的简化点与约束
- 继续复用 SDK `textinput` 的文本编辑、光标与提交逻辑。
- Fluent 风格外观与静态 preview 语义收口在 custom 层，不改 `sdk/EmbeddedGUI`。
- 页面保持最小 reference 结构，先确保控件级闭环，再考虑未来是否补充搜索或多行派生控件。
