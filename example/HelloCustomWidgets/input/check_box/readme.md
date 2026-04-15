# check_box 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CheckBox`
- 开源母本：`WPF UI`
- 对应组件名：`CheckBox`
- 本轮保留语义：`standard / compact / read only / focused / checked`
- 本轮移除内容：页面级 guide、状态说明文案、preview 快照切换、旧录制轨道里的额外收尾态
- EGUI 适配说明：继续复用 SDK `checkbox` 的基础点击语义，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`check_box` 用来表达某个布尔选项当前是否被选中，适合通知订阅、离线同步、权限确认、同意条款和批量选择等场景。它和 `toggle_button` 的区别在于这里强调的是表单字段语义，而不是按钮化命令入口。

## 2. 为什么现有控件不够用

- `toggle_button` 更接近按钮语义，视觉重心也更像命令触发器。
- `switch` 更偏即时开关轨道，不是标准复选框字段。
- `radio_button` 表达的是互斥选择，不适合独立布尔字段。
- 仓库里当前 `check_box` 页面虽然已经 reference 化，但录制轨道、静态 preview 单测和 README 仍保留旧 workflow，没有真正收口到当前 static preview 模板。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `check_box` -> 底部 `compact / read only` 双静态 preview。
- 主区只保留真实 `CheckBox` 的触摸勾选与键盘 `Space / Enter` 切换。
- 底部 `compact` preview 固定显示 `Auto` 且保持选中，不再承担快照切换职责。
- 底部 `read only` preview 固定显示 `Accepted` 且保持选中，作为只读静态对照。
- 两个 preview 都通过 `hcw_check_box_override_static_preview_api()` 收口：
  - 吞掉新增 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed`
  - 不改 `text / checked / region_screen / palette / font / mark_style`

目标目录：`example/HelloCustomWidgets/input/check_box/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 158`
- 主控件：`196 x 34`
- 底部对照行：`216 x 28`
- `compact` preview：`104 x 28`
- `read only` preview：`104 x 28`

视觉约束：

- 使用浅色 page panel、轻边框和低噪音勾选填充，不回退到 showcase 式装饰壳。
- 主控件保留轻量 focus ring，不叠加厚重阴影。
- `compact` 只压缩尺寸和间距，不改变复选框语义。
- `read only` 保留勾选结果展示，但不承担真实输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 158` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Check Box` | 页面标题 |
| `control_primary` | `egui_view_checkbox_t` | `196 x 34` | `Email alerts` / unchecked | 主控件 |
| `control_compact` | `egui_view_checkbox_t` | `104 x 28` | `Auto` / checked | 紧凑静态 preview |
| `control_read_only` | `egui_view_checkbox_t` | `104 x 28` | `Accepted` / checked | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Email alerts` / unchecked | 默认态 |
| 主控件 | `Email alerts` / checked | 触摸勾选后 |
| 主控件 | `Offline sync` / unchecked | 第二组主快照 |
| 主控件 | `Offline sync` / checked | `Enter` 切换后与最终稳定帧 |
| `compact` preview | `Auto` / checked | 全程静态对照 |
| `read only` preview | `Accepted` / checked | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` same-target release、`Space` 和 `Enter` 键盘切换闭环。
- `set_checked()`、`set_text()`、`set_mark_style()`、`set_mark_icon()` 与 `set_icon_font()` 必须在切换时清理残留 `pressed`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例必须验证：
  - `text / checked / font / icon_font / mark_style` 不变
  - `region_screen / palette / text_gap / alpha / background` 不变
  - `g_checked_count == 0` 且 `g_last_checked == 0xFF`
  - `is_pressed` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Email alerts` 未选中，同时恢复底部 `compact / read only` 静态 preview，并输出首帧。
2. 触摸点击主控件，把 `Email alerts` 切到已选中。
3. 输出触摸后的主区截图。
4. 切换到第二组主快照 `Offline sync` 未选中。
5. 输出第二组主快照截图。
6. 发送 `Enter`，把 `Offline sync` 切到已选中。
7. 输出键盘切换后的主区截图。
8. 保持 `Offline sync` 已选中不变，作为尾帧稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生状态变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持静态一致。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/check_box PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/check_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/check_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_check_box
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Email alerts unchecked / checked` 与 `Offline sync unchecked / checked` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `text / checked / region_screen / palette / font / mark_style`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_check_box/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前只覆盖标准二态 `CheckBox`，不扩展三态 `indeterminate`。
- 不做富文本标签、嵌入链接或多列表单布局。
- 页面保持最小 reference 结构，不额外承载说明行或描述文本。

## 13. 与现有控件的边界

- 相比 `toggle_button`：这里保留表单字段语义，而不是按钮式命令语义。
- 相比 `switch`：这里强调复选框视觉与清单场景，而不是立即生效的开关轨道。
- 相比 `radio_button`：这里支持独立布尔值，不承担互斥组职责。

## 14. EGUI 适配时的简化点与约束

- 继续复用 SDK `checkbox` 的基础绘制和 same-target release 触摸语义，避免重复实现底层状态机。
- 样式、键盘闭环和静态 preview 语义全部收口在 custom 层，不改 `sdk/EmbeddedGUI`。
- 主控件保留最小必要的真实触摸和键盘闭环，preview 不再承担切换或收尾职责。
- 先完成 reference 级 `CheckBox` 收口，再决定是否补充三态或附带描述文本版本。
