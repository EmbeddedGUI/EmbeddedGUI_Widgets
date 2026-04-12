# activity_ring 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressRing`
- 对应组件名：`ProgressRing`
- 本次保留状态：`standard`、`compact`、`paused`
- 本次删除效果：多环 fitness 风格、页面级 guide、外部 preview 标签和与进度环无关的场景壳层
- EGUI 适配说明：直接复用 SDK `activity_ring`，custom 层把多环能力收窄为单环 `ProgressRing` reference，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`activity_ring` 适合表达圆形进度或持续任务状态，常见于同步、加载和后台处理反馈。当前 `HelloCustomWidgets` 的 `reference` 主线里还没有一版对齐 Fluent / WPF UI `ProgressRing` 的页面，因此需要补齐。

## 2. 为什么现有控件不够用？
- `progress_bar` 适合线性进度，不适合需要圆形视觉重心的状态。
- `skeleton` 表达内容占位，不表达单一任务进度。
- SDK `activity_ring` 默认更偏多环能力展示，没有当前仓库统一的 Fluent `ProgressRing` reference 页面。

## 3. 目标场景与示例概览
- 主控件展示单环 determinate `ProgressRing`，录制动作覆盖 `24% -> 62% -> 86%` 三个关键进度。
- 主控件下方保留一行轻量状态文案，直接显示当前百分比，方便 runtime 截图确认数值变化。
- 底部左侧展示 `compact` 静态对照，验证更小尺寸下的单环语义。
- 底部右侧展示 `paused` 静态对照，验证暖色暂停态。
- 页面结构统一收口为：标题 -> 主 `activity_ring` -> 当前值文案 -> `compact / paused` 双 preview。
- preview 统一通过 `hcw_activity_ring_override_static_preview_api()` 吞掉 `touch / key`，不参与交互，只做 reference 对照。

目标目录：`example/HelloCustomWidgets/feedback/activity_ring/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 188`
- 主控件尺寸：`88 x 88`
- 状态文案尺寸：`196 x 14`
- 底部对照行尺寸：`104 x 48`
- `compact` 预览：`48 x 48`
- `paused` 预览：`48 x 48`
- 页面结构：标题 + 主进度环 + 当前值文案 + 底部双预览
- 样式约束：
  - 页面背景保持浅灰 panel。
  - 主环只保留单环 `ProgressRing` 语义，不展示多环健身记录风格。
  - `standard` 和 `compact` 保持蓝色主线，`paused` 使用暖色强调暂停状态。
  - 不引入额外图标、厚重卡片或高噪音渐变。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 188` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ProgressRing` | 页面标题 |
| `activity_ring_primary` | `egui_view_activity_ring_t` | `88 x 88` | `24%` | 主标准进度环 |
| `activity_ring_status` | `egui_view_label_t` | `196 x 14` | `24% active` | 主进度值文案 |
| `activity_ring_compact` | `egui_view_activity_ring_t` | `48 x 48` | `38%` | `compact` 静态对照 |
| `activity_ring_paused` | `egui_view_activity_ring_t` | `48 x 48` | `56%` | `paused` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `24%` | 默认标准进度 |
| 主控件 | `62%` | 中间进度态 |
| 主控件 | `86%` | 接近完成态 |
| `compact` | `38%` | 更小尺寸下的单环对照 |
| `paused` | `56%` | 暂停态暖色对照 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主进度环、`compact` 和 `paused` 到默认状态。
2. 请求默认截图。
3. 程序化把主进度切到 `62%`。
4. 请求第二张截图。
5. 程序化把主进度切到 `86%`。
6. 请求第三张截图。
7. 再请求一张最终稳定帧，确认文案和底部 preview 没有黑白屏、裁切或脏态。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/activity_ring --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
```

验收重点：
- 主进度环、当前值文案和底部 `compact / paused` preview 都必须完整可见。
- 三张关键截图要能直接看出主环进度变化。
- preview 不能响应触摸或键盘输入。
- 页面不能回到多环 activity demo 的非主线风格。

## 9. 已知限制与后续方向
- 当前只覆盖单环 determinate `ProgressRing`，不扩展旋转式 indeterminate 动画。
- 当前不叠加图标、剩余时间或业务步骤说明。
- 当前优先保证 `reference` 页面、单测和 catalog 闭环，后续如需更强 loading 语义再单独评估 `spinner`。

## 10. 与现有控件的边界
- 相比 `progress_bar`：这里用圆形视觉重心表达进度，而不是线性条形。
- 相比 `skeleton`：这里表达任务进度，不表达内容占位。
- 相比 SDK 原始 `activity_ring` 示例：这里不保留多环运动记录风格，只收敛为单环 `ProgressRing`。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressRing`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`ProgressRing`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `paused`

## 13. 相比参考原型删除的效果或装饰
- 删除多环 fitness 结构和强对比配色。
- 删除页面级 guide、说明标签和外部 preview 标题。
- 删除与业务流程强绑定的说明壳层。

## 14. EGUI 适配时的简化点与约束
- 直接复用 SDK `activity_ring` 的圆弧绘制，只在 custom 层固定为单环语义。
- 页面中的进度变化全部程序化驱动，保证 runtime 录制稳定。
- preview 统一走静态 API，不承担任何真实交互职责。
- 当前只作为 `HelloCustomWidgets` 的 `reference widget` 维护，是否下沉框架层后续再评估。
