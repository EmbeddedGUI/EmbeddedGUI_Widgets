# Figure reference 控件

## 1. 为什么需要这个控件

`Figure` 是 WPF FlowDocument 体系中用于在文本流中放置浮动内容块的容器。它表达的不是普通卡片，而是“一个带锚点的浮动 child host，周围保留可环绕的文本上下文”。

## 2. 为什么现有控件不够用

`ContentControl` 只能承载内容，`BlockUIContainer` 只表达块级内嵌宿主，`InlineUIContainer` 只表达行内宿主。它们都不能表达 left / right / center anchor、floating figure host 与 wrap text context 的组合语义。

## 3. 目标场景与示例概览

示例页面包含标题、主 `Figure` 和底部两个静态 preview。主区域按录制轨道切换 left anchored、right anchored、centered compact、read only 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 主控件尺寸：`184 x 80`；preview 尺寸：`92 x 48`。
- leading / trailing 文本占据上下文行，中间区域放置 figure host 与 wrap text。
- left / right anchor 时，wrap text 放在 figure 旁侧。
- center anchor 时，wrap text 放在 figure 下方。
- child 在 figure content 区域居中布局，figure 左侧保留克制的 accent rail。

## 5. 控件清单与状态矩阵

- `egui_view_figure_t`：派生自 `egui_view_group_t`，绘制 figure surface、leading / wrap / trailing 文本，并承载一个 child view。
- `standard`：左锚定 figure，白色 surface，蓝色 host accent。
- `accent`：右锚定 figure，浅蓝 surface，用于强调浮动内容。
- `compact`：居中 figure，更小尺寸、gap 和圆角。
- `read only`：弱化文本、figure host 和 accent，仅作为静态 preview。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖 left anchor、right anchor、center compact、read-only 四个主状态，并最终回到默认 left anchor 状态；底部 `compact` 与 `read only` preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/figure PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe figure` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF `Figure` 的核心语义：floating content、horizontal anchor、wrapped text context、FlowDocument 上下文以及 child host。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF`
- `reference_component`: `Figure`

## 10. 保留的核心状态与删除的装饰效果

保留 leading / wrap / trailing 文本、figure host、left / right / center anchor、compact 和 read-only 状态。删除完整 FlowDocument 排版、自动浮动避让、分页、复杂 block collection、文本选择和编辑，不把 reference 控件扩展成富文档编辑器。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件支持一个 child view，文本上下文采用固定区域布局；暂不实现 WPF 的完整 document layout engine、浮动避让、分页、嵌套 block 或 selection 行为。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
