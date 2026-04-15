# toggle_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI ToggleButton`
- 开源母本：`WPF UI`
- 对应组件名：`ToggleButton`
- 本轮保留语义：`standard / compact / read only / on / off / static preview`
- 本轮移除内容：页面级 `guide`、旧 preview 快照轮换、preview 点击清焦桥接、额外收尾态、外部标签、section divider、showcase 化装饰
- EGUI 适配说明：继续复用 custom 层现有 `egui_view_toggle_button` 绘制与输入语义，本轮只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`toggle_button` 用来表达“按钮本身就是一个持久开关命令”的场景，例如提醒开关、可见性切换、收藏态和固定态。它不是设置页拨杆，也不是带菜单入口的复合命令，而是最小可用的 checked command。

## 2. 为什么现有控件不够用
- `switch` 更像设置项里的开关拨杆，不强调命令按钮语义。
- `button` 只有瞬时动作，不保留 checked 状态。
- `split_button` 与 `toggle_split_button` 都带入了额外菜单或分段入口，信息密度更高。
- 当前 `reference` 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleButton`。

因此这里继续保留 `toggle_button` reference 控件，但示例页必须回到统一的静态 preview 工作流。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `toggle_button` -> 底部 `compact / read only` 双静态 preview。
- 主区保留 3 组 reference 状态：`Alerts`、`Visible`、`Favorite`。
- 主控件继续承载真实 toggle 语义，保留 `Enter / Space` 键盘闭环与 `same-target release` 触摸提交规则。
- 底部 `compact` preview 固定显示 `Visible`，只作为紧凑样式对照，不再参与轮换。
- 底部 `read only` preview 固定显示 `Pinned`，只作为只读样式对照，不再承担交互职责。
- 两个 preview 统一通过 `hcw_toggle_button_override_static_preview_api()` 收口。
- preview 收到 `touch` 或 `dispatch_key_event()` 后，只允许清理残留 `is_pressed`，不能改写 `text / icon / on_color / off_color / corner_radius / icon_text_gap / is_toggled / region_screen / alpha`。

目标目录：`example/HelloCustomWidgets/input/toggle_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 142`
- 主控件：`196 x 52`
- 底部对照行：`216 x 44`
- `compact` preview：`104 x 44`
- `read only` preview：`104 x 44`

视觉约束：
- 保持浅色 `page panel`、低噪音描边和 Fluent 风格的单按钮 checked 表达。
- 主区只保留图标 + 文本 + on/off 填充色，不再叠加旧 demo 的解释性 chrome。
- runtime 轨道里只允许主区 reference 状态变化，底部 `compact / read only` preview 必须全程静态。
- 不再保留旧版 preview 轮换、preview 清焦桥接和额外收尾态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 142` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toggle Button` | 页面标题 |
| `button_primary` | `egui_view_toggle_button_t` | `196 x 52` | `Alerts / On` | 主控件 |
| `button_compact` | `egui_view_toggle_button_t` | `104 x 44` | `Visible / On` | 紧凑静态 preview |
| `button_read_only` | `egui_view_toggle_button_t` | `104 x 44` | `Pinned / On` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Alerts / On` | 默认 reference 状态 |
| 主控件 | `Visible / Off` | 第二组 reference 状态 |
| 主控件 | `Favorite / On` | 第三组 reference 状态，也是最终稳定态 |
| `compact` preview | `Visible / On` | 全程静态，不参与录制轮换 |
| `read only` preview | `Pinned / On` | 全程静态，不参与录制轮换 |

## 7. 交互语义与单测口径
- 主控件继续保留标准 toggle 语义：
  - 触摸 `DOWN(inside) -> UP(inside)` 才提交切换。
  - `DOWN(inside) -> MOVE(outside) -> UP(outside)` 不提交。
  - 只有回到原命中区后 `UP(inside)` 才重新提交。
  - 键盘 `Enter / Space` 通过完整 `ACTION_DOWN -> ACTION_UP` 闭环切换。
- `set_toggled()`、样式 helper、`touch cancel`、`!enable` guard、无关键盘输入都必须清理残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 `test_toggle_button_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `icon`
  - `text`
  - `font`
  - `icon_font`
  - `on_toggled`
  - `text_color`
  - `on_color`
  - `off_color`
  - `corner_radius`
  - `icon_text_gap`
  - `is_toggled`
  - `alpha`
  - `toggled_count == 0`
  - `is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：
1. 恢复主控件默认 `Alerts / On`，同步底部 `Visible / Pinned` 静态 preview，并请求首帧截图。
2. 程序化切到 `Visible / Off`，请求第二帧主区截图。
3. 程序化切到 `Favorite / On`，请求第三帧主区截图。
4. 保持 `Favorite / On` 不变，等待最终稳定帧。
5. 请求最终稳定帧。

录制只允许主区发生变化。底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toggle_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toggle_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Alerts`、`Visible`、`Favorite` 三组可识别状态。
- 底部 `Visible / Pinned` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `icon / text / on_toggled / on_color / off_color / corner_radius / icon_text_gap / is_toggled / region_screen / alpha`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_toggle_button/default`
- 截图帧数：`8`
- 主区变化边界：`(44, 160) - (435, 237)`
- 主区唯一状态数：`3`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁切起点：`y >= 238`

复核结论：
- 遮罩主区变化边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `3` 组唯一状态，对应 `Alerts`、`Visible`、`Favorite` 三态轨道。
- 按 `y >= 238` 裁切底部 preview 区后全部帧保持单哈希，确认底部 `compact / read only` preview 全程静态一致。

## 12. 已知限制
- 当前版本只保留最小 `checked command`，不覆盖 toolbar group、menu button 组合关系。
- 底部 `compact / read only` preview 只承担静态对照，不承载真实交互职责。
- 当前不做 hover、focus ring、Acrylic 和系统级主题动画。

## 13. 与现有控件的边界
- 相比 `switch`：这里是页内命令按钮，不是设置拨杆。
- 相比 `button`：这里保留 checked 状态，而不是一次性点击动作。
- 相比 `split_button`：这里没有下拉入口。
- 相比 `toggle_split_button`：这里只保留单入口切换，不保留复合分段。

## 14. EGUI 适配时的简化点与约束
- 基于现有 `egui_view_toggle_button` 核心控件做 reference 收口，不下沉到框架层。
- 颜色、圆角和图标间距继续由当前目录的样式包装统一管理。
- 主控件保留最小必要的键盘闭环；底部 preview 统一通过 `hcw_toggle_button_override_static_preview_api()` 固定为静态对照。
- 本轮补齐 `PRIMARY_SNAPSHOT_COUNT`、`TOGGLE_BUTTON_RECORD_FINAL_WAIT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_button()` 和 `request_page_snapshot()`，让主区轨道、底部静态 preview 与最终稳定帧共用同一套页面恢复路径。
