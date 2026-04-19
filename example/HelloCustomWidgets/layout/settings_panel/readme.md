# SettingsPanel 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingCardGroup`
- 对应组件：`SettingCardGroup`
- 当前保留形态：`Workspace settings`、`Backup and alerts`、`Release controls`、`Account review`、`Compact`、`Read only`
- 当前保留交互：主区保留 panel click 与键盘 `Enter / Space`；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：旧 preview focus bridge、会改变 preview 状态的第二条 `compact` 轨道，以及录制尾部的 `preview dismiss / preview click`
- EGUI 适配说明：继续复用仓库内 `layout/settings_panel` 现有实现；本轮只收口 README、reference 录制说明、静态 preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_panel` 用来表达“多行设置项分组”的标准语义。它不是普通列表，也不是单张设置卡，而是把一组带 `value / switch / chevron` 尾部 affordance 的设置行，稳定组织到同一块 Fluent 风格容器里。

## 2. 为什么现有控件不够用
- `settings_card` 只覆盖单个设置入口，不承载多行分组。
- `settings_expander` 面向“设置头部 + 可展开内容”，不是平铺式 setting group。
- `data_list_panel` 更接近通用列表，不强调 setting row 的层级、tone 和尾部控件节奏。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `settings_panel` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `4` 组录制快照：
  - `Workspace settings`
  - `Backup and alerts`
  - `Release controls`
  - `Account review`
- 录制最终稳定帧显式回到默认 `Workspace settings`。
- 底部左侧是 `Compact` 静态 preview，只负责对照紧凑布局密度。
- 底部右侧是 `Read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_settings_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改变 `current_snapshot / compact_mode / read_only_mode`
  - 不触发 panel click

目标目录：
- `example/HelloCustomWidgets/layout/settings_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留 `4` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Workspace settings`
2. 快照 2
   `Backup and alerts`
3. 快照 3
   `Release controls`
4. 快照 4
   `Account review`
5. 最终稳定帧
   回到默认 `Workspace settings`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 258`
- 主控件：`196 x 132`
- 底部 preview 行：`216 x 84`
- 单个 preview：`104 x 84`
- 页面结构：标题 -> 主 `settings_panel` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 tone 差异；tone 仅保留在 eyebrow、focus row 与尾部 affordance 上；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或状态切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Workspace settings` | `Compact` | `Read only` |
| 快照 2 | `Backup and alerts` | 保持不变 | 保持不变 |
| 快照 3 | `Release controls` | 保持不变 | 保持不变 |
| 快照 4 | `Account review` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Workspace settings` | 保持不变 | 保持不变 |
| panel click 与 `Enter / Space` | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_settings_panel.c` 当前覆盖 `9` 条用例：

1. `set_snapshots()` 的 clamp、重置当前快照与空数据回退。
2. `set_current_snapshot()` 对越界输入忽略，重复设置也会清理残留 `pressed`。
3. `set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要先清理 `pressed`。
4. 主区保留真实 `touch down/up` click 行为。
5. 主区保留键盘 `Enter / Space` click 行为。
6. `compact_mode` 切换后清掉旧 `pressed`，但不破坏已有 click 语义。
7. `read_only` 与 `!enable` 都会清理残留 `pressed`，并屏蔽后续 `touch / key`。
8. static preview 吞掉 `touch / key`，且保持 `current_snapshot / compact_mode / read_only_mode` 不变，不触发 click。
9. helper 覆盖 focus index、tone color、pill width、spacing 等内部口径。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Workspace settings`，同时重放底部 `Compact / Read only` preview 固定状态。
2. 抓取默认主区首帧。
3. 切到 `Backup and alerts` 并抓帧。
4. 切到 `Release controls` 并抓帧。
5. 切到 `Account review` 并抓帧。
6. 恢复主区默认 `Workspace settings`，同时重放底部 preview 固定状态。
7. 等待 `SETTINGS_PANEL_RECORD_FINAL_WAIT` 后抓取最终稳定帧。

说明：
- 主区继续保留真实 panel click 与 `Enter / Space` 键盘语义，供手动复核与单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照与最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板，初始化、主区切换、回落默认态和最终抓帧都走同一条显式布局重放路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_panel
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏、裁切或重叠。
- 主区录制只允许出现 `Workspace settings`、`Backup and alerts`、`Release controls`、`Account review` `4` 组可识别状态，且最终稳定帧必须回到默认态。
- 主区继续保留 panel click 与键盘 `Enter / Space` 语义。
- `snapshot / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮或位移。
- 底部 `Compact / Read only` preview 必须在整条 runtime 轨道里保持静态一致。
- static preview 输入后不能改动 `current_snapshot / compact_mode / read_only_mode`，也不能触发 click。
- WASM demo 必须能以 `HelloCustomWidgets_layout_settings_panel` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_panel/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区唯一状态分组：`[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区差分边界：`(46, 75) - (433, 259)`
  - 遮罩主区后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 260` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `settings_card`：这里是多行设置分组，不是单个入口卡。
- 相比 `settings_expander`：这里不承接 `expand / collapse`，只保留平铺 rows。
- 相比 `data_list_panel`：这里强调 setting row 语义与尾部 affordance，而不是通用列表。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Workspace settings`
  - `Backup and alerts`
  - `Release controls`
  - `Account review`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - panel click
  - 键盘 `Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview focus bridge
  - 第二条 `compact` preview 轨道
  - 录制尾部 `preview dismiss / preview click`

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `settings_panel` suite `9 / 9`
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
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_settings_panel/default`
  - 共捕获 `11` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1967 colors=184`
- 截图复核结论：
  - 主区覆盖 `Workspace settings / Backup and alerts / Release controls / Account review` 四组 reference 快照
  - 最终稳定帧显式回到默认 `Workspace settings`
  - 主区差分边界收敛到 `(46, 75) - (433, 259)`
  - 主区外区域与底部 `Compact / Read only` preview 全程保持静态一致
