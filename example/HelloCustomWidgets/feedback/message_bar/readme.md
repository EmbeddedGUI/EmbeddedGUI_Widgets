# MessageBar 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI InfoBar`
- 补充参考实现：`ModernWpf`
- 对应组件名：`MessageBar / InfoBar`
- 当前保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 当前移除内容：旧 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作
- EGUI 适配说明：继续使用仓库内 `message_bar` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`message_bar` 对齐的是 Fluent / WinUI 的 `InfoBar` 语义，用来承载页内常驻但不阻塞的反馈信息。它覆盖 `info / success / warning / error` 四类常见状态，适合设置页、同步页和后台管理页顶部的轻量提示。

## 2. 为什么现有控件不够用
- `toast_stack` 更偏短时通知，不适合作为单条页内反馈条。
- `dialog_sheet` 是阻断式确认层，不承担常驻信息提示。
- `badge_group` 只能表达汇总提醒，不能同时承载标题、正文和动作语义。
- 仓库仍需要一版贴近 Fluent / WPF UI `MessageBar / InfoBar` 的 `reference` 示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Message Bar`
- 主区：一个主 `message_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `message_bar` -> `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/message_bar/`

## 4. 主区 reference 轨道
主控件录制轨道只保留四组主区 snapshot 和最终稳定帧：

1. `Updates ready`
   `info`，带 action
2. `Settings saved`
   `success`，带 action
3. `Storage almost full`
   `warning`，带 action
4. `Connection lost`
   `error`，带 action
5. `Updates ready`
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
- 根布局尺寸：`224 x 220`
- 主 `message_bar` 尺寸：`196 x 96`
- 底部容器尺寸：`216 x 82`
- 单个 preview 尺寸：`104 x 82`
- 页面结构：标题 -> 主 `message_bar` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色 surface、低噪音边框、柔和 severity accent 和明确但克制的标题 / 正文 / action 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `info` | 是 | 否 | 是 |
| `success` | 是 | 否 | 否 |
| `warning` | 是 | 是 | 否 |
| `error` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 否 |
| `read_only_mode` | 否 | 否 | 是 |
| `show_action` | 是 | 是 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `message_bar` 仍然是 display-first 的页内反馈控件，但保留标题、正文和 action 层级。
- 触摸继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `compact / read only` preview 统一通过 `egui_view_message_bar_override_static_preview_api()` 吞掉 `touch / key`，不再承担任何 snapshot 切换或焦点桥接职责。
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
- 主区变化只来自四组 severity snapshot 以及最终回落到默认帧。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_message_bar.c` 当前覆盖八部分：

1. `set_snapshots()` 钳制与状态复位
2. `current_snapshot` guard 与字体、palette、模式 setter 的 `pressed` 清理
3. 触摸 same-target release 与 cancel 行为
4. 键盘 `Enter` 激活 click listener
5. `compact_mode` 切换后清理 `pressed` 并保留点击行为
6. `read_only_mode / !enable` guard
7. 静态 preview 不变性断言
8. 内部 helper、severity glyph 和颜色映射覆盖

其中静态 preview 用例通过 `message_bar_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / snapshots / font / on_click_listener / api / palette / snapshot_count / current_snapshot / compact_mode / read_only_mode / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/message_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_message_bar
```

## 11. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc`
- `HelloUnitTest`：`PASS`，在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `message_bar` suite `8 / 8`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`11 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_message_bar/default`
- feedback 分类 compile/runtime 回归：`PASS`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_message_bar`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1678 colors=133`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_message_bar/default`

- 总帧数：`11`
- 主区 RGB 差分边界：`(45, 103) - (231, 244)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`4`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `message_bar` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `11` 帧里主区只出现 `4` 组唯一状态，对应 `Updates ready / Settings saved / Storage almost full / Connection lost` 四组 severity snapshot，最终稳定帧回到默认 `Updates ready`。
- 按 `y >= 245` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 13. 已知限制
- 当前版本继续使用固定 snapshot 数据，不接真实业务状态流。
- 当前不实现真实关闭动作、多 action 按钮和展开正文。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部反馈容器。

## 14. 与现有控件的边界
- 相比 `toast_stack`：这里是页内常驻提示，不是短时通知。
- 相比 `dialog_sheet`：这里不阻断流程，只表达页内反馈。
- 相比 `badge_group`：这里承载标题、正文和 action，不是汇总提醒。

## 15. EGUI 适配说明
- 保持 `message_bar` custom 实现继续驻留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用静态 preview API，吞掉输入并清理残留 `pressed`。
- README、demo、单测和 runtime 验收口径已经对齐到当前 reference workflow。
