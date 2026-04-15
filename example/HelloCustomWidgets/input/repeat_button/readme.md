# RepeatButton 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI RepeatButton`
- 开源母本：`WPF UI`
- 对应组件名：`RepeatButton`
- 本轮保留语义：`standard / compact / disabled visual reference / focused / press-and-hold repeat / static preview`
- 本轮移除内容：旧主面板说明文案、preview 说明标签、preview 快照轮换、键盘驱动录制长按、额外收尾态
- EGUI 适配说明：继续使用当前目录下的 `egui_view_repeat_button` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，把 reference 页面、README、录制轨道和单测统一收口到 static preview 工作流

## 1. 为什么需要这个控件
`RepeatButton` 用于表达“按下立即触发一次，继续按住则持续重复触发”的标准命令语义，适合音量步进、数值加减、滚动微调和列表连续翻页这类离散但连续可重复的操作入口。

仓库里已有普通 `button`，但还缺少一个明确承接 press-and-hold repeat 语义、带独立 reference 页面、README、单测和 Web 链路的 `RepeatButton`。

## 2. 为什么现有控件不够用
- `button` 只表达单次点击，不表达按住连发。
- `toggle_button` 表达状态切换，不适合连续步进。
- `slider` 适合连续拖拽，不适合离散重复命令。
- 如果继续在 demo 页用普通 `button` 手工拼 repeat timer，会把触摸移出/移回、键盘长按、静态 preview 输入吞掉和 attach/detach timer 恢复语义分散在页面层，难以复用和验证。

因此继续保留 `input/repeat_button` reference 控件，但页面结构和录制轨道要与当前 input 主线的 static preview 工作流对齐。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `repeat_button` -> 底部 `compact / disabled` 双静态 preview。
- 主区保留 3 组 reference 状态：
  - `Volume 12`
  - `Volume 15`
  - `Volume 18`
- 底部 `compact` preview 固定显示 `Fast`。
- 底部 `disabled` preview 固定显示 `Locked`。
- 两个 preview 都通过 `egui_view_repeat_button_override_static_preview_api()` 收口，只作为静态样式对照，不参与录制轮换。

目标目录：`example/HelloCustomWidgets/input/repeat_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主按钮：`160 x 40`
- 底部对照行：`200 x 32`
- 单个 preview：`96 x 32`

视觉约束：
- 页面保持浅色圆角底板和低噪音布局，不再保留旧页面里的说明型 chrome。
- 主区只保留标准 `RepeatButton` 本体、图标和 focus ring。
- runtime 轨道里只允许主区 `Volume 12 / 15 / 18` 变化，底部 `compact / disabled` preview 必须全程静态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 128` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `RepeatButton` | 页面标题 |
| `primary_widget` | `egui_view_repeat_button_t` | `160 x 40` | `Volume 12` | 主交互区 |
| `compact_widget` | `egui_view_repeat_button_t` | `96 x 32` | `Fast` | 紧凑静态 preview |
| `disabled_widget` | `egui_view_repeat_button_t` | `96 x 32` | `Locked` | 禁用视觉静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主区 | `Volume 12` | 默认 reference 状态 |
| 主区 | `Volume 15` | 第 2 组 reference 状态 |
| 主区 | `Volume 18` | 第 3 组 reference 状态，也是最终稳定态 |
| `compact` preview | `Fast` | 全程静态，不参与录制轮换 |
| `disabled` preview | `Locked` | 全程静态，不参与录制轮换 |

## 7. 交互语义与单测口径
- 主区继续保留真实 `RepeatButton` 语义：
  - `ACTION_DOWN(inside)` 立即触发一次 click，并启动 repeat timer。
  - `ACTION_MOVE(outside)` 停止 timer，清理 pressed。
  - `ACTION_MOVE(back inside)` 恢复 pressed 并重启 timer，但不额外追加一次立即 click。
  - `ACTION_UP / ACTION_CANCEL` 停止 timer，清理 pressed。
  - 键盘 `Space / Enter` 走完整 `dispatch_key_event()` 闭环；`KEY_DOWN` 立即 click，`KEY_UP` 停止 timer。
- 页面里的真实 click 行为仍然存在：`on_primary_click()` 把主值按 `1` 递增，直到 `99` 封顶。
- 为避免 `egui_view_repeat_button_set_text()` 清理 pressed/timer，页面更新主区文本时直接调用 `egui_view_label_set_text(EGUI_VIEW_OF(&primary_widget), ...)`，保证真实 repeat 语义不被 setter 打断。
- 录制轨道不再真实驱动触摸长按或键盘长按，而是程序化导出 `Volume 12 / 15 / 18` 三态，再追加最终稳定帧；真实 repeat 语义由 `HelloUnitTest` 覆盖。
- 静态 preview 用例统一收口为 `test_repeat_button_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `text`
  - `icon`
  - `font`
  - `icon_font`
  - `background`
  - `color`
  - `alpha`
  - `icon_text_gap`
  - `on_click_listener`
  - `initial_delay_ms`
  - `repeat_interval_ms`
  - `g_click_count == 0`
  - `is_pressed`、`touch_active`、`key_active` 与 timer 均被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：
1. 恢复主区默认 `Volume 12`，同步底部双静态 preview，并请求首帧截图。
2. 程序化切到 `Volume 15`，请求第 2 组主区截图。
3. 程序化切到 `Volume 18`，请求第 3 组主区截图。
4. 保持最终状态不变，等待最终稳定帧。
5. 请求最终稳定帧。

录制过程中不再通过真实长按驱动主区状态，也不再让底部 preview 参与轮换。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Volume 12`、`Volume 15`、`Volume 18` 3 组可识别状态。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到 `touch / key` 后，不能改写 `region_screen / text / icon / font / icon_font / background / color / alpha / icon_text_gap / on_click_listener / initial_delay_ms / repeat_interval_ms`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_repeat_button/default`
- 截图帧数：`8`
- 主区变化边界：`(82, 173) - (397, 228)`
- 主区唯一状态数：`3`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁切起点：`y >= 245`

复核结论：
- 按主按钮实际占位裁剪，主区边界位于 `(82, 173) - (397, 228)`；遮罩该边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `3` 组唯一状态，对应 `Volume 12`、`Volume 15`、`Volume 18` 三态轨道。
- 按 `y >= 245` 裁剪底部 preview 区域后全部帧保持单哈希，确认底部 `compact / disabled` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前版本只保留最小 `RepeatButton` reference 页面，不扩展旧 demo 里的说明性面板、状态摘要或场景化包装。
- 底部 `compact / disabled` preview 只承担静态对照，不承载真实交互职责。
- runtime 录制轨道只验证主区三态与底部静态 preview，不在截图轨道里直接表现真实长按计时过程。

## 13. 与现有控件的边界
- 相比 `button`：这里强调按下立即触发并支持按住重复，而不是一次性点击。
- 相比 `toggle_button`：这里没有持久选中态。
- 相比 `slider`：这里不承担连续拖拽输入。
- 相比 `number_box`：这里不负责文本输入和解析。

## 14. EGUI 适配时的简化点与约束
- 基于现有 `egui_view_repeat_button` custom view 做 `reference` 收口，不下沉到框架层。
- 本轮补齐 `REPEAT_BUTTON_RECORD_FINAL_WAIT`、`REPEAT_BUTTON_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`ui_ready`、`apply_primary_snapshot()`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_widget()` 和 `request_page_snapshot()`，让主区轨道与底部静态 preview 共享同一套页面恢复路径。
- runtime 只导出主区三态和最终稳定帧，底部 preview 全程保持单一静态哈希。
