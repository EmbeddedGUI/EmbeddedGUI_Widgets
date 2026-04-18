# command_bar_flyout 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CommandBarFlyout`
- 开源母本：`WPF UI`
- 对应组件名：`CommandBarFlyout`
- 当前保留形态：`standard`、`compact`、`disabled`
- 当前保留交互：主区保留真实 `trigger / primary rail / secondary rows` 与 `Left / Right / Tab / Home / End / Up / Down / Enter / Space / Escape` 键盘闭环、`touch` same-target release；底部 `compact / disabled` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 快照轮换、preview 点击清焦桥接、旧录制轨道里的额外收尾态

## 1. 为什么需要这个控件?
`command_bar_flyout` 用来表达“由触发入口展开的一组页面命令”。它不是常驻命令栏，也不是纯菜单，而是保留顶部触发器、主命令 rail 和次级命令列表的轻量命令面板，适合编辑、审核和布局调整这类需要分层命令的场景。

## 2. 为什么现有控件不够用
- `command_bar` 是常驻命令栏，没有 flyout 触发入口和次级命令分层。
- `menu_flyout` 只有纵向菜单，不保留主命令 rail。
- `split_button` 和 `drop_down_button` 只覆盖单入口，不承载整组主命令和次级命令。
- 仓库里当前 `input/command_bar_flyout` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Command Bar Flyout`
- 主区：一个保留真实 trigger、主命令 rail 和次级命令列表导航闭环的 `command_bar_flyout`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick`
- 右侧 preview：`disabled`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/command_bar_flyout/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Edit / Save`
2. 切到 `Review` 主快照后 `Down`
   `Review / Escalate`
3. 切到 `Quick` 主快照
   `Quick / Trigger`
4. 切到 `Layout` 主快照
   `Layout / Density`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Quick / compact`
2. `disabled`
   `Locked / disabled`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 272`
- 主控件：`196 x 160`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `command_bar_flyout` -> 底部 `compact / disabled`
- 风格约束：保持浅色 page panel、低噪音边框和单层命令面板；主区只保留 `eyebrow + trigger + panel header + primary rail + secondary rows` 的最小完整语义；`compact / disabled` 只做静态 reference 对照；focus 仅在主控件内部流转，不做额外 preview 焦点桥接

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Edit / Save` | `Quick / compact` | `Locked / disabled` |
| 快照 2 | `Review / Escalate` | 保持不变 | 保持不变 |
| 快照 3 | `Quick / Trigger` | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Layout / Density` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件导航闭环，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Edit / Save`，同时恢复底部 `compact / disabled` 固定状态
2. 抓取首帧
3. 切到 `Review` 主快照并发送 `Down`，把当前 part 移动到 `Escalate`
4. 抓取第二组主区快照
5. 切到 `Quick` 主快照，保持关闭态 trigger
6. 抓取第三组主区快照
7. 切到 `Layout` 主快照，保留默认 `Density` 焦点
8. 抓取第四组主区快照
9. 保持 `Layout / Density` 不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `egui_view_command_bar_flyout_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立即清理残留 `pressed_part / is_pressed`
- preview 不改 `snapshots / current_snapshot / current_part / open_state / region_screen / palette`

当前 `test.c` 已保持统一 finalize 模板：保留 `COMMAND_BAR_FLYOUT_RECORD_FINAL_WAIT`、`COMMAND_BAR_FLYOUT_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_snapshot()`、`apply_preview_states()`、`focus_primary_flyout()`、`layout_page()` 与 `request_page_snapshot()`，初始化阶段在 root view 挂载前后统一回放默认态与 preview，确保主区四组快照与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_command_bar_flyout.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `set_snapshots()`、`set_current_snapshot()`、`set_open()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_disabled_mode()`、`touch` same-target release、`trigger / primary / secondary` 命中与 `Left / Right / Tab / Home / End / Up / Down / Enter / Space / Escape` 键盘闭环，以及 `pressed_part / is_pressed` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `snapshots`、`font`、`meta_font`、`on_action`、`region_screen`、`alpha`、`surface_color`、`section_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`danger_color`、`neutral_color`、`shadow_color`、`snapshot_count`、`current_snapshot`、`current_part`、`open_state`、`compact_mode`、`disabled_mode` 不变，并要求 `g_action_count == 0`、`g_action_snapshot == 0xFF`、`g_action_part == PART_NONE`，且 `is_pressed / pressed_part` 被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar_flyout PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar_flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar_flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar_flyout
```

## 10. 验收重点
- 主区和底部 `compact / disabled` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Edit / Save`、`Review / Escalate`、`Quick / Trigger` 与 `Layout / Density` 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、键盘导航与 setter 状态清理链路收口后不能残留 `pressed_part / is_pressed` 污染。
- 底部 `compact / disabled` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_command_bar_flyout/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(44, 63) - (435, 302)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 303` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `command_bar`：这里有 trigger 和次级命令分层，不是常驻命令栏。
- 相比 `menu_flyout`：这里保留主命令 rail，不是纯纵向菜单。
- 相比 `split_button` 和 `drop_down_button`：这里承载的是一组命令，不是单入口按钮。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Edit / Save`
  - `Review / Escalate`
  - `Quick / Trigger`
  - `Layout / Density`
  - `compact`
  - `disabled`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 快照轮换
  - preview 点击清焦桥接
  - 旧录制轨道中的额外收尾态

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/command_bar_flyout PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `command_bar_flyout` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar_flyout --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_command_bar_flyout/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar_flyout`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar_flyout`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2074 colors=198`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(44, 63) - (435, 302)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 303` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Edit / Save`、`Review / Escalate`、`Quick / Trigger` 与 `Layout / Density` 四组 reference 状态，最终稳定帧保持 `Layout / Density`，底部 `compact / disabled` preview 全程静态
