# card_panel 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`Card`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、过重的 header strap、过重的 metric chip、过强的 footer action chrome
- EGUI 适配说明：保留结构化卡片、summary slot、detail strip 和轻量 action pill，仅在 `HelloCustomWidgets` 内维护 `reference widget` 版本，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`card_panel` 用来承载一张结构化信息卡：顶部 badge、标题、正文摘要、右侧 summary slot、底部 detail strip 和可选 action pill。它适合出现在设置页、概览页和详情页中，承担“在一张卡片里读完主信息和次级摘要”的职责。

## 2. 为什么现有控件不够用？
- `card` 更偏通用容器，不负责结构化信息层级。
- `layer_stack` 强调叠层和深度，不适合作为标准 Fluent 信息卡。
- `message_bar` 和 `toast_stack` 属于反馈语义，不是常驻内容摘要卡。
- 旧 showcase 页面保留了过多场景化 chrome，不适合作为当前 `reference` 主线。

## 3. 目标场景与示例概览
- 主控件展示标准 `card_panel`，录制轨道覆盖 `accent / warning / success / neutral` 四组 snapshot。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下 badge、summary 和 detail strip 仍能稳定阅读。
- 底部右侧展示 `read only` 静态对照，验证弱化 tone、隐藏 action 后的只读摘要态。
- 页面结构统一收口为：标题 -> 主 `card_panel` -> `compact / read only`。
- 两个 preview 都禁用 `touch` 和 `focus`，只承担 reference 对照，不再承接页面桥接逻辑。

目标目录：`example/HelloCustomWidgets/display/card_panel/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 90`
- `compact` 预览：`104 x 90`
- `read only` 预览：`104 x 90`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 `page panel`、白底卡片和低噪音浅边框。
  - 顶部 tone strap 只保留极轻的强调，不再作为高对比装饰。
  - 右侧 summary slot 保留 `value + label` 语义，但视觉重量低于标题和正文。
  - 底部 detail strip 与 footer 继续保留，但 read-only 版本需明显灰蓝弱化。
  - `compact` 和 `read only` 直接通过控件模式表达，不依赖外部标签说明。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Card Panel` | 页面标题 |
| `panel_primary` | `egui_view_card_panel_t` | `196 x 122` | `OVERVIEW` | 标准结构化卡片 |
| `panel_compact` | `egui_view_card_panel_t` | `104 x 90` | `TASK` | `compact` 静态对照 |
| `panel_read_only` | `egui_view_card_panel_t` | `104 x 90` | `ARCHIVE` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_card_panel_snapshot_t[4]` | - | `OVERVIEW / SYNC / DEPLOY / ARCHIVE` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_card_panel_snapshot_t[2]` | - | `TASK / WARN` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_card_panel_snapshot_t[1]` | - | `ARCHIVE` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `OVERVIEW` | 默认 `accent + emphasized` |
| 主控件 | `SYNC` | `warning + emphasized` |
| 主控件 | `DEPLOY` | `success` |
| 主控件 | `ARCHIVE` | `neutral` |
| `compact` | `TASK` | 默认紧凑对照 |
| `compact` | `WARN` | 第二组紧凑 snapshot |
| `read only` | `ARCHIVE` | 固定只读对照，隐藏 action 并弱化 tone |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `SYNC`。
4. 请求第二张截图。
5. 程序化切换主控件到 `DEPLOY`。
6. 请求第三张截图。
7. 程序化切换主控件到 `ARCHIVE`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览必须完整可见。
- header strap、metric chip、detail strip 和 footer 之间需要保留清晰层级，但不能回到高对比 showcase 风格。
- `read only` 预览必须明显弱化 tone，并隐藏 action pill。
- 页面中不再出现旧的列容器壳、guide、状态回显和外部 preview 标签。
- 底部两个 preview 只作静态 reference 对照，不承接交互职责。

## 9. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题和超长 value。
- 当前不做 hover、focus ring、键盘导航等桌面级细节。
- 当前不接入真实图标资源，只保留文本和块面层级。
- 是否沉入 `src/widget/` 作为通用控件，后续单独评估。

## 10. 与现有控件的边界
- 相比 `card`：这里不是任意容器，而是结构化信息卡。
- 相比 `layer_stack`：这里强调稳定的信息组织，不强调叠层景深。
- 相比 `message_bar`：这里是常驻内容块，不是反馈条。
- 相比 `toast_stack`：这里表达的是稳定摘要，不是短时通知。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`Card`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`

## 13. 相比参考原型删掉了哪些效果或装饰？
- 删除页面级 `guide`、状态回显、preview 标签和旧列容器壳。
- 删除过重的 header strap、metric chip、footer action chrome 和装饰性高对比 tone。
- 删除 Acrylic、真实阴影扩散和桌面级系统效果。
- 删除 hover、pressed、focus ring 等完整桌面交互细节。
- 删除多列模板化内容和任意嵌套布局扩展。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据和固定 summary slot，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件模式，减少页面级桥接逻辑。
- 录制动作只程序化切换主轨道和 `compact` 对照，`read only` 固定保持被动态。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续再评估是否下沉框架层。
