# AdornerDecorator reference 控件

## 1. 为什么需要这个控件

`AdornerDecorator` 是 WPF 布局体系中为内容提供 adorner layer 的容器。它本身不表达业务操作，而是在 child view 外侧承载焦点、校验、resize handle 等覆盖层反馈，适合把装饰状态和实际内容分离。

## 2. 为什么现有控件不够用

`border` 只能提供背景、边框、圆角和 padding；`resize_grip` 只表达单独的 resize 手柄；`content_control` / `content_presenter` 只负责内容承载。`AdornerDecorator` 需要同时保留 child host 与 overlay layer 语义，现有控件无法完整表达这层关系。

## 3. 目标场景与示例概览

示例页面包含标题、主 `AdornerDecorator` 和底部两个静态 preview。主区域按录制轨道切换 `standard`、`validation`、`resize`、`read only` 四组状态；底部固定展示 `compact` 与 `read only` 对照。

## 4. 视觉与布局规格

- 主控件尺寸：`166 x 82`；preview 尺寸：`84 x 42`。
- 默认圆角：`10px`；紧凑和只读样式为 `6px`。
- 默认 layer inset：`4px`；resize 样式为 `5px`，紧凑和只读样式为 `3px`。
- 默认内边距：标准 `18 / 18 / 16 / 16`，紧凑和只读 `10 / 10 / 8 / 8`。
- child view 通过 `egui_view_group_t` 管理，并在装饰区域内居中布局。

## 5. 控件清单与状态矩阵

- `egui_view_adorner_decorator_t`：派生自 `egui_view_group_t`，绘制 surface、child host、adorner layer，并承载一个 child view。
- `standard`：浅色 surface、白色 child host、蓝色 focus adorner。
- `validation`：弱红 surface、红色 validation adorner 与提示 marker。
- `resize`：浅青 surface、蓝色 focus adorner 与青色 resize handles。
- `compact`：更小 padding、圆角和 layer inset，用于紧凑静态预览。
- `read only`：弱化 overlay 颜色，仅保留 muted resize handles。

## 6. 录制动作设计

录制轨道只做程序化状态切换，不依赖用户输入。帧序列覆盖默认 focus、validation、resize、read-only 四个主状态，并最终回到默认 focus 状态；底部 `compact` 与 `read only` preview 在整条轨道中保持静态。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/adorner_decorator PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe adorner_decorator` 必须通过。
- runtime 截图不能出现黑屏、白屏、主体缺失、文字重叠或裁切。
- 分类 compile/runtime、WASM 构建和 web smoke 必须通过。

## 8. 参考设计体系与开源母本

参考 WPF `AdornerDecorator` 的核心语义：单 child host、独立 adorner layer、焦点覆盖层、校验覆盖层和 resize handles。视觉语言收口到 Fluent 2 的浅色 surface、克制边框、低噪声状态色和明确但不过度的 overlay。

## 9. 对应的 Fluent / WPF UI 组件名

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF`
- `reference_component`: `AdornerDecorator`

## 10. 保留的核心状态与删除的装饰效果

保留 child host、adorner layer、focus、validation、resize handles、compact 和 read-only 状态。删除完整的 WPF adorner visual tree、复杂命中测试、拖拽 resize 行为、阴影层级、业务图标和强场景化装饰，避免把 reference 容器做成特定业务组件。

## 11. EGUI 适配简化点与限制

当前实现是 `HelloCustomWidgets` reference 版本，不下沉到 SDK。控件支持一个 child view，通过 `egui_view_group_layout_childs()` 居中布局；adorner layer 由固定 flag 驱动绘制，暂不实现 WPF 的完整 `AdornerLayer` 子树、per-corner radius 和动态拖拽 resize 行为。

## 12. 当前验收结果

本轮将按 workflow 完成编译、单测、runtime、分类回归、WASM、web smoke 和 30 轮本地 iteration 归档，最终结果登记在 tracker 的最近完成记录中。
