# RelativePanel 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 / RelativePanel`
- 对应组件：`RelativePanel`
- 当前保留形态：`Anchored summary`、`Status rail`、`Dense detail`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `same-target release`、`Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘闭环与 `rule focus` 语义；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_relative_panel`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`relative_panel` 用来表达“多个内容块通过相对关系完成排布，而不是固定网格或线性栈”的布局语义，适合欢迎页摘要、关系图面板、设置总览和多卡片摘要页这类需要强调 `align / below / right of / align edge` 规则的场景。

## 2. 为什么现有控件不够用
- `card_control`、`master_detail` 更偏固定栏位，不负责表达通用的相对定位规则。
- `grid`、`stack_panel`、`split_view` 强调规则化分栏或线性布局，不适合自由关系布局。
- `scroll_presenter` 关注 viewport 平移，不承担布局规则可视化与关系语义。
- `canvas` 更偏绝对定位，不提供当前项、规则焦点和提交语义。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `relative_panel` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Anchored summary`
  - `Status rail`
  - `Dense detail`
- 录制最终稳定帧显式回到默认 `Anchored summary`。
- 底部左侧是 `Compact` 静态 preview，只负责对照小尺寸下的关系压缩结构。
- 底部右侧是 `Read only` 静态 preview，只负责对照冻结关系图的弱化层级。
- 两个 preview 统一通过 `egui_view_relative_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_item / focus_on_rule`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/relative_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Anchored summary`
2. 快照 2
   `Status rail`
3. 快照 3
   `Dense detail`
4. 最终稳定帧
   回到默认 `Anchored summary`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部 preview 行：`216 x 88`
- 单个 preview：`104 x 88`
- 页面结构：标题 -> 主 `relative_panel` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和清晰的 `rule badge / relation text / footer hint` 层级；主区突出相对关系规则与当前块切换；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Anchored summary` | `Compact anchor` | `Read only anchor` |
| 快照 2 | `Status rail` | 保持不变 | 保持不变 |
| 快照 3 | `Dense detail` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Anchored summary` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 / 规则焦点 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_relative_panel.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认项回落、空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()`、`set_current_item()` 的 pressed 清理与状态更新。
3. `get_item_region()`、`activate_current_item()` 与 listener 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
5. 键盘 `Home / End / Left / Right / Up / Down / Tab / Enter` 行为，含 `rule focus` 切换和跨 snapshot 的 `Enter` 循环。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_item` 不变并清理 pressed；恢复后继续验证 `Home / End + Enter`。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_item / focus_on_rule / compact_mode / read_only_mode` 不变，同时不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Anchored summary`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Status rail`，等待 `RELATIVE_PANEL_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Dense detail`，等待 `RELATIVE_PANEL_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Anchored summary`，同时重放底部 preview 固定状态，等待 `RELATIVE_PANEL_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `RELATIVE_PANEL_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 same-target release、键盘导航、规则焦点与 listener 语义，供手动复核和单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/relative_panel PORT=pc

# 在 X:\ 短路径下执行
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

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Anchored summary`、`Status rail`、`Dense detail` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、键盘导航、规则焦点与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_item / focus_on_rule / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_relative_panel` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_relative_panel/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(59, 68) - (435, 280)`
  - 以 `y >= 281` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Anchored summary`

## 12. 与现有控件的边界
- 相比 `grid` / `stack_panel`：这里强调块之间的相对关系，而不是行列或线性堆叠。
- 相比 `card_control`：这里负责多块关系规则和当前规则高亮，不只是单卡片排版。
- 相比 `scroll_presenter`：这里负责关系语义本身，不负责 viewport 平移。
- 相比 `canvas`：这里不是绝对坐标布局，而是带关系规则和当前项的 reference 页面。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Anchored summary`
  - `Status rail`
  - `Dense detail`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/relative_panel PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 当前环境下直接执行 `X:\output\main.exe` 输出异常为 `Hello, egui!`，本轮按本地 unit 日志复核总计 `845 / 845`，其中 `relative_panel` suite `7 / 7`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_relative_panel/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/relative_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_relative_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2165 colors=463`
- 截图复核结论：
  - 主区覆盖 `Anchored summary / Status rail / Dense detail` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Anchored summary`
  - 主区差分边界收敛到 `(59, 68) - (435, 280)`
  - 以 `y >= 281` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
