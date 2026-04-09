# badge_group 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`BadgeGroup`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`mixed group`
- 本次删除效果：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、过重的 focus badge fill、过强的 mixed row 对比、过重的 footer summary chrome
- EGUI 适配说明：保留多 badge 组合、focus badge 驱动摘要和 read-only 对照语义，仅在 `HelloCustomWidgets` 内维护 `reference widget` 版本，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`badge_group` 用来展示一组语义相关的 badge，并让其中一个 focus badge 驱动整张卡片的 tone 和 footer summary。它适合出现在概览页、审阅页和状态面板里，表达“这一组标签是同一条信息的多个维度”。

## 2. 为什么现有控件不够用？
- `notification_badge` 只解决单个角标或计数，不解决多 badge 并列展示。
- `chips` 更偏交互筛选和选中态，不适合作为静态信息组合。
- `tag_cloud` 强调自由分布和权重表达，不强调 focus badge 与 summary。
- `card_panel` 更偏结构化信息卡，不适合做轻量 badge 集群。

## 3. 目标场景与示例概览
- 主控件展示标准 `badge_group`，录制轨道覆盖 `accent / success / warning / neutral` 四组 snapshot。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下 badge 组合仍能稳定阅读。
- 底部右侧展示 `read only` 静态对照，验证 tone 弱化和输入抑制后的被动态。
- 页面结构统一收口为：标题 -> 主 `badge_group` -> `compact / read only`。
- 两个 preview 都禁用 `touch` 和 `focus`，只承担 reference 对照，不再承接页面桥接逻辑。

目标目录：`example/HelloCustomWidgets/display/badge_group/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 242`
- 主控件尺寸：`196 x 118`
- 底部对照行尺寸：`216 x 84`
- `compact` 预览：`104 x 84`
- `read only` 预览：`104 x 84`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 `page panel`、白底卡片和低噪音浅边框。
  - focus badge 的 tone 只作为轻量强调，不再压过标题和正文层级。
  - badge 允许 `filled / outlined` 混排，但整体对比需收敛到 Fluent/WPF UI 浅灰蓝主线。
  - footer summary 保留焦点 badge 驱动语义，但 read-only 版本必须明显灰蓝弱化。
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 242` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Badge Group` | 页面标题 |
| `group_primary` | `egui_view_badge_group_t` | `196 x 118` | `TRIAGE` | 标准 badge 组合卡 |
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

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `QUEUE`。
4. 请求第二张截图。
5. 程序化切换主控件到 `RISK`。
6. 请求第三张截图。
7. 程序化切换主控件到 `CALM`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览必须完整可见。
- badge 组合最多两行，不能压到 footer summary。
- `read only` 预览除了 tone 弱化外，还必须抑制输入，不能再触发点击或键盘激活。
- 主控件录制切换后的关键帧里，focus badge、footer summary 和 mixed row 层级必须仍然清晰。
- 页面中不再出现旧列容器壳、guide、状态回显和外部 preview 标签。

## 9. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长 label 和超过 6 个 badge 的数据。
- 当前不做真实图标、hover、focus ring 和复杂桌面交互细节。
- 当前 badge 宽度估算基于简化字符宽度，不是完整文本测量系统。
- 是否沉入 `src/widget/` 作为通用控件，后续单独评估。

## 10. 与现有控件的边界
- 相比 `notification_badge`：这里不是单个计数泡，而是一组可混合 tone 的 badge 集群。
- 相比 `chips`：这里不是交互筛选条，不强调选中、取消和筛选结果。
- 相比 `tag_cloud`：这里不是权重词云，不做自由散点布局。
- 相比 `card_panel`：这里更轻、更扁平，重点在 badge 组合与 focus summary。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`BadgeGroup`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`
  - `mixed group`

## 13. 相比参考原型删掉了哪些效果或装饰？
- 删除页面级 `guide`、状态回显、preview 标签和旧列容器壳。
- 删除过重的 focus badge fill、mixed row 高对比边框和 footer summary chrome。
- 删除真实图标、上下文菜单、拖拽排序和复杂过渡动画。
- 删除 hover、focus ring 等完整桌面交互细节。
- 删除动态扩容的可滚动 badge 池，只保留示例级固定容量。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + item` 数据，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件模式，减少页面级桥接逻辑。
- `read only` 不仅弱化视觉，也抑制输入，避免语义和交互脱节。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续再评估是否下沉框架层。
