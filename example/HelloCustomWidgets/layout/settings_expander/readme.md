# SettingsExpander 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingsExpander`
- 对应组件：`SettingsExpander`
- 当前保留形态：`Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence`、`Compact`、`Read only`
- 当前保留交互：主区保留 `HEADER / ROW` 的真实导航、same-target release、`Home / End / Up / Down / Tab / Enter / Space / Escape` 键盘闭环；底部 preview 保留静态 reference 对照
- 当前移除内容：旧 preview 清主控件焦点桥接、会改写 preview 状态的第二条 `compact` 轨道，以及录制中的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/settings_expander`，底层仍复用仓库内现有 `egui_view_settings_expander` 实现；本轮只收口 reference 页面结构、录制轨道、README 口径和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_expander` 用来表达“设置头部承载一组可展开子项”的标准设置页语义。它不是通用正文 `Expander`，也不是只负责单卡入口的 `SettingCard`，而是 Fluent 设置页里常见的“主设置项 + nested rows”结构。

仓库里已有 `settings_card`、`settings_panel` 和 `expander`，但仍缺一个能稳定承接 `SettingsExpander` 单项设置展开语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `settings_card` 只覆盖单个设置入口，不承接 nested rows。
- `settings_panel` 更偏设置分组容器，不强调单个 header 的展开 / 折叠交互。
- `expander` 更通用，不包含 settings header、value、tone 和 nested rows 的节奏。

## 3. 当前页面结构
- 标题：`Settings Expander`
- 主区：1 个保留真实 `SettingsExpander` 语义的 `settings_expander`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact`，保持 `compact_mode = 1`、`expanded_state = 1`
- 右侧 preview：`read only`，固定显示 `Read only`，保持 `read_only_mode = 1`、`expanded_state = 1`

目录：
- `example/HelloCustomWidgets/layout/settings_expander/`

## 4. 主区 reference 快照
主区录制轨道只保留 `4` 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Backup options`
2. 快照 2
   `Sharing scope`
3. 快照 3
   `Quiet hours`
   该快照为 `collapsed`，用于验证 header-only 折叠态
4. 快照 4
   `Rollout cadence`
5. 最终稳定帧
   回到默认 `Backup options`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact`
2. `read only`
   `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 270`
- 主控件：`196 x 146`
- 底部 preview 行：`216 x 88`
- 单个 preview：`104 x 88`
- 页面结构：标题 -> 主 `settings_expander` -> 底部 `compact / read only`
- 风格约束：浅色 Fluent 容器、低噪音边框和轻量 tone 差异；`accent / success / neutral / warning` 只在 header、row 图标、value / switch 和 footer 上局部变化；`Quiet hours` 的折叠态只保留 header，不回退到 showcase 式夸张表现；底部两个 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或收尾叙事

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Backup options` | `Compact` | `Read only` |
| 快照 2 | `Sharing scope` | 保持不变 | 保持不变 |
| 快照 3 | `Quiet hours`（collapsed） | 保持不变 | 保持不变 |
| 快照 4 | `Rollout cadence` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Backup options` | 保持不变 | 保持不变 |
| `HEADER / ROW` 导航、same-target release、键盘激活 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态，并通过 `request_page_snapshot()` 抓取首帧
2. 切到 `Sharing scope`，等待 `SETTINGS_EXPANDER_RECORD_WAIT = 90`
3. 抓取第二组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT = 180`
4. 切到 `Quiet hours`，等待 `SETTINGS_EXPANDER_RECORD_WAIT = 90`
5. 抓取第三组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT = 180`
6. 切到 `Rollout cadence`，等待 `SETTINGS_EXPANDER_RECORD_WAIT = 90`
7. 抓取第四组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT = 180`
8. 恢复默认主区和底部 preview 固定状态，等待 `SETTINGS_EXPANDER_RECORD_FINAL_WAIT = 280`
9. 抓取最终稳定帧，再等待 `SETTINGS_EXPANDER_RECORD_FINAL_WAIT = 280`

说明：
- 录制阶段只导出主区状态变化，不再真实发送底部 preview 输入。
- 底部 preview 统一通过 `egui_view_settings_expander_override_static_preview_api()` 吞掉 `touch / key`，只清理残留 `pressed`。
- `request_page_snapshot()` 统一走 `layout + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照和最终稳定帧口径一致。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_settings_expander.c` 当前覆盖 `9` 条用例：

1. `set_snapshots / set_current_part` 的 clamp、默认态回退和状态清理。
2. 字体、配色、`expanded / compact / read only` setter 的 pressed 清理与状态更新。
3. metrics、命中测试和 `HEADER / ROW` 区域边界行为。
4. `activate_current_part()` 与 `on_action` listener：`HEADER` 只切 `expanded_state`，`ROW` 激活才触发动作。
5. 触摸 same-target release 与 cancel 语义。
6. 键盘导航与激活闭环。
7. `read only / !enable` guard 清理残留 `pressed` 且屏蔽输入。
8. static preview 吞掉 `touch / key` 且保持 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode` 不变。
9. 内部 helper、row 解析和附属区域逻辑。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏、裁切或重叠。
- 主区录制只允许出现 `Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence` `4` 组可识别状态，最终稳定帧必须回到默认 `Backup options`。
- `HEADER` 只切 `expanded_state`，`ROW` 激活才触发 `on_action`；same-target release、键盘导航、`Escape` 收起、`read only / !enable` guard 都必须保持有效。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧中保持静态一致，并持续吞掉 `touch / key` 且不改 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode`。
- setter、guard 和 static preview 都必须先清理残留 `pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_settings_expander` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_expander/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 全帧唯一状态分组为 `[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界为 `(46, 66) - (433, 274)`
  - 遮罩主区后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 275` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `settings_card`：这里承接 nested rows 与 expand / collapse，不是单卡入口。
- 相比 `settings_panel`：这里是单个 setting header 的折叠语义，不是多卡设置分组。
- 相比 `expander`：这里强调 settings header、value、tone 与 row 节奏，不是通用 disclosure 正文块。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Backup options`
  - `Sharing scope`
  - `Quiet hours`
  - `Rollout cadence`
  - `Compact`
  - `Read only`
  - `expanded / collapsed`
- 本次保留交互：
  - `HEADER / ROW` 导航
  - same-target release
  - 键盘 `Home / End / Up / Down / Tab / Enter / Space / Escape`
- 本次删减内容：
  - preview 清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `settings_expander` suite `9 / 9`
  - 无关 warning：`test_split_view.c:186:13: warning: 'get_view_center' defined but not used`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_settings_expander/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_expander`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_expander`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2059 colors=186`
- 截图复核结论：
  - 主区覆盖默认 `Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence` `4` 组 reference 快照
  - 最终稳定帧显式回到默认 `Backup options`
  - 底部 `Compact / Read only` preview 全程静态
