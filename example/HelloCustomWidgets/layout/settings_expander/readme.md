# settings_expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`SettingsExpander`
- 本次保留状态：`expanded`、`collapsed`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、状态说明、外部 preview 标签、Acrylic、重阴影、复杂 reveal/hover 光效、整页桥接式设置导航
- EGUI 适配说明：保留 setting header、description、value、expand-collapse 和 nested rows 的核心语义，在 `HelloCustomWidgets` 内维护 reference widget，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`settings_expander` 用来表达“一个设置入口 + 一组按需展开的子设置行”。它既不是普通 `expander`，也不是纯展示型 `settings_panel`，而是 Fluent / WPF UI 里用于承载嵌套设置项的标准 setting card 语义。

## 2. 为什么现有控件不够用？
- `settings_panel` 更偏向静态 setting rows 分组，不负责单个 header 的展开/折叠。
- `expander` 负责 disclosure 结构，但不包含 setting header、value、nested rows 和 settings tone 节奏。
- `card_panel`、`data_list_panel` 更偏摘要卡片或通用列表，不适合表达 SettingsExpander 的单卡片嵌套设置语义。

## 3. 目标场景与示例概览
- 主控件展示一个标准 `SettingsExpander`：header 含 `eyebrow / icon / title / description / value`，展开后显示 nested rows。
- 底部左侧展示 `compact` 静态对照，用于验证小尺寸下 header 和 rows 的密度。
- 底部右侧展示 `read only` 静态对照，用于验证弱化 tone 和输入抑制后的被动态。
- 页面结构统一收口为：标题 -> 主 `settings_expander` -> `compact / read only`
- 两个 preview 都通过 `egui_view_settings_expander_override_static_preview_api()` 固定为静态 reference，只负责清理残留 `pressed` 并协助主控件收尾 focus。

目标目录：`example/HelloCustomWidgets/layout/settings_expander/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 270`
- 主控件：`196 x 146`
- 底部对照容器：`216 x 88`
- `compact` preview：`104 x 88`
- `read only` preview：`104 x 88`
- 视觉原则：
  - 维持浅色 page panel、低噪音白色 setting card 和轻量 tone 差异。
  - tone 只在 header 顶边、eyebrow、row icon、value/switch 和 footer 上保留低强度提示。
  - nested rows 必须保持统一节奏，不能挤压 header，也不能让 trailing affordance 抢过标题层级。
  - `read only` 不只弱化视觉，也必须抑制 `touch / key` 输入。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 270` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Settings Expander` | 页面标题 |
| `expander_primary` | `egui_view_settings_expander_t` | `196 x 146` | `accent expanded` | 主 SettingsExpander |
| `expander_compact` | `egui_view_settings_expander_t` | `104 x 88` | `compact expanded` | 紧凑静态对照 |
| `expander_read_only` | `egui_view_settings_expander_t` | `104 x 88` | `read only expanded` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup options expanded` | accent + row focus |
| 主控件 | `Sharing scope expanded` | success + nested rows |
| 主控件 | `Quiet hours collapsed` | collapsed header |
| 主控件 | `Rollout cadence expanded` | warning + emphasized rows |
| `compact` | `compact accent` | 小尺寸对照 |
| `compact` | `compact warning` | 紧凑切换对照 |
| `read only` | `read only neutral` | 静态只读对照 |

## 7. 交互与状态语义
- header part 始终可用；row parts 只有在 `expanded` 时才存在。
- `focus_part` 只用于 expanded 态的默认焦点；collapsed 态统一回落到 header。
- `ACTION_MOVE` 采用 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交。
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交。
- header 激活只切换 `expanded_state`，不会触发行 action listener。
- row 激活触发 `on_action(snapshot_index, part)`，但不会自动改写 snapshot。
- `read_only_mode`、`!enable` 和 static preview 都必须先清理残留 `pressed`，再拒绝后续输入。
- static preview 的 `touch / key` 只消费事件并保持当前 `snapshot / expanded / part` 不变。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认快照，并给主控件请求 focus。
2. 请求初始帧。
3. 通过键盘移动到下一条 row，记录 row focus。
4. 请求第二帧。
5. 切换到 `Sharing scope` 快照。
6. 请求第三帧。
7. 对主控件发送 `Escape`，记录 collapsed 状态。
8. 请求第四帧。
9. 切换到 `Rollout cadence` 快照。
10. 请求第五帧。
11. 点击主 header，再次切换展开/折叠。
12. 请求第六帧。
13. 切换 `compact` 到 warning 对照。
14. 请求第七帧。
15. 点击 `compact` preview，仅执行静态收尾逻辑并保留最终稳定帧。

## 9. 编译、交互、runtime、WASM 与文档验收路径
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_expander
```

## 10. 验收重点
- 主控件和底部 `compact / read only` 对照必须完整可见，不能黑白屏、裁切或重叠。
- header、description、value、nested rows 和 footer 的层级在 expanded / collapsed 间必须稳定。
- row part 必须遵守 same-target release；移出命中区后不能误触发 action listener。
- header 激活只能切换展开状态，row 激活才允许回调 action listener。
- `snapshot / expanded / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- static preview 不能改动 `snapshot / expanded / current_part`，也不能触发 action listener。
- WASM demo 必须能正常加载，文档面板可渲染 `README.md`。

## 11. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题、超长 value 或超过 4 条 nested rows 的真实布局策略。
- 当前 trailing switch / chevron 仅表达语义，不接业务状态同步。
- 当前不做真实图标、hover、focus ring 或复杂展开动画。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里是“单 setting header + nested rows”，不是整组 settings group。
- 相比 `expander`：这里强调 settings 语义和 trailing/value 节奏，不是通用 disclosure 正文块。
- 相比 `data_list_panel`：这里不做通用列表浏览，而是围绕 settings row 的层级与可读性。
- 相比 `card_panel`：这里自带展开/折叠与 row part 交互，不是静态摘要卡片。

## 13. 对参考原型删除了哪些效果或装饰？
- 删除页面级 guide、外层 preview 标签、复杂场景化说明和整页设置导航桥接。
- 删除 Acrylic、重阴影、长列表滚动、复杂 hover/reveal 动画。
- 删除真实业务联动，只保留 `SettingsExpander` 的核心 reference 语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + row` 数据模型，保证 `480 x 480` 下的审阅稳定性。
- `current_part` 统一用 `header + row parts` 表达，不引入额外 page-level 焦点桥接。
- `compact` 与 `read only` 直接复用同一控件实现，通过模式与 static preview API 收口。
- 继续作为 `HelloCustomWidgets` 里的 reference widget 维护，避免提前修改 SDK。
