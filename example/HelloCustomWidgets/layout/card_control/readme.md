# CardControl 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / CardControl`
- 对应组件：`CardControl`
- 当前保留形态：`Workspace flow`、`Identity checks`、`Release approval`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `same-target release`、`Home / End / Tab / Enter / Space` 键盘闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_card_control`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`card_control` 用来表达“整张卡片可点击，同时右侧保留轻量 control affordance”的标准卡片语义。它适合工作流入口、审批摘要、快捷操作和设置分组中的单卡入口场景。

## 2. 为什么现有控件不够用
- `card_action` 更偏纯 action 入口，不保留右侧独立 control affordance。
- `settings_card` 更偏设置项语义，不适合通用内容卡片入口。
- `card_panel` 更偏信息摘要，不强调“整卡可激活 + 右侧 control”。
- `expander / settings_expander` 自带展开语义，不适合作为基础卡片入口。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `card_control` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Workspace flow`
  - `Identity checks`
  - `Release approval`
- 录制最终稳定帧显式回到默认 `Workspace flow`。
- 底部左侧是 `Compact` 静态 preview，只负责对照紧凑尺寸下的卡片层级。
- 底部右侧是 `Read only` 静态 preview，只负责对照冻结后的弱化视觉。
- 两个 preview 统一通过 `egui_view_card_control_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_part`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/card_control/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Workspace flow`
2. 快照 2
   `Identity checks`
3. 快照 3
   `Release approval`
4. 最终稳定帧
   回到默认 `Workspace flow`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主控件：`196 x 102`
- 底部 preview 行：`216 x 74`
- 单个 preview：`104 x 74`
- 页面结构：标题 -> 主 `card_control` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 tone 区分；整卡点击与右侧 `value / switch / chevron` control affordance 的关系通过主区状态表达；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Workspace flow` | `Compact flow` | `Read only card` |
| 快照 2 | `Identity checks` | 保持不变 | 保持不变 |
| 快照 3 | `Release approval` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Workspace flow` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_card_control.c` 当前覆盖 `9` 条用例：

1. `set_snapshots()` 的 clamp、默认 part 回落、空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()` 的 pressed 清理与状态更新。
3. `get_metrics()`、`resolve_hit()`、`get_part_region()` 与 compact 布局下的区域计算。
4. `activate_current_part()` 与 listener 行为。
5. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
6. 键盘 `Home / End / Tab / Enter / Space` 行为。
7. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_part / compact_mode / read_only_mode` 不变并清理 pressed；恢复后继续验证 `Home / End + Enter / Space`。
8. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_part / compact_mode / read_only_mode` 不变，同时不触发 `on_action`。
9. helper 函数覆盖 `clamp_snapshot_count()`、文本长度判断、tone 取色、pill 宽度、part 存在性与 disabled 混色。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Workspace flow`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Identity checks`，等待 `CARD_CONTROL_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Release approval`，等待 `CARD_CONTROL_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Workspace flow`，同时重放底部 preview 固定状态，等待 `CARD_CONTROL_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `CARD_CONTROL_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 same-target release 与键盘激活语义，供手动复核和单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_control PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_control --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_control
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_control
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Workspace flow`、`Identity checks`、`Release approval` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、键盘激活与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_part / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_card_control` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_card_control/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(52, 98) - (427, 234)`
  - 以 `y >= 235` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Workspace flow`

## 12. 与现有控件的边界
- 相比 `card_action`：这里强调整卡交互与右侧附加 control，而不是纯 action 入口。
- 相比 `settings_card`：这里强调通用内容卡片，而不是设置项。
- 相比 `card_panel`：这里强调整卡交互与右侧 control affordance，而不是信息摘要。
- 相比 `expander / settings_expander`：这里不承担展开折叠和嵌套行语义。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Workspace flow`
  - `Identity checks`
  - `Release approval`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/card_control PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `card_control` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_control --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_card_control/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_control`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=173`
- 截图复核结论：
  - 主区覆盖 `Workspace flow / Identity checks / Release approval` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Workspace flow`
  - 主区差分边界收敛到 `(52, 98) - (427, 234)`
  - 以 `y >= 235` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
