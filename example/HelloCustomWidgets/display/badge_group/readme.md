# badge_group 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件语义：`BadgeGroup`
- 当前保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`mixed group`
- 当前删除内容：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、过重的 focus badge fill、过强的 mixed row 对比、过重的 footer summary chrome
- EGUI 适配范围：只维护 `HelloCustomWidgets` 里的 `reference widget` 版本，不改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`badge_group` 用来展示一组语义相关的 badge，并让其中一个 focus badge 驱动整张卡片的 tone 和 footer summary。它适合出现在概览页、审阅页和状态面板中，用于表达“同一条信息有多个维度，但仍然属于一组”的轻量展示。

## 2. 为什么现有控件不够用
- `notification_badge` 只解决单个角标或计数，不解决多 badge 并列组合。
- `chips` 更偏交互筛选和选中态，不适合作为静态信息组合。
- `tag_cloud` 强调自由分布和权重表达，不强调 focus badge 对摘要的驱动关系。
- `card_panel` 更偏结构化卡片，不适合做低噪声的 badge 集群。

## 3. 目标场景与示例概览
- 主控件展示标准 `badge_group`，录制轨道覆盖 `accent / success / warning / neutral` 四组 snapshot。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下 badge 组合仍能稳定阅读。
- 底部右侧展示 `read only` 静态对照，验证 tone 弱化、输入抑制和只读语义。
- 页面结构固定为：标题 -> 主 `badge_group` -> `compact / read only` 双 preview。
- 两个 preview 都使用静态 preview API：吞掉 `touch / key`，只清残留 `pressed`，不改 `current_snapshot`，也不触发 click。
- preview 点击只负责收主控件焦点，不承担任何 snapshot 切换职责。

目标目录：`example/HelloCustomWidgets/display/badge_group/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 242`
- 主控件尺寸：`196 x 118`
- 底部对照行尺寸：`216 x 84`
- `compact` preview：`104 x 84`
- `read only` preview：`104 x 84`
- 页面结构：标题 + 主控件 + 底部双 preview
- 视觉约束：
  - 使用浅灰 `page panel`、白底卡片和低噪声浅边框。
  - focus badge 的 tone 只做轻量强化，不压过标题和正文层级。
  - badge 允许 `filled / outlined` 混排，但整体对比收回到 Fluent / WPF UI 的浅灰蓝主线。
  - `read only` 版本除了灰蓝弱化，还要明确抑制 `touch / key` 输入。
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 242` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Badge Group` | 页面标题 |
| `group_primary` | `egui_view_badge_group_t` | `196 x 118` | `TRIAGE` | 标准 badge 组合卡片 |
| `group_compact` | `egui_view_badge_group_t` | `104 x 84` | `SET` | `compact` 静态对照 |
| `group_read_only` | `egui_view_badge_group_t` | `104 x 84` | `ARCHIVE` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_badge_group_snapshot_t[4]` | - | `TRIAGE / QUEUE / RISK / CALM` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_badge_group_snapshot_t[2]` | - | `SET / HOLD` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_badge_group_snapshot_t[1]` | - | `ARCHIVE` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `TRIAGE` | 默认 `accent` 焦点 badge |
| 主控件 | `QUEUE` | `success` 焦点 badge |
| 主控件 | `RISK` | `warning` 焦点 badge |
| 主控件 | `CALM` | `neutral` 焦点 badge |
| `compact` | `SET` | 默认紧凑对照 |
| `compact` | `HOLD` | 第二组紧凑 snapshot |
| `read only` | `ARCHIVE` | 固定只读对照，弱化 tone 并抑制输入 |

## 7. 交互语义要求
- 主控件保持标准 clickable 语义，必须满足 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交 click
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交 click
- `ACTION_CANCEL` 必须清理 `pressed`，且不能触发 click。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()` 必须先清理残留 `pressed` 再刷新。
- `read only / disabled` 的 `touch / key guard` 必须在拒绝输入前清理残留 `pressed`。
- 静态 preview 必须吞掉 `touch / key`，不能修改 `current_snapshot`，不能触发 click。
- preview 点击只负责收主控件焦点，用于 runtime 录制生成交互后的收尾帧和最终稳定帧。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 与 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切到主控件 `QUEUE`。
4. 请求第二张截图。
5. 程序化切到主控件 `RISK`。
6. 请求第三张截图。
7. 程序化切到主控件 `CALM`。
8. 请求第四张截图。
9. 程序化切到 `compact` 第二组 snapshot，并重新请求主控件 focus。
10. 请求 `compact` 第二组截图。
11. 点击 `compact` preview 中心，只触发主控件焦点收尾。
12. 请求 preview 点击后的收尾帧。
13. 再请求一张最终稳定帧，确认渲染没有残留污染。

## 9. 编译、交互、runtime 与文档检查

```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc
make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` preview 必须完整可见。
- badge 组合最多两行，不能压到 footer summary。
- `read only` preview 除了 tone 弱化外，还必须抑制 `touch / key` 输入。
- `snapshot / compact / read only` 切换后不能残留 `pressed` 高亮。
- `same-target release`、`cancel` 和 `static preview` 必须全部通过单测。
- runtime 的 preview 点击收尾帧和最终稳定帧不能出现黑白屏、裁切、整屏污染或残留 `pressed`。

## 10. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长 label 和超过 `6` 个 badge 的数据。
- 当前不做真实图标、hover、focus ring 和复杂列表交互细节。
- 当前 badge 宽度估算基于简化字符宽度，不是完整文本测量系统。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 11. 与现有控件的边界
- 相比 `notification_badge`：这里不是单个计数泡，而是一组可混合 tone 的 badge 集群。
- 相比 `chips`：这里不是交互筛选条，不强调选中、取消和筛选结果。
- 相比 `tag_cloud`：这里不是权重词云，不做自由散点布局。
- 相比 `card_panel`：这里更轻、更扁平，重点在 badge 组合和 focus summary。

## 12. 对应组件名与保留状态
- 对应组件名：`BadgeGroup`
- 当前保留的核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`
  - `mixed group`

## 13. 删除了哪些效果或装饰
- 删除页面级 `guide`、状态回显、preview 标签和旧列容器壳。
- 删除过重的 focus badge fill、mixed row 高对比边框和 footer summary chrome。
- 删除真实图标、上下文菜单、拖拽排序和复杂过渡动画。
- 删除完整 hover / focus ring 等桌面交互细节。
- 删除动态扩容的可滚动 badge 池，只保留示例级固定容量。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + item` 数据，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件模式，减少页面级桥接逻辑。
- `read only` 不仅弱化视觉，也抑制输入，避免语义和交互脱节。
- `snapshot / compact / read only` 共用同一套 `pressed` 清理语义，保证模式切换后的卡片、focus badge 和 footer summary 不残留旧高亮。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续再评估是否下沉框架层。
