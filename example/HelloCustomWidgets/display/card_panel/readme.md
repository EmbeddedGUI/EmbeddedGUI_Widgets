# card_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件：`Card`
- 当前保留形态：主区 `OVERVIEW`、`SYNC`、`DEPLOY`、`ARCHIVE` 四组 reference 快照，底部 `compact / read only` 双静态 preview
- 当前保留交互：主区保留程序化 snapshot 切换、same-target release 与键盘 click；`snapshot / compact / read_only / disabled` 切换清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：preview snapshot 轮换、preview 点击清主区 focus 的桥接动作、与结构化信息卡无关的额外交互录制和旧说明壳层
- EGUI 适配说明：继续复用当前目录下的 `egui_view_card_panel` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，把 `reference` 页面统一收口到主区四态快照加底部双静态 preview

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
- 页面结构：标题 -> 主 `card_panel` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/display/card_panel/`

## 4. 主区 reference 快照

主区录制轨道保留 `4` 组 reference 快照和最终稳定帧：

1. 默认态
   eyebrow：`OVERVIEW`
   title：`Workspace status`
   summary：`Three flows stay aligned.`
   summary slot：`98% / uptime`
   meta：`Today`
   detail：`Two checks wait.`
   footer：`Footer stays readable.`
   action：`Open`
2. 快照 2
   eyebrow：`SYNC`
   title：`Design review`
   summary：`New handoff needs approval.`
   summary slot：`4 / changes`
   meta：`Next step`
   detail：`Confirm spacing tokens.`
   footer：`Summary stays close.`
   action：`Review`
3. 快照 3
   eyebrow：`DEPLOY`
   title：`Release notes`
   summary：`Ready for staged publish.`
   summary slot：`6 / items`
   meta：`Channel`
   detail：`Internal preview for QA.`
   footer：`Card stays calm on dense pages.`
   action：`Publish`
4. 快照 4
   eyebrow：`ARCHIVE`
   title：`Readback summary`
   summary：`Older detail stays available.`
   summary slot：`12 / pages`
   meta：`History`
   detail：`Summary stays visible.`
   footer：`Read only mode still works.`
   action：`Browse`
5. 最终稳定帧
   eyebrow：`OVERVIEW`
   title：`Workspace status`
   summary：`Three flows stay aligned.`
   summary slot：`98% / uptime`
   meta：`Today`
   detail：`Two checks wait.`
   footer：`Footer stays readable.`
   action：`Open`

底部 preview 在整条录制轨道中始终固定：

1. `compact`
   eyebrow：`TASK`
   title：`Compact`
   summary：`Short.`
   summary slot：`12 / tasks`
   meta：`Focus`
   footer：`Clear layout.`
   action：`Open`
2. `read only`
   eyebrow：`ARCHIVE`
   title：`Archive`
   summary：`Muted.`
   summary slot：`7 / notes`
   meta：`History`
   footer：`Preview only.`
   action：隐藏

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 252`
- 标题：`224 x 18`
- 主 `card_panel`：`196 x 122`
- 底部 preview 行：`216 x 90`
- 单个 preview：`104 x 90`
- 页面风格：浅色 page panel、白底卡片、低噪音边框，顶部 tone strap 只保留轻量强调，不回退到旧 showcase 风格

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `OVERVIEW / Workspace status / Open` | 是 | 否 | 否 |
| `SYNC / Design review / Review` | 是 | 否 | 否 |
| `DEPLOY / Release notes / Publish` | 是 | 否 | 否 |
| `ARCHIVE / Readback summary / Browse` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `TASK / Compact / Open` | 否 | 是 | 否 |
| `ARCHIVE / Archive / hidden action` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_card_panel.c` 当前覆盖 `8` 条用例：

1. `set_snapshots clamps and clears pressed state`
   覆盖 `set_snapshots()` 的数量钳制、空快照清理与 `pressed` 复位。
2. `snapshot and setters clear pressed state`
   覆盖 `set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 的 `pressed` 清理与参数落位。
