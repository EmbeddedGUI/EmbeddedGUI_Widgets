# ToastStack 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Toast / Snackbar`
- 对应组件名：`Toast / Snackbar`
- 当前保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 当前移除内容：旧版 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作
- EGUI 适配说明：继续使用仓库内 `toast_stack` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`toast_stack` 对齐的是 Fluent / WinUI 里的轻量级 `Toast / Snackbar` 语义，用来承载短时、非阻塞、带状态层级的通知反馈。它适合设置页、同步页、工作台首页这类需要同时展示最近两到三条提醒的场景，比单条横幅更能表达“前卡 + 背卡”的层级关系。

## 2. 为什么现有控件不够用
- `message_bar` 更偏向常驻的页内反馈，不强调多条 toast 的叠卡层级。
- `dialog_sheet` 是阻塞式确认层，不适合承载轻量通知。
- `badge_group` 只能做聚合提醒，不能同时承载正文、动作和时间信息。
- 仓库仍需要一版贴近 Fluent / WPF UI `Toast / Snackbar` 的 `reference` 示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Toast Stack`
- 主区：一个主 `toast_stack`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `toast_stack` -> `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/toast_stack/`

## 4. 主区 reference 轨道
主控件录制轨道只保留四组主区 snapshot 和最终稳定帧：

1. `Backup ready`
   `info`，带 action
2. `Draft`
   `success`，带 action
3. `Storage low`
   `warning`，带 action
4. `Upload failed`
   `error`，带 action
5. `Backup ready`
   回到默认 `info`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Quota alert`
   紧凑布局、warning tone、静态 action
2. `read only`
   `Policy note`
   只读弱化配色、无 action

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 232`
- 主 `toast_stack` 尺寸：`196 x 108`
- 底部容器尺寸：`216 x 83`
- 单个 preview 尺寸：`104 x 83`
- 页面结构：标题 -> 主 `toast_stack` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色 surface、低噪音边框、柔和 severity accent，以及克制的标题 / 正文 / action 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `info` | 是 | 否 | 是 |
| `success` | 是 | 否 | 否 |
| `warning` | 是 | 是 | 否 |
| `error` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `action` | 是 | 是 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `toast_stack` 继续保留四类 severity 变化、前卡动作层级和 same-target release 语义。
- 触摸继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `compact / read only` preview 统一通过 `egui_view_toast_stack_override_static_preview_api()` 吞掉 `touch / key`，不再承担 snapshot 切换或焦点桥接职责。
- `read only` preview 保持 `set_read_only_mode(..., 1)`，只作为静态对照，不参与真实交互。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主控件默认 `info` 和底部 preview 固定状态
2. 请求首帧
3. 切到 `success`
4. 请求第二帧
5. 切到 `warning`
6. 请求第三帧
7. 切到 `error`
8. 请求第四帧
9. 回到默认 `info` 并请求最终稳定帧

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自四组 severity snapshot 以及最终回落到默认态。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_toast_stack.c` 当前覆盖八部分：

1. `set_snapshots()` 钳制与状态复位
2. `current_snapshot` guard 与字体、meta font、palette、模式 setter 的 `pressed` 清理
3. 触摸 same-target release 与 cancel 行为
4. 键盘 `Enter` 激活 click listener
5. `compact_mode` 切换后清理 `pressed` 并保留点击行为
6. `read_only_mode / !enable` guard
7. 静态 preview 输入被消费后状态保持不变
8. 内部 helper、severity glyph 和颜色映射覆盖

其中静态 preview 用例通过 `toast_stack_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / snapshots / font / meta_font / on_click_listener / api / palette / snapshot_count / current_snapshot / compact_mode / read_only_mode / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/toast_stack
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_toast_stack
```

## 11. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc`
- `HelloUnitTest`：`PASS`，在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `toast_stack` suite `8 / 8`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`11 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_toast_stack/default`
- feedback 分类 compile/runtime 回归：`PASS`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_toast_stack`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1769 colors=209`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_toast_stack/default`

- 总帧数：`11`
- 主区 RGB 差分边界：`(45, 94) - (404, 253)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`4`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `toast_stack` reference 主体，遮罩该边界后主区外页面 chrome 在整条轨道中保持静态。
- `11` 帧里主区只出现 `4` 组唯一状态，对应 `Backup ready / Draft / Storage low / Upload failed` 四组 severity snapshot，最终稳定帧回到默认 `Backup ready`。
- 按 `y >= 254` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 13. 已知限制
- 当前版本继续使用固定 snapshot 数据，不接真实通知队列。
- 当前不实现自动弹入 / 弹出动画，也不做滑动关闭。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部通知中心。

## 14. 与现有控件的边界
- 相比 `message_bar`：这里是短时叠卡通知，不是常驻页内反馈。
- 相比 `dialog_sheet`：这里不阻断流程，只表达轻量反馈。
- 相比 `badge_group`：这里承载标题、正文、action 和时间元信息，不是汇总提醒。

## 15. EGUI 适配说明
- 保持 `toast_stack` custom 实现继续驻留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用静态 preview API，吞掉输入并清理残留 `pressed`。
- README、demo、单测和 runtime 验收口径已经对齐到当前 `reference` workflow。
