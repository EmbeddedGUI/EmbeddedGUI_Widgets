# card_expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`CardExpander`
- 本次保留状态：`expanded`、`collapsed`、`compact`、`read only`、`accent`、`success`、`warning`
- 本次删除效果：页面级 `guide`、状态说明、额外标签、Acrylic、重阴影、Reveal/Hover 光效和 showcase 外壳
- EGUI 适配说明：在 custom 层实现单卡片 `header -> detail body` 折叠结构，统一 same-target release、键盘激活与静态 preview 输入抑制，不修改 `sdk/EmbeddedGUI`
- 补充对照实现：`ModernWpf`

## 1. 为什么需要这个控件？
`card_expander` 用来表达“卡片摘要 + 卡内展开正文”的 disclosure 语义。它比普通 `Expander` 更强调卡片壳层和摘要信息，适合策略说明、审核条目和需要按需展开细节的卡片入口。

## 2. 为什么现有控件不够用？
- `card_action` 强调整卡 action，不保留展开后的正文区域。
- `card_control` 强调右侧附加 control，不承载折叠正文。
- `expander` 当前示例更偏多段 disclosure，不是单卡片壳层。
- `settings_expander` 面向设置行和 nested rows，不适合承载通用正文说明。

因此这里单独保留 `card_expander`，把语义收口到单卡片、单 header、单段正文的 reference 形态。

## 3. 目标场景与示例概览
- 主控件展示标准 `card_expander`，覆盖 `expanded / collapsed / warning` 三组关键状态。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下的卡片折叠壳层。
- 底部右侧展示 `read only` 静态对照，验证只读弱化后的视觉层级与交互抑制。
- 页面结构统一收口为：标题 -> 主 `card_expander` -> `compact / read only` 双 preview。
- 两个 preview 都通过 `egui_view_card_expander_override_static_preview_api()` 固定为静态 reference，只吞掉输入并协助主控件收尾 focus。

目标目录：`example/HelloCustomWidgets/layout/card_expander/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 244`
- 主控件尺寸：`196 x 124`
- 底部对照行尺寸：`216 x 76`
- `compact` preview：`104 x 76`
- `read only` preview：`104 x 76`
- 页面结构：标题 + 主卡片 + 底部双 preview
- 风格约束：
  - 保持浅底、白色卡片、低噪音描边和温和 tone 差异。
  - `expanded / collapsed` 只通过 chevron 方向、正文区域和局部 tone 微差表达。
  - 主控件遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到 `A` 后 `UP(A)` 才提交。
  - `set_current_snapshot / set_expanded / set_font / set_meta_font / set_palette / set_compact_mode / set_read_only_mode` 后都不能残留旧的 `pressed` 高亮。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 244` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Card Expander` | 页面标题 |
| `card_primary` | `egui_view_card_expander_t` | `196 x 124` | `Workspace policy expanded` | 主卡片 |
| `card_compact` | `egui_view_card_expander_t` | `104 x 76` | `Compact card expanded` | `compact` 静态对照 |
| `card_read_only` | `egui_view_card_expander_t` | `104 x 76` | `Read only card expanded` | `read only` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace policy expanded` | 初始默认态 |
| 主控件 | `Identity review collapsed` | 验证收起后的壳层稳定性 |
| 主控件 | `Release approval expanded` | 作为 warning 收尾稳定态 |
| `compact` | `Compact card expanded` | 初始紧凑展开态 |
| `compact` | `Compact hold collapsed` | runtime 第二组关键帧 |
| `read only` | `Read only card expanded + read only` | 固定只读静态对照 |

## 7. 交互与状态语义
- 主控件只保留一个交互 part：`header`。
- `activate_current_part()` 触发 header 切换，统一处理 `expanded -> collapsed` 和 `collapsed -> expanded`。
- `ACTION_MOVE` 会实时更新按压渲染：
  - 移出原目标时取消 `pressed`；
  - 回到原目标后恢复 `pressed`；
  - 只有 `UP` 时仍停留在原目标上才提交切换。
- `ACTION_CANCEL` 只清理 `pressed`，不修改 `current_snapshot / expanded_state`。
- `Escape` 会在主控件处于展开态时收起。
- `read_only_mode`、`!enable` 和空数据状态都会先清理残留 `pressed`，再拒绝新的 `touch / key` 输入。
- static preview 的 `touch / key` 只消费事件并清理残留 `pressed`，不会改状态，也不会触发 listener。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认状态。
2. 请求初始帧。
3. 切到 `Identity review collapsed`。
4. 请求第二帧。
5. 切到 `Release approval expanded`。
6. 请求第三帧。
7. 切换 `compact` 到 `Compact hold collapsed`。
8. 请求 `compact` 第二帧。
9. 主动给主控件请求 `focus`。
10. 点击 `compact` preview，只执行 preview 的静态收尾逻辑。
11. 请求 preview 点击后的收尾帧，确认没有残留 `pressed / focus`。

## 9. 编译、Touch、Runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_expander PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_expander
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 折叠时只保留 header 区域；展开时正文区域必须稳定出现，不能留下异常空洞。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不提交，回到原目标后释放才提交。
- setter、guard 和 preview 统一遵守“先清理残留 `pressed` 再处理后续状态”的语义。
- static preview 的 `touch / key` 不能改动 `current_snapshot / expanded_state`，也不能触发 listener。

## 11. 已知限制与后续方向
- 当前仍是固定尺寸 `reference` 示例，不覆盖更长正文和富文本内容。
- 当前不做展开动画，只保留结构状态切换。
- 当前正文仍由静态 snapshot 驱动，不接真实数据模型。

## 12. 与现有控件的边界
- 相比 `card_action`：这里保留卡内展开正文，不是单纯 action 入口。
- 相比 `card_control`：这里不承载右侧 value / switch 等附加 control，只保留 expand / collapse affordance。
- 相比 `expander`：这里固定为单卡片壳层，不是多项 accordion。
- 相比 `settings_expander`：这里承载通用正文，而不是 nested setting rows。
