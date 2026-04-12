# divider 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI / WPF Separator`
- 对应组件名：`Separator`
- 本次保留状态：`standard`、`subtle`、`accent`
- 本次删除效果：页面级 guide、showcase 式故事壳层、额外状态标签和与分隔线无关的强装饰容器
- EGUI 适配说明：直接复用 SDK `divider`，custom 层只补轻量样式 helper、palette setter 和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`divider` 用于在低噪音界面里划分相关内容块，让信息层级更清晰，但又不引入额外的边框、卡片或标题壳。当前 `HelloCustomWidgets` 的 `reference` 主线里还没有一个对齐 Fluent / WPF UI 语义的 `Separator` 页面，因此需要补齐。

## 2. 为什么现有控件不够用？
- `card_panel` 是结构化信息卡，不是最轻量的分隔语义。
- `badge_group`、`persona_group` 关注内容集合展示，不承担页面节奏切分。
- SDK 自带 `divider` 只有基础绘制，没有当前仓库要求的 Fluent 风格 reference 页面、静态 preview API 和 catalog 闭环。

## 3. 目标场景与示例概览
- 主控件展示标准 `Separator`，并通过录制动作覆盖 `standard -> accent -> subtle` 三个关键变体。
- 主场景采用“标题 + 分隔线 + 两行说明”的轻量布局，确保截图里能直接看出分隔线的页面作用。
- 底部左侧展示 `subtle` 静态对照，用更轻的灰线表达次级区块分隔。
- 底部右侧展示 `accent` 静态对照，用强调色表达活动区域或重点段落的切分。
- 页面结构统一收口为：标题 -> 主 `divider` 场景 -> `subtle / accent` 双 preview。
- 两个 preview 通过 `hcw_divider_override_static_preview_api()` 统一吞掉 `touch / key`，只做 reference 对照。

目标目录：`example/HelloCustomWidgets/display/divider/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 208`
- 主场景面板尺寸：`196 x 84`
- 底部对照行尺寸：`216 x 72`
- `subtle` 预览：`104 x 72`
- `accent` 预览：`104 x 72`
- 页面结构：标题 + 主场景面板 + 底部双预览
- 样式约束：
  - 页面背景保持浅灰 panel。
  - 主场景与预览面板保持低对比白底。
  - `standard` 使用中性冷灰线。
  - `subtle` 使用更浅的灰线与较低 alpha。
  - `accent` 使用 Fluent 蓝色强调线。
  - 不引入额外图标、按钮、说明条或故事化布局。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 208` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Separator` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 84` | `standard` | 主场景面板 |
| `primary_divider` | `egui_view_divider_t` | `176 x 2` | `standard` | 主分隔线 |
| `subtle_panel` | `egui_view_linearlayout_t` | `104 x 72` | `subtle` | 次级静态对照 |
| `subtle_divider` | `egui_view_divider_t` | `84 x 1` | `subtle` | 轻量分隔线 |
| `accent_panel` | `egui_view_linearlayout_t` | `104 x 72` | `accent` | 强调静态对照 |
| `accent_divider` | `egui_view_divider_t` | `84 x 2` | `accent` | 强调分隔线 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `standard` | 默认中性分隔 |
| 主控件 | `accent` | 强调活动区域 |
| 主控件 | `subtle` | 密集布局中的轻量分隔 |
| 左预览 | `subtle` | 固定静态对照 |
| 右预览 | `accent` | 固定静态对照 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主分隔线为 `standard`，同步底部 `subtle / accent` preview。
2. 请求默认截图。
3. 程序化把主分隔线切到 `accent`。
4. 请求第二张截图。
5. 程序化把主分隔线切到 `subtle`。
6. 请求第三张截图。
7. 恢复主分隔线为 `standard` 并请求最终稳定帧，确认页面没有裁切、脏态或空白。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/divider PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/divider --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主场景面板和底部 `subtle / accent` preview 都必须完整可见。
- 三张关键截图要能直接看出分隔线 tone 的变化。
- `subtle` 不能淡到不可辨识，`accent` 不能重到脱离 Fluent 主线。
- preview 不能响应触摸或键盘输入。

## 9. 已知限制与后续方向
- 当前只覆盖水平 `Separator`，不扩展垂直方向与复杂容器适配。
- 当前不做带标题的 section header 复合控件，只维护最轻量的分隔语义。
- 当前优先保证 `reference` 页面、单测和 catalog 闭环，后续如需更强页面结构组件再单独补齐。

## 10. 与现有控件的边界
- 相比 `card_panel`：这里不承载结构化卡片内容，只负责低成本切分。
- 相比 `badge_group`：这里不负责状态集合呈现，只提供布局节奏。
- 相比 `persona_group`：这里不表达集合焦点与状态，只表达区域边界。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI / WPF Separator`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`Separator`
- 本次保留核心状态：
  - `standard`
  - `subtle`
  - `accent`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 guide、标签切换和状态说明条。
- 删除强叙事卡片壳、额外图标和与分隔线无关的说明容器。
- 删除复杂 hover、动画和桌面级材质效果。

## 14. EGUI 适配时的简化点与约束
- 继续复用 SDK `divider` 的基础绘制，不下沉修改框架层。
- 通过 custom helper 统一收口颜色、alpha 和静态 preview 输入抑制。
- 当前只做固定尺寸 reference 页面，优先保证 `480 x 480` 审阅稳定性。
