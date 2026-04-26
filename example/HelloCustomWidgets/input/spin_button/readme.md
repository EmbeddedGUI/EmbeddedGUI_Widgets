# Spin Button 控件说明

## 控件定位

`spin_button` 对齐 Fluent 2 / Fluent UI React 的 `SpinButton` 语义：在一个数值字段旁提供上 / 下步进按钮，用于离散数值的快速微调。它适合列数、间距、延迟、数量这类需要范围、步长和单位后缀的设置项。

现有控件不完全覆盖该语义：
- `NumberBox` 更强调可编辑数值框，允许用户直接输入或配合数值校验。
- `Slider` 面向连续拖动，不适合明确表达每次增加或减少一个固定步长。
- `TextBox` 是自由文本输入，不自带范围、步长、上下步进按钮和数值提交语义。

## 目标场景

示例页面保留三个层级：
- 主 `SpinButton`：`Columns` 字段，范围 `2` 到 `12`，步长 `2`，显示 `cols` 后缀。
- 底部 `compact` preview：固定显示 `8 px`，用于验证紧凑态布局。
- 底部 `read only` preview：固定显示 `16 ms`，用于验证只读态和静态预览。

目录：`example/HelloCustomWidgets/input/spin_button/`

## 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 156`
- 主控件：`196 x 72`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 风格约束：浅色 Fluent 面板、白色字段、低噪声边框、右侧上下堆叠 stepper、清晰的焦点和 pressed 反馈。

## 状态矩阵

| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `6 cols` | `8 px` | `16 ms` |
| 触控增加 | `8 cols` | 保持不变 | 保持不变 |
| 键盘增加 | `10 cols` | 保持不变 | 保持不变 |
| 键盘 End | `12 cols` | 保持不变 | 保持不变 |
| 触控减少 | `10 cols` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到默认 `6 cols` | 保持不变 | 保持不变 |

## 交互语义

- 触控只在 `DOWN(A) -> UP(A)` 或 `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 时提交。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，避免滑出后的误触发。
- `ACTION_CANCEL`、`compact`、`read only`、`disabled` 都会清理 `active_part / is_pressed`。
- 键盘覆盖 `Up / Down / Home / End / Left / Right / Enter / Space`：方向键切换步进或焦点，`Home / End` 跳到范围边界，`Enter / Space` 激活当前 stepper。
- 静态 preview 覆盖 touch / key 输入但不改变数值、焦点、字体、颜色或布局状态。

## 单测覆盖

`example/HelloUnitTest/test/test_spin_button.c` 覆盖：
1. `range / value` clamp 和 listener 边界。
2. `step / large_step` 归一化。
3. setter 清理 active / pressed 状态。
4. 文本、字体、调色板、compact、read only 配置。
5. `adjust()` 在交互态和 guard 态下的返回值。
6. 触控增减、字段焦点、same-target release 和 cancel。
7. 键盘导航与 `Enter / Space` 激活。
8. `compact / read only / disabled` 忽略输入并清理状态。
9. static preview 消费输入且保持状态不变。

## 录制动作

`egui_port_get_recording_action()` 的 reference 轨道顺序：
1. 恢复默认 `6 cols`，请求首帧截图。
2. 触控点击 increment，进入 `8 cols`。
3. 请求触控增加后的截图。
4. 键盘 `Up`，进入 `10 cols`。
5. 请求键盘增加后的截图。
6. 键盘 `End`，进入 `12 cols`。
7. 请求范围上限截图。
8. 触控点击 decrement，进入 `10 cols`。
9. 请求减少后的截图。
10. 恢复默认 `6 cols`。
11. 请求最终稳定帧。

## 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=input/spin_button PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe spin_button
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/sync_widget_catalog.py --check
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/spin_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/spin_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_spin_button
```

## 参考体系

- Reference system：`Fluent 2`
- Reference library：`Fluent UI React`
- Reference component：`SpinButton`

## EGUI 适配说明

- 当前实现停留在 `HelloCustomWidgets` reference 层，不修改 `sdk/EmbeddedGUI`。
- 采用 `int16_t` 范围和值，保留 `large_step` API 作为未来 PageUp / PageDown 或加速步进扩展点。
- 文本显示为 `value + suffix`，不实现自由编辑、错误消息、文本选择或复杂格式化。
- 主体仅保留标准数值步进、焦点、pressed、只读、禁用和静态 preview，移除额外装饰动画和场景化说明。
