# ContentDialog 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI ContentDialog`
- 补充参考实现：`ModernWpf`
- 对应组件名：`ContentDialog`
- 当前保留形态：`Sync issue`、`Delete draft`、`Template`、`Publishing`、`compact`、`read only`
- 当前保留交互：主区保留 `warning / error / accent / success` 四组 snapshot 切换、`same-target release` 提交语义，以及 `current_snapshot / current_action / font / meta_font / compact / read_only / palette` setter 统一清理 `pressed`；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持对照状态不变
- 当前移除内容：旧 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `dialog_sheet` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`dialog_sheet` 对齐的是 Fluent / WinUI 的 `ContentDialog` 语义，用来承载轻量确认、删除提醒、模板应用和发布确认这类短文本、低噪音、动作明确的模态收口场景。它不是常驻反馈条，也不是 toast 通知，而是页面层级更高、动作更聚焦的确认层。

## 2. 为什么现有控件不够用
- `message_bar` 面向页内反馈，不承担阻断式确认收口。
- `toast_stack` 偏短时通知，不适合作为主要确认入口。
- `teaching_tip` 是锚定式上下文提示，不提供居中确认层语义。
- 仓库仍需要一版贴近 Fluent / WPF UI `ContentDialog` 的 `reference` 示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Dialog Sheet`
- 主区：一个主 `dialog_sheet`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Network`
- 右侧 preview：`read only`，固定显示 `Read only`
- 页面结构统一收口为：标题 -> 主 `dialog_sheet` -> `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/dialog_sheet/`

## 4. 主区 reference 快照
主控件录制轨道只保留四组主区 snapshot 与最终稳定帧，底部 preview 在整条轨道中保持固定：

1. `Sync issue`
   `warning`，双动作，默认焦点落在 `PRIMARY`
2. `Delete draft`
   `error`，双动作，默认焦点落在 `SECONDARY`
3. `Template`
   `accent`，单动作，`show_close = 0`
4. `Publishing`
   `success`，单动作，`show_close = 1`
5. `Sync issue`
   回到默认 `warning`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Network`
   紧凑布局、单动作、warning tone
2. `read only`
   `Read only`
   只读弱化配色、无动作

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 258`
- 主 `dialog_sheet` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 86`
- 单个 preview 尺寸：`104 x 86`
- 页面结构：标题 -> 主 `dialog_sheet` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、低对比 overlay、白色 surface、柔和边框和低噪音 Fluent 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `warning` | 是 | 是 | 否 |
| `error` | 是 | 否 | 否 |
| `accent` | 是 | 否 | 否 |
| `success` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `show_close` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_dialog_sheet.c` 当前覆盖 `11` 条用例：

1. `set_snapshots()` 钳制 snapshot 数量上限，并在重置数据后同步清理 `current_snapshot / current_action / pressed`。
2. `current_snapshot / current_action` guard、normalize 与 listener 通知口径，包括 secondary-only 与 no-action 场景。
3. `font / meta_font / compact / read_only / palette` setter 统一清理 `pressed`，并覆盖默认字体回落。
4. 触摸命中测试覆盖主次动作区域切换与按下态更新。
5. same-target release 语义保持为 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
6. `ACTION_CANCEL` 只清理 `pressed`，不改变当前选择动作。
7. 键盘导航覆盖 `LEFT / TAB / END / HOME / RIGHT` 与对应 guard。
8. `read_only_mode` 会清理残留 `pressed`，并忽略后续 `touch / key` 输入。
9. `!enable` 状态下忽略输入，不触发动作变更。
10. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / snapshots / font / meta_font / on_action_changed / api / palette / snapshot_count / current_snapshot / current_action / compact_mode / read_only_mode / pressed_action / alpha / enable / is_focused / is_pressed / padding`。
11. 内部 helper 覆盖 `tone / glyph / metrics / region` 计算与归一化口径。

补充说明：
- 预览测试使用 `dialog_sheet_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `compact / read only` preview 统一通过 `egui_view_dialog_sheet_override_static_preview_api()` 吞掉输入，不再承担 snapshot 切换或焦点桥接职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用默认 `warning` 主区与底部 preview 固定状态，请求首帧并等待 `DIALOG_SHEET_RECORD_FRAME_WAIT`。
2. 切到 `error`，等待 `DIALOG_SHEET_RECORD_WAIT`。
3. 请求第二帧并等待 `DIALOG_SHEET_RECORD_FRAME_WAIT`。
4. 切到 `accent`，等待 `DIALOG_SHEET_RECORD_WAIT`。
5. 请求第三帧并等待 `DIALOG_SHEET_RECORD_FRAME_WAIT`。
6. 切到 `success`，等待 `DIALOG_SHEET_RECORD_WAIT`。
7. 请求第四帧并等待 `DIALOG_SHEET_RECORD_FRAME_WAIT`。
8. 回到默认 `warning`，等待 `DIALOG_SHEET_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `DIALOG_SHEET_RECORD_FINAL_WAIT`。

说明：
- 录制期间统一通过 `request_page_snapshot()` 触发布局、刷新和截图请求。
- 底部 `compact / read only` preview 在整条轨道中不发生视觉变化。
- 主区最终稳定帧显式回到默认 `Sync issue`，不会停在 `Publishing`。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `DIALOG_SHEET_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，默认态恢复与稳定收尾都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_dialog_sheet
```

## 10. 验收重点
- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Sync issue / Delete draft / Template / Publishing` 四组状态必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Sync issue`，不能停在 `Publishing`。
- `same-target release`、键盘导航与 `read_only / disabled` guard 需要与单测口径一致。
- `current_snapshot / current_action / font / meta_font / compact / read_only / palette` setter 清理 `pressed` 的行为不能回退。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_dialog_sheet/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Sync issue / Delete draft / Template / Publishing`
  - 主区 RGB 差分边界为 `(48, 87) - (431, 266)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 267` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Sync issue`

## 12. 与现有控件的边界
- 相比 `message_bar`：这里表达阻断式确认，不是页内提示。
- 相比 `toast_stack`：这里承载明确动作，不是短时通知。
- 相比 `teaching_tip`：这里是居中确认层，不是锚定式说明。

## 13. 本轮保留与删减
- 保留的主区状态：`Sync issue`、`Delete draft`、`Template`、`Publishing`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`same-target release`、`current_snapshot / current_action / font / meta_font / compact / read_only / palette` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：旧 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `dialog_sheet` suite `11 / 11`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_dialog_sheet/default`
  - 共捕获 `11` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_dialog_sheet`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1967 colors=167`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Sync issue / Delete draft / Template / Publishing`
  - 主区 RGB 差分边界为 `(48, 87) - (431, 266)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 267` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Sync issue`，底部 `compact / read only` preview 全程保持静态
