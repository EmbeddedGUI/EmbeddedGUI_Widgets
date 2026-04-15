# split_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 SplitButton`
- 开源母本：`WPF UI`
- 对应组件名：`SplitButton`
- 本轮保留语义：`primary action / menu action / compact / disabled / static preview`
- 本轮移除内容：页面级 `guide`、旧 preview 快照轮换、preview 清焦桥接、额外收尾态、真实 flyout 弹层、多级菜单和 showcase 化包装
- EGUI 适配说明：继续复用 custom 层现有 `egui_view_split_button` 绘制与输入语义，本轮只收口 `reference` 页面结构、录制轨道、静态 preview、README 与单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`split_button` 用来表达“默认主动作 + 更多动作入口”并存的复合命令按钮。它适合保存、分享、导出和归档这类场景：用户可以直接触发主动作，也可以进入菜单段选择扩展动作。

## 2. 为什么现有控件不够用
- 普通 `button` 只有单一动作，无法表达 split 结构。
- `drop_down_button` 只有统一菜单入口，没有主动作段。
- `menu_flyout` 是独立弹出菜单容器，不是页内复合按钮。
- `command_bar` 承担工具栏语义，不是单个复合命令按钮。

因此这里继续保留 `split_button` reference 控件，用来承接 Fluent / WPF UI 里的双入口复合命令按钮。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `split_button` -> 底部 `compact / disabled` 双静态 preview。
- 主区保留 4 组 reference 状态：`Save draft`、`Share handoff`、`Export file`、`Archive page`。
- 主控件继续保留真实 split 语义，状态中同时包含 `primary` 与 `menu` 两个 part。
- 底部 `compact` preview 固定显示 `Quick / Save`，只作为紧凑静态对照。
- 底部 `disabled` preview 固定显示 `Locked / Publish`，只作为禁用静态对照。
- 两个 preview 统一通过 `egui_view_split_button_override_static_preview_api()` 收口。
- preview 收到 `touch` 或 `dispatch_key_event()` 后，只允许清理残留 `is_pressed / pressed_part`，不能改写 `snapshots / current_snapshot / current_part / compact_mode / disabled_mode / region_screen / palette`。

目标目录：`example/HelloCustomWidgets/input/split_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 154`
- 主控件：`196 x 74`
- 底部对照行：`216 x 44`
- `compact` preview：`104 x 44`
- `disabled` preview：`104 x 44`

视觉约束：
- 保持浅色 `page panel`、低噪音描边和 Fluent 风格的 split 双段结构。
- 主区保留 `title + split row + helper` 的最小信息层级，明确主动作段与菜单段边界。
- runtime 轨道里只允许主区变化，底部 `compact / disabled` preview 必须全程静态。
- 不再保留旧版 `guide`、状态回显、外部 preview 标签、preview 清焦桥接和 preview 快照轮换。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 154` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Split Button` | 页面标题 |
| `button_primary` | `egui_view_split_button_t` | `196 x 74` | `Save draft` | 主控件 |
| `button_compact` | `egui_view_split_button_t` | `104 x 44` | `Quick / Save` | 紧凑静态 preview |
| `button_disabled` | `egui_view_split_button_t` | `104 x 44` | `Locked / Publish` | 禁用静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Save draft / primary` | 默认主状态，焦点落在主动作段 |
| 主控件 | `Share handoff / menu` | 第二态，默认落在菜单段 |
| 主控件 | `Export file / menu` | 第三态，保留菜单段语义 |
| 主控件 | `Archive page / primary` | 第四态，也是最终稳定态 |
| `compact` preview | `Quick / Save` | 全程静态，不参与轮换 |
| `disabled` preview | `Locked / Publish` | 全程静态，不参与轮换 |

## 7. 交互语义与单测口径
- 主控件继续保留 split 双段的 `same-target release` 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交。
  - 只有回到原命中段后 `UP(A)` 才提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()` 都必须先清理残留 `pressed`。
- 主控件的 `touch cancel`、无关键盘输入、`compact / disabled / !enable` guard 都不能留下残留 `pressed_part / is_pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接走旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 `test_split_button_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `snapshots`
  - `font`
  - `meta_font`
  - `on_part_changed`
  - 全 palette 字段
  - `snapshot_count`
  - `current_snapshot`
  - `current_part`
  - `compact_mode`
  - `disabled_mode`
  - `alpha`
  - `changed_count == 0`
  - `last_part == PART_NONE`
  - `is_pressed / pressed_part` 残留被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Save draft`，同步底部 `Quick / Locked` 静态 preview，请求首帧截图。
2. 切到 `Share handoff`，请求第二帧主区截图。
3. 切到 `Export file`，请求第三帧主区截图。
4. 切到 `Archive page`，请求第四帧主区截图。
5. 保持 `Archive page` 不变，等待最终稳定帧。
6. 请求最终稳定帧。

录制只允许主区发生变化。底部 `compact / disabled` preview 在整条 reference 轨道里必须保持单一静态对照。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/split_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/split_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/split_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_split_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Save draft`、`Share handoff`、`Export file`、`Archive page` 四组可识别状态。
- 底部 `Quick / Locked` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `snapshots / current_snapshot / current_part / compact_mode / disabled_mode / region_screen / palette`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_split_button/default`
- 截图帧数：`10`
- 主区变化边界：`(52, 157) - (428, 256)`
- 主区唯一状态数：`4`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁剪起点：`y >= 257`

复核结论：
- 遮罩主区变化边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `4` 组唯一状态，符合 `Save draft / Share handoff / Export file / Archive page` 四态轨道。
- 按 `y >= 257` 裁剪底部 preview 区域后全部帧保持单哈希，确认 `Quick / Locked` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前版本只保留 snapshot 驱动的 reference `SplitButton`，不实现真实 popup 菜单与定位系统。
- glyph 继续使用轻量字母占位，不接入真实图标资源。
- 底部 `compact / disabled` preview 只承担静态对照，不承载真实交互职责。

## 13. 与现有控件的边界
- 相比 `button`：这里是双入口复合命令按钮，不是单动作按钮。
- 相比 `drop_down_button`：这里同时保留主动作段和菜单段。
- 相比 `menu_flyout`：这里是页内复合按钮，不是独立弹出菜单容器。
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责。

## 14. EGUI 适配时的简化点与约束
- 用固定 snapshot 驱动 reference，优先保证 `480 x 480` 页面内的稳定审阅。
- 主控件保留最小必要的 `title + split row + helper` 信息层级。
- `compact` preview 通过 `egui_view_split_button_override_static_preview_api()` 固定为静态对照。
- `disabled` preview 通过 `disabled_mode + compact_mode` 固定为静态禁用态。
- 本轮额外补齐了 `PRIMARY_SNAPSHOT_COUNT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_button()` 与 `request_page_snapshot()` 对应的页面收口逻辑，保证主区轨道、底部静态 preview 与最终稳定帧使用同一套布局恢复路径。
