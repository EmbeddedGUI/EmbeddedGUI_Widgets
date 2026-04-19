# ToggleSplitButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI ToggleSplitButton`
- 参考开源库：`WPF UI`
- 对应组件：`ToggleSplitButton`
- 当前保留形态：`Alert routing / checked / primary`、`Sync monitor / unchecked / menu`、`Follow thread / checked / primary`、`Record scene / unchecked / menu`、`compact`、`read only`
- 当前保留交互：主区保留 `checked` 切换、`primary / menu` 双段 same-target release、键盘 `Tab / Left / Right / Space / Enter / Plus / Minus` 导航与静态 preview 对照
- 当前移除内容：页面级 `guide`、preview 快照轮换、preview 清焦桥接、额外收尾态、真实 flyout、多级菜单、showcase 包装，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `input/toggle_split_button`，底层仍复用仓库内现有 `egui_view_toggle_split_button` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`toggle_split_button` 用来表达“主按钮本身是一个持久开关，同时右侧还保留更多动作入口”的复合命令按钮，适合告警路由、后台同步、订阅跟随和录制开关这类场景：既要保留 `on / off` 状态，又要通过菜单段切换附加模式。

仓库里已有 `toggle_button`、`split_button` 和 `command_bar`，但仍缺少一个能稳定承接 `ToggleSplitButton` 双入口开关语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `toggle_button` 只有 checked 主动作，没有菜单入口。
- `split_button` 有主动作和菜单入口，但不保留 checked 语义。
- `menu_flyout` 是独立弹出菜单容器，不是页内复合按钮。
- `command_bar` 承担工具栏语义，不是单个复合开关命令按钮。

## 3. 当前页面结构
- 标题：`Toggle Split Button`
- 主区：1 个保留真实 toggle split 语义的 `toggle_split_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick / Quick`
- 右侧 preview：`read only`，固定显示 `Locked / Publish`

目录：
- `example/HelloCustomWidgets/input/toggle_split_button/`

## 4. 主区 reference 快照
主区录制轨道只保留 4 组程序化快照，不再在录制阶段真实驱动 touch / key，也不再让底部 preview 参与轮换：

1. 默认态
   `Alert routing / checked / primary`
2. 快照 2
   `Sync monitor / unchecked / menu`
3. 快照 3
   `Follow thread / checked / primary`
4. 快照 4
   `Record scene / unchecked / menu`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Quick / Quick`
2. `read only`
   `Locked / Publish`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 160`
- 主控件：`196 x 80`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 页面结构：标题 -> 主 `toggle_split_button` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音描边、明确的主动作段与菜单段边界，以及稳定的 helper 文案层级，不回退到旧 demo 的 guide 与场景包装

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Alert routing / checked / primary` | `Quick / Quick` | `Locked / Publish` |
| 快照 2 | `Sync monitor / unchecked / menu` | 保持不变 | 保持不变 |
| 快照 3 | `Follow thread / checked / primary` | 保持不变 | 保持不变 |
| 快照 4 | `Record scene / unchecked / menu` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Record scene / unchecked / menu` | 保持不变 | 保持不变 |
| 键盘 `Tab / Left / Right` part 导航 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_toggle_split_button.c` 当前覆盖 `12` 条用例：

1. setter 清理 `pressed`。
   覆盖 `set_snapshots()`、`set_current_snapshot()`、`set_checked()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed_part / is_pressed` 的清理。
2. `Tab` 循环 `primary / menu` part。
   覆盖 part 切换后的 listener 回调与状态同步。
3. 主段 `touch` same-target release 切换 checked。
   覆盖主段点击后 `checked` 翻转与通知回调。
4. `ACTION_CANCEL` 清理按压态。
   覆盖取消输入后 `pressed_part / is_pressed` 回收与重复 cancel 拒绝。
5. 菜单段 `touch` same-target release 切换 snapshot。
   覆盖 `current_snapshot / current_part / checked` 联动更新。
6. 键盘激活。
   覆盖 `Space` 激活主段与 `Enter` 激活菜单段。
7. `Plus / Minus` 切换 snapshot。
   覆盖菜单段键盘切换与回调参数更新。
8. compact mode 清理 `pressed` 并忽略输入。
   覆盖 compact 模式下的 `touch / key` 拒绝与模式恢复。
9. snapshot 状态保持。
   覆盖跨 snapshot 切换后 `checked` 状态保留。
10. read only mode 清理 `pressed` 并忽略输入。
   覆盖只读模式下的 `touch / key` 拒绝与恢复后重新可交互。
11. `!enable` guard 清理 `pressed` 并拒绝输入。
   覆盖 view disabled 后的 `touch / key` 拒绝与状态恢复。
12. static preview 吞掉输入且保持状态不变。
   固定校验 `region_screen / snapshots / font / meta_font / on_changed / surface_color / border_color / text_color / muted_text_color / accent_color / success_color / warning_color / danger_color / neutral_color / checked_states / snapshot_count / current_snapshot / current_part / compact_mode / read_only_mode / alpha` 不变，并要求 `changed_count == 0`、`last_snapshot == 0xFF`、`last_checked == 0xFF`、`last_part == EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE`，且 `is_pressed / pressed_part` 被清理。

说明：
- 主区真实交互继续保留 `checked / current snapshot / current part` 联动、`primary / menu` same-target release 和键盘 part 导航。
- setter、模式切换、`!enable` guard 和 static preview 都统一要求先清理残留 `pressed_part / is_pressed`，再处理后续状态。
- 底部 `compact / read only` preview 统一通过 `egui_view_toggle_split_button_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不改 `snapshots / checked_states / current_snapshot / current_part / compact_mode / read_only_mode / region_screen / palette`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态并抓取首帧，等待 `TOGGLE_SPLIT_BUTTON_RECORD_FRAME_WAIT`。
2. 切到 `Sync monitor`，等待 `TOGGLE_SPLIT_BUTTON_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `TOGGLE_SPLIT_BUTTON_RECORD_FRAME_WAIT`。
4. 切到 `Follow thread`，等待 `TOGGLE_SPLIT_BUTTON_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `TOGGLE_SPLIT_BUTTON_RECORD_FRAME_WAIT`。
6. 切到 `Record scene`，等待 `TOGGLE_SPLIT_BUTTON_RECORD_WAIT`。
7. 抓取第四组主区快照，等待 `TOGGLE_SPLIT_BUTTON_RECORD_FRAME_WAIT`。
8. 保持 `Record scene / unchecked / menu` 不变并导出最终稳定帧，等待 `TOGGLE_SPLIT_BUTTON_RECORD_FINAL_WAIT`。

