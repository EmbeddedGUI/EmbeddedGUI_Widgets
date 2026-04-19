# SettingsExpander 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingsExpander`
- 对应组件：`SettingsExpander`
- 当前保留形态：`Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence`、`Compact`、`Read only`
- 当前保留交互：主区保留 `HEADER / ROW` 真实导航、same-target release 与键盘 `Home / End / Up / Down / Tab / Enter / Space / Escape` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：旧 preview 清主控件焦点桥接、会改写 preview 状态的第二条 `compact` 轨道、录制中的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/settings_expander`，底层仍复用仓库内现有 `egui_view_settings_expander` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_expander` 用来表达“设置头部承载一组可展开子项”的设置页语义。它不是通用正文 `Expander`，也不是只负责单卡入口的 `SettingCard`，而是 Fluent 设置页里常见的“主设置项 + nested rows”结构。

## 2. 为什么现有控件不够用
- `settings_card` 只覆盖单个设置入口，不承接 nested rows。
- `settings_panel` 更偏设置分组容器，不强调单个 header 的展开 / 折叠交互。
- `expander` 更通用，不包含 settings header、value、tone 和 nested rows 的节奏。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `settings_expander` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `4` 组录制快照：
  - `Backup options`
  - `Sharing scope`
  - `Quiet hours`
  - `Rollout cadence`
- 录制最终稳定帧显式回到默认 `Backup options`。
- `Quiet hours` 是默认折叠的 `collapsed` 快照，用来验证 header-only 状态。
- 底部左侧是 `Compact` 静态 preview，固定对照 `Compact`，保持 `compact_mode = 1`、`expanded_state = 1`。
- 底部右侧是 `Read only` 静态 preview，固定对照 `Read only`，保持 `read_only_mode = 1`、`expanded_state = 1`。
- 两个 preview 统一通过 `egui_view_settings_expander_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode`
  - 不触发 `on_action_listener`

目标目录：
- `example/HelloCustomWidgets/layout/settings_expander/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Backup options`
2. 快照 2
   `Sharing scope`
3. 快照 3
   `Quiet hours`
   该快照为 `collapsed`
4. 快照 4
   `Rollout cadence`
5. 最终稳定帧
   回到默认 `Backup options`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 270`
- 主控件：`196 x 146`
- 底部 preview 行：`216 x 88`
- 单个 preview：`104 x 88`
- 页面结构：标题 -> 主 `settings_expander` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 tone 差异；`accent / success / neutral / warning` 只在 header、row 图标、value / switch 和 footer 上局部变化；`Quiet hours` 的折叠态只保留 header；底部 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或收尾叙事。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Backup options` | `Compact` | `Read only` |
| 快照 2 | `Sharing scope` | 保持不变 | 保持不变 |
| 快照 3 | `Quiet hours`（collapsed） | 保持不变 | 保持不变 |
| 快照 4 | `Rollout cadence` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Backup options` | 保持不变 | 保持不变 |
| `HEADER / ROW` 导航、same-target release、键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_settings_expander.c` 当前覆盖 `9` 条用例：

1. `set_snapshots()`、`set_current_part()` 的 clamp、默认态回退和状态清理。
2. 字体、配色、`expanded / compact / read_only` setter 的 `pressed` 清理与状态更新。
3. metrics、命中测试和 `HEADER / ROW` 区域边界行为。
4. `activate_current_part()` 与 `on_action` listener：`HEADER` 只切 `expanded_state`，`ROW` 激活才触发动作。
5. 触摸 same-target release 与 `ACTION_CANCEL` 语义。
6. 键盘导航与激活闭环。
7. `read_only / !enable` 守卫清理残留 `pressed` 并屏蔽输入；恢复后重新允许 `HOME / END + ENTER / SPACE`。
8. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode` 不变，同时不触发 `on_action_listener`。
9. 内部 helper、row 解析和附属区域逻辑。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- setter、guard 与 static preview 路径都统一要求先清理残留 `pressed`，再处理后续状态。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Backup options`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT`。
2. 切到 `Sharing scope`，等待 `SETTINGS_EXPANDER_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT`。
4. 切到 `Quiet hours`，等待 `SETTINGS_EXPANDER_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT`。
6. 切到 `Rollout cadence`，等待 `SETTINGS_EXPANDER_RECORD_WAIT`。
7. 抓取第四组主区快照，等待 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT`。
8. 恢复主区默认 `Backup options`，同时重放底部 preview 固定状态，等待 `SETTINGS_EXPANDER_RECORD_FINAL_WAIT`。
9. 通过最终抓帧输出稳定的默认态，并继续等待 `SETTINGS_EXPANDER_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `4` 组快照和最终稳定帧的布局口径一致。
- README 这里按当前 `test.c` 如实保留中间切换使用 `SETTINGS_EXPANDER_RECORD_WAIT`、抓帧使用 `SETTINGS_EXPANDER_RECORD_FRAME_WAIT`、最终回落与最终抓帧使用 `SETTINGS_EXPANDER_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc

# 在 X:\ 短路径下执行
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
- 主区录制只允许出现 `Backup options`、`Sharing scope`、`Quiet hours`、`Rollout cadence` 四组可识别状态，最终稳定帧必须回到默认 `Backup options`。
- `HEADER` 只切 `expanded_state`，`ROW` 激活才触发 `on_action`；same-target release、键盘导航、`Escape` 收起、`read_only / !enable` 守卫都必须保持有效。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧中保持静态一致，并持续吞掉 `touch / key` 且不改 `current_snapshot / current_part / expanded_state / compact_mode / read_only_mode`。
- static preview、setter 和 guard 路径都必须先清理残留 `pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_settings_expander` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_expander/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区唯一状态分组：`[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界：`(46, 66) - (433, 274)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 275` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，最终稳定帧显式回到默认 `Backup options`

## 12. 与现有控件的边界
- 相比 `settings_card`：这里承接 nested rows 与 expand / collapse，不是单卡入口。
- 相比 `settings_panel`：这里是单个 setting header 的折叠语义，不是多卡设置分组。
- 相比 `expander`：这里强调 settings header、value、tone 与 row 节奏，不是通用 disclosure 正文块。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Backup options`
  - `Sharing scope`
  - `Quiet hours`
  - `Rollout cadence`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - `HEADER / ROW` 导航
  - same-target release
  - 键盘 `Home / End / Up / Down / Tab / Enter / Space / Escape`
- 删减的旧桥接与旧轨道：
  - preview 清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_expander PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `settings_expander` suite `9 / 9`
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
  - 共捕获 `11` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_expander`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_expander`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2059 colors=186`
- 截图复核结论：
  - 主区覆盖 `Backup options / Sharing scope / Quiet hours / Rollout cadence` 四组 reference 状态
  - 最终稳定帧显式回到默认 `Backup options`
  - 主区 RGB 差分边界收敛到 `(46, 66) - (433, 274)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact / Read only` preview 以 `y >= 275` 裁切后全程保持单哈希静态
