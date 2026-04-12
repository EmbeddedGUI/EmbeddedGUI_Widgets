# radio_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI RadioButtons`
- 对应组件名：`RadioButton`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`、`selected`
- 删除效果：页面级 `guide`、状态说明文案、额外装饰卡片、场景化 demo 壳
- EGUI 适配说明：复用 SDK `radio_button` 的单选组与 same-target release 语义，在 custom 层补齐 Fluent 风格样式、`Space / Enter` 键盘闭环、setter pressed 清理、轻量 focus ring 与静态 preview 输入吞掉

## 1. 为什么需要这个控件
`radio_button` 用于表达“多个候选项中只能选一个”的标准表单语义，适合通知方式、同步策略、显示密度、隐私级别这类互斥配置。它和 `check_box` 的差别在于：这里不是独立布尔字段，而是整组互斥选择。

## 2. 为什么现有控件不够用
- `check_box` 支持独立布尔值，不承担互斥组选项职责。
- `toggle_button` 更接近命令式按钮语义，不适合标准单选字段。
- `segmented_control` 强调胶囊切换与导航呈现，不等价于表单中的 `RadioButton`。

因此这里补上一版 `input/radio_button` reference，把标准 `RadioButton` 接入 `HelloCustomWidgets` 主线。

## 3. 目标场景与示例概览
- 主区域展示一组标准 `radio_button`，覆盖互斥选项选择和键盘闭环。
- 左下 `compact` preview 展示紧凑版静态对照。
- 右下 `read only` preview 展示只读版静态对照。
- 页面只保留标题、主单选组和底部双 preview，不再保留额外说明 chrome。

目录：
- `example/HelloCustomWidgets/input/radio_button/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 214`
- 页面结构：标题 -> 主 `radio_button` 组 -> `compact / read only` 双 preview
- 主单选组：`196 x 98`
- 单个主项：`196 x 30`
- 底部 preview 行：`216 x 52`
- 单个 preview 列：`104 x 52`
- 单个 preview 项：`104 x 24`

视觉约束：
- 使用浅色 page panel、低噪音描边和 Fluent 主色点选态。
- 主单选项保留轻量 focus ring，不做 showcase 风格重阴影。
- `compact` 仅压缩尺寸与间距，不改变互斥语义。
- `read only` 保留已选结果展示，但不承担输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 214` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Radio Button` | 页面标题 |
| `primary_buttons[3]` | `egui_view_radio_button_t` | `196 x 30` | `Home` 选中 | 主互斥组 |
| `compact_buttons[2]` | `egui_view_radio_button_t` | `104 x 24` | `Auto` 选中 | 紧凑静态预览 |
| `read_only_buttons[2]` | `egui_view_radio_button_t` | `104 x 24` | `Tablet` 选中 | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 已选中 | 是 | 是 | 是 |
| 键盘提交 | 是 | 否 | 否 |
| same-target release | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focus ring | 是 | 否 | 否 |

## 7. 交互语义
- `Space / Enter`：按下进入 pressed，抬起后把当前项提交为选中项。
- 已选中的 `radio_button` 再次提交时保持选中，不支持像 `check_box` 那样取消。
- `DOWN(A) -> MOVE(B) -> UP(B)`：不提交选中。
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)`：只在回到同一目标后提交。
- 主控件继续复用 SDK `radio_button` 的默认触摸释放语义，不改 `sdk/EmbeddedGUI`。
- 底部 `compact / read only` preview 通过 `hcw_radio_button_override_static_preview_api()` 统一吞掉 `touch / key`，输入到达时立即清理残留 `pressed`。

## 8. 本轮收口内容
- 新增 `egui_view_radio_button.h/.c`，作为 SDK `radio_button` 的 Fluent reference 包装层。
- 在包装层补齐：
  - `standard / compact / read only` 样式 helper
  - `set_checked()`、文本 / 字体 / 图标相关 setter 的 pressed 清理
  - `Space / Enter` 键盘闭环
  - 静态 preview 输入吞掉
  - 主单选项 focus ring
- 新增 `test.c` reference 页面，只保留主单选组与底部双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、setter 清理、组互斥、same-target release、键盘提交、禁用态和静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 还原主组、`compact` 和 `read only` 默认快照，并给第一项 request focus
2. 截默认态
3. 对第二项发送 `Space`
4. 截第二项选中结果
5. 切到第二组文本快照，并把焦点移到第三项
6. 对第三项发送 `Enter`
7. 截第三项选中结果
8. 切换最终文本与 `compact` 对照，截收尾帧

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_button PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_button
```

验收重点：
- 主单选组与底部双 preview 必须完整可见，不允许黑屏、白屏或裁切。
- `Space / Enter` 必须稳定提交新选项，且已选项不会被再次点击取消。
- 触摸释放只允许在同一目标上提交，不允许漂移提交。
- `compact / read only` preview 必须吞掉输入并保持静态对照。

## 11. 已知限制
- 当前只覆盖标准二态 `RadioButton` 互斥组，不扩展说明文本、副标题或嵌入式列表项。
- 不做多列复杂表单编排，也不承担导航式胶囊切换职责。
- 不做 `RadioButtons` 容器级额外标题栏、帮助文案或动态说明区域。

## 12. 与现有控件的差异边界
- 相比 `check_box`：这里是互斥组，只允许一个选项保持选中。
- 相比 `segmented_control`：这里保留标准表单字段语义，而不是胶囊式切换器。
- 相比 `toggle_button`：这里强调字段选择结果，不是按钮式命令反馈。

## 13. EGUI 适配时的简化点与约束
- 继续复用 SDK `radio_button` 的组管理、基础绘制和 same-target release 逻辑，避免重写底层选择状态机。
- 在 custom 层统一覆盖样式、键盘和静态 preview 语义，不改 `sdk/EmbeddedGUI`。
- 先完成 reference 级 `RadioButton`，后续再评估是否需要容器级 `RadioButtons` 扩展。
