# progress_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressBar`
- 对应组件名：`ProgressBar`
- 本次保留状态：`standard`、`paused`、`error`
- 本次删除效果：页面级 guide、外部 preview 标签、场景化装饰和与进度条无关的说明壳层
- EGUI 适配说明：直接复用 SDK `progress_bar`，custom 层只补 palette helper、value setter 和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`progress_bar` 用于表达明确的数值型进度，例如安装、同步、下载和处理任务。当前 `HelloCustomWidgets` 的 `reference` 主线里还没有一个对齐 Fluent / WPF UI 语义的 `ProgressBar` 页面，因此需要补齐。

## 2. 为什么现有控件不够用？
- `skeleton` 适合内容占位，不表达确定百分比。
- `message_bar` 更偏消息提示，不承担过程进度反馈。
- `toast_stack` 负责短时通知，不适合持续展示任务完成度。
- SDK 自带 `progress_bar` 只有基础绘制，没有当前仓库统一的 Fluent 风格 reference 页面与 catalog 闭环。

## 3. 目标场景与示例概览
- 主控件展示标准 determinate `ProgressBar`，并通过录制动作覆盖 `28% -> 58% -> 92%` 三个关键进度。
- 主控件下方保留一行轻量状态文案，直接显示当前百分比，便于 runtime 截图确认进度变化。
- 底部左侧展示 `paused` 静态对照，用暖色表达“已暂停但保留进度”。
- 底部右侧展示 `error` 静态对照，用红色表达“失败状态下的最后进度”。
- 页面结构统一收口为：标题 -> 主 `progress_bar` -> 当前值文案 -> `paused / error` 双 preview。
- 两个 preview 通过 `hcw_progress_bar_override_static_preview_api()` 统一吞掉 `touch / key`，不参与交互，只做 reference 对照。

目标目录：`example/HelloCustomWidgets/feedback/progress_bar/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 116`
- 主控件尺寸：`196 x 18`
- 状态文案尺寸：`196 x 14`
- 底部对照行尺寸：`200 x 12`
- `paused` 预览：`96 x 12`
- `error` 预览：`96 x 12`
- 页面结构：标题 + 主进度条 + 当前值文案 + 底部双预览
- 样式约束：
  - 页面背景保持浅灰 panel。
  - `standard` 使用蓝色 fill + 冷灰 track。
  - `paused` 使用橙棕 fill + 暖灰 track。
  - `error` 使用红色 fill + 浅粉灰 track。
  - 不引入额外卡片、图标、动画指示器和强装饰性标签。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 116` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ProgressBar` | 页面标题 |
| `progress_bar_primary` | `egui_view_progress_bar_t` | `196 x 18` | `28%` | 主标准进度条 |
| `progress_bar_status` | `egui_view_label_t` | `196 x 14` | `28% complete` | 主进度值文案 |
| `progress_bar_paused` | `egui_view_progress_bar_t` | `96 x 12` | `46%` | `paused` 静态对照 |
| `progress_bar_error` | `egui_view_progress_bar_t` | `96 x 12` | `82%` | `error` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `28%` | 默认标准进度 |
| 主控件 | `58%` | 处理中间态 |
| 主控件 | `92%` | 接近完成态 |
| `paused` | `46%` | 暂停状态下保留既有进度 |
| `error` | `82%` | 错误状态下保留失败前的最后进度 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主进度条、`paused` 和 `error` 到默认状态。
2. 请求默认截图。
3. 程序化把主进度切到 `58%`。
4. 请求第二张截图。
5. 程序化把主进度切到 `92%`。
6. 请求第三张截图。
7. 再请求一张最终稳定帧，确认主进度值文案和底部 preview 没有黑白屏、裁切或脏态。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/progress_bar PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/progress_bar --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主进度条、当前值文案和底部 `paused / error` preview 都必须完整可见。
- 三张关键截图要能直接看出标准进度的数值变化。
- `paused` 和 `error` 的色彩语义必须明确，但不能过度饱和。
- preview 不能响应触摸或键盘输入。

## 9. 已知限制与后续方向
- 当前只覆盖 determinate `ProgressBar`，不扩展 indeterminate 动画。
- 当前不叠加任务图标、剩余时间或步骤文案。
- 当前优先保证 `reference` 页面、单测和 catalog 闭环，后续如需环形进度再单独补 `circular_progress_bar`。

## 10. 与现有控件的边界
- 相比 `skeleton`：这里表达确定数值进度，不表达内容骨架。
- 相比 `message_bar`：这里是持续进度反馈，不是一次性消息提示。
- 相比 `toast_stack`：这里强调过程状态，不做短时通知堆栈。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressBar`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`ProgressBar`
- 本次保留核心状态：
  - `standard`
  - `paused`
  - `error`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 guide、说明标签和外部 preview 标题。
- 删除与业务流程强绑定的场景文案。
- 删除 indeterminate、striped 或更重的动画装饰，先保留最基础的 determinate 语义。

## 14. EGUI 适配时的简化点与约束
- 继续使用 SDK 自带线性进度绘制，不改核心 widget 实现。
- 页面中的进度变化全部程序化驱动，保证 runtime 录制稳定。
- preview 统一走静态 API，不承担任何真实交互职责。
- 当前只作为 `HelloCustomWidgets` 的 `reference widget` 维护，是否下沉框架层后续再评估。
