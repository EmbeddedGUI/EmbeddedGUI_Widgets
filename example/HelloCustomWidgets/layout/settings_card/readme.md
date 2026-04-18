# settings_card 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingCard`
- 对应组件名：`SettingCard`
- 本次保留语义：`Backup window`、`Sharing scope`、`Rollout ring`、`compact`、`read only`
- 本次删减内容：旧 preview 清主卡焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_settings_card` reference 实现，本轮只收口参考页结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_card` 用来表达“单个设置入口卡片”的标准语义。它不是设置分组容器，也不是可展开面板，而是把 `leading / title / description / trailing / footer` 汇聚到同一张卡片上的基础设置入口。

## 2. 为什么现有控件不够用
- `settings_panel` 更偏向多行设置项分组，不承担单卡点击与焦点语义。
- `settings_expander` 面向“设置头部 + 嵌套内容”，不适合只表达单个设置入口。
- `card_panel` 是信息摘要卡，不强调设置项的 `leading / trailing` 节奏和只读弱化状态。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `settings_card` 和两个底部静态 preview。
- 主控件展示三组 snapshot：
  - `Backup window`
  - `Sharing scope`
  - `Rollout ring`
- 左下角是 `compact` 静态 preview，只负责对照紧凑密度。
- 右下角是 `read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_settings_card_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改动 `current_snapshot / current_part / compact_mode / read_only_mode`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/settings_card/`

## 4. 视觉与布局规格
- 根布局：`224 x 232`
- 主控件：`196 x 96`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 页面结构：标题 -> 主 `settings_card` -> `compact / read only`
- 样式约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量 tone 提示。
  - tone 只保留在顶部 accent line、eyebrow、leading 区和 trailing affordance。
  - 底部 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或收尾叙事。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `card_primary` | `egui_view_settings_card_t` | `196 x 96` | `Backup window` | 主 `SettingCard` |
| `card_compact` | `egui_view_settings_card_t` | `104 x 72` | `Compact backup` | 紧凑静态 preview |
| `card_read_only` | `egui_view_settings_card_t` | `104 x 72` | `Read only policy` | 只读静态 preview |
| `primary_snapshots` | `egui_view_settings_card_snapshot_t[3]` | - | `Backup / Sharing / Rollout` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Backup window` | 默认状态，accent + switch trailing |
| 主控件 | `Sharing scope` | 第二组 snapshot，success + value trailing |
| 主控件 | `Rollout ring` | 第三组 snapshot，warning + chevron trailing |
| `compact` | `Compact backup` | 固定静态对照，只验证紧凑布局 |
| `read only` | `Read only policy` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- `current_part` 只有一个可交互 part：`EGUI_VIEW_SETTINGS_CARD_PART_CARD`
- 主控件保留真实交互闭环：
  - `Home / End / Tab` 保持主卡 part 为当前目标
  - `Enter / Space` 激活当前 part 并触发 `on_action(snapshot_index, part)`
  - 触摸遵循 same-target release：
    - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
    - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_snapshots()`、`set_current_snapshot()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`
- `read_only` 和 `!enable` 期间：
  - `touch / dispatch_key_event` 都不能改状态
  - `current_snapshot / current_part / compact_mode / read_only_mode` 保持符合预期
  - 不触发 listener
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / current_part / compact_mode / read_only_mode` 不变
  - 不触发 listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Backup window`
2. 切到 `Sharing scope`，输出第二组主状态
3. 切到 `Rollout ring`，输出第三组主状态
4. 恢复主控件默认状态并输出最终稳定帧

录制只导出主控件状态变化。底部两个 preview 在整条 reference 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview dismiss / preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_card PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_card --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_card
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_card
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑白屏、裁切或重叠。
- 主区三组 `SettingCard` 状态变化要清晰可辨，底部 preview 全程保持静态。
- `same-target release / keyboard activation / read only / !enable / static preview keeps state` 要全部通过单测。
- `snapshot / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_card/default`
- 复核目标：
  - 主区存在 3 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到底部 preview

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里是单卡 `entry`，不是多卡分组。
- 相比 `settings_expander`：这里不承接 nested rows，也不做展开/折叠。
- 相比 `card_panel`：这里强调设置项语义与 trailing affordance，不是信息摘要卡。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Backup window`
  - `Sharing scope`
  - `Rollout ring`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Tab / Enter / Space`
- 删减的旧桥接与轨道：
  - preview 点击清主卡焦点
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_card PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `settings_card` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_card --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_settings_card/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_card`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_card`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1769 colors=172`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(52, 101) - (427, 228)`
  - 按 `y >= 230` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Backup window / Sharing scope / Rollout ring` 三组 reference 状态，最终稳定帧已显式回到默认快照
