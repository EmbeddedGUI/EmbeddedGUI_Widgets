# Flyout 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI Flyout`
- 补充参考控件：`tool_tip`、`teaching_tip`
- 对应组件名：`Flyout`
- 当前保留形态：`Review`、`Search`、`Sync`、`Pinned`、`compact`、`disabled`
- 当前保留交互：主区保留 `target / bubble / primary / secondary / top / bottom / open / closed` 语义、`same-target release`、target 点击 dismiss，以及 `set_snapshots / current_snapshot / open / current_part / font / meta_font / compact / disabled / palette` setter 统一清理 `pressed`；底部 `compact / disabled` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变
- 当前移除内容：旧录制轨道里的键盘切换、preview 焦点桥接和与 reference 页面无关的额外收尾动作，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `flyout` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`flyout` 用来表达围绕单一目标展开的轻量操作面板。它比 `tool_tip` 更重，因为可以承载标题、正文和动作；又比 `teaching_tip` 更轻，因为不承担教学卡片和长篇引导叙事。仓库需要一版贴近 Fluent / WPF UI 的 `Flyout` reference，实现与其它 feedback 控件的清晰边界。

## 2. 为什么现有控件不够用
- `tool_tip` 只有提示，不承载动作按钮。
- `teaching_tip` 偏教学和 coachmark，视觉与语义都更重。
- `menu_flyout` 偏菜单列表，不强调 target + bubble 的锚定关系。
- `dialog_sheet` 是更高层级的确认层，不适合作为局部上下文面板。

## 3. 当前页面结构
- 标题：`Flyout`
- 主区：一个主 `flyout`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Filter`
- 右侧 preview：`disabled`，固定显示 `Locked`
- 页面结构统一收口为：标题 -> 主 `flyout` -> `compact / disabled`

目录：
- `example/HelloCustomWidgets/feedback/flyout/`

## 4. 主区 reference 快照
主控件录制轨道只保留四组主区 snapshot 和最终稳定帧，底部 preview 在整条轨道中保持固定：

1. `Review`
   `bottom placement / open`，默认落在 `primary`
2. `Search`
   `top placement / open`，默认落在 `secondary`
3. `Sync`
   `warning / bottom placement / open`
4. `Pinned`
   `closed`，主区只保留 target
5. `Review`
   回到默认 `bottom placement / open`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Filter`
   紧凑尺寸、accent tone、静态打开态
2. `disabled`
   `Locked`
   禁用弱化配色、静态关闭态

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 252`
- 主 `flyout` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 -> 主 `flyout` -> 底部 `compact / disabled`
- 页面风格：浅灰 page panel、白色 surface、低噪音阴影和明确但克制的 target + bubble 锚定关系

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| `bottom placement` | 是 | 是 | 是 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `open_state = 1` | 是 | 是 | 否 |
| `open_state = 0` | 是 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `disabled_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_flyout.c` 当前覆盖 `6` 条用例：

1. `set_snapshots / current_snapshot / open / current_part / font / meta_font / compact / disabled / palette` setter 钳制与 `pressed` 清理。
2. 默认 part、snapshot guard 和空数据 guard，包括关闭态只允许落回 `TARGET` 的口径。
3. 触摸交互覆盖 same-target release、动作触发后关闭、以及 target 点击 dismiss 语义。
4. 键盘导航覆盖 `RIGHT / HOME / DOWN / END / UP / SPACE / ESCAPE`，并校验激活与关闭行为。
5. `disabled_mode / !enable` guard 会清理残留 `pressed`，并忽略后续输入。
6. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / snapshots / font / meta_font / on_action / api / palette / snapshot_count / current_snapshot / current_part / open_state / compact_mode / disabled_mode / pressed_part / alpha / enable / is_focused / is_pressed / padding`。

补充说明：
- 预览测试使用 `flyout_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `compact / disabled` preview 统一通过 `egui_view_flyout_override_static_preview_api()` 吞掉输入，不再承担 snapshot 切换、target dismiss 或焦点桥接职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主控件默认 `Review` 和底部 preview 固定状态，请求首帧并等待 `FLYOUT_RECORD_FRAME_WAIT`。
2. 切到 `Search`，等待 `FLYOUT_RECORD_WAIT`。
3. 请求第二帧并等待 `FLYOUT_RECORD_FRAME_WAIT`。
4. 切到 `Sync`，等待 `FLYOUT_RECORD_WAIT`。
5. 请求第三帧并等待 `FLYOUT_RECORD_FRAME_WAIT`。
6. 切到 `Pinned`，等待 `FLYOUT_RECORD_WAIT`。
7. 请求第四帧并等待 `FLYOUT_RECORD_FRAME_WAIT`。
8. 回到默认 `Review`，并同步重放 preview 固定状态，等待 `FLYOUT_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `FLYOUT_RECORD_FINAL_WAIT`。

说明：
- 录制期间统一通过 `request_page_snapshot()` 触发布局、刷新和截图请求。
- 底部 `compact / disabled` preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自四组 snapshot 以及最终回落到默认 `Review`。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `FLYOUT_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，默认态恢复与稳定收尾都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_flyout
```

## 10. 验收重点
- 主区与底部 `compact / disabled` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Review / Search / Sync / Pinned` 四组状态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Review`，不能停在 `Pinned`。
- `same-target release`、target dismiss、键盘导航和 `disabled / !enable` guard 需要与单测口径一致。
- `set_snapshots / current_snapshot / open / current_part / font / meta_font / compact / disabled / palette` setter 清理 `pressed` 的行为不能回退。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_flyout/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Review / Search / Sync / Pinned`
  - 主区 RGB 差分边界为 `(60, 86) - (420, 207)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 208` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Review`

## 12. 与现有控件的边界
- 相比 `tool_tip`：这里承载动作，不是纯提示。
- 相比 `teaching_tip`：这里是轻量局部面板，不是教学卡片。
- 相比 `menu_flyout`：这里强调 target + bubble 关系，不是命令列表。
- 相比 `dialog_sheet`：这里是局部上下文面板，不是模态确认层。

## 13. 本轮保留与删减
- 保留的主区状态：`Review`、`Search`、`Sync`、`Pinned`
- 保留的底部对照：`compact`、`disabled`
- 保留的交互与实现约束：`target / bubble / primary / secondary / top / bottom / open / closed` 语义、`same-target release`、target dismiss、static preview 对照
- 删减的旧桥接与旧装饰：旧录制轨道里的键盘切换、preview 焦点桥接、与 reference 页面无关的额外收尾动作、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `flyout` suite `6 / 6`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/flyout --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_flyout/default`
  - 共捕获 `11` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/flyout`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_flyout`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1921 colors=153`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Review / Search / Sync / Pinned`
  - 主区 RGB 差分边界为 `(60, 86) - (420, 207)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 208` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Review`，底部 `compact / disabled` preview 全程保持静态
