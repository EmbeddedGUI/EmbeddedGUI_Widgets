# dialog_sheet 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ContentDialog`
- 补充对照实现：`ModernWpf`
- 对应组件名：`ContentDialog`
- 本次保留状态：`warning`、`error`、`accent`、`success`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态说明、外部 preview 标签、旧双列容器壳、过重 overlay、过亮 hero / tag / footer chrome
- EGUI 适配说明：沿用仓库内 `dialog_sheet` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`dialog_sheet` 用来表达轻量确认与动作收口语义，适合同步恢复、删除确认、套用模板和发布确认这类需要短正文与明确动作按钮的场景。它不是页内反馈条，也不是通知堆栈，而是更接近 Fluent / WPF UI `ContentDialog` 的浅色确认层。

## 2. 为什么现有控件不够用？
- `message_bar` 是页内常驻反馈，不承担阻塞式动作收口。
- `toast_stack` 偏短时通知，不适合作为主要确认入口。
- `teaching_tip` 是锚定式上下文提示，不提供居中收口语义。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI 的 `ContentDialog` 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `dialog_sheet`，通过录制动作覆盖 `warning / error / accent / success` 四组 snapshot。
- 底部左侧展示 `compact` 静态对照，验证紧凑尺寸下的 title、body、tag 和单动作布局。
- 底部右侧展示 `read only` 静态对照，验证锁定后的灰蓝弱化状态。
- 页面结构统一收口为：标题 -> 主 `dialog_sheet` -> `compact / read only`。
- 底部两个 preview 都禁用 touch 和 focus，只做静态 `reference` 对照。

目标目录：`example/HelloCustomWidgets/feedback/dialog_sheet/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 258`
- 主控件尺寸：`196 x 132`
- 底部对照行尺寸：`216 x 86`
- `compact` 预览：`104 x 86`
- `read only` 预览：`104 x 86`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、低对比 overlay、白底 sheet surface 和柔和浅边框。
  - 保留 handle、hero、eyebrow、title、body、footer summary、tag 和 action row 这些核心层级。
  - `compact` 直接通过控件模式表达，不再依赖外部标签或说明文字。
  - `read only` 保留完整结构，但 tone、button 和 footer chrome 都需要明显弱化。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 258` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Dialog Sheet` | 页面标题 |
| `sheet_primary` | `egui_view_dialog_sheet_t` | `196 x 132` | `Sync issue` | 标准主控件 |
| `sheet_compact` | `egui_view_dialog_sheet_t` | `104 x 86` | `Network` | `compact` 静态对照 |
| `sheet_read_only` | `egui_view_dialog_sheet_t` | `104 x 86` | `Read only` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_dialog_sheet_snapshot_t[4]` | - | `Sync / Delete / Template / Publishing` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_dialog_sheet_snapshot_t[2]` | - | `Network / Review` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_dialog_sheet_snapshot_t[1]` | - | `Read only` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Sync issue` | 默认 `warning`，双动作 |
| 主控件 | `Delete draft` | `error`，焦点落到 secondary |
| 主控件 | `Template` | `accent`，单动作 |
| 主控件 | `Publishing` | `success`，带 close |
| `compact` | `Network` | 默认紧凑对照 |
| `compact` | `Review` | 第二组紧凑对照 |
| `read only` | `Read only` | 固定只读对照，禁用 touch / focus |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认 `warning` 截图。
3. 程序化切换主控件到 `error`。
4. 请求第二张截图。
5. 程序化切换主控件到 `accent`。
6. 请求第三张截图。
7. 程序化切换主控件到 `success`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- overlay、sheet、hero、footer summary、tag 和 action row 都必须完整可见。
- `warning / error / accent / success` 四态要一眼可辨，但整体不能回到高饱和 showcase 风格。
- 页面中不再出现 `guide`、状态说明、外部 preview 标签和旧双列包裹壳层。
- `compact` 与 `read only` 只作静态对照展示，不承担点击切换职责。
- 单测已有的 snapshot、palette、touch 和 key 语义不能回归。

## 9. 已知限制与后续方向
- 当前版本仍使用固定 snapshot 数据，不接真实业务状态流。
- 当前不做真实 modal 动画、遮罩渐变和关闭行为。
- 当前不做真实图标资源，仅保留 tone glyph。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不下沉到通用框架层。

## 10. 与现有控件的边界
- 相比 `message_bar`：这里强调确认层，不是页内反馈条。
- 相比 `toast_stack`：这里强调主动作收口，不是通知堆栈。
- 相比 `teaching_tip`：这里是居中的确认层，不是目标锚定提示。
- 相比旧版 showcase 页面：这里回到统一的 Fluent `reference` 结构，不保留叙事式页面壳层。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ContentDialog`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`ContentDialog`
- 本次保留核心状态：
  - `warning`
  - `error`
  - `accent`
  - `success`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、状态说明、preview 标签和旧双列预览容器壳。
- 删除 preview 参与点击切换、页面桥接和焦点承接的职责。
- 删除过重 overlay、过亮 hero circle、过强 footer summary / tag / action chrome。
- 删除系统级模糊、复杂阴影、入场动画和拖拽关闭语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 snapshot 保证 runtime 稳定抓取状态变化。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
