# command_bar_flyout 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CommandBarFlyout`
- 开源母本：`WPF UI`
- 对应组件名：`CommandBarFlyout`
- 本轮保留语义：`trigger / primary commands / secondary commands / compact / disabled`
- 本轮移除内容：页面级 guide、preview 快照轮换、preview 点击清焦桥接、旧录制轨道里的额外收尾态
- EGUI 适配说明：继续复用 custom 层现有 `egui_view_command_bar_flyout` 绘制与输入语义，本轮只收口 `reference` 页面结构、静态 preview、单测断言和验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`command_bar_flyout` 用来表达“由触发入口展开的一组页面命令”。它不是常驻命令栏，也不是纯菜单，而是保留顶部触发器、主命令 rail 和次级命令列表的轻量命令面板，适合编辑、审核和布局调整这类需要分层命令的场景。

## 2. 为什么现有控件不够用
- `command_bar` 是常驻命令栏，没有 flyout 触发入口和次级命令分层。
- `menu_flyout` 只有纵向菜单，不保留主命令 rail。
- `split_button` 和 `drop_down_button` 只覆盖单入口，不承载整组主命令和次级命令。
- 因此这里保留 `command_bar_flyout` reference widget，用来补齐 “trigger + primary rail + secondary rows” 的组件语义。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `command_bar_flyout` -> 底部 `compact / disabled` 双静态 preview。
- 主区保留真实 `CommandBarFlyout` 的 `Left / Right / Tab / Home / End / Up / Down / Enter / Space / Escape` 键盘闭环。
- 底部 `compact` preview 固定显示 `Quick` 紧凑 flyout，不再承担快照切换或清焦职责。
- 底部 `disabled` preview 固定显示 `Locked` 禁用 flyout，作为静态 reference 对照。
- 两个 preview 都通过 `egui_view_command_bar_flyout_override_static_preview_api()` 收口。
- preview 收到 `touch / dispatch_key_event()` 后只清理残留 `pressed_part / is_pressed`。
- preview 不改 `snapshots / current_snapshot / current_part / open_state / region_screen / palette`。

目标目录：`example/HelloCustomWidgets/input/command_bar_flyout/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 272`
- 主控件：`196 x 160`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `disabled` preview：`104 x 72`

视觉约束：
- 保持浅色 page panel、低噪音边框和单层命令面板，不回退到 showcase 式页面包装。
- 主区保留 `eyebrow + trigger + panel header + primary rail + secondary rows` 的最小完整语义。
- runtime 轨道里只允许主区变化，底部 `compact / disabled` preview 必须全程静态。
- focus 仅在主控件内部流转，不再做额外 preview 焦点桥接。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 272` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Command Bar Flyout` | 页面标题 |
| `flyout_primary` | `egui_view_command_bar_flyout_t` | `196 x 160` | `Edit / Save` | 主控件 |
| `flyout_compact` | `egui_view_command_bar_flyout_t` | `104 x 72` | `Quick / compact` | 静态 preview |
| `flyout_disabled` | `egui_view_command_bar_flyout_t` | `104 x 72` | `Locked / disabled` | 静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Edit / Save` | 默认展开态，焦点落在主命令 `Save` |
| 主控件 | `Review / Escalate` | 切到 `Review` 后向下移动到次级命令 |
| 主控件 | `Quick / Trigger` | 关闭态，只保留触发器 |
| 主控件 | `Layout / Density` | 再次展开，用作最终稳定态 |
| `compact` preview | `Quick / compact` | 全程静态对照 |
| `disabled` preview | `Locked / disabled` | 全程静态对照 |

## 7. 交互语义与单测口径
- 主控件继续保留 `touch` same-target release 与完整键盘导航。
- `set_snapshots()`、`set_current_snapshot()`、`set_open()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 必须清理残留 `pressed_part / is_pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直连旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 固定断言覆盖 `snapshots / font / meta_font / on_action` 不变。
- preview 固定断言覆盖 `region_screen / alpha / surface_color / section_color / border_color / text_color / muted_text_color / accent_color / success_color / warning_color / danger_color / neutral_color / shadow_color` 不变。
- preview 固定断言覆盖 `snapshot_count / current_snapshot / current_part / open_state / compact_mode / disabled_mode` 不变。
- preview 固定断言覆盖 `g_action_count == 0`、`g_action_snapshot == 0xFF`、`g_action_part == PART_NONE`。
- preview 收到输入后必须清理 `is_pressed / pressed_part`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Edit / Save` 状态，并同步底部 `compact / disabled` 静态 preview，输出首帧。
2. 切到 `Review` 主快照并发送 `Down`，把焦点移动到 `Escalate`。
3. 输出 `Review / Escalate` 主区截图。
4. 切到 `Quick` 主快照，保持关闭态 trigger。
5. 输出 `Quick / Trigger` 主区截图。
6. 切到 `Layout` 主快照，保留默认 `Density` 焦点。
7. 输出 `Layout / Density` 主区截图。
8. 保持 `Layout / Density` 不变，做最终稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生变化。底部 `compact / disabled` preview 在整条 reference 轨道里必须保持单一静态对照。

## 9. 编译、单测、运行时与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar_flyout PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar_flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar_flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar_flyout
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Edit / Save`、`Review / Escalate`、`Quick / Trigger`、`Layout / Density` 四组可识别状态。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `snapshots / current_snapshot / current_part / open_state / region_screen / palette`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_command_bar_flyout/default`
- 主区裁剪后只出现 `4` 组唯一状态。
- 遮罩主区变化边界后，边界外区域保持单哈希。
- 按底部 preview 区域裁剪后，所有帧保持单哈希。

## 12. 已知限制
- 当前版本只保留固定 snapshot 驱动的 reference flyout，不做真实 popup 定位与屏幕边缘避让。
- 主命令和次级命令都由静态 snapshot 数据驱动，不接业务回调。
- glyph 继续使用双字母占位，不接入真实图标资源。

## 13. 与现有控件的边界
- 相比 `command_bar`：这里有 trigger 和次级命令分层，不是常驻命令栏。
- 相比 `menu_flyout`：这里保留主命令 rail，不是纯纵向菜单。
- 相比 `split_button` 和 `drop_down_button`：这里承载的是一组命令，不是单入口按钮。

## 14. EGUI 适配时的简化点与约束
- 继续复用 custom 层现有 `command_bar_flyout` 实现，不修改 `sdk/EmbeddedGUI`。
- 主控件只保留最小必要的键盘导航和 same-target release 语义。
- preview 固定放在底部双列，并统一挂接 static preview API，避免继续承担快照切换或焦点收尾逻辑。
- 先完成 reference 级 `CommandBarFlyout` 收口，再决定是否补充真实 popup、响应式折叠或图标资源。
