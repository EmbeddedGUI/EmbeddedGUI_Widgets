# drop_down_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 DropDownButton`
- 开源母本：`WPF UI`
- 对应组件名：`DropDownButton`
- 本轮保留语义：`single-entry menu trigger / compact / read only / static preview`
- 本轮移除内容：页面级 `guide`、旧 preview 快照轮换、preview 清焦桥接、额外收尾态、真实 flyout 定位、多级菜单与 showcase 化包装
- EGUI 适配说明：继续复用 custom 层现有 `egui_view_drop_down_button` 绘制与输入语义，本轮只收口 `reference` 页面结构、录制轨道、静态 preview、README 与单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`drop_down_button` 用来表达“整个按钮本身就是下拉入口”的标准命令触发语义。它适合排序、布局、主题和筛选这类场景：用户点击整个按钮，进入一组可选动作，而不是像 `split_button` 那样在主动作段和菜单段之间做双入口分工。

## 2. 为什么现有控件不够用
- `split_button` 强调“主动作 + 菜单段”的双入口，不等于整个按钮统一展开。
- `toggle_split_button` 还携带 `checked` 语义，不适合纯下拉入口。
- `menu_flyout` 是独立弹出菜单容器，不是页内按钮控件。
- `command_bar` 承担的是工具栏语义，不是单个下拉按钮语义。

因此这里继续保留 `drop_down_button` reference 控件，用来承接 Fluent / WPF UI 里的单入口下拉命令按钮。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `drop_down_button` -> 底部 `compact / read only` 双静态 preview。
- 主区保留 4 组 reference 状态：`Sort`、`Layout`、`Theme`、`Filter`。
- 主控件继续保留真实 `touch` 点击与 `Enter` 键闭环，用于驱动主区 snapshot 切换。
- 底部 `compact` preview 固定显示 `Quick`，只作为紧凑静态对照。
- 底部 `read only` preview 固定显示 `Locked`，只作为只读静态对照。
- 两个 preview 统一通过 `egui_view_drop_down_button_override_static_preview_api()` 收口。
- preview 收到 `touch` 或 `dispatch_key_event()` 后，只允许清理残留 `is_pressed`，不能改写 `snapshots / current_snapshot / compact_mode / read_only_mode / region_screen / palette / click_count`。

目标目录：`example/HelloCustomWidgets/input/drop_down_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 160`
- 主控件：`196 x 76`
- 底部对照行：`216 x 44`
- `compact` preview：`104 x 44`
- `read only` preview：`104 x 44`

视觉约束：
- 保持浅色 `page panel`、低噪音描边和 Fluent 风格的单入口按钮表面。
- 主区保留 `title + glyph + helper` 的最小信息层级，明确“整块点击打开选项”。
- runtime 轨道里只允许主区变化，底部 `compact / read only` preview 必须全程静态。
- 不再保留旧版 `guide`、状态回显、外部 preview 标签、preview 点击清焦桥接和额外收尾动作。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 160` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Drop Down Button` | 页面标题 |
| `button_primary` | `egui_view_drop_down_button_t` | `196 x 76` | `Sort` | 主控件 |
| `button_compact` | `egui_view_drop_down_button_t` | `104 x 44` | `Quick` | 紧凑静态 preview |
| `button_read_only` | `egui_view_drop_down_button_t` | `104 x 44` | `Locked` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Sort` | 默认主状态，accent tone |
| 主控件 | `Layout` | 点击主按钮后的第二态 |
| 主控件 | `Theme` | `Enter` 键切换后的第三态 |
| 主控件 | `Filter` | 第二次点击后的第四态，也是最终稳定态 |
| `compact` preview | `Quick` | 全程静态，不参与轮换 |
| `read only` preview | `Locked` | 全程静态，不参与轮换 |

## 7. 交互语义与单测口径
- 主控件继续保留单按钮面的 `same-target release` 语义：
  - `DOWN(inside) -> MOVE(outside) -> UP(outside)` 不提交。
  - 只有回到原命中区后 `UP(inside)` 才提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`。
- 主控件的 `touch cancel`、无关键盘输入、`Enter` 的 `ACTION_DOWN -> ACTION_UP` 闭环都不能留下残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接走旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 `test_drop_down_button_static_preview_consumes_input_and_keeps_state`。
- preview 固定断言覆盖：
  - `region_screen`
  - `snapshots`
  - `font`
  - `meta_font`
  - `on_click_listener`
  - 全 palette 字段
  - `snapshot_count`
  - `current_snapshot`
  - `compact_mode`
  - `read_only_mode`
  - `alpha`
  - `click_count == 0`
  - `is_pressed` / `pressed` 残留被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Sort`，同步底部 `Quick / Locked` 静态 preview，请求首帧截图。
2. 对主控件执行一次真实 `touch` 点击，切到 `Layout`。
3. 请求 `Layout` 主区截图。
4. 对主控件注入一次 `Enter` 键闭环，切到 `Theme`。
5. 请求 `Theme` 主区截图。
6. 对主控件再次执行一次真实 `touch` 点击，切到 `Filter`。
7. 请求 `Filter` 主区截图。
8. 保持 `Filter` 不变，等待最终稳定帧。
9. 请求最终稳定帧。

录制只允许主区发生变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持单一静态对照。

## 9. 编译、单测、runtime 与 web 验收链
```bash
make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/drop_down_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_drop_down_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Sort`、`Layout`、`Theme`、`Filter` 四组可识别状态。
- 底部 `Quick / Locked` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `snapshots / current_snapshot / compact_mode / read_only_mode / region_screen / palette`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. runtime 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_drop_down_button/default`
- 截图帧数：`10`
- 主区变化边界：`(50, 150) - (429, 257)`
- 主区唯一状态数：`4`
- 主区外唯一哈希数：`1`
- preview 区唯一哈希数：`1`
- preview 裁剪起点：`y >= 258`

复核结论：
- 遮罩主区变化边界后，边界外区域保持单哈希，确认主区外全程静态。
- 按主区裁剪后共出现 `4` 组唯一状态，符合 `Sort / Layout / Theme / Filter` 四态轨道。
- 按 `y >= 258` 裁剪底部 preview 区域后全部帧保持单哈希，确认 `Quick / Locked` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前版本只保留 snapshot 驱动的 reference `DropDownButton`，不实现真实 popup 定位与菜单布局系统。
- glyph 继续使用轻量字母占位，不接入真实图标资源。
- 底部 `compact / read only` preview 只承担静态对照，不承载真实交互职责。

## 13. 与现有控件的边界
- 相比 `split_button`：这里是整个按钮统一展开，没有主动作段 / 菜单段双入口。
- 相比 `toggle_split_button`：这里没有 `checked` 复合语义。
- 相比 `menu_flyout`：这里是按钮控件，不是独立弹出菜单容器。
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责。

## 14. EGUI 适配时的简化点与约束
- 用固定 snapshot 驱动 reference，优先保证 `480 x 480` 页面内的稳定审阅。
- 主控件保留最小必要的 `title + glyph + helper` 信息层级。
- `compact` preview 通过 `egui_view_drop_down_button_override_static_preview_api()` 固定为静态对照。
- `read only` preview 通过 `read_only_mode + compact_mode` 固定为静态锁定态。
- 本轮额外补齐了 `PRIMARY_SNAPSHOT_COUNT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()`、`focus_primary_button()` 与 `request_page_snapshot()` 对应的页面收口逻辑，保证主区轨道、底部静态 preview 与最终稳定帧使用同一套布局恢复路径。
