# Border reference 控件

## 1. 为什么需要这个控件

`Border` 是 WPF / WinUI 布局体系里的基础容器，用来为一个子内容提供背景、边框、圆角和内边距。它不是业务卡片，也不表达复杂状态，只负责把内容放进一个可见的边界盒中。

## 2. 为什么现有控件不够用

已有 `card_control`、`settings_card` 和 `card_panel` 都带有卡片语义或专用内容结构，不能替代基础容器。`Border` 保留最小视觉职责，适合后续控件在不引入卡片语义的情况下复用边界、背景和 padding。

## 3. 目标场景与示例概览

示例页面包含标题、主 `Border` 和底部两个静态 preview。主区域按录制轨道切换 `standard`、`accent`、`compact`、`read only` 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 默认尺寸：主控件 `166 x 92`，preview `88 x 44`。
- 默认圆角：`10px`；紧凑和只读样式为 `6px`。
- 默认边框：`1px`，颜色保持浅灰蓝。
- 默认内边距：标准 `14 / 14 / 12 / 12`，紧凑和只读 `8 / 8 / 6 / 6`。
- 子内容通过 `egui_view_group_t` child 管理居中布局。

## 5. 控件清单与状态矩阵

- `egui_view_border_t`：派生自 `egui_view_group_t`，绘制背景、边框、accent rail，并承载一个 child view。
- `standard`：白色 surface、浅灰蓝边框、蓝色 accent rail。
- `accent`：浅蓝 surface、蓝色边框和 accent rail。
- `compact`：更小圆角与 padding，绿色 accent rail。
- `read only`：弱化 surface、边框和 accent rail，作为静态 reference 预览。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖标准态、accent 态、compact 态、read-only 态，并最终回到默认标准态；底部两个 preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/border PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe border` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF / WinUI `Border` 的核心语义：`Background`、`BorderBrush`、`BorderThickness`、`CornerRadius`、`Padding` 和单一 child。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF / WinUI`
- `reference_component`: `Border`

## 10. 保留的核心状态与删除的装饰效果

保留背景、边框、圆角、内边距、子内容居中承载、紧凑和只读状态。删除业务卡片、阴影层级、标题栏、按钮、复杂图标和强场景化装饰，避免与 `Card` / `SettingCard` 混淆。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件支持一个 child view，并通过 `egui_view_group_layout_childs()` 居中布局；暂不实现 WPF 的完整 brush 模型和 per-corner radius。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
