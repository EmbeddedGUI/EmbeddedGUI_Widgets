# button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`Button`
- 本次保留状态：`standard`、`compact`、`disabled`、`focused`
- 删除效果：页面级说明文案、额外标签、section divider、showcase 式装饰容器
- EGUI 适配说明：复用 SDK `button` 的点击语义与图标文本排版，只在 custom 层补齐 Fluent 风格背景、`Space / Enter` 键盘闭环、setter `pressed` 清理与静态 preview 输入吞掉

## 1. 为什么需要这个控件？
`button` 是最基础的命令触发控件，用于提交、确认、同步、跳转等一次性动作。`HelloCustomWidgets` 目前缺少一个与 `Fluent 2 / WPF UI` 主线一致的 `Button` reference 页面，因此需要补齐。

## 2. 为什么现有控件不够用？
- `toggle_button` 表达的是开关状态，不适合一次性命令触发。
- `split_button` 与 `drop_down_button` 带有二级动作或菜单，不是最小语义按钮。
- SDK 已有基础 `button`，但当前仓库还没有面向 reference 主线的 `input/button` 页面与单测闭环。

## 3. 目标场景与示例概览
- 主区域展示一个可交互的标准按钮，保留真实点击与 `Space / Enter` 键盘触发。
- 左下 `compact` preview 展示更紧凑的次级尺寸。
- 右下 `disabled` preview 展示禁用态视觉参考，但作为静态 preview 不承担真实输入职责。
- 页面只保留标题、一个主按钮和底部两个 preview。

目录：
- `example/HelloCustomWidgets/input/button/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 128`
- 页面结构：标题 -> 主 `button` -> `compact / disabled` 双 preview
- 主按钮尺寸：`140 x 40`
- 底部 preview 行：`200 x 32`
- 单个 preview：`96 x 32`

视觉约束：
- 标准态使用 Fluent 主按钮蓝色底。
- `compact` 保留按钮语义，仅压缩尺寸和间距。
- `disabled` 使用低对比浅灰视觉，不承载可交互语义。
- 焦点态仅保留轻量 ring，不引入厚重阴影。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 128` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Button` | 页面标题 |
| `button_primary` | `egui_view_button_t` | `140 x 40` | enabled | 主按钮 |
| `button_compact` | `egui_view_button_t` | `96 x 32` | static preview | 紧凑预览 |
| `button_disabled` | `egui_view_button_t` | `96 x 32` | static preview | 禁用态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主按钮 | Compact | Disabled |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 点击提交 | 是 | 否 | 否 |
| same-target release | 是 | 否 | 否 |
| 键盘 `Space / Enter` | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focused | 是 | 否 | 否 |

## 7. 交互语义
- `Enter / Space`：按下进入 `pressed`，抬起时触发 click listener。
- `DOWN(A) -> MOVE(B) -> UP(B)`：不提交点击。
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)`：回到原目标后才提交。
- 主按钮继续复用 SDK `button` 的默认触摸 same-target release 语义，不改 `sdk/EmbeddedGUI`。
- 底部两个 preview 通过 `hcw_button_override_static_preview_api()` 统一吞掉 `touch / key`，同时清理残留 `pressed`。

## 8. 本轮收口内容
- 新增 `egui_view_button.h/.c`，作为 SDK `button` 的 Fluent reference 包装层。
- 在包装层补齐：
  - `standard / compact / disabled` 样式 helper
  - `set_text()`、`set_icon()`、`set_icon_font()`、`set_icon_text_gap()` 的 `pressed` 清理
  - `Space / Enter` 键盘 click 闭环
  - 静态 preview 输入吞掉
- 新增 `test.c` reference 页面，只保留主按钮与双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、setter 清理、same-target release、键盘点击、disabled 与静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 链路：
1. 还原主按钮、`compact` 与 `disabled` 默认快照，并给主按钮 request focus。
2. 截默认态。
3. 触摸点击主按钮，切到第二组快照。
4. 截点击后状态。
5. 发送 `Space`，切到第三组快照。
6. 截键盘切换状态。
7. 把 `compact` 切到第二组预览，并发送 `Enter` 把主按钮切回第一组快照。
8. 截最终收尾帧。

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/button PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_button
```

验收重点：
- 主按钮与底部双 preview 必须完整可见，不黑屏、不白屏、不裁切。
- `Space / Enter` 必须稳定触发 click listener。
- 触摸释放只允许 same-target 提交。
- `compact / disabled` preview 必须保持静态 reference，对输入只做吞掉和焦点收尾。

## 11. 已知限制
- 当前只覆盖最小 `Button` reference，不扩展图文混排按钮组或命令栏组合。
- 不做 hover 动画与系统主题联动。
- 页面不额外承载说明性标签，避免偏离 reference 主线。

## 12. 与现有控件的差异边界
- 相比 `toggle_button`：这里是一次性命令触发，而不是状态切换。
- 相比 `split_button`：这里没有主次动作分裂。
- 相比 `drop_down_button`：这里没有展开式菜单语义。

## 13. 对应组件名，以及本次保留的核心状态
- 对应组件名：`Button`
- 本次保留状态：
  - `standard`
  - `compact`
  - `disabled`
  - `focused`

## 14. EGUI 适配时的简化点与约束
- 继续复用 SDK `button` 的图标文本排版与默认触摸点击语义。
- Fluent 风格的背景、键盘补齐和静态 preview 语义全部收口在 custom 层，不改 `sdk/EmbeddedGUI`。
- 页面保持最小 reference 结构，先确保控件级闭环，再考虑未来是否补充更复杂的按钮族页面。
