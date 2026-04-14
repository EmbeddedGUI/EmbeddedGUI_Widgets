# progress_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressBar`
- 对应组件名：`ProgressBar`
- 本次保留状态：`standard`、`paused`、`error`、`indeterminate`
- 本次删除效果：页面级 guide、外部 preview 标签、场景化装饰和与进度条无关的说明壳层
- EGUI 适配说明：继续复用 SDK `progress_bar` 的 determinate 绘制能力，在 custom 层补齐 `ProgressBar` 的 `indeterminate` 语义、timer 生命周期和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`progress_bar` 用于表达明确的数值型进度，例如安装、同步、下载和处理任务。仓库里的 `ProgressBar` reference 页面已经覆盖 determinate 语义，但还缺少 Fluent / WinUI 同样核心的 `indeterminate` loading 状态，因此需要继续收口。

## 2. 为什么现有控件不够用？
- `skeleton` 适合内容占位，不表达确定百分比。
- `message_bar` 更偏消息提示，不承担过程进度反馈。
- `toast_stack` 负责短时通知，不适合持续展示任务完成度。
- SDK 自带 `progress_bar` 只有基础 determinate 绘制，没有当前仓库统一的 Fluent 风格 `indeterminate` 动画语义。

## 3. 目标场景与示例概览
- 主控件默认展示 `indeterminate` 主条，用来表达“正在同步”。
- 主控件下方保留一行轻量状态文案：loading 时显示 `Syncing...`，回落到 determinate 后再显示当前百分比。
- 底部左侧展示 `paused` 静态对照，用暖色表达“已暂停但保留进度”。
- 底部右侧展示 `error` 静态对照，用红色表达“失败状态下的最后进度”。
- 页面结构统一收口为：标题 -> 主 `progress_bar` -> 当前值文案 -> `paused / error` 双 preview。
- 两个 preview 通过 `hcw_progress_bar_override_static_preview_api()` 统一吞掉 `touch / key`，不参与交互，只做 reference 对照。
- runtime 录制先抓两帧动画中的 loading，再切到 `92% complete` 的 determinate 完成态。

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
  - `indeterminate` 使用单段低噪音流动条，不引入 striped 或更重的装饰动画。
  - 不引入额外卡片、图标和强装饰性标签。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 116` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ProgressBar` | 页面标题 |
| `progress_bar_primary` | `egui_view_progress_bar_t` | `196 x 18` | `indeterminate` | 主进度条 |
| `progress_bar_status` | `egui_view_label_t` | `196 x 14` | `Syncing...` | 主状态文案 |
| `progress_bar_paused` | `egui_view_progress_bar_t` | `96 x 12` | `46%` | `paused` 静态对照 |
| `progress_bar_error` | `egui_view_progress_bar_t` | `96 x 12` | `82%` | `error` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `indeterminate` | attach 后开始轻量流动动画 |
| 主控件 | `92% complete` | 录制收尾时回落到 determinate 完成态 |
| `paused` | `46%` | 暂停状态下保留既有进度 |
| `error` | `82%` | 错误状态下保留失败前的最后进度 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主进度条到 `indeterminate`，并同步 `paused` 和 `error` preview。
2. 请求默认截图。
3. 等待动画推进一段时间。
4. 请求第二张截图，确认流动条已经前移。
5. 程序化把主进度切到 `92%` determinate。
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
- 两张 loading 关键截图要能直接看出 `indeterminate` 条带位置变化，第三张截图要能看出回落到 determinate 完成态。
- `paused` 和 `error` 的色彩语义必须明确，但不能过度饱和。
- preview 不能响应触摸或键盘输入。

## 9. 已知限制与后续方向
- 当前不叠加任务图标、剩余时间或步骤文案。
- 当前优先保证 `reference` 页面、单测和 catalog 闭环，后续如需环形进度再单独补 `circular_progress_bar`。
- 当前 `indeterminate` 动画通过 custom 层 timer + 自定义绘制补齐，不下沉到 SDK。

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
  - `indeterminate`
  - `standard`
  - `paused`
  - `error`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 guide、说明标签和外部 preview 标题。
- 删除与业务流程强绑定的场景文案。
- 删除 striped、脉冲或更重的动画装饰，只保留单段低噪音流动条语义。

## 14. EGUI 适配时的简化点与约束
- determinate 继续使用 SDK 自带线性进度绘制，不改核心 widget 实现。
- `indeterminate` 仅在 custom 层通过附加状态表、attach / detach timer 和自定义绘制补齐。
- `set_value()` 会退出 `indeterminate` 并恢复 determinate 百分比语义。
- 页面中的进度变化全部程序化驱动，保证 runtime 录制稳定。
- preview 统一走静态 API，不承担任何真实交互职责。
- 当前只作为 `HelloCustomWidgets` 的 `reference widget` 维护，是否下沉框架层后续再评估。
