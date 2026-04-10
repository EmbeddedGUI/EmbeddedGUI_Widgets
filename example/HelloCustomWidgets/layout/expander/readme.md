# expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`Expander` / `CardExpander`
- 本次保留状态：`expanded`、`collapsed`、`compact`、`read only`、`accent`、`success`、`warning`
- 本次删除效果：页面级 `guide`、状态说明、外部 `standard / section / preview` 标签、Acrylic、重阴影、Reveal/Hover 光效和 showcase 外壳
- EGUI 适配说明：继续复用仓库内 `expander` 基础实现，本轮只收口 `reference` 页面结构、交互语义和 runtime 收尾，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`expander` 用来表达“点击标题行后展开正文，再次点击可收起”的标准 disclosure 结构。它适合设置说明、帮助段落、审核规则和需要按需展开正文的轻量信息组织场景。

## 2. 为什么现有控件不够用？
- `settings_panel` 更偏设置行和 trailing controls，不负责正文展开。
- `tree_view` 更偏层级导航，不是单层 disclosure。
- `master_detail` / `split_view` 更偏双栏联动，不适合页内垂直展开说明。
- `card_panel` 只承载固定摘要，没有展开/收起语义。

因此这里继续保留 `expander`，但示例页只保留符合 Fluent / WPF UI 主线的 `reference` 结构。

## 3. 目标场景与示例概览
- 主控件展示标准 `Expander`：覆盖 `expanded / collapsed / warning` 等关键状态。
- 底部左侧展示 `compact` 静态对照，用来验证紧凑布局和展开正文在小尺寸下的渲染稳定性。
- 底部右侧展示 `read only` 静态对照，用来验证只读弱化后的视觉层级和交互抑制。
- 页面结构统一收口为：标题 -> 主 `expander` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_expander_override_static_preview_api()` 收口：
  - preview 吞掉 `touch / key` 输入；
  - preview 只负责清理残留 `pressed`；
  - preview 不会修改 `current_index / expanded_index`，也不会触发 listener；
  - preview 点击只保留一个最小收尾逻辑：清主控件 `focus`。

目标目录：`example/HelloCustomWidgets/layout/expander/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 226`
- 主控件尺寸：`196 x 110`
- 底部对照行尺寸：`216 x 76`
- `compact` preview：`104 x 76`
- `read only` preview：`104 x 76`
- 页面结构：标题 + 主控件 + 底部双 preview
- 风格约束：
  - 保持浅底、白色卡片、低噪音描边和温和 tone 差异。
  - `warning / success / neutral` 只通过局部 tone 微差表达，不回退成高饱和 showcase 风格。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到 `A` 后 `UP(A)` 才提交。
  - `set_current_index / set_expanded_index / set_font / set_meta_font / set_palette / set_compact_mode / set_read_only_mode` 后都不能残留旧的 `pressed` 高亮。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 226` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Expander` | 页面标题 |
| `panel_primary` | `egui_view_expander_t` | `196 x 110` | `Workspace policy expanded` | 主 `expander` |
| `panel_compact` | `egui_view_expander_t` | `104 x 76` | `Mode compact` | `compact` 静态对照 |
| `panel_read_only` | `egui_view_expander_t` | `104 x 76` | `Audit read only` | `read only` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace policy expanded` | 初始默认态 |
| 主控件 | `Sync rules expanded` | 验证 success tone |
| 主控件 | `Sync rules collapsed` | 验证 collapsed 收口 |
| 主控件 | `Release notes expanded` | 作为 warning 收尾稳定态 |
| `compact` | `Mode collapsed` | 初始紧凑态 |
| `compact` | `Backup expanded` | runtime 第二组关键帧 |
| `read only` | `Audit expanded + read only` | 固定只读静态对照 |

## 7. 交互与状态语义
- 主控件保留真实 header 点击切换、键盘导航和展开/收起语义。
- `ACTION_MOVE` 会实时更新按压渲染：
  - 移出原目标时取消 `pressed`；
  - 回到原目标后恢复 `pressed`；
  - 只有 `UP` 时仍停留在原目标上才提交切换。
- `ACTION_CANCEL` 只清理 `pressed`，不修改 `current_index / expanded_index`。
- `read_only_mode`、`!enable` 和空数据状态都会先清理残留 `pressed`，再拒绝新的 `touch / key` 输入。
- static preview 的 `touch / key` 只消费事件并清理残留 `pressed`，不会改状态，也不会触发 listener。
- preview 点击不会承担额外交互职责，只用于静态对照和主控件 `focus` 收尾。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认状态。
2. 请求初始帧。
3. 切到 `Sync rules expanded`。
4. 请求第二帧。
5. 切到 `Sync rules collapsed`。
6. 请求第三帧。
7. 切到 `Release notes expanded`。
8. 请求第四帧。
9. 切换 `compact` 到 `Backup expanded`，并主动给主控件请求 `focus`。
10. 请求 `compact` 第二帧。
11. 点击 `compact` preview，只执行 preview 的静态收尾逻辑。
12. 请求 preview 点击后的收尾帧。
13. 再请求最终稳定帧，确认没有残留 `pressed / focus`、黑白屏、裁切或整屏污染。

## 9. 编译、Touch、Runtime、单测与文档检查
```bash
make clean APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc
make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- header、正文和 footer 的层级必须稳定，collapsed 后不能留下异常空洞。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不提交，回到原目标后释放才提交。
- setter、guard 和 preview 统一遵守“先清理残留 `pressed` 再处理后续状态”的语义。
- static preview 的 `touch / key` 不能改动 `current_index / expanded_index`，也不能触发 listener。
- runtime 需要重点复核初始帧、success 展开帧、collapsed 帧、warning 稳定帧、preview 点击后的收尾帧和最终稳定帧。

## 11. 已知限制与后续方向
- 当前仍是固定尺寸 `reference` 示例，不覆盖更长正文和多层嵌套 expander。
- 当前不做展开动画，只保留结构状态切换。
- 当前正文仍由静态 item 数据驱动，不接真实数据模型。
- 若后续要沉入框架层，再单独评估数据模型、动画和多层嵌套语义。

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里强调 inline disclosure，不是设置行 trailing controls。
- 相比 `tree_view`：这里没有多层树结构，只保留单层 disclosure。
- 相比 `master_detail` / `split_view`：这里不是双栏布局，而是页内纵向展开。
- 相比 `card_panel`：这里自带展开 / 收起状态，不是固定摘要卡片。
