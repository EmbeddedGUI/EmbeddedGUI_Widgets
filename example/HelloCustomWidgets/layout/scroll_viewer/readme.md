# ScrollViewer 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 / ScrollViewer`
- 对应组件：`ScrollViewer`
- 当前保留形态：`Release pane`、`Diagnostics lane`、`Backlog feed`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `track same-target release`、`thumb drag` 与键盘 `Tab / Up / Down / Left / Right / Home / End / + / - / Enter / Space` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、`compact` 第二条 preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_scroll_viewer`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`scroll_viewer` 用来表达“内容区域本身可滚动，同时保留显式滚动条 chrome”的语义，适合长设置页、诊断摘要、富文本说明、卡片列表等内容超过单屏高度、但又不想绑定到特定数据容器的场景。

## 2. 为什么现有控件不够用
- `scroll_bar` 只覆盖滚动条本身，不承担 viewport、内容裁切与滚动 surface 的完整语义。
- `data_list_panel` 更偏向列表数据容器，不适合表达任意内容块的通用滚动区域。
- `split_view`、`card_panel` 这类控件能承载内容，但不直接提供 Fluent / WinUI `ScrollViewer` 的 `reference` 验证闭环。
- 当前仓库仍需要一个能完整走通 `reference`、单测、runtime 和 web 发布链路的 `ScrollViewer` 页面。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `scroll_viewer` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Release pane`
  - `Diagnostics lane`
  - `Backlog feed`
- 录制最终稳定帧显式回到默认 `Release pane`。
- 底部左侧是 `Compact` 静态 preview，只负责对照窄尺寸滚动容器与滚动条密度。
- 底部右侧是 `Read only` 静态 preview，只负责对照只读弱化效果与输入屏蔽语义。
- 两个 preview 统一通过 `egui_view_scroll_viewer_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed / pressed_part / dragging_thumb`
  - 不改 `current_snapshot / vertical_offset / horizontal_offset`
  - 不触发 `on_view_changed`

目标目录：
- `example/HelloCustomWidgets/layout/scroll_viewer/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Release pane`
2. 快照 2
   `Diagnostics lane`
3. 快照 3
   `Backlog feed`
4. 最终稳定帧
   回到默认 `Release pane`

底部 preview 在整条轨道中固定为：
1. `Compact pane`
2. `Read only pane`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部 preview 行：`216 x 88`
- 单个 preview：`104 x 88`
- 页面结构：标题 -> 主 `scroll_viewer` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、可见的 `scrollbar / thumb` chrome 与清晰的 `viewport / offset` 信息层级；主区突出内容块、helper 和滚动条位置；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Release pane` | `Compact pane` | `Read only pane` |
| 快照 2 | `Diagnostics lane` | 保持不变 | 保持不变 |
| 快照 3 | `Backlog feed` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Release pane` | 保持不变 | 保持不变 |
| track release / thumb drag / 键盘滚动 / Tab 焦点切换 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_scroll_viewer.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认 offset 回落、空快照 reset，以及默认 `scrollbar_visibility` 校验。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_scrollbar_visibility()` 的 `pressed / pressed_part / dragging_thumb` 清理与状态更新。
3. `get_part_region()`、`scroll_page()`、`scroll_line()` 与 `on_view_changed` listener 行为。
4. 触摸轨道 same-target release、移出取消与 thumb drag 行为，验证 `pressed` 清理和滚动位置更新。
5. 键盘 `Tab / Enter / Home / End / Left / Right / - / Escape` 行为，覆盖 `surface / thumb` 焦点切换、滚动与当前部位回落。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / vertical_offset / horizontal_offset` 不变并清理 `pressed / dragging_thumb`；恢复后继续验证 `End` 导航。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / vertical_offset / horizontal_offset / scrollbar_visibility / current_part / compact_mode / read_only_mode` 不变，同时不触发 `on_view_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Release pane`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Diagnostics lane`，等待 `SCROLL_VIEWER_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Backlog feed`，等待 `SCROLL_VIEWER_RECORD_FINAL_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Release pane`，同时重放底部 preview 固定状态，等待 `SCROLL_VIEWER_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `SCROLL_VIEWER_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实轨道 same-target release、thumb drag、键盘导航与 `on_view_changed` 语义，供手动复核和单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径；README 这里按当前实现如实保留第 `6` 步使用 `SCROLL_VIEWER_RECORD_WAIT` 的等待口径。

## 9. 验收命令
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

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Release pane`、`Diagnostics lane`、`Backlog feed` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留轨道 same-target release、thumb drag、键盘导航与 `on_view_changed` 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / vertical_offset / horizontal_offset / scrollbar_visibility / current_part / compact_mode / read_only_mode`，也不能触发 `on_view_changed`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_scroll_viewer` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_scroll_viewer/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(49, 63) - (430, 285)`
  - 以 `y >= 286` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Release pane`

## 12. 与现有控件的边界
- 相比 `scroll_presenter`：这里保留显式 `scrollbar / thumb` chrome，而不是强调内容 surface 自身的平移。
- 相比 `scroll_bar`：这里负责完整 `viewport / surface / scrollbar / offset`，而不只是滚动条本体。
- 相比 `data_list_panel`：这里承载任意内容块，不绑定列表数据结构。
- 相比 `card_panel`：这里强调完整滚动容器和滚动位置语义，而不是单纯卡片承载。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Release pane`
  - `Diagnostics lane`
  - `Backlog feed`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - 轨道 same-target release
  - thumb drag
  - 键盘 `Tab / Up / Down / Left / Right / Home / End / + / - / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - `compact` 第二条 preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/scroll_viewer PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `scroll_viewer` suite `7 / 7`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_scroll_viewer/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_viewer`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_viewer`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2165 colors=374`
- 截图复核结论：
  - 主区覆盖 `Release pane / Diagnostics lane / Backlog feed` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Release pane`
  - 主区差分边界收敛到 `(49, 63) - (430, 285)`
  - 以 `y >= 286` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
