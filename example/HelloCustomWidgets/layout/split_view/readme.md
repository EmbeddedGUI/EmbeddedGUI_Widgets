# SplitView 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / SplitView`
- 补充参考：`ModernWpf`
- 对应组件：`SplitView`
- 当前保留形态：`Overview open`、`Overview compact`、`Review`、`Archive`、`Compact`、`Read only`
- 当前保留交互：主区保留 row 选中、pane toggle 与键盘导航；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 点击桥接、额外的 `compact` preview 交互轨道，以及录制尾部的 `preview click` 收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/split_view` 现有实现；本轮只收口 README、reference 录制说明、静态 preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`split_view` 用来表达“可折叠导航 pane + detail 面板”的双栏结构。它适合设置中心、资料库、审核工作区这类需要左侧导航与右侧内容同屏存在的场景。

## 2. 为什么现有控件不够用
- `nav_panel` 只覆盖导航 rail，不承载 detail pane。
- `data_list_panel` 更偏单卡片列表，不等同于可折叠 pane 结构。
- `master_detail` 更偏主从阅读，不是 `SplitView` 的导航抽屉语义。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `split_view` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `4` 组录制快照：
  - `Overview open`
  - `Overview compact`
  - `Review`
  - `Archive`
- 录制最终稳定帧显式回到默认 `Overview open`。
- 底部左侧是 `Compact` 静态 preview，固定展示紧凑态和收起 pane。
- 底部右侧是 `Read only` 静态 preview，固定展示只读态。
- 两个 preview 统一通过 `egui_view_split_view_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改变 `current_index / pane_expanded / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed / on_pane_state_changed`

目标目录：
- `example/HelloCustomWidgets/layout/split_view/`

## 4. 主区 reference 快照
主区录制轨道只保留 `4` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Overview open`
2. 快照 2
   `Overview compact`
3. 快照 3
   `Review`
4. 快照 4
   `Archive`
5. 最终稳定帧
   回到默认 `Overview open`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主控件：`196 x 104`
- 底部 preview 行：`216 x 74`
- 单个 preview：`104 x 74`
- 页面结构：标题 -> 主 `split_view` -> 底部 `Compact / Read only`
- 风格约束：保持浅底、白色 pane 卡片、低噪音边框和轻量 detail 面板层级；`warning / neutral` 只通过 tone 差异表达，不回退到高噪音 showcase 风格；主区继续遵守 same-target release 与 `pressed` 清理语义。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Overview open` | `Compact` | `Read only` |
| 快照 2 | `Overview compact` | 保持不变 | 保持不变 |
| 快照 3 | `Review` | 保持不变 | 保持不变 |
| 快照 4 | `Archive` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Overview open` | 保持不变 | 保持不变 |
| 主区真实 row 选中、pane toggle 与键盘导航 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_split_view.c` 当前覆盖 `10` 条用例：

1. `set_items()` 的 clamp、`current_index` 回退、listener guard 与空数据回退。
2. `set_font()`、`set_meta_font()`、`set_palette()`、`set_pane_expanded()`、`set_compact_mode()`、`set_read_only_mode()` 都要先清理 `pressed`。
3. metrics、hit testing 与 compact/collapsed 布局下的 pane/detail 区域口径。
4. toggle 与 row 的 same-target release / cancel 语义。
5. `compact_mode` 切换后清掉旧 `pressed`，但不破坏已有 selection 行为。
6. 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space` 导航与 pane toggle。
7. `read_only_mode` 切换会清理残留 `pressed`，并屏蔽后续 `touch / key`。
8. view disabled 会清理残留 `pressed`，并屏蔽后续 `touch / key`。
9. static preview 吞掉 `touch / key`，且保持 `current_index / item_count / pane_expanded / compact_mode / read_only_mode / content / pane / toggle / title / detail / rows` 不变，不触发 listener。
10. `read_only / !enable / empty items` 的额外 guard 回归。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Overview open`，同时重放底部 `Compact / Read only` preview 固定状态。
2. 抓取默认主区首帧。
3. 切到 `Overview compact` 并抓帧。
4. 切到 `Review` 并抓帧。
5. 切到 `Archive` 并抓帧。
6. 恢复主区默认 `Overview open`，同时重放底部 preview 固定状态。
7. 通过最终稳定抓帧输出默认态收尾。

说明：
- 主区继续保留真实 row 选中、pane toggle 与键盘导航语义，供手动复核与单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留额外的 `compact` preview 轨道和 `preview click` 收尾动作。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照与最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板；中间状态切换保留 `SPLIT_VIEW_RECORD_WAIT / SPLIT_VIEW_RECORD_FRAME_WAIT`，最终稳定帧使用 `SPLIT_VIEW_RECORD_FINAL_WAIT`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/split_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_split_view
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Overview open`、`Overview compact`、`Review`、`Archive` `4` 组可识别状态，且最终稳定帧必须回到默认态。
- 主区继续保留真实 row 选中、pane toggle 与键盘导航语义。
- `item / pane / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮或位移。
- 底部 `Compact / Read only` preview 必须在整条 runtime 轨道里保持静态一致。
- static preview 输入后不能改动 `current_index`、`pane_expanded`、布局区域或 listener 状态。
- WASM demo 必须能以 `HelloCustomWidgets_layout_split_view` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_split_view/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区唯一状态分组：`[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区差分边界：`(50, 104) - (429, 249)`
  - 以 `y >= 250` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Overview open`

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里包含 detail pane，不只是导航 rail。
- 相比 `data_list_panel`：这里强调可折叠 pane，而不只是列表选择。
- 相比 `master_detail`：这里更接近 `SplitView` 的导航抽屉语义。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Overview open`
  - `Overview compact`
  - `Review`
  - `Archive`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - 页面级 guide 与状态说明
  - preview 点击桥接
  - 额外的 `compact` preview 交互轨道
  - 录制里的 `preview click` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `split_view` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_split_view/default`
  - 共捕获 `11` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/split_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_split_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=251`
- 截图复核结论：
  - 主区覆盖 `Overview open / Overview compact / Review / Archive` 四组 reference 快照
  - 最终稳定帧显式回到默认 `Overview open`
  - 主区差分边界收敛到 `(50, 104) - (429, 249)`
  - 底部 `Compact / Read only` preview 全程保持静态一致
