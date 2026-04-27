# InlineUIContainer reference 控件

## 1. 为什么需要这个控件

`InlineUIContainer` 是 WPF 文本流体系中用于把一个 `UIElement` 嵌入 inline 文本行的容器。它不是普通卡片或按钮，而是表达“文本 run 中间出现一个可承载 child 的内联槽位”。

## 2. 为什么现有控件不够用

`TextBlock` 只能呈现文本；`ContentControl` 只能承载内容；`Border` 只能提供边界盒。它们都不能表达 prefix text、child host、suffix text 在同一 baseline 上排列的内联文本流语义。

## 3. 目标场景与示例概览

示例页面包含标题、主 `InlineUIContainer` 和底部两个静态 preview。主区域按录制轨道切换 `standard`、`accent baseline`、`compact`、`read only` 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 主控件尺寸：`184 x 42`；preview 尺寸：`92 x 30`。
- 默认 prefix slot：`48px`；紧凑和只读样式为 `36px`。
- 默认 inline gap：`6px`；紧凑和只读样式为 `4px`。
- 默认 baseline offset：`0px`；accent 样式为 `-2px`，read only 样式为 `1px`。
- child host 由 child 尺寸加固定内边距得到，并与 prefix / suffix 位于同一行。

## 5. 控件清单与状态矩阵

- `egui_view_inline_ui_container_t`：派生自 `egui_view_group_t`，绘制 inline surface、prefix / suffix 文本、child host，并承载一个 child view。
- `standard`：白色 surface、蓝色 child host accent、普通 baseline。
- `accent`：浅蓝 surface、上移 baseline offset，用于强调 inline child。
- `compact`：更小 prefix slot、gap 和圆角，保留紧凑文本流。
- `read only`：弱化文本、host 和 accent，仅作为静态 inline preview。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖 standard、accent、compact、read-only 四个主状态，并最终回到默认 standard 状态；底部 `compact` 与 `read only` preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/inline_ui_container PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe inline_ui_container` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF `InlineUIContainer` 的核心语义：inline text run、embedded UIElement、baseline 对齐、child host 和文本流上下文。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF`
- `reference_component`: `InlineUIContainer`

## 10. 保留的核心状态与删除的装饰效果

保留 prefix / suffix 文本、child host、baseline offset、compact 和 read-only 状态。删除完整 FlowDocument 排版、复杂 inline collection、文本选择、编辑、换行和自动测量，不把 reference 控件扩展成富文本编辑器。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件支持一个 child view，prefix / suffix 采用固定 slot 与剩余区域布局；暂不实现 WPF 的完整 inline layout engine、文本换行、嵌套 inline 或 selection 行为。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
