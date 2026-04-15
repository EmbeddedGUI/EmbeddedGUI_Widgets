# radio_buttons 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI RadioButtons`
- 开源母本：`WPF UI`
- 对应组件名：`RadioButtons`
- 本轮保留语义：`single choice / focused / compact / read only / static preview`
- 本轮移除内容：旧页面里的 heading、summary、note、preview 说明文案、preview 快照轮换、键盘驱动录制切换和额外收尾态
- EGUI 适配说明：继续使用当前目录下的 `egui_view_radio_buttons` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，把 reference 页面、录制轨道、README 和单测统一收口到 static preview 工作流

## 1. 为什么需要这个控件
`radio_buttons` 用于表达“一组互斥选项里始终只保留一个当前值”的标准表单语义，适合通知渠道、同步频率、主题模式这类单选但需要整组呈现的配置场景。它强调组内切换和最终选中结果，而不是单个布尔开关。

## 2. 为什么现有控件不够用
- `radio_button` 更偏单项控件，reference 页面仍需要外层手工组装和整组键盘行为收口。
- `segmented_control` 更偏导航式切换，不等价于表单里的 `RadioButtons` 组选项。
- 如果继续在页面层手工拼装，会把 same-target release、键盘组内切换、静态 preview 输入抑制和布局约束分散到 demo 代码里，难以复用和验证。

因此继续保留 `input/radio_buttons` reference 控件，但页面结构和录制轨道需要和当前 input 主线的 static preview 工作流对齐。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `radio_buttons` -> 底部 `compact / read only` 双静态 preview。
- 主区保留 3 组 reference 状态：
  - `Email / Push / SMS`，选中 `Email`
  - `Daily / Weekly / Monthly`，选中 `Weekly`
  - `Auto / Light / Dark`，选中 `Dark`
- 底部 `compact` preview 固定显示 `Auto / Manual`，选中 `Auto`。
- 底部 `read only` preview 固定显示 `Desktop / Tablet`，选中 `Tablet`。
- 两个 preview 都通过 `egui_view_radio_buttons_override_static_preview_api()` 收口，只作为静态样式对照，不参与录制轮换。

目标目录：`example/HelloCustomWidgets/input/radio_buttons/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 196`
- 主区：`196 x 90`
- 底部对照行：`216 x 60`
- 单个 preview：`104 x 60`

视觉约束：
- 页面保持浅色圆角底板和低噪音布局，避免额外说明性 chrome 干扰主区状态。
- 主区只保留 `RadioButtons` 本身的面板、选中态和 focus ring，不再叠加外层引导文案。
- runtime 轨道里只允许主区 reference 状态变化，底部 `compact / read only` preview 必须全程静态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 196` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `RadioButtons` | 页面标题 |
| `primary_widget` | `egui_view_radio_buttons_t` | `196 x 90` | `Email` 选中 | 主交互区 |
| `compact_widget` | `egui_view_radio_buttons_t` | `104 x 60` | `Auto` 选中 | 紧凑静态 preview |
| `read_only_widget` | `egui_view_radio_buttons_t` | `104 x 60` | `Tablet` 选中 | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主区 | `Email / Push / SMS` | 默认 reference 状态 |
| 主区 | `Daily / Weekly / Monthly` | 第 2 组 reference 状态 |
| 主区 | `Auto / Light / Dark` | 第 3 组 reference 状态，也是最终稳定态 |
| `compact` preview | `Auto / Manual` | 全程静态，不参与录制切换 |
| `read only` preview | `Desktop / Tablet` | 全程静态，不参与录制切换 |

## 7. 交互语义与单测口径
- 主区保留标准 `RadioButtons` 语义：
  - 触摸 `DOWN(inside) -> UP(inside)` 才提交选中。
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交。
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才重新提交。
  - `ACTION_CANCEL` 只清理 `pressed`，不改当前选中项。
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter` 走 `dispatch_key_event()` 闭环。
- `set_items()`、`set_current_index()`、`set_font()`、`set_compact_mode()`、`set_read_only_mode()` 和 `set_palette()` 都必须清理残留 `pressed`。
- 静态 preview 用例统一收口为 `test_radio_buttons_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `on_selection_changed`
  - `items`
  - `font`
  - `surface_color`
  - `border_color`
  - `text_color`
  - `muted_text_color`
  - `accent_color`
  - `item_count`
  - `current_index`
  - `compact_mode`
  - `read_only_mode`
  - `pressed_index`
  - `alpha`
  - `g_listener_count == 0`
  - `g_listener_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE`
  - `is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：
1. 恢复主区默认 `Email / Push / SMS`，同步底部双静态 preview，并请求首帧截图。
2. 程序化切换到 `Daily / Weekly / Monthly`，请求第 2 组主区截图。
3. 程序化切换到 `Auto / Light / Dark`，请求第 3 组主区截图。
4. 保持最终状态不变，等待最终稳定帧。
5. 请求最终稳定帧。

录制过程中不再通过键盘驱动主区切换，也不再让底部 preview 参与轮换。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_buttons PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_buttons --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_buttons
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_buttons
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Email / Push / SMS`、`Daily / Weekly / Monthly`、`Auto / Light / Dark` 3 组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到 `touch / key` 后，不能改写 `region_screen / on_selection_changed / items / font / palette / item_count / current_index / compact_mode / read_only_mode / pressed_index / alpha`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_radio_buttons/default`
- 截图帧数：`8`
- 主区变化边界：`(54, 142) - (425, 231)`
- 主区唯一状态数：`3`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁切起点：`y >= 232`

复核结论：
- 按 RGB 差分得到主区变化边界位于 `(54, 142) - (425, 231)`；遮罩该边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `3` 组唯一状态，符合 `Email / Push / SMS`、`Daily / Weekly / Monthly`、`Auto / Light / Dark` 三态轨道。
- 按 `y >= 232` 裁剪底部 preview 区域后全部帧保持单哈希，确认底部 `compact / read only` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前版本只保留最小 `RadioButtons` reference 页面，不扩展标题副文本、描述区和组合式表单场景。
- 底部 `compact / read only` preview 只承担静态对照，不承载真实交互职责。
- 当前不做 hover、系统主题联动和更复杂的 item 模板。

## 13. 与现有控件的边界
- 相比 `radio_button`：这里强调整组互斥选择和组内键盘导航，而不是单项控件拼装。
- 相比 `check_box`：这里不支持多选，始终只保留一个选中项。
- 相比 `segmented_control`：这里保留表单字段语义，而不是胶囊式导航切换。

## 14. EGUI 适配时的简化点与约束
- 基于当前 `egui_view_radio_buttons` custom view 做 reference 收口，不下沉到框架层。
- 本轮补齐 `RADIO_BUTTONS_RECORD_FINAL_WAIT`、`RADIO_BUTTONS_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`ui_ready`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_widget()` 和 `request_page_snapshot()`，让主区轨道和底部静态 preview 共享同一套页面恢复路径。
- runtime 只导出主区三态和最终稳定帧，底部 preview 全程保持单一静态哈希。
