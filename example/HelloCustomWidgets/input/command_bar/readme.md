# command_bar 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CommandBar`
- 开源母本：`WPF UI`
- 对应组件名：`CommandBar`
- 本轮保留语义：`standard / compact / disabled / focused`
- 本轮移除内容：页面级 guide、状态说明文案、preview 快照轮换、preview 点击清焦桥接、旧录制轨道里的额外收尾态
- EGUI 适配说明：继续复用 custom 层现有 `command_bar` 结构与输入语义，本轮只收口 `reference` 页面结构、静态 preview 轨道、单测断言和验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`command_bar` 用来承载页面内长期驻留的一组高频命令，强调主命令、当前 scope 和 overflow 入口的分层表达。它适合编辑、审核、布局和发布等场景，不等同于一次性弹出菜单，也不等同于页面导航。

## 2. 为什么现有控件不够用

- `menu_flyout` 解决的是弹出式菜单，不是常驻命令栏。
- `nav_panel`、`breadcrumb_bar`、`tab_strip` 解决的是导航，不承担主操作语义。
- `split_button`、`drop_down_button` 只表达单个入口，不表达一整组页面命令。
- 当前仓库里的 `command_bar` 页面虽然已经 reference 化，但录制轨道、静态 preview 单测和 README 仍停留在旧 workflow，没有真正收口到当前 static preview 模板。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `command_bar` -> 底部 `compact / disabled` 双静态 preview。
- 主区保留真实 `CommandBar` 的 `Left / Right / Tab / Home / End` 键盘闭环。
- 底部 `compact` preview 固定显示 `Quick` 命令栏，不再承担快照切换职责。
- 底部 `disabled` preview 固定显示 `Locked` 命令栏，作为禁用静态对照。
- 两个 preview 都通过 `egui_view_command_bar_override_static_preview_api()` 收口：
  - 吞掉 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed`
  - 不改 `snapshots / current_snapshot / current_index / region_screen / palette`

目标目录：`example/HelloCustomWidgets/input/command_bar/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 192`
- 主控件：`196 x 88`
- 底部对照行：`216 x 64`
- `compact` preview：`104 x 64`
- `disabled` preview：`104 x 64`

视觉约束：

- 保持浅色 page panel、低噪音边框和单层命令栏容器，不回退到 showcase 式重装饰页面。
- 主区保留 `eyebrow + title + scope + command rail + footer` 的最小完整语义，不额外叠加外部状态回显。
- `compact` 与 `disabled` 只做静态 reference 对照，整条 runtime 轨道中必须保持不变。
- focus 仅在主控件内部的命令项间切换，不做夸张 glow 或额外 preview 焦点桥接。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 192` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Command Bar` | 页面标题 |
| `bar_primary` | `egui_view_command_bar_t` | `196 x 88` | `Edit` / `Save` | 主控件 |
| `bar_compact` | `egui_view_command_bar_t` | `104 x 64` | `Quick` / compact | 静态 preview |
| `bar_disabled` | `egui_view_command_bar_t` | `104 x 64` | `Locked` / disabled | 静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Edit / Save` | 默认状态 |
| 主控件 | `Review / Block` | `Right` 后 |
| 主控件 | `Layout / Overflow` | `End` 后 |
| 主控件 | `Publish / Stage` | `Right` 后 |
| `compact` preview | `Quick / Save` | 全程静态对照 |
| `disabled` preview | `Locked / Save` | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` same-target release 与 `Left / Right / Tab / Home / End` 键盘闭环。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_index()`、`set_compact_mode()`、`set_disabled_mode()` 与 `set_palette()` 必须清理残留 `pressed_index / is_pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直连旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 固定断言覆盖：
  - `snapshots / font / meta_font / on_selection_changed` 不变
  - `region_screen / alpha / surface_color / section_color / border_color / text_color / muted_text_color / accent_color / success_color / warning_color / danger_color / neutral_color` 不变
  - `snapshot_count / current_snapshot / current_index / compact_mode / disabled_mode` 不变
  - `changed_count == 0` 且 `last_index == EGUI_VIEW_COMMAND_BAR_INDEX_NONE`
  - `is_pressed` 与 `pressed_index` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Edit / Save` 状态，并同步底部 `compact / disabled` 静态 preview，输出首帧。
2. 切到 `Review` 主快照并发送 `Right`，把主区当前命令移动到 `Block`。
3. 输出 `Review / Block` 主区截图。
4. 切到 `Layout` 主快照并发送 `End`，把主区当前命令移动到 `Overflow`。
5. 输出 `Layout / Overflow` 主区截图。
6. 切到 `Publish` 主快照并发送 `Right`，把主区当前命令移动到 `Stage`。
7. 输出 `Publish / Stage` 主区截图。
8. 保持 `Publish / Stage` 不变，做最终稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生变化。底部 `compact / disabled` preview 在整条 reference 轨道里必须保持单一静态对照。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Edit / Save`、`Review / Block`、`Layout / Overflow`、`Publish / Stage` 四组可识别状态。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `snapshots / current_snapshot / current_index / region_screen / palette`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_command_bar/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前版本只保留固定 snapshot 驱动的 reference 命令栏，不做真实响应式隐藏与测量。
- overflow 只保留入口语义，不展开真实菜单。
- glyph 继续使用双字母占位，不接入真实图标资源。

## 13. 与现有控件的边界

- 相比 `menu_flyout`：这里是常驻命令 rail，不是弹出式命令面板。
- 相比 `drop_down_button` / `split_button`：这里承载的是命令组，不是单个入口按钮。
- 相比 `nav_panel` / `breadcrumb_bar` / `tab_strip`：这里表达操作命令，不表达导航结构。

## 14. EGUI 适配时的简化点与约束

- 继续复用 custom 层现有 `command_bar` 的绘制与输入语义，不改 `sdk/EmbeddedGUI`。
- 主控件只保留最小必要的键盘导航与 same-target release 闭环。
- preview 固定放在底部双列，并统一挂接 static preview API，避免继续承担快照切换或焦点收尾逻辑。
- 先完成 reference 级 `CommandBar` 收口，再决定是否补充真实 overflow、响应式折叠或图标资源。