说明：
- 主区仍保留真实 `checked / current snapshot / current part` 交互、same-target release 和键盘 part 导航，供运行时手动交互。
- runtime 录制阶段不再真实发送主区点击或菜单切换输入，主区状态只通过 `apply_primary_snapshot()` 切换。
- 底部 preview 统一通过 `egui_view_toggle_split_button_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 4 组主区快照与最终稳定帧的布局口径一致。
- README 这里按当前 `test.c` 如实保留 `TOGGLE_SPLIT_BUTTON_RECORD_WAIT / TOGGLE_SPLIT_BUTTON_RECORD_FRAME_WAIT / TOGGLE_SPLIT_BUTTON_RECORD_FINAL_WAIT` 三档等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_split_button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_split_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toggle_split_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toggle_split_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Alert routing`、`Sync monitor`、`Follow thread`、`Record scene` 4 组可识别状态。
- 主区真实交互仍需保留 `checked` 切换、`primary / menu` 双段 same-target release 与键盘 part 导航语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `snapshots / checked_states / current_snapshot / current_part / compact_mode / read_only_mode / region_screen / palette`。
- WASM demo 必须能以 `HelloCustomWidgets_input_toggle_split_button` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_toggle_split_button/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界收敛到 `(53, 156) - (426, 215)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 216` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `toggle_button`：这里强调 checked 状态与菜单入口并存，而不是单入口切换。
- 相比 `split_button`：这里同时保留 checked 语义和菜单段。
- 相比 `menu_flyout`：这里是页内复合按钮，不是独立弹出菜单容器。
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Alert routing / checked / primary`
  - `Sync monitor / unchecked / menu`
  - `Follow thread / checked / primary`
  - `Record scene / unchecked / menu`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `checked` 切换
  - `primary / menu` same-target release
  - 键盘 `Tab / Left / Right / Space / Enter / Plus / Minus`
  - setter / 模式切换状态清理
  - 静态 preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide
  - preview 快照轮换
  - preview 清焦桥接
  - 额外收尾态
  - 真实 flyout 与多级菜单

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/toggle_split_button PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `toggle_split_button` suite `12 / 12`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_split_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_toggle_split_button/default`
  - 共捕获 `10` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toggle_split_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toggle_split_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.122 colors=145`
- 截图复核结论：
  - 主区覆盖默认 `Alert routing / checked / primary`、`Sync monitor / unchecked / menu`、`Follow thread / checked / primary` 与 `Record scene / unchecked / menu` 4 组 reference 快照
  - 最终稳定帧保持 `Record scene / unchecked / menu`
  - 主区 RGB 差分边界收敛到 `(53, 156) - (426, 215)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 216` 裁切后全程保持单哈希静态
