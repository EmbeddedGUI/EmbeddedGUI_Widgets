# settings_expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingsExpander`
- 对应组件名：`SettingsExpander`
- 本次保留语义：`Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence`、`compact`、`read only`、`expanded / collapsed`
- 本次删减内容：旧 preview 清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_settings_expander` reference 实现，本轮只收口参考页结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_expander` 用来表达“一个 setting header + 一组按需展开的 nested rows”。它不是通用正文 `Expander`，也不是纯静态设置卡，而是 Fluent 设置页里常见的“主设置项承载子设置项”的标准语义。

## 2. 为什么现有控件不够用
- `settings_card` 只覆盖单个设置入口，不负责 nested rows。
- `settings_panel` 偏设置分组容器，不强调单个 header 的展开/折叠交互。
- `expander` 更通用，不包含 settings 头部、value、tone 和子行节奏。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `settings_expander` 和两个底部静态 preview。
- 主控件展示四组 snapshot：
  - `Backup options expanded`
  - `Sharing scope expanded`
  - `Quiet hours collapsed`
  - `Rollout cadence expanded`
- 左下角是 `compact` 静态 preview，只负责对照紧凑密度。
- 右下角是 `read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_settings_expander_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改动 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/settings_expander/`

## 4. 视觉与布局规格
- 根布局：`224 x 270`
- 主控件：`196 x 146`
- 底部对照行：`216 x 88`
- `compact` preview：`104 x 88`
- `read only` preview：`104 x 88`
- 页面结构：标题 -> 主 `settings_expander` -> `compact / read only`
- 样式约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量 tone 提示。
  - tone 只保留在 header、row icon、value/switch 和 footer 的轻量差异上。
  - collapsed 态只保留 header，可辨识但不过度强调。
  - 底部 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或收尾叙事。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `expander_primary` | `egui_view_settings_expander_t` | `196 x 146` | `Backup options expanded` | 主 `SettingsExpander` |
| `expander_compact` | `egui_view_settings_expander_t` | `104 x 88` | `Compact` | 紧凑静态 preview |
| `expander_read_only` | `egui_view_settings_expander_t` | `104 x 88` | `Read only` | 只读静态 preview |
| `primary_snapshots` | `egui_view_settings_expander_snapshot_t[4]` | - | `Backup / Sharing / Quiet / Rollout` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup options expanded` | 默认状态，accent + nested rows |
| 主控件 | `Sharing scope expanded` | 第二组 snapshot，success tone + grouped rows |
| 主控件 | `Quiet hours collapsed` | 第三组 snapshot，验证 header-only 折叠态 |
| 主控件 | `Rollout cadence expanded` | 第四组 snapshot，warning tone + emphasized rows |
| `compact` | `Compact expanded` | 固定静态对照，只验证紧凑布局 |
| `read only` | `Read only expanded` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实交互闭环：
  - `HEADER` 始终存在
  - `ROW` 只在 `expanded` 时可交互
  - `Home / End / Up / Down / Tab` 在 `header + rows` 间导航
  - `Enter / Space` 激活当前 part
  - `Escape` 在展开时收起到 `HEADER`
- `HEADER` 激活只切换 `expanded_state`，不触发 `on_action`
- `ROW` 激活才触发 `on_action(snapshot_index, part)`
- 触摸保持 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_snapshots()`、`set_current_snapshot()`、`set_expanded()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`
- `read_only` 和 `!enable` 期间：
  - `touch / dispatch_key_event` 都不能改状态
  - `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode` 保持符合预期
  - 不触发 listener
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode` 不变
  - 不触发 listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Backup options expanded`
2. 切到 `Sharing scope expanded`，输出第二组主状态
3. 切到 `Quiet hours collapsed`，输出第三组主状态
4. 切到 `Rollout cadence expanded`，输出第四组主状态
5. 恢复主控件默认状态并输出最终稳定帧

录制只导出主控件状态变化。底部两个 preview 在整条 reference 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview dismiss / preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_expander
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑白屏、裁切或重叠。
- 主区四组 `SettingsExpander` 状态变化要清晰可辨，底部 preview 全程保持静态。
- `same-target release / keyboard navigation / header toggle / row action / Escape / read only / !enable / static preview keeps state` 要全部通过单测。
- `snapshot / expanded / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_expander/default`
- 复核目标：
  - 主区存在多组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到底部 preview

## 12. 与现有控件的边界
- 相比 `settings_card`：这里承接 nested rows 与 expand/collapse，不是单卡入口。
- 相比 `settings_panel`：这里是单个 setting header 的折叠语义，不是设置分组。
- 相比 `expander`：这里强调 setting header、value、tone 与 row 节奏，不是通用 disclosure 正文块。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Backup options`
  - `Sharing scope`
  - `Quiet hours`
  - `Rollout cadence`
  - `compact`
  - `read only`
  - `expanded / collapsed`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Up / Down / Tab / Enter / Space / Escape`
- 删减的旧桥接与轨道：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `settings_expander` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_expander --track reference --timeout 10 --keep-screenshots`
  - `11 frames captured -> runtime_check_output/HelloCustomWidgets_layout_settings_expander/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_expander`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_expander`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2059 colors=186`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区变化边界保持在 `(46, 66) - (433, 274)`
  - 按 `y >= 276` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Backup options / Sharing scope / Quiet hours / Rollout cadence` 四组 reference 状态，最终稳定帧已显式回到默认快照
