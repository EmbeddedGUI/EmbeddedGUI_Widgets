# card_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件语义：`Card`
- 当前保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前删除内容：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、过重的 header strap、过重的 metric chip、过强的 footer action chrome
- EGUI 适配范围：只维护 `HelloCustomWidgets` 中的 `reference widget` 版本，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`card_panel` 用来承载一张结构化信息卡：顶部 badge、标题、正文摘要、右侧 summary slot、底部 detail strip 与可选 action pill。在 Fluent / WPF UI 语义里，它适合放在设置页、概览页和详情页中，用一张低噪音卡片承载“主信息 + 次级摘要 + 轻量动作”。

## 2. 为什么现有控件不够用
- `card` 更偏通用容器，不负责固定的信息层级。
- `layer_stack` 强调叠层和景深，不适合作为标准信息卡。
- `message_bar` 和 `toast_stack` 属于反馈控件，不是常驻摘要卡。
- 旧版 `card_panel` demo 仍保留了较多 showcase chrome，不符合当前 `reference` 主线。

## 3. 目标场景与示例概览
- 主控件展示标准 `card_panel`，录制轨道覆盖 `OVERVIEW / SYNC / DEPLOY / ARCHIVE` 四组 snapshot。
- 底部左侧展示 `compact` 静态 preview，验证小尺寸下标题、摘要、summary slot 与 footer 仍能稳定阅读。
- 底部右侧展示 `read only` 静态 preview，验证弱化 tone、隐藏动作与输入抑制后的只读语义。
- 页面结构固定为：标题 -> 主 `card_panel` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_card_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清残留 `pressed`
  - 不改 `current_snapshot`
  - 不触发 click
- preview 点击只负责清主控件 focus，用于 runtime 录制生成交互后的收尾帧和最终稳定帧。

目标目录：`example/HelloCustomWidgets/display/card_panel/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 90`
- `compact` preview：`104 x 90`
- `read only` preview：`104 x 90`
- 页面结构：标题 + 主控件 + 底部双 preview
- 视觉约束：
  - 使用浅灰 `page panel`、白底卡片和低噪音浅边框。
  - 顶部 tone strap 只保留轻量强调，不再承担高对比装饰。
  - 右侧 summary slot 维持 `value + label` 语义，但视觉权重低于标题和正文。
  - `compact` 继续保留结构化卡片层级，不退化成简单摘要块。
  - `read only` 除了色调弱化，还要真实抑制后续 `touch / key` 输入。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Card Panel` | 页面标题 |
| `panel_primary` | `egui_view_card_panel_t` | `196 x 122` | `OVERVIEW` | 标准结构化卡片 |
| `panel_compact` | `egui_view_card_panel_t` | `104 x 90` | `TASK` | `compact` 静态对照 |
| `panel_read_only` | `egui_view_card_panel_t` | `104 x 90` | `ARCHIVE` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_card_panel_snapshot_t[4]` | - | `OVERVIEW / SYNC / DEPLOY / ARCHIVE` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_card_panel_snapshot_t[2]` | - | `TASK / WARN` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_card_panel_snapshot_t[1]` | - | `ARCHIVE` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `OVERVIEW` | 默认 `accent` 状态 |
| 主控件 | `SYNC` | `warning` 状态 |
| 主控件 | `DEPLOY` | `success` 状态 |
| 主控件 | `ARCHIVE` | `neutral` 状态 |
| `compact` | `TASK` | 默认紧凑对照 |
| `compact` | `WARN` | 第二组紧凑 snapshot |
| `read only` | `ARCHIVE` | 固定只读对照，隐藏 action 并弱化 tone |

## 7. 交互语义要求
- 主控件保持标准 clickable 语义，必须满足 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交 click
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交 click
- `ACTION_CANCEL` 必须清理 `pressed`，且不能触发 click。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()` 必须先清理残留 `pressed` 再刷新。
- `set_snapshots(NULL, 0)` 必须把 `snapshot_count` 收口到 `0`，并同步清理 `pressed`。
- `read only / disabled` 的 `touch / key guard` 必须在拒绝输入前清理残留 `pressed`。
- 静态 preview 必须吞掉 `touch / key`，不能修改 `current_snapshot`，也不能触发 click。
- preview 点击只负责收主控件焦点，用于确认交互后的渲染收尾没有残留污染。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切到主控件 `SYNC`。
4. 请求第二张截图。
5. 程序化切到主控件 `DEPLOY`。
6. 请求第三张截图。
7. 程序化切到主控件 `ARCHIVE`。
8. 请求第四张截图。
9. 程序化切到 `compact` 第二组 snapshot，并重新请求主控件 focus。
10. 请求 `compact` 第二组截图。
11. 点击 `compact` preview 中心，只触发主控件焦点收尾。
12. 请求 preview 点击后的收尾帧。
13. 再请求一张最终稳定帧，确认渲染没有残留污染。

## 9. 编译、交互、runtime 与文档检查
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc
make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` preview 必须完整可见。
- 主控件四组 snapshot、`compact` 第二组 snapshot、preview 点击后的收尾帧、最终稳定帧都要人工复核。
- `same-target release`、`cancel`、`read only / disabled guard`、`static preview` 必须全部通过单测。
- preview 点击后不能误触发主控件交互，也不能留下 `pressed`、整屏污染、黑白屏或裁切。
- `read only` preview 必须既弱化视觉，也真实吞掉 `touch / key` 输入。

## 10. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题、超长摘要和超长 `value`。
- 当前不做真实图标、hover、完整 focus ring 和桌面级键盘导航细节。
- 当前 summary slot 与 footer 文本宽度仍依赖简化估算，不是完整文本测量系统。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 11. 与现有控件的边界
- 相比 `card`：这里不是任意容器，而是结构化信息卡。
- 相比 `layer_stack`：这里强调稳定的信息组织，不强调叠层景深。
- 相比 `message_bar`：这里是常驻内容块，不是反馈条。
- 相比 `toast_stack`：这里表达的是稳定摘要，不是短时通知。

## 12. 对应组件名与保留状态
- 对应组件名：`Card`
- 当前保留的核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`

## 13. 删除了哪些效果或装饰
- 删除页面级 `guide`、状态回显、preview 标签和旧列容器壳。
- 删除过重的 header strap、metric chip、footer action chrome 与高对比 tone 装饰。
- 删除 Acrylic、真实阴影扩散和桌面级系统特效。
- 删除完整 hover / focus ring / 富键盘导航等桌面交互细节。
- 删除与 reference 主线无关的场景化桥接逻辑。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据和固定 summary slot，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件模式，减少页面级桥接逻辑。
- `snapshot / compact / read only` 共用一套 `pressed` 清理语义，避免模式切换后残留高亮。
- preview 统一收口为静态 reference 对照，只负责清主控件 focus，不再承担额外交互职责。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续再评估是否下沉框架层。
