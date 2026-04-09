# message_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI InfoBar`
- 补充对照控件：`toast_stack`、`dialog_sheet`
- 对应组件名：`MessageBar / InfoBar`
- 本次保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态说明、外部 preview 标签、旧双列包裹壳层、过强 severity chrome、过重 action button 强调
- EGUI 适配说明：沿用仓库内 `message_bar` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`message_bar` 用于表达页内常驻但不阻塞的反馈信息，覆盖 `info / success / warning / error` 四类常见状态，适合设置页、表单页、同步页和后台管理页顶部的轻量提示。

## 2. 为什么现有控件不够用？
- `toast_stack` 更偏瞬时通知，不适合承担页内单条反馈条。
- `dialog_sheet` 是阻塞式弹层，不适合常驻提示。
- `badge_group` 只能表达汇总提醒，不能同时承载标题、正文和动作语义。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI `MessageBar / InfoBar` 的示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `message_bar`，通过录制动作覆盖 `Info / Success / Warning / Error`。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下的标题、正文和动作层级。
- 底部右侧展示 `read only` 静态对照，验证禁用外部交互后的弱化状态。
- 页面结构统一收口为：标题 -> 主 `message_bar` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和页面桥接逻辑全部移除。

目标目录：`example/HelloCustomWidgets/feedback/message_bar/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 220`
- 主控件尺寸：`196 x 96`
- 底部对照行尺寸：`216 x 82`
- `compact` 预览：`104 x 82`
- `read only` 预览：`104 x 82`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白底 message bar 和低噪音浅边框。
  - 保留 severity accent、leading glyph、标题 / 正文 / action 层级，但整体回到更柔和的 Fluent / WPF UI 语法。
  - `compact` 与 `read only` 直接通过控件模式表达，不再依赖外部标签。
  - 底部两个 preview 都禁用 touch 和 focus，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 220` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Message Bar` | 页面标题 |
| `bar_primary` | `egui_view_message_bar_t` | `196 x 96` | `Info` | 标准主控件 |
| `bar_compact` | `egui_view_message_bar_t` | `104 x 82` | `Warning` | `compact` 静态对照 |
| `bar_read_only` | `egui_view_message_bar_t` | `104 x 82` | `Policy note` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_message_bar_snapshot_t[4]` | - | `Info / Success / Warning / Error` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_message_bar_snapshot_t[2]` | - | `Quota alert / Sync failed` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_message_bar_snapshot_t[1]` | - | `Policy note` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Updates ready` | 默认 `Info` |
| 主控件 | `Settings saved` | `Success` |
| 主控件 | `Storage almost full` | `Warning` |
| 主控件 | `Connection lost` | `Error` |
| `compact` | `Quota alert` | 默认 `compact` 对照 |
| `compact` | `Sync failed` | 第二组 `compact` 对照 |
| `read only` | `Policy note` | 固定只读对照，禁用 touch / focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `Success`。
4. 请求第二张截图。
5. 程序化切换主控件到 `Warning`。
6. 请求第三张截图。
7. 程序化切换主控件到 `Error`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见。
- `Info / Success / Warning / Error` 四态需要一眼可辨，但不能回到高饱和 showcase 风格。
- `compact` 在小尺寸下仍要保留 message bar 的信息层级。
- `read only` 只做静态展示，不能响应 touch、focus 或页面桥接。
- 单测已有的 snapshot、palette、touch 和 key click 语义不能回归。

## 9. 已知限制与后续方向
- 当前版本仍使用固定 snapshot 数据，不接真实业务状态流。
- 当前不做真实关闭动作、多动作按钮和展开正文。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不联动外部反馈容器。

## 10. 与现有控件的边界
- 相比 `toast_stack`：这里强调单条页内反馈，不是堆叠通知。
- 相比 `dialog_sheet`：这里是非阻塞反馈，不占用弹层语义。
- 相比 `badge_group`：这里承载标题、正文、action 和 close 语义。
- 相比旧版 showcase 页面：这里回到统一的 Fluent `reference` 结构，不保留叙事式壳层。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI InfoBar`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`MessageBar / InfoBar`
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
- 删除过重的 severity strip、glyph circle、action button 和只读 pin chrome。
- 删除与 `reference` 无关的说明性外壳和场景化叙事。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 snapshot 保证 runtime 稳定抓取状态变化。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
