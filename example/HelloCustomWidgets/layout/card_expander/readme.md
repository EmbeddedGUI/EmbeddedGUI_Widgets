# card_expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI CardExpander`
- 对应组件名：`CardExpander`
- 本次保留语义：`workspace policy / identity review / release approval`、`compact`、`read only`、`expanded / collapsed`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_card_expander`，本轮只收口 `reference` 页面结构、录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`card_expander` 用来表达“卡片摘要 + 卡内展开正文”的 disclosure 语义。它比普通 `Expander` 更强调卡片壳层、摘要信息和正文仍留在同一张卡片内的关系，适合策略说明、审核条目和需要按需展开细节的卡片入口。

## 2. 为什么现有控件不够用
- `card_action` 强调整卡 action，不保留展开后的正文区域。
- `card_control` 强调右侧附加 control，不承载折叠正文。
- `expander` 当前示例更偏多段 disclosure，不是单卡片壳层。
- `settings_expander` 面向设置行和 nested rows，不适合承载通用正文说明。

## 3. 目标场景与示例概览
- 主控件保留真实 `CardExpander` 语义，展示 `Workspace policy`、`Identity review`、`Release approval` 三组 snapshot。
- 底部左侧是 `compact` 静态 preview，只用于对照缩小尺寸下的卡片折叠壳层。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化视觉。
- 页面只保留标题、一个主 `card_expander` 和两个静态 preview，不再让 preview 负责清焦点或收尾叙事。
- 两个 preview 统一通过 `egui_view_card_expander_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改动 `current_snapshot / current_part / expanded_state`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/card_expander/`

## 4. 视觉与布局规格
- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部对照行：`216 x 76`
- `compact` preview：`104 x 76`
- `read only` preview：`104 x 76`
- 页面结构：标题 -> 主 `card_expander` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 tone 区分。
  - `expanded / collapsed` 只通过 chevron 方向、正文区域和局部 tone 微差表达。
  - 主区只保留 header 作为真实交互目标，不再叠加旧 preview focus bridge。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `card_primary` | `egui_view_card_expander_t` | `196 x 124` | `Workspace policy` | 主 `CardExpander` |
| `card_compact` | `egui_view_card_expander_t` | `104 x 76` | `Compact card` | 紧凑静态对照 |
| `card_read_only` | `egui_view_card_expander_t` | `104 x 76` | `Read only card` | 只读静态对照 |
| `primary_snapshots` | `egui_view_card_expander_snapshot_t[3]` | - | `Workspace / Identity / Release` | 主控件语义轨道 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace policy expanded` | 默认状态，展示 accent + 展开正文 |
| 主控件 | `Identity review collapsed` | 第二组 snapshot，验证收起后的壳层稳定性 |
| 主控件 | `Release approval expanded` | 第三组 snapshot，展示 warning + 展开正文 |
| `compact` | `Compact card expanded` | 固定静态对照，验证紧凑尺寸下的折叠壳层 |
| `read only` | `Read only card expanded` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件只保留一个交互 part：`header`。
- `activate_current_part()` 统一处理 `expanded -> collapsed` 和 `collapsed -> expanded` 的切换。
- 触摸交互保持 same-target release：只有同一 `header` 上的 `DOWN -> UP` 才提交；移出后不提交，回到原目标后再释放才提交。
- `Home / End / Tab / Up / Down` 会把当前交互目标保持在 `header`；`Enter / Space` 切换展开状态；`Escape` 在展开时收起。
- `set_snapshots()`、`set_current_snapshot()`、`set_expanded()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`。
- `read only`、`!enable` 和 static preview 都会拒绝新的交互输入，同时保持状态不变。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Workspace policy expanded`。
2. 切到 `Identity review collapsed`，输出第二组主状态。
3. 切到 `Release approval expanded`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 `reference` 轨道里保持静态一致，不再承担 preview dismiss 或焦点清理职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_expander PORT=pc

# 在 X:\ 短路径下执行，修改单测后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_expander
```

验收重点：
- 主控件必须直接看出 `CardExpander` 在三组 snapshot 下保持稳定卡片折叠结构。
- `same-target release / keyboard activation / Escape / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向
- 当前只收口单卡 `CardExpander` reference，不覆盖更长正文、富文本或复杂动画。
- 当前仍使用 snapshot 驱动的固定文本数据，不承接真实业务模型。
- 若后续确认复用价值稳定，再评估是否抽象为更通用的 SDK 折叠卡片控件。

## 11. 与现有控件的边界
- 相比 `card_action`：这里保留卡内展开正文，不是单纯 action 入口。
- 相比 `card_control`：这里不承载右侧 value / switch control，只保留 expand / collapse affordance。
- 相比 `expander`：这里固定为单卡片壳层，不是多项 accordion。
- 相比 `settings_expander`：这里承载通用正文，而不是 nested setting rows。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `workspace policy`
  - `identity review`
  - `release approval`
  - `compact`
  - `read only`
  - `expanded / collapsed`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Tab / Up / Down / Enter / Space / Escape`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作
