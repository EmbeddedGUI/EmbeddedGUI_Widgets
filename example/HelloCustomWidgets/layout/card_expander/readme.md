# CardExpander 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / CardExpander`
- 对应组件：`CardExpander`
- 当前保留形态：`Workspace policy`、`Identity review`、`Release approval`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `expanded / collapsed`、same-target release、`Home / End / Tab / Up / Down / Enter / Space / Escape` 键盘闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_card_expander`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`card_expander` 用来表达“卡片摘要 + 卡内展开正文”的 disclosure 语义。它比普通 `Expander` 更强调卡片壳层、摘要信息和正文仍留在同一张卡片内的关系，适合策略说明、审核条目和需要按需展开细节的卡片入口。

## 2. 为什么现有控件不够用
- `card_action` 强调整卡 action，不保留展开后的正文区域。
- `card_control` 强调右侧附加 control，不承载折叠正文。
- `expander` 当前示例更偏多段 disclosure，不是单卡片壳层。
- `settings_expander` 面向设置行和 nested rows，不适合承载通用正文说明。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `card_expander` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Workspace policy`
  - `Identity review`
  - `Release approval`
- 录制最终稳定帧显式回到默认 `Workspace policy`。
- 底部左侧是 `Compact` 静态 preview，只负责对照紧凑尺寸下的卡片壳层。
- 底部右侧是 `Read only` 静态 preview，只负责对照冻结后的弱化视觉。
- 两个 preview 统一通过 `egui_view_card_expander_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_part / expanded_state`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/card_expander/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Workspace policy`
2. 快照 2
   `Identity review`
3. 快照 3
   `Release approval`
4. 最终稳定帧
   回到默认 `Workspace policy`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- 页面结构：标题 -> 主 `card_expander` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 tone 区分；`expanded / collapsed` 只通过 chevron 方向、正文区域和局部 tone 微差表达；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Workspace policy` | `Compact card` | `Read only card` |
| 快照 2 | `Identity review` | 保持不变 | 保持不变 |
| 快照 3 | `Release approval` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Workspace policy` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 / Escape | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_card_expander.c` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认 part 回落、空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`set_current_snapshot()`、`set_expanded()` 的 pressed 清理与状态更新。
3. `get_part_region()`、`activate_current_part()` 与 listener 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
5. 键盘 `Home / End / Tab / Up / Down / Enter / Escape` 行为。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_part / expanded` 不变并清理 pressed。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_part / expanded / compact_mode / read_only_mode` 不变，同时不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Workspace policy`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Identity review`，等待 `CARD_EXPANDER_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Release approval`，等待 `CARD_EXPANDER_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Workspace policy`，同时重放底部 preview 固定状态，等待 `CARD_EXPANDER_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，等待 `CARD_EXPANDER_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 header 交互、same-target release、键盘激活与 `Escape` 收起语义，供手动复核与单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_expander PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_expander
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Workspace policy`、`Identity review`、`Release approval` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 header same-target release、键盘激活、`Escape` 收起与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_part / expanded / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_card_expander` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_card_expander/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(52, 92) - (427, 261)`
  - 以 `y >= 262` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Workspace policy`

## 12. 与现有控件的边界
- 相比 `card_action`：这里保留卡内展开正文，不是单纯 action 入口。
- 相比 `card_control`：这里不承载右侧 `value / switch` control，只保留 `expand / collapse` affordance。
- 相比 `expander`：这里固定为单卡片壳层，不是多项 accordion。
- 相比 `settings_expander`：这里承载通用正文，而不是 nested setting rows。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Workspace policy`
  - `Identity review`
  - `Release approval`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Tab / Up / Down / Enter / Space / Escape`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/card_expander PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `card_expander` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_expander --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_card_expander/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_expander`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_expander`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.186 colors=167`
- 截图复核结论：
  - 主区覆盖 `Workspace policy / Identity review / Release approval` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Workspace policy`
  - 主区差分边界收敛到 `(52, 92) - (427, 261)`
  - 以 `y >= 262` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
