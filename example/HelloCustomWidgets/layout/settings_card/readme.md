# SettingCard 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingCard`
- 对应组件：`SettingCard`
- 当前保留形态：`Backup window`、`Sharing scope`、`Rollout ring`、`Compact backup`、`Read only policy`
- 当前保留交互：主区保留单卡激活、same-target release 与 `Home / End / Tab / Enter / Space` 键盘闭环；底部 preview 保留静态 reference 对照
- 当前移除内容：旧 preview 清主卡焦点桥接、第二条 `compact` preview 轨道，以及录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/settings_card`，底层仍复用仓库内现有 `egui_view_settings_card` 实现；本轮只收口 reference 页面结构、录制轨道、README 口径与静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_card` 用来表达“单个设置入口卡片”的标准语义。它不是设置分组容器，也不是可展开面板，而是把 `leading / title / description / trailing / footer` 聚合到一张卡片上的基础设置入口。

仓库里已有 `settings_panel`、`settings_expander`、`card_panel` 和 `card_action`，但仍缺一个能稳定承接 `SettingCard` 单卡入口语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `settings_panel` 更偏多行设置分组，不承担单卡点击与焦点语义。
- `settings_expander` 面向“设置头部 + 嵌套内容”，不适合只表达单个设置入口。
- `card_panel` 更偏信息摘要卡，不强调设置项的 `leading / trailing` 节奏和只读弱化状态。
- `card_action` 只有轻量操作卡语义，缺少 `description / footer / trailing` 的设置项表达。

## 3. 当前页面结构
- 标题：`Settings card`
- 主区：1 个保留真实 `SettingCard` 单卡激活语义的 `settings_card`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact backup`
- 右侧 preview：`read only`，固定显示 `Read only policy`

目录：
- `example/HelloCustomWidgets/layout/settings_card/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Backup window`
2. 快照 2
   `Sharing scope`
3. 快照 3
   `Rollout ring`
4. 最终稳定帧
   回到默认 `Backup window`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact backup`
2. `read only`
   `Read only policy`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主控件：`196 x 96`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `settings_card` -> 底部 `compact / read only`
- 风格约束：浅色 Fluent 容器、低噪音边框和轻量 tone 差异；tone 只保留在顶部 accent line、eyebrow、leading 区和 trailing affordance；底部两个 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或收尾叙事

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Backup window` | `Compact backup` | `Read only policy` |
| 快照 2 | `Sharing scope` | 保持不变 | 保持不变 |
| 快照 3 | `Rollout ring` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Backup window` | 保持不变 | 保持不变 |
| `same-target release`、单卡激活与键盘闭环 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Sharing scope`
4. 抓取第二组主区快照
5. 切到 `Rollout ring`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实单卡激活、same-target release 和 `Home / End / Tab / Enter / Space` 键盘语义，供运行时手动复核。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- 底部 preview 统一通过 `egui_view_settings_card_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，主区中间状态抓帧使用 `SETTINGS_CARD_RECORD_WAIT / SETTINGS_CARD_RECORD_FRAME_WAIT`，最终稳定抓帧使用 `SETTINGS_CARD_RECORD_FINAL_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_settings_card.c` 当前覆盖 `9` 条用例：

1. `set_snapshots / set_current_part` 的 clamp、默认态回退和状态清理。
2. 字体、配色、`compact / read only` setter 的 pressed 清理与状态更新。
3. metrics、命中测试和区域边界行为。
4. `activate_current_part()` 与 `on_action` listener 触发。
5. 触摸 same-target release 与 cancel 语义。
6. 键盘 `Home / End / Tab / Enter / Space` 导航与激活闭环。
7. `read only / !enable` guard 清理残留 `pressed` 且屏蔽输入。
8. static preview 吞掉 `touch / key` 且保持 `current_snapshot / current_part / compact_mode / read_only_mode` 不变。
9. 内部 helper、颜色混合和 trailing/文本区域辅助逻辑。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_card PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Backup window`、`Sharing scope`、`Rollout ring` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、`read only / !enable` guard 和键盘单卡激活语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- setter、guard 和 static preview 都必须统一遵守“先清理残留 `pressed` 再处理后续状态”的语义。
- WASM demo 必须能以 `HelloCustomWidgets_layout_settings_card` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_card/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(52, 101) - (427, 228)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 230` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里是单卡 `entry`，不是多卡设置分组。
- 相比 `settings_expander`：这里不承接 nested rows，也不做展开 / 折叠。
- 相比 `card_panel`：这里强调设置项语义与 trailing affordance，不是信息摘要卡。
- 相比 `card_action`：这里保留 `description / footer / trailing` 的设置项结构，而不是轻量动作卡。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Backup window`
  - `Sharing scope`
  - `Rollout ring`
  - `Compact backup`
  - `Read only policy`
- 本次保留交互：
  - same-target release
  - 键盘 `Home / End / Tab / Enter / Space`
- 删减的装饰或桥接：
  - preview 点击清主卡焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/settings_card PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `settings_card` suite `9 / 9`
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
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_card --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_settings_card/default`
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
  - 主区 RGB 差分边界为 `(52, 101) - (427, 228)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 230` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Backup window`、`Sharing scope` 与 `Rollout ring` `3` 组 reference 快照，最终稳定帧显式回到默认 `Backup window`，底部 `Compact backup / Read only policy` preview 全程静态
