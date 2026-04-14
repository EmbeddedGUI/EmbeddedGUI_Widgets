# toast_stack 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI Snackbar`
- 对应组件名：`Toast / Snackbar`
- 本次保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 本次删除效果：preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与轻量 toast 叠卡无关的额外交互录制
- EGUI 适配说明：沿用仓库内 `toast_stack` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Toast / Snackbar` 用于表达页内短时但非阻塞的反馈信息，适合设置页、同步页和工作台首页展示最近 `2` 到 `3` 条轻量通知。它不是单条横幅，也不是阻塞弹层，而是更接近 Fluent / WPF UI 的轻量 stacked toast 语义。

## 2. 为什么现有控件不够用
- `message_bar` 更偏单条页内反馈，不强调连续 toast 的叠卡层级。
- `dialog_sheet` 是阻塞式弹层，不适合轻量临时消息。
- `badge_group` 只能表达汇总提醒，不能同时承载正文、动作和时间信息。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `Toast / Snackbar` 的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `toast_stack`，通过录制轨道覆盖 `info / success / warning / error` 四态。
- 底部左侧展示 `compact` 静态对照，保留前卡与两层叠卡的层级关系。
- 底部右侧展示 `read only` 静态对照，保留只读状态下的弱化 stacked toast。
- 页面结构统一收口为：标题 -> 主 `toast_stack` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_toast_stack_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/feedback/toast_stack/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 232`
- 主控件尺寸：`196 x 108`
- 底部对照行尺寸：`216 x 83`
- `compact` 预览：`104 x 83`
- `read only` 预览：`104 x 83`

视觉约束：
- 页面保持浅灰 page panel、白底 stacked toast 和低噪音浅边框。
- 主控件保留 severity strip、前卡标题 / 正文 / action / meta 层级，但整体仍需维持柔和的 Fluent 语法。
- 两层后卡只保留叠卡关系和标题摘要，不再额外叠加 showcase 式页面说明。
- `compact / read only` 都必须是静态 preview，只负责 reference 对照，不再承接任何录制桥接逻辑。
- 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toast Stack` | 页面标题 |
| `stack_primary` | `egui_view_toast_stack_t` | `196 x 108` | `Backup ready` | 主 toast 叠卡 |
| `stack_compact` | `egui_view_toast_stack_t` | `104 x 83` | `Quota alert / static` | 紧凑静态对照 |
| `stack_read_only` | `egui_view_toast_stack_t` | `104 x 83` | `Policy note / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup ready` | 默认 `info` |
| 主控件 | `Draft` | `success` |
| 主控件 | `Storage low` | `warning` |
| 主控件 | `Upload failed` | `error` |
| `compact` | `Quota alert` | 紧凑静态 preview |
| `read only` | `Policy note` | 只读静态 preview |

## 7. 交互与状态语义
- 主 `toast_stack` 继续保留四态 severity 变化、前卡动作层级和 same-target release 语义。
- `compact / read only` preview 都通过 static preview API 吞掉 `touch / key`，作为真正静态的 reference 对照。
- `read only` preview 继续使用 `set_read_only_mode(..., 1)`，保证不承接真实交互。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/feedback/toast_stack/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview snapshot 切换
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件 `info / success / warning / error` 切换与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `info`，并同步两个静态 preview 状态。
2. 请求默认截图。
3. 切到 `success`。
4. 请求第二张截图。
5. 切到 `warning`。
6. 请求第三张截图。
7. 切到 `error`。
8. 请求第四张截图。
9. 恢复默认 `info` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc
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

验收重点：
- 主控件四张关键截图必须能清晰区分 `info / success / warning / error`。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或 snapshot 切换产生额外变化。
- 页面不能出现黑白屏、裁切、pressed 残留或底部 preview 脏态。
- preview 不响应触摸或键盘输入，也不改变 `current_snapshot`。

## 11. 已知限制
- 当前版本仍使用固定 snapshot 数据，不接真实通知队列。
- 当前不做自动弹入 / 弹出动画，也不做滑动关闭。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部通知中心。
