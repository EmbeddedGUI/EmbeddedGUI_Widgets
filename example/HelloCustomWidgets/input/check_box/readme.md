# check_box 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI CheckBox`
- 对应组件名：`CheckBox`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`、`checked`
- 删除效果：页面级 `guide`、状态说明文案、额外装饰卡片、场景化 demo 壳
- EGUI 适配说明：直接复用 SDK `checkbox` 的绘制与 same-target release 触摸语义，在 custom 层补齐 Fluent 风格样式、`Space / Enter` 键盘闭环、setter 状态清理与静态 preview 输入吞掉

## 1. 为什么需要这个控件
`check_box` 用来表达“某个布尔选项当前是否被选中”的标准表单语义，适合通知开关、同步策略、权限确认、同意条款和批量选择一类场景。它和 `toggle_button` 的差别在于：这里强调的是复选框字段，而不是按钮化命令入口。

## 2. 为什么现有控件不够用
- `toggle_button` 更接近按钮语义，视觉重心也更像命令触发器。
- `switch` 属于 SDK 层基础控件，但当前仓库没有一版对齐 `Fluent 2 / WPF UI` 的 `CheckBox` reference 页面。
- `radio_button` 表达的是互斥选择，不适合独立布尔字段。

因此这里补上一版 `input/check_box` reference，把标准 `CheckBox` 接入 `HelloCustomWidgets` 主线。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `check_box`，覆盖通知、同步和归档这类常见布尔字段。
- 左下 `compact` preview 展示紧凑版本的静态对照。
- 右下 `read only` preview 展示只读版本的静态对照。
- 页面只保留标题、主控件和底部 `compact / read only` 双 preview，不再保留额外说明 chrome。

目录：
- `example/HelloCustomWidgets/input/check_box/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 158`
- 页面结构：标题 -> 主 `check_box` -> `compact / read only` 双 preview
- 主控件尺寸：`196 x 34`
- 底部 preview 行：`216 x 28`
- 单个 preview：`104 x 28`

视觉约束：
- 使用浅色 page panel、轻边框和低噪音勾选填充。
- 主控件保留轻量 focus ring，不做 showcase 风格重阴影。
- `compact` 只压缩尺寸和间距，不改变语义。
- `read only` 保留选中结果展示，但不承担输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 158` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Check Box` | 页面标题 |
| `control_primary` | `egui_view_checkbox_t` | `196 x 34` | `Email alerts` / unchecked | 主控件 |
| `control_compact` | `egui_view_checkbox_t` | `104 x 28` | `Auto` / checked | 紧凑静态预览 |
| `control_read_only` | `egui_view_checkbox_t` | `104 x 28` | `Accepted` / checked | 只读静态预览 |

## 6. 状态覆盖矩阵
| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 已选中 | 是 | 是 | 是 |
| 键盘切换 | 是 | 否 | 否 |
| same-target release | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focus ring | 是 | 否 | 否 |

## 7. 交互语义
- `Space / Enter`：按下时进入 pressed，抬起时切换 `checked`。
- `DOWN(A) -> MOVE(B) -> UP(B)`：不提交切换。
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)`：只在回到同一目标后提交切换。
- 主控件继续复用 SDK `checkbox` 的默认触摸释放语义，不改动 `sdk/EmbeddedGUI`。
- 底部 `compact / read only` preview 通过 `hcw_check_box_override_static_preview_api()` 统一吞掉 `touch / key`，输入到达时立即清理残留 `pressed`。

## 8. 本轮收口内容
- 新增 `egui_view_check_box.h/.c`，作为 SDK `checkbox` 的 Fluent reference 包装层。
- 在包装层补齐：
  - `standard / compact / read only` 样式 helper
  - `set_checked()`、`set_text()`、勾选标记相关 setter 的 pressed 清理
  - `Space / Enter` 键盘闭环
  - 静态 preview 输入吞掉
  - 主控件 focus ring
- 新增 `test.c` reference 页面，只保留主控件与底部双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、setter 清理、same-target release、键盘切换、disabled 与静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 还原主控件、`compact` 和 `read only` 默认快照，并给主控件 request focus
2. 截默认态
3. 发送 `Space`
4. 截选中结果
5. 切换到第二组主快照并重新 request focus
6. 发送 `Enter`
7. 截第二组结果
8. 切换 `compact` 与主控件最终快照，截收尾对照帧

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/check_box PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/check_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/check_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_check_box
```

验收重点：
- 主 `check_box` 与底部双 preview 必须完整可见，不允许黑屏、白屏或裁切。
- `Space / Enter` 的切换结果必须稳定，focus ring 可辨认。
- 触摸释放只允许在同一目标上提交，不允许漂移提交。
- `compact / read only` preview 必须吞掉输入并保持静态对照。

## 11. 已知限制
- 当前只做标准二态 `CheckBox` reference，不扩展三态 `indeterminate`。
- 不做富文本标签、嵌入链接或多列表单布局。
- 不做额外说明行、描述文本或批量选择面板。

## 12. 与现有控件的差异边界
- 相比 `toggle_button`：这里保留表单字段语义，而不是按钮式命令语义。
- 相比 `switch`：这里强调复选框视觉与清单场景，而不是立即生效的开关轨道。
- 相比 `radio_button`：这里支持独立布尔值，不承担互斥组职责。

## 13. EGUI 适配时的简化点与约束
- 继续复用 SDK `checkbox` 的基础绘制和触摸释放逻辑，避免重复实现底层点击状态机。
- 在 custom 层统一覆盖样式、键盘和静态 preview 语义，不改 `sdk/EmbeddedGUI`。
- 先完成 reference 级 `CheckBox`，后续再评估是否需要三态或附带描述文本版本。
