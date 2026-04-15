# radio_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI RadioButton`
- 开源母本：`WPF UI`
- 对应组件名：`RadioButton`
- 本轮保留语义：`standard / compact / read only / focused / selected / static preview`
- 本轮移除内容：旧 preview 快照轮换、preview 清焦桥接、键盘驱动录制切换、额外收尾态、说明文案、场景化装饰
- EGUI 适配说明：继续复用当前目录下的 `egui_view_radio_button` 包装层与 SDK `radio_button` 基础绘制，本轮只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`radio_button` 用于表达“多个候选项中只能选一个”的标准表单语义，适合通知方式、同步策略、频率模式和终端形态这类互斥配置。它强调组内互斥和最终选择结果，而不是单个布尔开关。

## 2. 为什么现有控件不够用
- `check_box` 面向独立布尔字段，不承接组内互斥选择。
- `toggle_button` 更接近命令式按钮，不适合作为表单里的单选项。
- `segmented_control` 强调分段切换和导航表现，不等价于标准 `RadioButton`。

因此这里继续保留 `input/radio_button` reference 控件，但示例页必须收口到仓内统一的 static preview 工作流。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主区 3 个 `radio_button` -> 底部 `compact / read only` 双静态 preview。
- 主区保留 3 组 reference 状态：
  - `Home / Alerts / Privacy`，选中第 1 项
  - `Email / Push / SMS`，选中第 2 项
  - `Daily / Weekly / Monthly`，选中第 3 项
- 底部 `compact` preview 固定显示 `Auto / Manual`，选中 `Auto`。
- 底部 `read only` preview 固定显示 `Desktop / Tablet`，选中 `Tablet`。
- 两个 preview 统一通过 `hcw_radio_button_override_static_preview_api()` 收口，只作为静态样式对照，不再参与录制轮换。

目标目录：`example/HelloCustomWidgets/input/radio_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主区列：`196 x 98`
- 主区单项：`196 x 30`
- 底部对照行：`216 x 52`
- 单个 preview 列：`104 x 52`
- 单个 preview 项：`104 x 24`

视觉约束：
- 保持浅色 `page panel`、低噪音描边和 Fluent 风格的标准 `RadioButton` 比例。
- 主区保留轻量 focus ring，不叠加旧 demo 的说明型 chrome。
- runtime 轨道里只允许主区 reference 状态变化，底部 `compact / read only` preview 必须全程静态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 214` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Radio Button` | 页面标题 |
| `primary_buttons[3]` | `egui_view_radio_button_t` | `196 x 30` | `Home` 选中 | 主互斥组 |
| `compact_buttons[2]` | `egui_view_radio_button_t` | `104 x 24` | `Auto` 选中 | 紧凑静态 preview |
| `read_only_buttons[2]` | `egui_view_radio_button_t` | `104 x 24` | `Tablet` 选中 | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主区 | `Home / Alerts / Privacy` | 默认 reference 状态 |
| 主区 | `Email / Push / SMS` | 第 2 组 reference 状态 |
| 主区 | `Daily / Weekly / Monthly` | 第 3 组 reference 状态，也是最终稳定态 |
| `compact` preview | `Auto / Manual` | 全程静态，不参与录制轮换 |
| `read only` preview | `Desktop / Tablet` | 全程静态，不参与录制轮换 |

## 7. 交互语义与单测口径
- 主区继续保留标准 `RadioButton` 语义：
  - 触摸 `DOWN(inside) -> UP(inside)` 才提交选中。
  - `DOWN(inside) -> MOVE(outside) -> UP(outside)` 不提交。
  - 只有回到原命中区后 `UP(inside)` 才重新提交。
  - 键盘 `Enter / Space` 通过完整 `ACTION_DOWN -> ACTION_UP` 闭环选中目标项。
- 已选中的主项再次提交时保持选中，不支持像 `check_box` 那样取消选中。
- `set_checked()`、文本 / 字体 / 图标相关 setter、样式 helper、`!enable` guard 和静态 preview 吞输入都必须清理残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直调旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 `test_radio_button_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `group`
  - `is_checked`
  - `alpha`
  - `circle_color`
  - `dot_color`
  - `text`
  - `font`
  - `text_color`
  - `text_gap`
  - `mark_style`
  - `mark_icon`
  - `icon_font`
  - `g_changed_count == 0`
  - `g_last_index == -1`
  - `is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：
1. 恢复主区默认 `Home / Alerts / Privacy`，同步底部双静态 preview，并请求首帧截图。
2. 程序化切到 `Email / Push / SMS`，请求第 2 组主区截图。
3. 程序化切到 `Daily / Weekly / Monthly`，请求第 3 组主区截图。
4. 保持最终状态不变，等待最终稳定帧。
5. 请求最终稳定帧。

录制只允许主区发生变化。底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Home / Alerts / Privacy`、`Email / Push / SMS`、`Daily / Weekly / Monthly` 3 组可识别状态。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `group / is_checked / alpha / circle_color / dot_color / text / font / text_color / text_gap / mark_style / mark_icon / icon_font / region_screen`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_radio_button/default`
- 截图帧数：`8`
- 主区变化边界：`(44, 106) - (435, 240)`
- 主区唯一状态数：`3`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁切起点：`y >= 241`

复核结论：
- 按 RGB 差分得到主区变化边界位于 `(44, 106) - (435, 240)`；遮罩该边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `3` 组唯一状态，对应 `Home / Alerts / Privacy`、`Email / Push / SMS`、`Daily / Weekly / Monthly` 三态轨道。
- 按 `y >= 241` 裁剪底部 preview 区域后全部帧保持单哈希，确认底部 `compact / read only` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前版本只保留最小 `RadioButton` 参考页，不扩展带副标题、说明文本或嵌入式表单布局。
- 底部 `compact / read only` preview 只承接静态对照，不承载真实交互职责。
- 当前不做 hover、系统主题联动和更复杂的设置行包装。

## 13. 与现有控件的边界
- 相比 `check_box`：这里是互斥组，只允许一个选项保持选中。
- 相比 `segmented_control`：这里保留标准表单字段语义，而不是胶囊式切换器。
- 相比 `toggle_button`：这里强调字段选择结果，而不是命令式按钮反馈。

## 14. EGUI 适配时的简化点与约束
- 基于现有 `egui_view_radio_button` 包装层做 `reference` 收口，不下沉到框架层。
- 主区继续复用 SDK `radio_button` 的基础触摸 / 绘制逻辑，只在 custom 层统一样式、键盘与静态 preview 语义。
- 本轮补齐 `RADIO_BUTTON_RECORD_FINAL_WAIT`、`RADIO_BUTTON_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_button()` 与 `request_page_snapshot()`，让主区轨道、底部静态 preview 和最终稳定帧共享同一套页面恢复路径。
