# relative_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 对应组件名：`RelativePanel`
- 本次保留语义：`relative rules`、`current item / rule focus`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_relative_panel`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`relative_panel` 用来表达“多个内容块通过相对关系完成排布，而不是固定网格或线性栈”的标准语义。它适合欢迎页摘要、关系图面板、设置总览和多卡片摘要页这类需要强调 `align / below / right of / align edge` 规则的场景。

## 2. 为什么现有控件不够用
- `card_control`、`master_detail` 更偏固定槽位，不负责表达通用的相对定位规则。
- `grid`、`stack_panel` 和 `split_view` 强调规则化分栏或线性布局，不适合自由关系布局。
- `scroll_presenter` 关注 viewport 平移，不承担布局规则可视化与关系语义。
- 当前仓库需要一个能完整走通 reference、单测、runtime 和 web 发布链路的 `RelativePanel` 页面。

## 3. 目标场景与示例概览
- 主控件保留真实 `RelativePanel` 语义，展示 `Anchored summary`、`Status rail`、`Dense detail` 三组关系快照。
- 底部左侧是 `compact` 静态 preview，只用于对照小尺寸下的关系压缩结果。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结关系图的弱化层级。
- 页面只保留标题、一个主 `relative_panel` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_relative_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改写 `current_snapshot / current_item / focus_on_rule`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/relative_panel/`

## 4. 视觉与布局规格
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部对照行：`216 x 88`
- `compact` preview：`104 x 88`
- `read only` preview：`104 x 88`
- 页面结构：标题 -> 主 `relative_panel` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和清晰的 rule badge / relation 文案层级。
  - 主控件继续显示 snapshot 标题、当前规则 pill、关系块与 footer 提示。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `relative_panel_primary` | `egui_view_relative_panel_t` | `196 x 160` | `Anchored summary` | 主 `RelativePanel` |
| `relative_panel_compact` | `egui_view_relative_panel_t` | `104 x 88` | `Compact anchor` | 紧凑静态对照 |
| `relative_panel_read_only` | `egui_view_relative_panel_t` | `104 x 88` | `Read only anchor` | 只读静态对照 |
| `primary_snapshots` | `egui_view_relative_panel_snapshot_t[3]` | - | `Summary / Rail / Dense` | 主控件录制轨道 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Anchored summary` | 默认状态，展示顶部摘要与右侧状态关系 |
| 主控件 | `Status rail` | 强调右侧 rail 与 review pin 的关系 |
| 主控件 | `Dense detail` | 展示更紧凑的下方详情关系 |
| `compact` | `Compact anchor` | 固定静态对照，验证窄尺寸布局压缩 |
| `read only` | `Read only anchor` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件保留真实 `RelativePanel` 键盘与触摸语义：
  - `Left / Right / Up / Down`：在可见关系块之间移动焦点
  - `Home / End`：跳到首块 / 末块
  - `Tab`：在当前规则和卡片块之间切换
  - `Enter / Space`：激活当前块或切换到下一组预设关系
- 触摸交互保持 same-target release：只在同一块 `DOWN -> UP` 时提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Anchored summary`。
2. 切到 `Status rail`，输出第二组主状态。
3. 切到 `Dense detail`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/relative_panel PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/relative_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/relative_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_relative_panel
```

验收重点：
- 主控件三组 snapshot 必须能直接看出相对关系与当前规则的变化。
- `same-target release / key navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向
- 当前只收口单容器 `RelativePanel` reference，不接入真实自动布局求解器或任意约束组合。
- 继续使用 snapshot 数组描述关系块和规则高亮，不下沉到 SDK 通用布局层。
- 若后续复用价值稳定，再评估是否与 `card_control`、`master_detail` 抽共享的关系高亮或卡片布局 helper。

## 11. 与现有控件的边界
- 相比 `grid` / `stack_panel`：这里强调块之间的相对关系，而不是行列或线性堆叠。
- 相比 `card_control`：这里负责多块关系规则和当前规则高亮，不只是单卡片排版。
- 相比 `scroll_presenter`：这里负责关系语义本身，不负责 viewport 平移。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `relative rules`
  - `current item / rule focus`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作

## 13. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/relative_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `relative_panel` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/relative_panel --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_relative_panel/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/relative_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_relative_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2165 colors=463`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8]`、`[2,3]`、`[4,5]`
  - 主区变化边界收敛到 `(59, 68) - (435, 280)`
  - 按 `y >= 281` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Anchored summary / Status rail / Dense detail` 三组 reference 状态，最终稳定帧已显式回到默认快照
