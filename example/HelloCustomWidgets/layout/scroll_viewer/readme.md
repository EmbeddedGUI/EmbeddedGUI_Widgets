# scroll_viewer 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`、`WPF UI`
- 对应组件名：`ScrollViewer`
- 本次保留语义：`surface / viewport / scrollbar`、`vertical / horizontal offset`、`compact`、`read only`
- 本次删减内容：preview 点击清主控件焦点、`compact` 第二条 preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续复用仓库内 `scroll_viewer` 基础实现，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`scroll_viewer` 用来表达“内容区域本身可滚动，而不是只摆一个滚动条”的标准语义。它适合长设置页、诊断摘要、富文本说明、卡片列表等内容超过单屏高度、但又不想绑定到特定数据容器的场景。

## 2. 为什么现有控件不够用
- `scroll_bar` 只覆盖滚动条本身，不承担 viewport、内容裁切与滚动 surface 的完整语义。
- `data_list_panel` 更偏向列表数据容器，不适合表达任意内容块的通用滚动区域。
- `split_view`、`card_panel` 这类控件能承载内容，但不直接提供 Fluent / WinUI `ScrollViewer` 的 `reference` 验证闭环。

## 3. 目标场景与示例概览
- 主控件保留真实 `ScrollViewer` 语义，展示 `Release pane`、`Diagnostics lane`、`Backlog feed` 三组 reference 状态。
- 底部左侧是 `compact` 静态 preview，用于对照窄尺寸滚动容器与滚动条密度。
- 底部右侧是 `read only` 静态 preview，用于对照只读弱化效果与输入屏蔽语义。
- 页面只保留标题、一个主 `scroll_viewer` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_scroll_viewer_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed / pressed_part / dragging_thumb`
  - 不改写 `current_snapshot / vertical_offset / horizontal_offset`
  - 不触发 `on_view_changed`

目标目录：`example/HelloCustomWidgets/layout/scroll_viewer/`

## 4. 视觉与布局规格
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部对照行：`216 x 88`
- `compact` preview：`104 x 88`
- `read only` preview：`104 x 88`
- 页面结构：标题 -> 主 `scroll_viewer` -> `compact / read only`
- 样式约束：
  - 保留可见的滚动条 chrome、thumb 和 viewport 裁切。
  - 主控件继续显示 snapshot 标题、helper、offset 与内容块分布。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `scroll_viewer_primary` | `egui_view_scroll_viewer_t` | `196 x 160` | `Release pane` | 主 `ScrollViewer` |
| `scroll_viewer_compact` | `egui_view_scroll_viewer_t` | `104 x 88` | `Compact pane` | 紧凑静态对照 |
| `scroll_viewer_read_only` | `egui_view_scroll_viewer_t` | `104 x 88` | `Read only pane` | 只读静态对照 |
| `primary_snapshots` | `egui_view_scroll_viewer_snapshot_t[3]` | - | `Release / Diagnostics / Backlog` | 主控件录制轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Release pane` | 默认状态，展示发布清单与主滚动轨道 |
| 主控件 | `Diagnostics lane` | 展示第二组诊断内容与滚动位置变化 |
| 主控件 | `Backlog feed` | 展示紧凑卡片流与水平偏移语义 |
| `compact` | `Compact pane` | 固定静态对照，验证窄尺寸滚动容器 |
| `read only` | `Read only pane` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件保留真实 `ScrollViewer` 触摸与键盘语义：
  - 触摸轨道点击与 thumb 连续拖拽
  - 键盘 `Tab / Up / Down / Left / Right / Home / End / + / - / Enter / Space`
- `set_snapshots()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed / pressed_part / dragging_thumb` 状态。
- `read only / !enable` 期间，触摸与键盘输入都不能改写当前 snapshot 或滚动位置。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Release pane`。
2. 切到 `Diagnostics lane`，输出第二组主状态。
3. 切到 `Backlog feed`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。当前 `test.c` 已把恢复默认态后的等待与最终抓帧统一收口到 `SCROLL_VIEWER_RECORD_FINAL_WAIT`，中间状态切换继续保留 `SCROLL_VIEWER_RECORD_WAIT / SCROLL_VIEWER_RECORD_FRAME_WAIT`。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/scroll_viewer PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_viewer --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_viewer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_viewer
```

验收重点：
- 主控件三组 snapshot 必须清楚体现滚动条、thumb、内容块与 `offset` 的差异。
- `same-target release / thumb drag / key navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 验收重点
- 主控件三组 snapshot 必须清晰体现滚动条、thumb、内容块和 `offset` 的差异。
- `same-target release / thumb drag / key navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。
- README、demo 录制轨道、单测入口和验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_scroll_viewer/default`
- 复核目标：
  - 主区裁剪后只出现 `3` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `scroll_presenter`：这里保留显式 `scrollbar / thumb` chrome，而不是强调内容 surface 自身的平移。
- 相比 `scroll_bar`：这里负责完整 `viewport / surface / scrollbar / offset`，而不只是滚动条本体。
- 相比 `data_list_panel`：这里承载任意内容块，不绑定列表数据结构。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `surface / viewport / scrollbar`
  - `vertical / horizontal offset`
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸轨道点击与 thumb 拖拽
  - 键盘 `Tab / Up / Down / Left / Right / Home / End / + / - / Enter / Space`
- 删减的装饰或桥接：
  - preview 点击清主控件焦点
  - `compact` 第二条 preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/scroll_viewer PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `scroll_viewer` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_viewer --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_scroll_viewer/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_viewer`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_viewer`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2165 colors=374`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(49, 63) - (430, 285)`
- 按 `y >= 286` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Release pane`、`Diagnostics lane` 与 `Backlog feed` 三组 reference 状态，最终稳定帧已显式回到默认快照