3. `touch same-target release and cancel behavior`
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交、`DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交，以及 `ACTION_CANCEL` 清理。
4. `keyboard click listener`
   覆盖主控件键盘 `Enter` 触发 click。
5. `compact mode clears pressed and keeps click behavior`
   覆盖 `compact` 模式切换先清残留 `pressed`，恢复后仍保留主卡 click 语义。
6. `read only and disabled guards clear pressed state`
   覆盖 `read only` / disabled 在新输入到来时清理高亮、拒绝 `touch / key` 提交，并在恢复后重新允许交互。
7. `static preview consumes input and keeps snapshot`
   覆盖静态 preview 吞掉输入后 `current_snapshot` 保持原值，且不触发 click listener。
8. `internal helpers cover tone text and pill width`
   覆盖 tone 颜色选择、防御式文本长度与 action pill 宽度估算等内部 helper。

补充说明：

- 主控件继续保留 same-target release 与键盘 click 语义，但不再让底部 preview 参与主区 focus 桥接。
- 底部 `compact / read only` preview 统一通过 `egui_view_card_panel_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 当前 README 与实现统一使用 `read_only_mode` 命名，不再沿用历史 `locked` 表述。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `OVERVIEW` 和底部 preview 固定状态，请求首帧并等待 `CARD_PANEL_RECORD_FRAME_WAIT = 170`
2. 切到 `SYNC / Design review / Review`，等待 `CARD_PANEL_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `DEPLOY / Release notes / Publish`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 切到 `ARCHIVE / Readback summary / Browse`，等待 `90`
7. 请求第四组主区快照并继续等待 `170`
8. 回到默认 `OVERVIEW / Workspace status / Open`，等待 `90`
9. 请求最终稳定帧，并继续等待 `CARD_PANEL_RECORD_FINAL_WAIT = 520`

说明：

- 录制轨道只导出主区四态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 录制阶段不再轮换 `compact` preview，也不再通过 preview 点击桥接去清主区焦点。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
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

## 10. 验收重点

- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `OVERVIEW / SYNC / DEPLOY / ARCHIVE` 四组卡片状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `set_snapshots()`、`snapshot / compact / read_only / disabled` 切换链路，以及 `touch / key` guard 不能残留 `pressed`。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_card_panel/default`
- 已归档复核结果：
  - 共捕获 `11` 帧
  - 主区 RGB 差分边界：`(44, 78) - (436, 261)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`4`
  - 按 `y >= 261` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `OVERVIEW / Workspace status / Open`

## 12. 与现有控件的边界

- 相比 `card`：这里不是通用容器，而是固定层级的结构化信息卡。
- 相比 `message_bar`、`toast_stack`：这里不是反馈控件，不承担临时告警和消息堆叠职责。
- 相比 `badge_group`：这里强调卡片结构、summary slot 和 footer，而不是一组轻量 badge 集群。

## 13. 本轮保留与删减

- 保留的主区状态：`OVERVIEW`、`SYNC`、`DEPLOY`、`ARCHIVE`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：结构化卡片层级、summary slot、action pill、主控件 same-target release 与键盘 click、`snapshot / compact / read_only / disabled` 切换清理 `pressed`、static preview 输入抑制
- 删减的旧桥接与装饰：preview snapshot 轮换、preview 点击清主区 focus 的桥接动作、额外交互录制和旧说明壳层

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc`
  - 本轮沿用 `2026-04-17` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `card_panel` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-17` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_card_panel/default`
  - 本轮沿用 `2026-04-17` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `11` 帧 / `4` 组主区状态 / `y >= 261` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-17` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_card_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1921 colors=182`
  - `web/demos/HelloCustomWidgets_display_card_panel` 构建结果沿用 `2026-04-17` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区 RGB 差分边界：`(44, 78) - (436, 261)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`4`
  - 按 `y >= 261` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `OVERVIEW / SYNC / DEPLOY / ARCHIVE` 四组 reference 快照，最终稳定帧已回到默认态，底部 `compact / read only` preview 全程静态
