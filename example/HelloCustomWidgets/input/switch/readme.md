# switch 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI ToggleSwitch`
- 开源母本：`WPF UI`
- 对应组件名：`ToggleSwitch`
- 本轮保留语义：`standard / compact / read only / checked / unchecked / static preview`
- 本轮移除内容：旧 preview 快照轮换、preview 点击清焦桥接、额外收尾态、说明文案、外部标签、section divider、showcase 化装饰
- EGUI 适配说明：继续复用当前目录下的 `egui_view_switch` 包装层与 SDK `switch` 基础绘制，本轮只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`switch` 用来表达“状态切换后立即生效”的设置项开关，适合通知、同步、自动化与权限类场景。它强调轨道与 thumb 的二态反馈，比命令按钮更贴近设置页里的标准开关语义。

## 2. 为什么现有控件不够用
- `toggle_button` 更像页内命令按钮，不是设置行里的拨杆。
- `check_box` 更适合表单字段和清单勾选，不强调轨道式开关反馈。
- `slider` 面向连续值调节，不表达离散的 on/off 切换。
- 当前 `reference` 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleSwitch` 页面与验收闭环。

因此这里继续保留 `switch` reference 控件，但示例页必须回到统一的静态 preview 工作流。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `switch` -> 底部 `compact / read only` 双静态 preview。
- 主区保留 3 组 reference 状态：
  - `checked`，仅显示 `done` on-icon。
  - `unchecked`，保持相同 icon 资源但切到 off 轨道。
  - `unchecked`，补齐 `done / cross` on/off icon 对照。
- 主控件继续承载真实 toggle 语义，保留 `Enter / Space` 键盘闭环与 `same-target release` 触摸提交规则。
- 底部 `compact` preview 固定显示 `checked + done`，只作为紧凑样式对照，不再参与轮换。
- 底部 `read only` preview 固定显示 `checked + done`，只作为只读样式对照，不再承担交互职责。
- 两个 preview 统一通过 `hcw_switch_override_static_preview_api()` 收口。
- preview 收到 `touch` 或 `dispatch_key_event()` 后，只允许清理残留 `is_pressed`，不能改写 `icon_on / icon_off / on_checked_changed / bk_color_on / bk_color_off / switch_color_on / switch_color_off / is_checked / icon_font / region_screen / alpha`。

目标目录：`example/HelloCustomWidgets/input/switch/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 132`
- 主控件：`108 x 44`
- 底部对照行：`160 x 32`
- `compact` preview：`76 x 32`
- `read only` preview：`76 x 32`

视觉约束：
- 保持浅色 `page panel`、低噪音描边和 Fluent 风格的标准 `ToggleSwitch` 比例。
- 主区只保留轨道、thumb、icon 与 focus ring，不再叠加旧 demo 的解释性 chrome。
- runtime 轨道里只允许主区 reference 状态变化，底部 `compact / read only` preview 必须全程静态。
- 不再保留旧版 preview 轮换、preview 清焦桥接和额外收尾态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 132` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toggle Switch` | 页面标题 |
| `primary_switch` | `egui_view_switch_t` | `108 x 44` | `checked + done` | 主控件 |
| `compact_switch` | `egui_view_switch_t` | `76 x 32` | `checked + done` | 紧凑静态 preview |
| `read_only_switch` | `egui_view_switch_t` | `76 x 32` | `checked + done` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `checked + done` | 默认 reference 状态 |
| 主控件 | `unchecked + done` | 第二组 reference 状态 |
| 主控件 | `unchecked + done/cross` | 第三组 reference 状态，也是最终稳定态 |
| `compact` preview | `checked + done` | 全程静态，不参与录制轮换 |
| `read only` preview | `checked + done` | 全程静态，不参与录制轮换 |

## 7. 交互语义与单测口径
- 主控件继续保留标准 `ToggleSwitch` 语义：
  - 触摸 `DOWN(inside) -> UP(inside)` 才提交切换。
  - `DOWN(inside) -> MOVE(outside) -> UP(outside)` 不提交。
  - 只有回到原命中区后 `UP(inside)` 才重新提交。
  - 键盘 `Enter / Space` 通过完整 `ACTION_DOWN -> ACTION_UP` 闭环切换。
- `set_checked()`、`set_state_icons()`、`set_icon_font()`、样式 helper、`!enable` guard 和无关键盘输入都必须清理残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 `test_switch_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `on_checked_changed`
  - `is_checked`
  - `icon_on`
  - `icon_off`
  - `icon_font`
  - `switch_color_on`
  - `switch_color_off`
  - `bk_color_on`
  - `bk_color_off`
  - `alpha`
  - `g_checked_count == 0`
  - `g_last_checked == 0xFF`
  - `is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：
1. 恢复主控件默认 `checked + done`，同步底部双静态 preview，并请求首帧截图。
2. 程序化切到 `unchecked + done`，请求第二帧主区截图。
3. 程序化切到 `unchecked + done/cross`，请求第三帧主区截图。
4. 保持最终状态不变，等待最终稳定帧。
5. 请求最终稳定帧。

录制只允许主区发生变化。底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/switch PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/switch --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/switch
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_switch
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `checked + done`、`unchecked + done`、`unchecked + done/cross` 三组可识别状态。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `icon_on / icon_off / on_checked_changed / bk_color_on / bk_color_off / switch_color_on / switch_color_off / is_checked / icon_font / region_screen / alpha`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_switch/default`
- 截图帧数：`8`
- 主区变化边界：`(132, 168) - (347, 233)`
- 主区唯一状态数：`3`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁切起点：`y >= 234`

复核结论：
- 遮罩主区变化边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `3` 组唯一状态，对应 `checked + done`、`unchecked + done`、`unchecked + done/cross` 三态轨道。
- 按 `y >= 234` 裁切底部 preview 区域后全部帧保持单哈希，确认底部 `compact / read only` preview 全程静态一致。

## 12. 已知限制
- 当前版本只保留最小 `ToggleSwitch` 参考页，不扩展带说明文案的设置行组合。
- 底部 `compact / read only` preview 只承担静态对照，不承载真实交互职责。
- 当前不做 hover、系统主题联动和更复杂的页面级设置表单包装。

## 13. 与现有控件的边界
- 相比 `toggle_button`：这里是设置页开关，不是页内命令按钮。
- 相比 `check_box`：这里强调轨道与 thumb 的二态反馈，而不是表单勾选。
- 相比 `slider`：这里是离散开关，不提供连续值调节。

## 14. EGUI 适配时的简化点与约束
- 基于现有 `egui_view_switch` 包装层做 `reference` 收口，不下沉到框架层。
- 主控件继续复用 SDK `switch` 的基础 touch/绘制逻辑，只在 custom 层统一样式、键盘与静态 preview 语义。
- 本轮补齐 `PRIMARY_SNAPSHOT_COUNT`、`SWITCH_RECORD_FINAL_WAIT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_switch()` 和 `request_page_snapshot()`，让主区轨道、底部静态 preview 与最终稳定帧共享同一套页面恢复路径。
