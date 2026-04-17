# card_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件：`Card`
- 当前保留语义：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前保留交互：主区保留程序化 snapshot 切换，底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：preview snapshot 轮换、preview 点击清主区 focus 的桥接动作、与结构化信息卡无关的额外交互录制和旧说明壳层
- EGUI 适配说明：继续复用当前目录下的 `egui_view_card_panel` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，把 `reference` 页面统一收口到主区四态 snapshots 加底部双静态 preview

## 1. 为什么需要这个控件
`card_panel` 用来承载一张结构化信息卡：顶部 badge、标题、正文摘要、右侧 summary slot、底部 detail strip 与可选 action pill。在 Fluent / WPF UI 语义里，它适合放在设置页、概览页和详情页中，用一张低噪音卡片承载“主信息 + 次级摘要 + 轻量动作”。

## 2. 为什么现有控件不够用
- `card` 更偏通用容器，不负责固定的信息层级。
- `layer_stack` 强调叠层和景深，不适合作为标准信息卡。
- `message_bar` 和 `toast_stack` 属于反馈控件，不是常驻摘要卡。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `Card` 语义的 `reference` 示例。

## 3. 当前页面结构
- 标题：`Card Panel`
- 主区：一个标准 `card_panel`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`

目录：
- `example/HelloCustomWidgets/display/card_panel/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference snapshots：

1. 默认态
   - eyebrow：`OVERVIEW`
   - title：`Workspace status`
   - summary：`Three flows stay aligned.`
   - summary slot：`98% / uptime`
   - meta：`Today`
   - detail：`Two checks wait.`
   - footer：`Footer stays readable.`
   - action：`Open`
2. 快照 2
   - eyebrow：`SYNC`
   - title：`Design review`
   - summary：`New handoff needs approval.`
   - summary slot：`4 / changes`
   - meta：`Next step`
   - detail：`Confirm spacing tokens.`
   - footer：`Summary stays close.`
   - action：`Review`
3. 快照 3
   - eyebrow：`DEPLOY`
   - title：`Release notes`
   - summary：`Ready for staged publish.`
   - summary slot：`6 / items`
   - meta：`Channel`
   - detail：`Internal preview for QA.`
   - footer：`Card stays calm on dense pages.`
   - action：`Publish`
4. 快照 4
   - eyebrow：`ARCHIVE`
   - title：`Readback summary`
   - summary：`Older detail stays available.`
   - summary slot：`12 / pages`
   - meta：`History`
   - detail：`Summary stays visible.`
   - footer：`Read only mode still works.`
   - action：`Browse`

底部 preview 在整条录制轨道中始终固定：

1. `compact`
   - eyebrow：`TASK`
   - title：`Compact`
   - summary：`Short.`
   - summary slot：`12 / tasks`
   - meta：`Focus`
   - footer：`Clear layout.`
   - action：`Open`
2. `read only`
   - eyebrow：`ARCHIVE`
   - title：`Archive`
   - summary：`Muted.`
   - summary slot：`7 / notes`
   - meta：`History`
   - footer：`Preview only.`
   - action：隐藏

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 252`
- 标题：`224 x 18`
- 主 `card_panel`：`196 x 122`
- 底部 preview 行：`216 x 90`
- 单个 preview：`104 x 90`
- 页面结构：标题 -> 主 `card_panel` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、白底卡片、低噪音边框，顶部 tone strap 只保留轻量强调，不回退到旧 showcase 风格

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `OVERVIEW / Workspace status / Open` | `TASK / Compact / Open` | `ARCHIVE / Archive / hidden action` |
| 快照 2 | `SYNC / Design review / Review` | 保持不变 | 保持不变 |
| 快照 3 | `DEPLOY / Release notes / Publish` | 保持不变 | 保持不变 |
| 快照 4 | `ARCHIVE / Readback summary / Browse` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `OVERVIEW / Workspace status / Open` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 snapshot 和底部 preview 固定状态
2. 抓取首帧
3. 切到 `SYNC / Design review / Review`
4. 抓取第二组主区快照
5. 切到 `DEPLOY / Release notes / Publish`
6. 抓取第三组主区快照
7. 切到 `ARCHIVE / Readback summary / Browse`
8. 抓取第四组主区快照
9. 回到默认 `OVERVIEW / Workspace status / Open`
10. 抓取最终稳定帧

说明：
- 录制阶段不再轮换 `compact` preview。
- 不再通过 preview 点击桥接去清主区焦点。
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 4 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `CARD_PANEL_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_card_panel.c` 当前覆盖 `8` 个用例：

1. `set_snapshots` 钳制与 `pressed` 清理
2. snapshot 与 setter 更新后的 `pressed` 清理
3. `same-target release`、`ACTION_CANCEL` 与 touch 行为
4. 键盘 click listener
5. `compact` 模式保持点击语义且清理残留 `pressed`
6. `read only / disabled` guard 清理残留 `pressed`
7. 静态 preview `consumes input and keeps snapshot`
8. tone / text / action pill 宽度等内部 helper

补充说明：
- 静态 preview 用例会固定验证 `touch / key` 事件被吞掉后，`current_snapshot` 仍保持原值，且不会触发 click listener。
- 主控件单测继续覆盖 palette、字体、snapshot 切换、same-target release 和只读禁用守卫。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/card_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_card_panel
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `card_panel` suite `8 / 8`
- `sync_widget_catalog.py`：已通过，本轮无额外目录变化
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots`，输出 `11 frames captured -> D:\workspace\gitee\EmbeddedGUI_Widgets\runtime_check_output\HelloCustomWidgets_display_card_panel\default`
- display 分类 compile/runtime 回归：已通过
  compile `21 / 21`
  runtime `21 / 21`
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/card_panel`，输出 `web/demos/HelloCustomWidgets_display_card_panel`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_card_panel`，结果 `PASS status=Running canvas=480x480 ratio=0.1921 colors=182`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_card_panel/default`

复核结果：
- 总帧数：`11`
- 主区 RGB 差分边界：`(44, 78) - (436, 261)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`4`
- 按 `y >= 261` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `4`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `card_panel` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `11` 帧里主区保持 `4` 组唯一状态：`[0,1,8,9,10]` 对应默认 `OVERVIEW / Workspace status / Open`，`[2,3]` 对应 `SYNC / Design review / Review`，`[4,5]` 对应 `DEPLOY / Release notes / Publish`，`[6,7]` 对应 `ARCHIVE / Readback summary / Browse`；最终稳定帧已显式回到默认态。
- 按 `y >= 261` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前版本仍使用固定 snapshot 数据和固定 summary slot，不覆盖超长标题、超长摘要和超长 `value`。
- 当前不做真实图标、hover、完整 focus ring 和桌面级键盘导航细节。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部卡片容器系统。
- 底部 `compact / read only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `card`：这里不是通用容器，而是固定层级的结构化信息卡。
- 相比 `message_bar`、`toast_stack`：这里不是反馈控件，不承担临时告警和消息堆叠职责。
- 相比 `badge_group`：这里强调卡片结构、summary slot 和 footer，而不是一组轻量 badge 集群。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_card_panel` custom view，不修改 SDK。
- 主区保留 `OVERVIEW`、`SYNC`、`DEPLOY`、`ARCHIVE` 四组 reference snapshots。
- 底部 preview 通过 `egui_view_card_panel_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 `4` 组 reference snapshots、底部 preview 全程静态，以及真实 touch / key 语义由单测闭环而不是由 runtime 录制承担。
