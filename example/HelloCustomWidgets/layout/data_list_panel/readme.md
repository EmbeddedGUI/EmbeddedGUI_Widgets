# DataListPanel 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / ListView`
- 补充参考：`ModernWpf / ItemsView`
- 对应组件：`ListView`
- 当前保留形态：`Sync queue`、`Asset review`、`Archive sweep`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 row selection 与键盘 `Left / Right / Up / Down / Home / End / Tab`；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、旧 preview click bridge、旧录制轨道里的第二条 `compact` preview 切换与收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/data_list_panel` 现有实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`data_list_panel` 用于在单张低噪音卡片里浏览结构化数据行。它适合任务队列、资产审核、归档巡检、同步记录这类需要在同一块面板里查看多条摘要数据的场景。

## 2. 为什么现有控件不够用
- `list` 更偏基础列表，不强调数据面板式的 row 层级和摘要结构。
- `data_grid` 更偏网格表格，不适合当前这种单卡片、低密度、摘要优先的浏览方式。
- `settings_panel` 强调 setting row 与 value cell，不适合表达记录型数据行。
- `master_detail` 强调主从联动，这里只需要单面板数据列表浏览，不拆出 detail pane。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `data_list_panel` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Sync queue`
  - `Asset review`
  - `Archive sweep`
- 录制最终稳定帧显式回到默认 `Sync queue`。
- 底部左侧是 `Compact` 静态 preview，只负责对照紧凑布局密度。
- 底部右侧是 `Read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_data_list_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_index / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed`

目标目录：
- `example/HelloCustomWidgets/layout/data_list_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Sync queue`
2. 快照 2
   `Asset review`
3. 快照 3
   `Archive sweep`
4. 最终稳定帧
   回到默认 `Sync queue`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主控件：`196 x 116`
- 底部 preview 行：`216 x 80`
- 单个 preview：`104 x 80`
- 页面结构：标题 -> 主 `data_list_panel` -> 底部 `Compact / Read only`
- 风格约束：保持浅灰 page panel、白底数据卡片、低噪音边框与分层文本；主区继续保留 `eyebrow / title / summary / rows / footer` 五层结构；底部 preview 固定为静态 reference 对照，不再承担状态切换或焦点收尾职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Sync queue` | `Compact` | `Read only` |
| 快照 2 | `Asset review` | 保持不变 | 保持不变 |
| 快照 3 | `Archive sweep` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Sync queue` | 保持不变 | 保持不变 |
| 主区真实 row selection 与键盘导航 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_data_list_panel.c` 当前覆盖 `10` 条用例：

1. `set_snapshots()` 的 clamp、非法 focus 回退、空快照 reset。
2. `set_current_snapshot()` / `set_current_index()` 的 guard、重复设置时的 pressed 清理与 listener 通知。
3. `set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 的 pressed 清理，以及 helper 口径。
4. `compact_mode` 切换后清理残留 `pressed`，但继续保留 selection 行为。
5. metrics / hit testing 与 compact 布局口径。
6. 主区真实 `touch` 选中、`cancel` 取消以及 `read_only / !enable` guard。
7. 键盘 `Left / Right / Up / Down / Home / End / Tab` 导航与 guard。
8. `read_only_mode` 清理残留 `pressed` 并屏蔽后续 `touch / key`。
9. view disabled 清理残留 `pressed` 并屏蔽后续 `touch / key`。
10. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_index / snapshot_count / compact_mode / read_only_mode / content_region / eyebrow_region / title_region / summary_region / footer_region / row_regions` 不变，同时不触发 `on_selection_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Sync queue`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Asset review`，等待 `DATA_LIST_PANEL_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Archive sweep`，等待 `DATA_LIST_PANEL_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Sync queue`，同时重放底部 preview 固定状态，等待 `DATA_LIST_PANEL_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，等待 `DATA_LIST_PANEL_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 touch selection 与键盘导航语义，供手动复核与单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板；主区首轮切换和最终稳定抓帧使用 `DATA_LIST_PANEL_RECORD_FINAL_WAIT`，中间状态切换继续保留 `DATA_LIST_PANEL_RECORD_WAIT / DATA_LIST_PANEL_RECORD_FRAME_WAIT`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_list_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_list_panel
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏、裁切或重叠。
- 主区录制只允许出现 `Sync queue`、`Asset review`、`Archive sweep` `3` 组可识别状态，且最终稳定帧必须回到默认态。
- 主区继续保留真实 row selection 与键盘导航语义。
- 底部 `Compact / Read only` preview 必须在整条 runtime 轨道里保持静态一致。
- static preview 输入后不能改动 `current_snapshot / current_index / compact_mode / read_only_mode`、布局区域或 listener 状态。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_data_list_panel` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_data_list_panel/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(50, 95) - (430, 259)`
  - 以 `y >= 259` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Sync queue`

## 12. 与现有控件的边界
- 相比 `list`：这里强调标准化数据行层级，而不只是基础列表。
- 相比 `data_grid`：这里保留卡片式摘要数据面板，而不是网格表格。
- 相比 `settings_panel`：这里不承担 setting row 与 value cell 语义。
- 相比 `master_detail`：这里不拆出 detail pane，而是把信息收敛在单卡内部。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Sync queue`
  - `Asset review`
  - `Archive sweep`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - touch selection
  - 键盘 `Left / Right / Up / Down / Home / End / Tab`
- 删减的旧桥接与旧轨道：
  - 页面级 guide 与状态说明文案
  - preview click bridge
  - 第二条 `compact` preview 轨道
  - 旧收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `data_list_panel` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_data_list_panel/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_list_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_list_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=203`
- 截图复核结论：
  - 主区覆盖 `Sync queue / Asset review / Archive sweep` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Sync queue`
  - 主区差分边界收敛到 `(50, 95) - (430, 259)`
  - 以 `y >= 259` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
