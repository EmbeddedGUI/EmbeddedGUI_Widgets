# info_badge 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`InfoBadge`
- 本次保留状态：`count`、`icon`、`attention dot`
- 本次删除内容：页面级 guide、showcase 式叙事容器、和 `InfoBadge` 无关的说明卡片与额外交互
- EGUI 适配说明：复用 SDK `notification_badge`，custom 层只补样式 helper、attention dot draw 语义、静态 preview API 与 reference 页面，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`InfoBadge` 用于在不打断主流程的前提下提示数量、状态或需要关注的细粒度信息。它常附着在设置项、列表行、摘要面板或状态行中，承担“轻量提醒”而不是“重型告警”的职责。

当前 `HelloCustomWidgets` 的 `reference` 主线里还没有一个对齐 Fluent / WPF UI `InfoBadge` 语义的页面，因此需要补齐。

## 2. 为什么现有控件不够用
- SDK `notification_badge` 只有基础计数/图标绘制，没有当前仓库要求的 `InfoBadge` reference 页面、静态 preview API 与 catalog 闭环。
- `badge_group` 关注一组 badge 的组合展示，不是单个附着型信息角标。
- `message_bar`、`toast_stack` 这类反馈控件层级更高，噪音更大，不适合替代行内提示。

## 3. 目标场景与示例概览
- 主场景面板展示三种核心语义：`count`、`icon`、`attention dot`
- 录制轨道覆盖三组 snapshot：默认提醒、警告提醒、安静总结
- 底部左侧 preview 展示 `99+` 溢出计数
- 底部右侧 preview 展示 attention dot 的最小语义
- 页面结构统一收口为：标题 -> 主面板 -> 两个静态 preview

目标目录：`example/HelloCustomWidgets/display/info_badge/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 246`
- 主面板尺寸：`196 x 118`
- 底部对照行尺寸：`216 x 80`
- 左侧 preview：`104 x 80`
- 右侧 preview：`104 x 80`
- 视觉约束：
  - 页面背景保持低噪音浅灰
  - 主面板与 preview 面板保持白底/浅白底
  - `count` 默认用危险红，`icon` 默认用 Fluent 蓝，`attention dot` 默认用危险红
  - 允许通过 palette setter 把 `count` / `icon` 切到成功色或警告色，但整体仍保持低面积、低噪音

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 246` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `InfoBadge` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 118` | default | 主参考面板 |
| `count_badge` | `egui_view_notification_badge_t` | `34 x 20` | `18` | `count` 语义 |
| `icon_badge` | `egui_view_notification_badge_t` | `20 x 20` | `info` | `icon` 语义 |
| `dot_badge` | `egui_view_notification_badge_t` | `12 x 12` | `attention dot` | 无字小圆点语义 |
| `overflow_badge` | `egui_view_notification_badge_t` | `40 x 20` | `99+` | 溢出计数 preview |
| `attention_badge` | `egui_view_notification_badge_t` | `12 x 12` | `attention dot` | 静态 dot preview |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主面板 | `Workspace alerts` | 默认 `count / icon / dot` 组合 |
| 主面板 | `Release board` | `warning` icon 与高优先级提醒 |
| 主面板 | `Calm summary` | 成功/安静 tone |
| 左侧 preview | `Overflow` | 固定 `99+` 溢出计数 |
| 右侧 preview | `Attention` | 固定 attention dot |

## 7. 录制动作设计
1. 重置主面板与底部 preview 到默认状态
2. 请求默认截图
3. 切到 `Release board`
4. 请求第二张截图
5. 切到 `Calm summary`
6. 请求第三张截图
7. 恢复默认状态并请求最终稳定帧

## 8. 编译、touch、runtime、文档与 catalog 检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/info_badge PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_badge --track reference --timeout 10 --keep-screenshots
```

验收重点：
- 主面板里的 `count / icon / dot` 都必须完整可见
- `99+` preview 不能因为文本溢出被裁切
- attention dot 必须在没有图标文本时仍保持稳定绘制
- 静态 preview 不能响应 `touch / key`

## 9. 已知限制与后续方向
- 当前只覆盖固定尺寸 reference 页面，不扩展到动态附着定位或复杂宿主布局
- SDK 原生没有 dot 模式，因此由 custom draw 在 `icon == NULL && style == ICON` 时收口 attention dot
- 当前不下沉到 `src/widget/`，先保持在 `HelloCustomWidgets` 的 reference 维护范围内

## 10. 与现有控件的边界
- 相比 `badge_group`：这里是单个 `InfoBadge`，不是多 badge 组合卡片
- 相比 `message_bar`：这里不承载块级反馈文案，只表达极小面积状态
- 相比 SDK `notification_badge`：这里补的是 Fluent / WPF UI `InfoBadge` 语义、页面与验证闭环

## 11. 对应组件名与本次保留的核心状态
- 对应组件名：`InfoBadge`
- 本次保留核心状态：
  - `count`
  - `icon`
  - `attention dot`

## 12. 删掉的效果或装饰
- 删除页面级 guide、标签切换和与 `InfoBadge` 无关的故事化说明块
- 删除高噪音 hover/动画装饰，只保留静态 reference 所需的变化
- 删除额外壳层和复杂组合布局，避免主语义被弱化

## 13. EGUI 适配时的简化点与约束
- 继续复用 SDK `notification_badge` 的计数与图标绘制
- custom 层只补一层 draw 适配，把 `attention dot` 收敛成无字圆点
- 通过 `hcw_info_badge_override_static_preview_api()` 统一吞掉 preview 的 `touch / key`
- palette setter 只暴露 badge/text 两个关键颜色，避免 reference 页面对外扩散多余样式面
