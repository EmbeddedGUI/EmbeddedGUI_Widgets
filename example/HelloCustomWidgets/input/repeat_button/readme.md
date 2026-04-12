# RepeatButton 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`RepeatButton`
- 本次保留语义：`standard`、`compact`、`disabled visual reference`、`press-and-hold repeat`
- 删除内容：复杂页面级说明区、Hover 装饰动画、真实命令栏包装、SDK 层改动
- EGUI 适配说明：在 `custom` 层新增自包含的 `egui_view_repeat_button`，保持 SDK `button` 的绘制入口，只补齐按下即触发、按住连发、静态 preview 输入吞掉与 attach/detach timer 生命周期

## 1. 为什么需要这个控件？
`RepeatButton` 适合音量、数值步进、滚动微调这类“按下立即执行一次，按住继续重复执行”的命令入口。当前仓库已经有普通 `button`，但还缺少一个明确表达持续触发语义的 `RepeatButton` reference 控件。

## 2. 为什么现有控件不够用？
- `button` 只表达单次点击，不表达按住连发。
- `toggle_button` 表达状态切换，不适合连续步进。
- `slider` 适合连续拖拽，不适合离散步进。
- SDK 基础 `button` 没有仓库级的 `RepeatButton` wrapper、README、demo、单测和 Web 链路。

## 3. 目标场景与示例概览
- 主区域展示一个可真实交互的 `RepeatButton`，驱动 `Volume 12 -> ...` 的步进值。
- 左下 `compact` preview 展示紧凑样式。
- 右下 `disabled` preview 展示禁用视觉参考。
- 录制轨道覆盖：
  - 初始态
  - 触摸按住后的重复态
  - 键盘 `Space` 按住后的重复态

目录：
- `example/HelloCustomWidgets/input/repeat_button/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 252`
- 主面板：`196 x 124`
- 主控件：`140 x 40`
- 底部容器：`216 x 82`
- 单个 preview 卡片：`104 x 82`
- preview 控件：`84 x 32`

视觉约束：
- 页面保持浅灰背景与低噪音白色卡片。
- 主按钮沿用 Fluent 蓝色主动作语义。
- `compact` 只压缩尺寸与间距，不改变重复语义。
- `disabled` preview 只做视觉参考，不承担真实输入职责。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 (W x H) | 用途 |
| --- | --- | ---: | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 124` | 主展示面板 |
| `primary_widget` | `egui_view_repeat_button_t` | `140 x 40` | 主 RepeatButton |
| `compact_widget` | `egui_view_repeat_button_t` | `84 x 32` | 紧凑静态 preview |
| `disabled_widget` | `egui_view_repeat_button_t` | `84 x 32` | 禁用视觉静态 preview |

## 6. 状态覆盖矩阵
| 状态 / 区域 | 主控件 | Compact | Disabled |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 按下立即 click | 是 | 否 | 否 |
| 长按重复 | 是 | 否 | 否 |
| 触摸移出再移回恢复 repeat | 是 | 否 | 否 |
| `Space / Enter` 按住重复 | 是 | 否 | 否 |
| 静态 preview 输入吞掉 | 否 | 是 | 是 |

说明：
- `RepeatButton` 与普通 `button` 不同，按下即触发第一次 click，不等到抬起。
- 触摸移出命中区时停止重复；移回命中区时重新启动 timer，但不会额外追加一次立即 click。

## 7. 交互语义
- `ACTION_DOWN(inside)`：立即触发一次 `egui_view_perform_click()`，并启动 repeat timer。
- `ACTION_MOVE(outside)`：停止 timer，`is_pressed = 0`。
- `ACTION_MOVE(back inside)`：恢复 `is_pressed = 1` 并重启 timer，但不额外 click。
- `ACTION_UP / ACTION_CANCEL`：停止 timer，清理 pressed。
- `Space / Enter`：
  - `KEY_DOWN` 立即 click 一次并启动 timer
  - `KEY_UP` 停止 timer，不追加 click
- 未处理 key、静态 preview 输入、失能态输入都会清理残留 pressed/timer。

默认 repeat 时序：
- `initial_delay_ms = 360`
- `repeat_interval_ms = 90`
- 最小 clamp：
  - `delay >= 80`
  - `interval >= 40`

## 8. 本轮收口内容
- 新增 `egui_view_repeat_button.h/.c`
- 提供：
  - `egui_view_repeat_button_init()`
  - `egui_view_repeat_button_apply_standard_style()`
  - `egui_view_repeat_button_apply_compact_style()`
  - `egui_view_repeat_button_apply_disabled_style()`
  - `egui_view_repeat_button_set_text()`
  - `egui_view_repeat_button_set_icon()`
  - `egui_view_repeat_button_set_font()`
  - `egui_view_repeat_button_set_icon_font()`
  - `egui_view_repeat_button_set_icon_text_gap()`
  - `egui_view_repeat_button_set_repeat_timing()`
  - `egui_view_repeat_button_override_static_preview_api()`
- 新增 `test.c` reference 页面，展示主控件与两个 preview。
- 新增 `HelloUnitTest` 单测，覆盖默认值、setter 清理、触摸/键盘重复、attach/detach、disabled guard 和静态 preview。

## 9. 录制动作设计
1. 重置为 `Volume 12`
2. 抓取初始帧
3. 触摸按下主控件并保持
4. 等待一个完整 repeat 周期后抓取长按帧
5. 释放触摸
6. 键盘 `Space` 按下并保持
7. 再次抓取键盘重复帧
8. 释放按键并收尾

## 10. 编译、测试与 runtime 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/repeat_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/repeat_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_repeat_button
```

验收重点：
- 主控件与两个 preview 必须完整可见，不黑屏、不白屏、不裁切。
- 主控件必须明确表现“按下即触发，再继续重复”。
- 触摸移出/移回时，不能出现额外多点一次的错误语义。
- `compact / disabled` preview 必须保持静态 reference，不接管真实交互。

## 11. 已知限制
- 当前只覆盖 `RepeatButton` 的最小 reference 语义，不扩展成通用数值框或滚动容器。
- 当前不做 Hover 动画、主题联动和复杂命令栏拼装。
- 当前 demo 只展示“增加”场景，其他业务含义由宿主通过 click listener 决定。

## 12. 与现有控件的差异边界
- 相比 `button`：这里强调按住重复，而不是一次性触发。
- 相比 `toggle_button`：这里没有选中态持久化。
- 相比 `slider`：这里不承担连续拖拽输入。
- 相比 `number_box`：这里不负责文本输入和解析。

## 13. 对应组件名，以及本次保留的核心状态
- 对应组件名：`RepeatButton`
- 本次保留状态：
  - `standard`
  - `compact`
  - `disabled visual reference`
  - `pressed`
  - `press-and-hold repeat`

## 14. EGUI 适配时的简化点与约束
- 不修改 `sdk/EmbeddedGUI`，只在仓库 `custom` 层新增 wrapper。
- 为了保证 `HelloUnitTest` 可直接 `#include .c` 编译，本实现不依赖 custom `button` 的私有实现，而是在本控件内自包含背景与 repeat 逻辑。
- runtime 与 Web 页面共用同一套 `test.c` 结构，保持 reference 链路一致。
