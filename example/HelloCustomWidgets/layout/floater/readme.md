# Floater reference 控件

## 1. 为什么需要这个控件

`Floater` 是 WPF FlowDocument 体系中用于在文本流中插入浮动 block 内容的容器。它表达的不是普通卡片，而是“一个可左对齐、右对齐或占满行宽的浮动内容块，周围保留文本流上下文”。

## 2. 为什么现有控件不够用

`ContentControl` 只能承载内容，`BlockUIContainer` 只表达块级内嵌宿主，`InlineUIContainer` 只表达行内宿主，`Figure` 更偏向带锚点的图文浮动对象。它们都不能单独表达 Floater 的 inline floating block、left / right / full align 和 block content context。

## 3. 目标场景与示例概览

示例页面包含标题、主 `Floater` 和底部两个静态 preview。主区域按录制轨道切换 left aligned、right aligned、full compact、read only 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 主控件尺寸：`184 x 80`；preview 尺寸：`92 x 48`。
- leading / trailing 文本占据上下文行，中间区域放置 floater host 与 wrap text。
- left / right align 时，wrap text 放在 floater 旁侧。
- full align 时，floater 占满中间行宽，wrap text 放在 floater 下方。
- child 作为简化后的 block content 在 floater 内容区居中布局，floater 左侧保留克制的 accent rail。

## 5. 控件清单与状态矩阵

- `egui_view_floater_t`：派生自 `egui_view_group_t`，绘制 floater surface、leading / wrap / trailing 文本，并承载一个 child view。
- `standard`：左对齐 floater，白色 surface，蓝色 host accent。
- `accent`：右对齐 floater，浅蓝 surface，用于强调浮动内容。
- `compact`：full align floater，更小高度、gap 和圆角。
- `read only`：弱化文本、floater host 和 accent，仅作为静态 preview。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖 left align、right align、full compact、read-only 四个主状态，并最终回到默认 left align 状态；底部 `compact` 与 `read only` preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/floater PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe floater` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF `Floater` 的核心语义：inline floating block、horizontal alignment、wrapped text context、FlowDocument 上下文以及 block content host。视觉语言收口到 Fluent 2 的浅色 surface、低噪声边框和克制 accent。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF`
- `reference_component`: `Floater`

## 10. 保留的核心状态与删除的装饰效果

保留 leading / wrap / trailing 文本、floater host、left / right / full align、compact 和 read-only 状态。删除完整 FlowDocument 排版、自动浮动避让、分页、复杂 block collection、文本选择和编辑，不把 reference 控件扩展成富文档编辑器。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件使用一个 child view 近似表达 block content，文本上下文采用固定区域布局；暂不实现 WPF 的完整 document layout engine、浮动避让、分页、嵌套 block 或 selection 行为。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
