# card_action 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`CardAction`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、外部 preview 标签、Acrylic、重阴影、复杂 hover / reveal、整页场景桥接
- EGUI 适配说明：保留 `header / icon / title / body / optional chevron` 这组核心语义，在 `HelloCustomWidgets` 内维护 reference widget，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`card_action` 表达的是 Fluent / WPF UI 里比 `CardControl` 更轻的一类交互卡片。它强调整卡点击与 optional chevron affordance，而不是右侧承载 value、switch 这类附加 control。

## 2. 为什么现有控件不够用？
- `card_control` 更偏“卡片 + 右侧附加 control”，适合 value / switch / chevron 混合场景。
- `settings_card` 更偏设置入口语义，不适合作为通用 action card。
- `card_panel` 是摘要看板，不强调整卡 action affordance。

## 3. 目标场景与示例概览
- 主控件展示标准 `card_action`，录制轨道覆盖 `accent / success / warning` 三组 snapshot。
- 底部左侧展示 `compact` 静态对照，用于验证缩小尺寸后的卡片层级。
- 底部右侧展示 `read only` 静态对照，用于验证弱化 tone 与输入抑制后的只读态。
- 页面结构统一收口为：标题 -> 主 `card_action` -> `compact / read only`
- 两个 preview 都通过 `egui_view_card_action_override_static_preview_api()` 固定为静态 reference，只吞掉输入并协助主控件收尾 focus。

目标目录：`example/HelloCustomWidgets/layout/card_action/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主控件：`196 x 98`
- 底部对照容器：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 视觉原则：
  - 保持浅色 page panel、低噪音白色卡片和轻量 tone 差异。
  - tone 只在顶部 accent line、header pill、icon 区和可选 chevron 上提供低强度提示。
  - `read only` 不只弱化视觉，也必须真实抑制 `touch / key` 输入。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Card Action` | 页面标题 |
| `card_primary` | `egui_view_card_action_t` | `196 x 98` | `accent` | 主卡片 |
| `card_compact` | `egui_view_card_action_t` | `104 x 72` | `compact` | 紧凑静态对照 |
| `card_read_only` | `egui_view_card_action_t` | `104 x 72` | `read only` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace entry` | accent + chevron |
| 主控件 | `Identity review` | success + chevron |
| 主控件 | `Release approval` | warning + chevron |
| `compact` | `Compact action` | 紧凑 chevron 对照 |
| `compact` | `Compact warning` | 紧凑 no-chevron 对照 |
| `read only` | `Read only action` | 只读弱化对照 |

## 7. 交互与状态语义
- `current_part` 只有一个可交互 part：`EGUI_VIEW_CARD_ACTION_PART_CARD`
- `ACTION_MOVE` 遵守 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `activate_current_part()` 只在主卡 part 上触发 `on_action(snapshot_index, part)`
- `read_only_mode`、`!enable` 和 static preview 都必须先清理残留 `pressed` 再拒绝后续输入
- static preview 的 `touch / key` 只消费事件并保持 `snapshot / current_part` 不变

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认 snapshot，并给主控件请求 focus。
2. 请求初始帧。
3. 切到 `Identity review`。
4. 请求第二帧。
5. 切到 `Release approval`。
6. 请求第三帧。
7. 切到第二组 `compact` 对照。
8. 请求第四帧。
9. 重新给主控件请求 focus。
10. 点击 `compact` preview，只执行静态收尾逻辑。
11. 请求最终稳定帧。

## 9. 编译、交互、runtime、WASM 与文档验收路径
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_action PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_action --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_action
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_action
```

## 10. 验收重点
- 主控件和底部 `compact / read only` 对照必须完整可见，不能黑白屏、裁切或重叠。
- `header / icon / title / body / optional chevron / meta` 在不同 snapshot 之间层级必须稳定。
- 整卡点击必须遵守 same-target release；移出命中区后不能误触发 listener。
- `snapshot / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- static preview 不能改动 `snapshot / current_part`，也不能触发 action listener。
- WASM demo 必须正常加载，文档面板可渲染 `README.md`。

## 11. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题、超长正文或自定义内容模板。
- 当前 `chevron_visible` 只表达 affordance，不接业务路由。
- 当前不做真实图标、hover glow、focus reveal 或复杂转场动画。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 12. 与现有控件的边界
- 相比 `card_control`：这里不承载右侧 value / switch control，只保留 optional chevron。
- 相比 `settings_card`：这里不绑定 settings entry 语义。
- 相比 `card_panel`：这里强调整卡 action，而不是摘要信息面板。

## 13. 对参考原型删掉了哪些效果或装饰？
- 删掉页面级 guide、preview 标签、整页桥接说明和复杂装饰 chrome。
- 删掉 Acrylic、重阴影、复杂 hover / reveal 与桌面级系统动效。
- 只保留 `CardAction` 最核心的 reference 语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据模型，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件实现，通过模式位与 static preview API 收口。
- 继续作为 `HelloCustomWidgets` 内的 reference widget 维护，避免提前修改 SDK。
