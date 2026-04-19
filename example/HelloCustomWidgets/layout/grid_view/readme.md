# GridView 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI GridView`
- 对应组件：`GridView`
- 当前保留形态：`Assets gallery`、`Template board`、`Team board`、`Compact grid`、`Read only grid`
- 当前保留交互：主区保留 tile 集合导航、same-target release 与键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：页面级 `guide`、preview 清主区焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/grid_view`，底层仍复用仓库内现有 `hcw_grid_view` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid_view` 用来表达“同构内容按多列磁贴方式浏览，并保留当前项焦点、选择和激活语义”的集合视图。它不是纯布局容器，而是 Fluent 集合页里承接 tile 浏览、当前项切换和激活动作的 reference 控件。

## 2. 为什么现有控件不够用
- `items_repeater` 更偏模板复用基础件，本身不承担 `GridView` 的当前项与激活语义。
- `uniform_grid` 更偏固定网格容器，不负责集合项焦点和输入闭环。
- `wrap_panel`、`virtualizing_wrap_panel` 更偏布局容器，不负责 snapshot 与当前项切换。
- `data_list_panel` 更偏单列摘要列表，不适合表达多列磁贴集合浏览。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `grid_view` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Assets gallery`
  - `Template board`
  - `Team board`
- 录制最终稳定帧显式回到默认 `Assets gallery`。
- 底部左侧是 `Compact` 静态 preview，固定对照 `Compact grid`，保持 `compact_mode = 1`。
- 底部右侧是 `Read only` 静态 preview，固定对照 `Read only grid`，保持 `compact_mode = 1`、`read_only_mode = 1`。
- 两个 preview 统一通过 `hcw_grid_view_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode`
  - 不触发 `on_action_listener`

目标目录：
- `example/HelloCustomWidgets/layout/grid_view/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Assets gallery`
2. 快照 2
   `Template board`
3. 快照 3
   `Team board`
4. 最终稳定帧
   回到默认 `Assets gallery`

底部 preview 在整条轨道中固定为：
1. `Compact grid`
2. `Read only grid`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 288`
- 主控件：`196 x 148`
- 底部 preview 行：`216 x 86`
- 单个 preview：`104 x 86`
- 页面结构：标题 -> 主 `grid_view` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 page panel、低噪音描边、轻量 title 与 helper 文案层级，以及稳定的 tile 密度和当前项高亮；底部 preview 固定为静态 reference 对照，不再承担 guide、焦点桥接或录制收尾叙事。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Assets gallery` | `Compact grid` | `Read only grid` |
| 快照 2 | `Template board` | 保持不变 | 保持不变 |
| 快照 3 | `Team board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Assets gallery` | 保持不变 | 保持不变 |
| tile 集合导航、same-target release、键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_grid_view.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认态回退，以及 `current_snapshot / current_item / layout_mode` 重置。
2. 字体、配色、`compact / read_only / layout` setter 的 `pressed` 清理与状态更新；`set_current_snapshot()`、`set_current_item()` 的状态同步。
3. item metrics、命中区域、`activate_current_item()` 与 `on_action` listener 触发。
4. 触摸 same-target release 与 `ACTION_CANCEL` 语义。
5. 键盘 `Home / End / Up / Down / Tab / Enter` 导航与激活闭环，包括跨 snapshot 切换。
6. `read_only / !enable` 守卫清理残留 `pressed` 并屏蔽 `touch / key / navigation`；恢复后重新允许 `HOME / END + ENTER`。
7. static preview 吞掉 `touch / key / navigation`，并保持 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode` 不变，同时不触发 `on_action_listener`。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- setter、guard 与 static preview 路径都统一要求先清理残留 `pressed`，再处理后续状态。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Assets gallery`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `GRID_VIEW_RECORD_FRAME_WAIT`。
2. 切到 `Template board`，等待 `GRID_VIEW_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `GRID_VIEW_RECORD_FRAME_WAIT`。
4. 切到 `Team board`，等待 `GRID_VIEW_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `GRID_VIEW_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Assets gallery`，同时重放底部 preview 固定状态并把焦点回到主区，等待 `GRID_VIEW_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `GRID_VIEW_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- `apply_primary_default_state()` 与 `apply_preview_states()` 只在 `ui_ready` 后触发布局，避免回到默认态时依赖旧的 focus/request 时序。
- README 这里按当前 `test.c` 如实保留中间切换使用 `GRID_VIEW_RECORD_WAIT`、抓帧使用 `GRID_VIEW_RECORD_FRAME_WAIT`、最终回落与最终抓帧使用 `GRID_VIEW_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_view
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Assets gallery`、`Template board`、`Team board` 三组可识别状态，最终稳定帧必须回到默认 `Assets gallery`。
- 主区真实交互仍需保留 tile 集合导航、same-target release、`read_only / !enable` 守卫和键盘激活语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key / navigation` 且不改 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode`。
- static preview、setter 和 guard 路径都必须先清理残留 `pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_grid_view` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(56, 62) - (403, 264)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Assets gallery`

## 12. 与现有控件的边界
- 相比 `items_repeater`：这里保留集合导航和激活语义，不只是模板重复器。
- 相比 `uniform_grid`：这里保留不等宽磁贴节奏和 snapshot 切换，不是固定网格容器。
- 相比 `data_list_panel`：这里强调多列磁贴浏览，不是单列摘要列表。
- 相比 `wrap_panel / virtualizing_wrap_panel`：这里是 reference 级集合控件，不是纯布局器。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Assets gallery`
  - `Template board`
  - `Team board`
- 保留的底部对照：
  - `Compact grid`
  - `Read only grid`
- 保留的交互：
  - tile 集合导航
  - same-target release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - 页面级 `guide`
  - preview 清主区焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `grid_view` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2196 colors=316`
- 截图复核结论：
  - 主区覆盖 `Assets gallery / Template board / Team board` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Assets gallery`
  - 主区 RGB 差分边界收敛到 `(56, 62) - (403, 264)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact grid / Read only grid` preview 以 `y >= 265` 裁切后全程保持单哈希静态
