# BlockUIContainer reference 控件

## 1. 为什么需要这个控件

`BlockUIContainer` 是 WPF FlowDocument 体系中把一个 `UIElement` 放入块级文档流的容器。它表达的不是普通卡片，而是“段落之间出现一个可承载 child 的块级宿主”。

## 2. 为什么现有控件不够用

`ContentControl` 只能承载内容，`Border` 只能提供边界盒，`TextBlock` 只呈现文本；它们都不能同时表达 leading text、block-level child host、trailing text 这组三段式文档流上下文。

## 3. 目标场景与示例概览

示例页面包含标题、主 `BlockUIContainer` 和底部两个静态 preview。主区域按录制轨道切换 `standard`、`accent`、`compact`、`read only` 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 主控件尺寸：`184 x 76`；preview 尺寸：`92 x 44`。
- leading / trailing 文本占据上下文行，中间 host 占据剩余块级区域。
- 默认 host padding 为 `8 x 6`，compact / read only 为 `6 x 4`。
- 默认 block gap 为 `5px`，accent 为 `4px`，compact / read only 为 `3px`。
- child 在 host content 区域居中布局，host 左侧保留克制的 accent rail。

## 5. 控件清单与状态矩阵

- `egui_view_block_ui_container_t`：派生自 `egui_view_group_t`，绘制 block surface、leading / trailing 文本、child host，并承载一个 child view。
- `standard`：白色 surface、蓝色 host accent、标准块级间距。
- `accent`：浅蓝 surface 与 host，用于强调块级嵌入 UI。
- `compact`：更小 padding、gap 和圆角，保留紧凑文档流。
- `read only`：弱化文本、host 和 accent，仅作为静态 block preview。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖 standard、accent、compact、read-only 四个主状态，并最终回到默认 standard 状态；底部 `compact` 与 `read only` preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/block_ui_container PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe block_ui_container` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF `BlockUIContainer` 的核心语义：block-level UIElement host、FlowDocument 上下文、段落前后文本以及 child host。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF`
- `reference_component`: `BlockUIContainer`

## 10. 保留的核心状态与删除的装饰效果

保留 leading / trailing 文本、child host、host padding、compact 和 read-only 状态。删除完整 FlowDocument 排版、浮动对象、复杂 block collection、文本选择、编辑、分页和自动测量，不把 reference 控件扩展成富文档编辑器。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件支持一个 child view，leading / trailing 文本采用固定上下文行，中间 host 采用剩余区域布局；暂不实现 WPF 的完整 document layout engine、分页、嵌套 block 或 selection 行为。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
