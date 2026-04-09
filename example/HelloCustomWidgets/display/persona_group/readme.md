# persona_group 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AvatarGroup`
- 补充对照实现：`ModernWpf`
- 对应组件名：`AvatarGroup`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`live`、`busy`、`away`、`idle`
- 本次删除效果：页面级 `guide`、状态说明、外部 preview 标签、旧双列容器壳、过重 avatar ring、过亮 overflow bubble、过强 footer summary chrome
- EGUI 适配说明：沿用仓库内 `persona_group` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`persona_group` 用来展示一组协作成员，并让当前焦点成员驱动整张卡片的阅读重心。它适合出现在审阅链、责任人概览、团队面板和归档页摘要里，表达“这一组成员属于同一条工作流”。

## 2. 为什么现有控件不够用？
- `badge_group` 表达的是状态标签，不承载成员身份和 presence。
- `card_panel` 更偏结构化信息卡，不适合作为轻量成员群组。
- `teaching_tip` 和 `dialog_sheet` 都是反馈层，不适合常驻团队摘要。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI 的 `AvatarGroup` 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `persona_group`，通过录制动作覆盖 `accent / success / neutral` 焦点成员组合。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下的成员重叠和单行摘要。
- 底部右侧展示 `read only` 静态对照，验证只读态下的灰蓝弱化结果。
- 页面结构统一收口为：标题 -> 主 `persona_group` -> `compact / read only`。
- 底部两个 preview 都禁用 touch 和 focus，只做静态 `reference` 对照。

目标目录：`example/HelloCustomWidgets/display/persona_group/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 230`
- 主控件尺寸：`196 x 114`
- 底部对照行尺寸：`216 x 76`
- `compact` 预览：`104 x 76`
- `read only` 预览：`104 x 76`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白底卡片和低噪音浅边框。
  - avatar overlap、presence dot、eyebrow、title 和 footer summary 都保留，但强调度要比旧版更轻。
  - `compact` 直接通过控件模式表达，不再依赖外部标签说明。
  - `read only` 保留完整成员结构，但 ring、presence 和 footer tone 都需要明显弱化。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 230` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Persona Group` | 页面标题 |
| `group_primary` | `egui_view_persona_group_t` | `196 x 114` | `Design squad` | 标准主控件 |
| `group_compact` | `egui_view_persona_group_t` | `104 x 76` | `Team` | `compact` 静态对照 |
| `group_read_only` | `egui_view_persona_group_t` | `104 x 76` | `Archive` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_persona_group_snapshot_t[3]` | - | `Design / Ops / Archive` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_persona_group_snapshot_t[2]` | - | `Team / Ops` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_persona_group_snapshot_t[1]` | - | `Archive` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Design squad` | 默认 `accent` 焦点成员 |
| 主控件 | `Design squad / Maya` | 同一 snapshot 内切到 `warning` 焦点成员 |
| 主控件 | `Ops desk` | `success` 焦点成员 |
| 主控件 | `Archive` | `neutral` 焦点成员 |
| `compact` | `Team` | 默认紧凑对照 |
| `compact` | `Ops` | 第二组紧凑对照 |
| `read only` | `Archive` | 固定只读对照，禁用 touch / focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件焦点到 `warning` 成员。
4. 请求第二张截图。
5. 程序化切换主控件到 `Ops desk`。
6. 请求第三张截图。
7. 程序化切换主控件到 `Archive`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见。
- avatar overlap、presence dot、标题、角色和 footer summary 之间要保留清晰留白。
- `accent / success / neutral` 焦点切换需要可辨认，但整体不能回到高对比 showcase 风格。
- 页面中不再出现 `guide`、状态说明、外部 preview 标签和旧双列包裹壳层。
- 底部预览只作静态对照展示，不承担点击切换职责。

## 9. 已知限制与后续方向
- 当前版本仍使用固定 snapshot 数据，不接真实头像资源和团队模型。
- 当前不做 hover、focus ring、更多成员面板和上下文菜单。
- 当前 overflow 只保留简化的 `+n` 气泡，不接真实更多成员弹层。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不下沉到通用框架层。

## 10. 与现有控件的边界
- 相比 `badge_group`：这里表达的是成员关系，不是状态标签组合。
- 相比 `card_panel`：这里更轻、更扁平，重点在成员焦点与 presence。
- 相比 `dialog_sheet`：这里是常驻团队摘要，不是确认层。
- 相比旧版 showcase 页面：这里回到统一的 Fluent `reference` 结构，不保留叙事式页面壳层。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AvatarGroup`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`AvatarGroup`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`
  - `live`
  - `busy`
  - `away`
  - `idle`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、状态说明、preview 标签和旧双列预览容器壳。
- 删除 preview 参与点击切换、页面桥接和焦点承接的职责。
- 删除过重 avatar ring、过亮 overflow bubble、过强 footer summary chrome。
- 删除真实头像图片、复杂阴影、hover / focus ring 和成员详情弹层。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + item` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 snapshot 和 focus index 保证 runtime 稳定抓取状态变化。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
