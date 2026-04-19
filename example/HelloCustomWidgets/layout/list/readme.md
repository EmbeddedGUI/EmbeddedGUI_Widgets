# List 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React / List`
- 对应组件：`List`
- 当前保留形态：`Inbox`、`Review`、`Archive`、`Compact`、`Read only`
- 当前保留交互：主区保留列表选择、same-target release 与键盘 `Up / Down / Left / Right / Home / End / Tab / Enter / Space` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：旧录制末尾额外选中第 3 行的收尾态、旧单测 `on_key_event` 注入路径、旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/list`，公开 C API 仍为 `egui_view_reference_list_*`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`list` 用来表达“同一组轻量条目按单列顺序排列，并保留当前项焦点与选择语义”的列表结构，适合消息队列、审核清单、归档入口、资源面板侧栏和小规模任务摘要这类需要快速浏览与切换当前项的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏页面级摘要列表，不适合轻量单列选择组件。
- `items_repeater` 只承担重复布局宿主，不负责当前项和列表输入闭环。
- `settings_panel`、`master_detail` 更偏设置页和主从布局，不适合承载小型列表本体。
- SDK 自带 `List` 更偏基础能力，本仓库仍需要一个 Fluent 2 reference 风格的自定义列表页面、单测和 web 验收入口。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `list` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Inbox`
  - `Review`
  - `Archive`
- 录制最终稳定帧显式回到默认 `Inbox`。
- 底部左侧是 `Compact` 静态 preview，固定对照 `Compact`。
- 底部右侧是 `Read only` 静态 preview，固定对照 `Read only`。
- 两个 preview 统一通过 `egui_view_reference_list_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_index / compact_mode / read_only_mode / item_count`
  - 不触发 `on_selection_changed_listener`

目标目录：
- `example/HelloCustomWidgets/layout/list/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Inbox`
2. 快照 2
   `Review`
3. 快照 3
   `Archive`
4. 最终稳定帧
   回到默认 `Inbox`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主控件：`196 x 112`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `list` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音分隔线和轻量 tone 差异；主区保留 `title / meta / badge` 三层信息，不叠加额外 snapshot header/footer；底部 preview 固定为静态 reference 对照，不再承担场景切换或额外交互职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Inbox` | `Compact` | `Read only` |
| 快照 2 | `Review` | 保持不变 | 保持不变 |
| 快照 3 | `Archive` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Inbox` | 保持不变 | 保持不变 |
| `selection focus`、键盘导航与 same-target release | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_list.c` 当前覆盖 `8` 条用例：

1. `set_items()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 的状态清理、clamp、getter 与 helper 行为。
2. 条目区域、命中测试、tone color、`set_current_index()` 与内部 helper 的边界行为。
3. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 语义。
4. 键盘 `Up / Down / Home / End / Tab / Enter / Space` 闭环。
5. `read_only` 守卫清理残留 `pressed` 并屏蔽 `touch / key`；恢复后重新允许 `Down / Enter / Space`。
6. `!enable` 守卫清理残留 `pressed` 并屏蔽 `touch / key`；恢复后重新允许 `Down / Enter / Space`。
7. static preview 吞掉 `touch / key`，并保持 `current_index / compact_mode / read_only_mode` 不变，同时不触发 `on_selection_changed_listener`。
8. 空数据状态忽略输入，不改状态且不触发 listener。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- setter、guard 与 static preview 路径都统一要求先清理残留 `pressed`，再处理后续状态。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Inbox`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `LIST_RECORD_FRAME_WAIT`。
2. 切到 `Review`，等待 `LIST_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照，等待 `LIST_RECORD_FRAME_WAIT`。
4. 切到 `Archive`，等待 `LIST_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `LIST_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Inbox`，同时重放底部 preview 固定状态，等待 `LIST_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `LIST_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- README 这里按当前 `test.c` 如实保留首轮切换使用 `LIST_RECORD_FINAL_WAIT`、中间切换使用 `LIST_RECORD_WAIT`、抓帧使用 `LIST_RECORD_FRAME_WAIT`、最终回落与最终抓帧使用 `LIST_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/list PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/list --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/list
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_list
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Inbox`、`Review`、`Archive` 三组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、`read_only / !enable / empty items` 守卫和键盘导航语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_index / compact_mode / read_only_mode / item_count`，也不能触发 `on_selection_changed_listener`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_list` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_list/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(52, 142) - (428, 225)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 225` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，最终稳定帧显式回到默认 `Inbox`

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里强调轻量单列列表，而不是页面级摘要面板。
- 相比 `items_repeater`：这里保留当前项和输入语义，而不只是模板重复器。
- 相比 `settings_panel`：这里不承担设置行布局，只保留列表本体。
- 相比 SDK 原生 `List`：这里是当前仓库维护的 Fluent 2 reference 页面与验收闭环。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Inbox`
  - `Review`
  - `Archive`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target release
  - 键盘 `Up / Down / Left / Right / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧口径：
  - 旧录制末尾额外选中第 3 行的收尾态
  - 旧单测 `on_key_event` 注入路径
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/list PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `list` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/list --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_list/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/list`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_list`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=206`
- 截图复核结论：
  - 主区覆盖 `Inbox / Review / Archive` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Inbox`
  - 主区 RGB 差分边界收敛到 `(52, 142) - (428, 225)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact / Read only` preview 以 `y >= 225` 裁切后全程保持单哈希静态
