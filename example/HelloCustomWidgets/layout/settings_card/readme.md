# settings_card 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`SettingCard`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、外部 preview 标签、Acrylic、重阴影、复杂 hover/reveal 动效、整页桥接式设置导航
- EGUI 适配说明：保留 `title / description / leading / trailing` 的单卡设置语义，在 `HelloCustomWidgets` 内维护 reference widget，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`settings_card` 用来表达“单个设置入口卡片”。它不是分组面板，也不是可展开容器，而是 Fluent / WPF UI 里最基础的设置卡片语义：一个 leading 区、标题、描述和 trailing affordance 聚合在同一张卡片上。

## 2. 为什么现有控件不够用？
- `settings_panel` 更偏向一组 setting rows 的集合，不负责单卡点击与单卡焦点语义。
- `settings_expander` 是“单 setting header + nested rows”的展开卡，不适合只表达单卡入口。
- `card_panel` 是摘要卡，不强调 settings entry 的 leading / trailing 节奏和只读弱化状态。

## 3. 目标场景与示例概览
- 主控件展示标准 `settings_card`，录制轨道覆盖 `accent / success / warning` 三组 snapshot。
- 底部左侧展示 `compact` 静态对照，用于验证小尺寸下卡片节奏。
- 底部右侧展示 `read only` 静态对照，用于验证弱化 tone 与输入抑制后的被动态。
- 页面结构统一收口为：标题 -> 主 `settings_card` -> `compact / read only`
- 两个 preview 都通过 `egui_view_settings_card_override_static_preview_api()` 固定为静态 reference，只负责吞掉输入并协助主控件收尾 focus。

目标目录：`example/HelloCustomWidgets/layout/settings_card/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主控件：`196 x 96`
- 底部对照容器：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 视觉原则：
  - 保持浅色 page panel、低噪音白色卡片和轻量 tone 差异。
  - tone 只在顶部 accent line、eyebrow、leading 区与 trailing affordance 上保留低强度提示。
  - `read only` 不只弱化视觉，也必须抑制 `touch / key` 输入。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Settings Card` | 页面标题 |
| `card_primary` | `egui_view_settings_card_t` | `196 x 96` | `accent` | 主卡片 |
| `card_compact` | `egui_view_settings_card_t` | `104 x 72` | `compact` | 紧凑静态对照 |
| `card_read_only` | `egui_view_settings_card_t` | `104 x 72` | `read only` | 只读静态对照 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup window` | accent + switch trailing |
| 主控件 | `Sharing scope` | success + value trailing |
| 主控件 | `Rollout ring` | warning + chevron trailing |
| `compact` | `Compact backup` | 紧凑对照 |
| `compact` | `Compact rollout` | 紧凑 warning 对照 |
| `read only` | `Read only policy` | 只读弱化对照 |

## 7. 交互与状态语义
- `current_part` 只有一个可交互 part：`EGUI_VIEW_SETTINGS_CARD_PART_CARD`
- `ACTION_MOVE` 采用 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `activate_current_part()` 只在主卡 part 上触发 `on_action(snapshot_index, part)`
- `read_only_mode`、`!enable` 和 static preview 都必须先清理残留 `pressed`，再拒绝后续输入
- static preview 的 `touch / key` 只消费事件并保持 `snapshot / current_part` 不变

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认快照，并给主控件请求 focus。
2. 请求初始帧。
3. 切到 `Sharing scope`。
4. 请求第二帧。
5. 切到 `Rollout ring`。
6. 请求第三帧。
7. 切到第二组 `compact` 对照。
8. 请求第四帧。
9. 重新给主控件请求 focus。
10. 点击 `compact` preview，只执行静态收尾逻辑。
11. 请求最终稳定帧。

## 9. 编译、交互、runtime、WASM 与文档验收路径
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_card PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_card --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_card
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_card
```

## 10. 验收重点
- 主控件和底部 `compact / read only` 对照必须完整可见，不能黑白屏、裁切或重叠。
- leading、title、description、trailing 和 footer 在不同 snapshot 之间层级必须稳定。
- 主卡点击必须遵守 same-target release；移出命中区后不能误触发 listener。
- `snapshot / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- static preview 不能改动 `snapshot / current_part`，也不能触发 action listener。
- WASM demo 必须正常加载，文档面板可渲染 `README.md`。

## 11. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题、超长 description 或复杂 trailing 自定义控件。
- 当前 trailing switch / chevron 只表达语义，不接业务状态同步。
- 当前不做真实图标、hover glow、focus reveal 或复杂转场动画。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里是“单卡 entry”，不是多卡分组。
- 相比 `settings_expander`：这里不承接 nested rows，也不做展开/折叠。
- 相比 `card_panel`：这里强调 settings 语义和 trailing affordance，不是摘要信息卡。

## 13. 对参考原型删掉了哪些效果或装饰？
- 删除页面级 guide、preview 标签、整页桥接说明和复杂装饰性 chrome。
- 删除 Acrylic、重阴影、复杂 hover/reveal 和业务联动。
- 只保留 `SettingCard` 最核心的 reference 语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据模型，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 和 `read only` 直接复用同一控件实现，通过模式位与 static preview API 收口。
- 继续作为 `HelloCustomWidgets` 内的 reference widget 维护，避免提前修改 SDK。
