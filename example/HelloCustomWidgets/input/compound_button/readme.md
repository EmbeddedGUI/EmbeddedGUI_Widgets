# CompoundButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React / CompoundButton`
- 对应组件：`CompoundButton`
- 当前保留形态：`Create workspace`、`Sync changes`、`Approve access`、`Publish rollout`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `touch` same-target release 与 `Space / Enter` 提交；底部 `Compact / Read only` preview 保持静态 reference 对照
- EGUI 适配说明：在 custom 层新增轻量 `egui_view_compound_button`；不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`compound_button` 表达“一个主要动作 + 一行辅助说明”的按钮语义。它比普通 `button` 多一层描述信息，适合用户在提交前需要快速理解动作影响的场景，例如创建工作区、同步策略、审批访问或发布试点。

## 2. 为什么现有控件不够用
- `button` 只有单行命令文案，不承载次级说明。
- `toggle_button` 表达状态切换，不是一次性命令提交。
- `split_button` 和 `drop_down_button` 带有菜单或分裂动作，不是复合按钮。
- `card_action` 是卡片入口，视觉和空间层级比按钮更重。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `compound_button` -> 底部 `Compact / Read only` 双 preview。
- 主区包含 `4` 组状态：
  - `Create workspace`
  - `Sync changes`
  - `Approve access`
  - `Publish rollout`
- 底部左侧是 `Compact` 静态 preview，展示紧凑尺寸下的复合按钮。
- 底部右侧是 `Read only` 静态 preview，展示冻结状态下的弱化视觉。
- 两个 preview 统一通过 `egui_view_compound_button_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `title / subtitle / icon / style / compact / read only`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/input/compound_button/`

## 4. 主区 reference 快照
录制轨道保留真实主按钮交互，底部 preview 在整条轨道中保持静态：

1. 默认态：`Create workspace`
2. 触摸提交后：`Sync changes`
3. `Space` 提交后：`Approve access`
4. `Enter` 提交后：`Publish rollout`
5. 最终稳定帧：回到 `Create workspace`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 168`
- 主控件：`188 x 58`
- 底部 preview 行：`216 x 58`
- 单个 preview：`104 x 58`
- 页面结构：标题 -> 主 `compound_button` -> 底部 `Compact / Read only`
- 风格约束：浅色 Fluent surface、低噪音边框、leading icon 圆形底、主标题与次级说明的清晰层级；`primary / default / subtle` 只改变按钮 tone，不引入额外装饰。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Create workspace` | `Compact` | `Read only` |
| 快照 2 | `Sync changes` | 保持不变 | 保持不变 |
| 快照 3 | `Approve access` | 保持不变 | 保持不变 |
| 快照 4 | `Publish rollout` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Create workspace` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_compound_button.c` 当前覆盖 `7` 条用例：

1. 文本 fitting helper 与按词边界省略。
2. `set_title()`、`set_subtitle()`、`set_icon()`、`set_content()`、`set_fonts()`、`set_style()`、`set_compact_mode()`、`set_read_only_mode()` 与 `set_palette()` 清理 pressed 并更新状态。
3. `activate()` 与 listener 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
5. 键盘 `Space / Enter` 激活和未处理按键清理 pressed。
6. `read_only` 与 `!enable` 守卫，保持 action 不触发并清理 pressed。
7. static preview 吞掉 `touch / key`，保持内容、样式、API、enable、focus 与 pressed 状态不变，并不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 使用真实主按钮交互和静态 preview 工作流：

1. 应用主区默认 `Create workspace`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 触摸点击主区，切到 `Sync changes`。
3. 抓取第二组主区快照。
4. 发送 `Space`，切到 `Approve access`。
5. 抓取第三组主区快照。
6. 发送 `Enter`，切到 `Publish rollout`。
7. 抓取第四组主区快照。
8. 恢复主区默认 `Create workspace`，同时重放底部 preview 固定状态。
9. 通过最终抓帧输出稳定默认态。

说明：
- 主区真实交互保留 `touch` same-target release、`Space / Enter` 与 listener 语义。
- runtime 录制阶段不真实发送底部 preview 输入，避免 preview 成为第二条交互轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证各快照布局口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/compound_button PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe compound_button
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/sync_widget_catalog.py --check
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/compound_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/compound_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_compound_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制必须覆盖 `Create workspace`、`Sync changes`、`Approve access`、`Publish rollout` 并回到默认态。
- 主控件 `touch`、`Space / Enter` 与 setter / style 状态清理链路不能残留 `pressed`。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写内容、样式或触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_input_compound_button` 正常加载。

## 11. 与现有控件的边界
- 相比 `button`：这里保留主标题和次级说明，是更高信息密度的命令按钮。
- 相比 `toggle_button`：这里不表达 on/off 状态。
- 相比 `split_button`：这里没有分裂动作或 menu flyout。
- 相比 `card_action`：这里仍是按钮，不是卡片入口。

## 12. 当前验收结果（2026-04-26）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/compound_button PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `output\main.exe compound_button`
  - `compound_button` suite `7 / 7`
  - 备注：Windows 长链接触发 `Error 87`，response-file fallback 成功生成 `output/main.exe`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - `python scripts/sync_widget_catalog.py --check`
  - 结果：input 触摸审计 `custom_audited=30 custom_skipped_allowlist=5`，文档编码 `140 files`，catalog `112 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/compound_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_compound_button/default`
  - 共捕获 `8` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `35 / 35` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/compound_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_compound_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1281 colors=141`
- 截图复核结论：
  - 主区覆盖 `Create workspace / Sync changes / Approve access / Publish rollout` 四组 reference 状态
  - 最终稳定帧显式回到默认 `Create workspace`
  - 底部 `Compact / Read only` preview 在关键帧中保持静态，无文字重叠或裁切
