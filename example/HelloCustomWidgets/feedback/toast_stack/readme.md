# toast_stack 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Snackbar`
- 补充对照控件：`message_bar`、`dialog_sheet`
- 对应组件名：`Toast / Snackbar`
- 本次保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态说明、外部 preview 标签、旧双列包裹壳层、过重 stacked card chrome、过亮 meta / action pill
- EGUI 适配说明：沿用仓库内 `toast_stack` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`toast_stack` 用于表达页内短时通知的叠卡语义，适合设置页、同步页和工作台首页展示最近 `2` 到 `3` 条轻量反馈。它不是单条横幅，也不是阻塞弹层，而是更接近 Fluent / WPF UI 的轻量 `toast / snackbar` 组合。

## 2. 为什么现有控件不够用？
- `message_bar` 更偏单条页内反馈，不强调连续 toast 的前后层级。
- `dialog_sheet` 是阻塞式弹层，不适合轻量临时消息。
- `badge_group` 只能表达汇总提醒，不能同时承载正文、动作和时间信息。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI `Toast / Snackbar` 的示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `toast_stack`，通过录制动作覆盖 `Info / Success / Warning / Error`。
- 底部左侧展示 `compact` 静态对照，保留前卡与两层叠卡语义。
- 底部右侧展示 `read only` 静态对照，验证只读状态下的弱化 stacked toast。
- 页面结构统一收口为：标题 -> 主 `toast_stack` -> `compact / read only`。
- 两个 preview 统一通过 `egui_view_toast_stack_override_static_preview_api()` 收口：
  - preview 自身吞掉 `touch / key`，不改 `current_snapshot`，也不触发 click。
  - preview 点击时只负责清主控件 focus，用于 runtime 复核交互后的收尾渲染。
- 旧的 preview 列容器、外部标签和页面桥接逻辑全部移除。

目标目录：`example/HelloCustomWidgets/feedback/toast_stack/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 232`
- 主控件尺寸：`196 x 108`
- 底部对照行尺寸：`216 x 83`
- `compact` 预览：`104 x 83`
- `read only` 预览：`104 x 83`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白底 stacked toast 和低噪音浅边框。
  - 保留 severity strip、前卡标题、正文、action 和 meta 层级，但整体回到更柔和的 Fluent / WPF UI 语法。
  - 两层后卡只保留叠卡关系和标题摘要，不再加额外页面说明壳层。
  - 底部两个 preview 不承接真实交互；`touch / key` 统一被 static preview API 吞掉，点击仅用于清主控件 focus。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toast Stack` | 页面标题 |
| `stack_primary` | `egui_view_toast_stack_t` | `196 x 108` | `Backup ready` | 标准主控件 |
| `stack_compact` | `egui_view_toast_stack_t` | `104 x 83` | `Quota alert` | `compact` 静态对照 |
| `stack_read_only` | `egui_view_toast_stack_t` | `104 x 83` | `Policy note` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_toast_stack_snapshot_t[4]` | - | `Info / Success / Warning / Error` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_toast_stack_snapshot_t[2]` | - | `Quota alert / Upload failed` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_toast_stack_snapshot_t[1]` | - | `Policy note` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup ready` | 默认 `Info` |
| 主控件 | `Draft` | `Success` |
| 主控件 | `Storage low` | `Warning` |
| 主控件 | `Upload failed` | `Error` |
| `compact` | `Quota alert` | 默认 `compact` 对照 |
| `compact` | `Upload failed` | 第二组 `compact` 对照 |
| `read only` | `Policy note` | 固定只读对照；static preview 吞掉 `touch / key`，点击只清主控件 focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `Success`。
4. 请求第二张截图。
5. 程序化切换主控件到 `Warning`。
6. 请求第三张截图。
7. 程序化切换主控件到 `Error`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot，并让主控件请求 focus。
10. 请求 `compact` 第二组 snapshot 截图。
11. 点击 `compact` preview，只清主控件 focus。
12. 请求 preview 点击后的收尾截图。
13. 再请求最终稳定帧，确认没有残留 focus ring、pressed 或整屏污染。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make clean APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc
make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见。
- `Info / Success / Warning / Error` 四态需要一眼可辨，但不能回到高饱和 showcase 风格。
- 前卡、后两层叠卡、action pill 和 meta pill 仍要保持清晰层级。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不能误提交，回到原目标后释放才提交。
- `read only / disabled` 不仅要忽略后续 `touch / key` 输入，还要在新输入或模式切换时清掉残留 `pressed` 渲染。
- static preview 必须吞掉 `touch / key`，且不能改 `current_snapshot`，也不能触发 click。
- preview 点击后的收尾帧和最终稳定帧都必须回稳，不能残留 focus ring、pressed、黑白屏、裁切或整屏污染。
- 单测必须覆盖 setter 清理 pressed、same-target release / cancel、`read only / disabled` guard 清理残留 pressed，以及 static preview 吞掉 `touch / key` 且不改 `current_snapshot`。

## 9. 已知限制与后续方向
- 当前版本仍使用固定 snapshot 数据，不接真实通知队列。
- 当前不做自动弹入 / 弹出动画，也不做滑动关闭。
- 当前优先验证 `reference` 语义、布局稳定性、交互切换后的渲染稳定性，不联动全局通知中心。

## 10. 与现有控件的边界
- 相比 `message_bar`：这里强调多条 toast 的叠卡关系，而不是单条页内反馈。
- 相比 `dialog_sheet`：这里是非阻塞反馈，不占用弹层语义。
- 相比 `badge_group`：这里承载正文、动作和时间信息，不只是数量提示。
- 相比旧版 showcase 页面：这里回到统一的 Fluent `reference` 结构，不保留叙事式壳层。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Snackbar`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`Toast / Snackbar`
- 本次保留核心状态：
  - `info`
  - `success`
  - `warning`
  - `error`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、状态说明、preview 标签和旧双列预览壳层。
- 删除 preview 参与点击切换、页面桥接和焦点承接的职责。
- 删除系统级阴影、Acrylic、自动进出场动画和过重的 stacked chrome。
- 删除与 `reference` 无关的说明性外壳和场景化叙事。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据和固定叠卡偏移保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 主控件遵守 same-target release，并在 `read only / disabled` 输入守卫与各类 setter 里共用同一套残留 `pressed` 清理语义。
- `compact / read only` 统一通过 `egui_view_toast_stack_override_static_preview_api()` 收口，吞掉 `touch / key`，不改 `current_snapshot`，点击仅用于清主控件 focus。
- 通过程序化切换 snapshot 保证 runtime 稳定抓取状态变化。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
