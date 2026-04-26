# info_bar 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI InfoBar`
- 开源母本：`WinUI 3`
- 对应组件：`InfoBar`
- 当前保留形态：`信息 / 成功 / 警告 / 错误` severity、标题、消息、可选 action、可关闭状态、`compact / read only` preview
- 当前保留交互：主区 `action` 与 `close` 使用 same-target release 触控语义；`Enter / Space` 触发 action，`Escape` 关闭可关闭 InfoBar
- 当前移除内容：动画展开、多行复杂内容、自定义图标模板、强阴影和页面级说明 chrome
- EGUI 适配说明：目录和 demo 使用 `feedback/info_bar`，实现保留在 `HelloCustomWidgets` 内，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`info_bar` 用于在页面内容流中展示可持续存在的状态反馈，适合同步完成、策略更新、容量预警、登录失效等需要用户在当前上下文中看到并处理的信息。它比临时提示更稳定，也比对话框更低打扰。

## 2. 为什么现有控件不够用
- `snackbar` 是底部临时反馈，生命周期短，不适合嵌入内容流。
- `message_bar` 更偏横幅消息，本轮 `info_bar` 保留 WinUI `IsOpen / Severity / Action / CloseButton` 语义。
- `toast_stack` 面向通知队列，不适合单条内联状态。
- `dialog_sheet` 会阻塞流程，不适合轻量状态提示。

## 3. 当前页面结构
- 标题：`InfoBar`
- 主区：一个可 action / close 的 `info_bar`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact note / View`
- 右侧 preview：`read only`，固定显示 `Read only`

目录：
- `example/HelloCustomWidgets/feedback/info_bar/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 成功态：`Sync complete / Details`
2. 信息态：`Policy update / Review`
3. 警告态：`Storage warning / Archive`
4. 错误态：`Sign-in required / Sign in`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 204`
- 主控件：`198 x 82`
- 底部 preview 行：`216 x 54`
- 单个 preview：`104 x 54`
- 页面结构：标题 -> 主 `info_bar` -> 底部 `compact / read only`
- 风格约束：浅色 surface、低噪音边框、左侧 severity 色条、标准圆形状态图标、右侧关闭按钮和轻量 action；不使用大阴影或场景化装饰

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Sync complete / Details` | `Compact note / View` | `Read only` |
| 快照 2 | `Policy update / Review` | 保持不变 | 保持不变 |
| 快照 3 | `Storage warning / Archive` | 保持不变 | 保持不变 |
| 快照 4 | `Sign-in required / Sign in` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到 `Sync complete / Details` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_info_bar.c` 覆盖以下口径：

1. `set_snapshots()` clamp、空列表、当前快照和打开态复位。
2. `set_current_snapshot()`、`set_opened()`、字体、palette、compact、read only setter 清理 `pressed_part / is_pressed`。
3. `action` 只在 `DOWN(action) -> UP(action)` 提交，跨到 `close` 或空白区域不提交。
4. `close` 只在 `DOWN(close) -> UP(close)` 关闭，并触发 open changed listener。
5. `Enter / Space` 触发 action，`Escape` 关闭可关闭 InfoBar。
6. read only、disabled、closed guard 拒绝输入并清理残留按压态。
7. static preview 吞掉输入并保持状态不变。
8. severity、文本省略、metrics 和命中区域 helper 的边界行为。

## 8. 录制动作设计
`egui_port_get_recording_action()` 只切换主区快照，底部 preview 保持静态：

1. 恢复默认 `Sync complete` 并抓取首帧。
2. 切到 `Policy update` 并抓取第二组主区快照。
3. 切到 `Storage warning` 并抓取第三组主区快照。
4. 切到 `Sign-in required` 并抓取第四组主区快照。
5. 回到默认 `Sync complete` 并抓取最终稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/info_bar PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output/main.exe info_bar

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/info_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/info_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_info_bar
```

## 10. 验收重点
- 主区 InfoBar 和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区四组状态需要能稳定区分 severity、标题、消息、action 和 close。
- `action / close` 必须满足 same-target release 语义，不允许跨目标 `MOVE` 后错误提交。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。
- catalog、web policy 和 WASM demo 需要包含 `feedback/info_bar`。

## 11. 与现有控件的边界
- 相比 `snackbar`：这里是内联持续反馈，不是底部临时提示。
- 相比 `message_bar`：这里明确保留 WinUI `InfoBar` 的 `IsOpen`、severity、action 和关闭语义。
- 相比 `toast_stack`：这里只表达当前上下文中的单条状态，不做通知队列。
- 相比 `dialog_sheet`：这里不阻塞流程，只提供轻量 action 或关闭入口。

## 12. EGUI 简化点与限制
- 不实现动画展开、图标模板替换和任意内容槽。
- 不实现复杂阴影，仅保留轻边框、左侧 severity 色条和状态圆形图标。
- 文本按固定宽度省略，优先在空格、连字符或斜杠边界截断。
- 低分辨率下 preview 只表达核心语义，不展示 close 按钮。
