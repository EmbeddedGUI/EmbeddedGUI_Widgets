# snackbar 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI Snackbar`
- 开源母本：`WPF UI`
- 对应组件：`Snackbar`
- 当前保留形态：`Saved`、`Offline`、`Sync failed`、`Queued`、`compact`、`read only`
- 当前保留交互：主区保留真实 `action` 与 `close` 的 same-target release 触控语义，以及 `Enter / Space / Escape` 键盘闭环；底部 `compact / read only` preview 统一为静态 reference 对照
- 当前移除内容：动画队列、多条堆叠、自动超时、页面级说明、强装饰阴影和业务化叙事
- EGUI 适配说明：目录和 demo 使用 `feedback/snackbar`，实现保留在 `HelloCustomWidgets` 内，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`snackbar` 用于展示短暂、低打扰的操作反馈，适合保存成功、离线提醒、同步失败、任务排队等场景。它通常贴近页面底部或当前上下文，消息短、动作少，并允许用户直接执行一个轻量操作或关闭提示。

## 2. 为什么现有控件不够用
- `toast_stack` 表达多条通知堆叠，偏系统通知流，不适合单条上下文反馈。
- `message_bar` 表达嵌入页面内容流的横幅，生命周期更稳定，不是临时底部提示。
- `dialog_sheet` 会阻塞用户决策，不适合保存、撤销、重试这类轻量反馈。

## 3. 当前页面结构
- 标题：`Snackbar`
- 主区：一个保留真实 `action / close` 语义的 `snackbar`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Archived / Undo`
- 右侧 preview：`read only`，固定显示 `Review only`

目录：
- `example/HelloCustomWidgets/feedback/snackbar/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 成功态：`Saved / Undo`
2. 警告态：`Offline / Retry`
3. 错误态：`Sync failed / Retry`
4. 信息态：`Queued / Open`

底部 preview 始终固定：

1. `compact`：`Archived / Undo`
2. `read only`：`Review only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 194`
- 主控件：`196 x 74`
- 底部 preview 行：`216 x 54`
- 单个 preview：`104 x 54`
- 页面结构：标题 -> 主 `snackbar` -> 底部 `compact / read only`
- 风格约束：浅色 surface、低噪音边框、标准 severity 圆形图标、短标题、单行动作按钮和右上关闭按钮；不使用高对比大阴影和营销式装饰

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Saved / Undo` | `Archived / Undo` | `Review only` |
| 快照 2 | `Offline / Retry` | 保持不变 | 保持不变 |
| 快照 3 | `Sync failed / Retry` | 保持不变 | 保持不变 |
| 快照 4 | `Queued / Open` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到 `Saved / Undo` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_snackbar.c` 覆盖以下口径：

1. `set_snapshots()` clamp、空列表、当前快照和打开态复位。
2. `set_current_snapshot()`、`set_opened()`、字体、palette、compact、read only setter 清理 `pressed_part / is_pressed`。
3. `action` 只在 `DOWN(action) -> UP(action)` 提交，跨到 `close` 或空白区域不提交。
4. `close` 只在 `DOWN(close) -> UP(close)` 关闭，并触发 open changed listener。
5. `Enter / Space` 触发 action，`Escape` 关闭可关闭 snackbar。
6. read only、disabled、closed guard 拒绝输入并清理残留按压态。
7. static preview 吞掉输入并保持状态不变。
8. severity、文本省略、metrics 和命中区域 helper 的边界行为。

## 8. 录制动作设计
`egui_port_get_recording_action()` 只切换主区快照，底部 preview 保持静态：

1. 恢复默认 `Saved` 并抓取首帧。
2. 切到 `Offline` 并抓取第二组主区快照。
3. 切到 `Sync failed` 并抓取第三组主区快照。
4. 切到 `Queued` 并抓取第四组主区快照。
5. 回到默认 `Saved` 并抓取最终稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/snackbar PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output/main.exe snackbar

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/snackbar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/snackbar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_snackbar
```

## 10. 验收重点
- 主区 snackbar 和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区四组状态需要能稳定区分 severity、标题、正文和 action。
- `action / close` 必须满足 same-target release 语义，不允许跨目标 `MOVE` 后错误提交。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。
- catalog、web policy 和 WASM demo 需要包含 `feedback/snackbar`。

## 11. 与现有控件的边界
- 相比 `toast_stack`：这里是单条上下文反馈，不展示堆叠层。
- 相比 `message_bar`：这里是临时反馈，不占据页面内容流。
- 相比 `dialog_sheet`：这里不阻塞流程，只提供一个轻量 action 或关闭入口。
- 相比 `teaching_tip`：这里不锚定控件解释功能，只反馈操作结果。

## 12. EGUI 简化点与限制
- 不实现动画入场、自动超时和多条队列。
- 不实现复杂阴影，仅保留轻边框和底部弱分隔线。
- 文本按固定宽度省略，优先在空格、连字符或斜杠边界截断。
- 低分辨率下 preview 只表达核心语义，不展示 close 按钮。
