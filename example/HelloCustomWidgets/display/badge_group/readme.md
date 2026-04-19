# badge_group 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件：`BadgeGroup`
- 当前保留形态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前保留交互：主区保留程序化 snapshot 切换；主控件继续保留 same-target release 与键盘 click 语义；底部 `compact / read only` 统一保持静态 preview 并吞掉输入
- 当前移除内容：`compact` preview 轮换、preview 点击收尾、页面级 `guide / status` chrome、旧双列包裹壳与额外录制桥接帧
- EGUI 适配说明：继续复用当前目录下的 `egui_view_badge_group` custom view，保留 `accent / success / warning / neutral` 四组主区快照与 `egui_view_badge_group_override_static_preview_api()` 静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`badge_group` 用来展示一组语义相关的 badge，并让 focus badge 驱动整张卡片的 tone 与 footer summary。它适合出现在概览页、审阅页和状态面板中，用于表达“同一条信息有多个维度，但仍然属于一组”的轻量展示。

## 2. 为什么现有控件不够用

- `notification_badge` 只解决单个角标或计数，不解决多 badge 并列组合。
- `chips` 更偏交互筛选和选中态，不适合作为静态信息组合。
- `tag_cloud` 强调自由分布和权重表达，不强调 focus badge 对摘要的驱动关系。
- `card_panel` 更偏结构化卡片，不适合做低噪声的 badge 集群。

## 3. 当前页面结构

- 标题：`Badge Group`
- 主区：一个标准 `badge_group`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `SET / Compact`
- 右侧 preview：`read only`，固定显示 `ARCHIVE / Read only`
- 页面结构：标题 -> 主 `badge_group` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/display/badge_group/`

## 4. 主区 reference 快照

主区录制轨道保留 `4` 组程序化快照和最终稳定帧：

1. 默认态
   眉标：`TRIAGE`
   标题：`Release lanes`
   tone：`accent`
   badge：`Review`、`Ready`、`Risk`、`Archive`
2. 快照 2
   眉标：`QUEUE`
   标题：`Ops handoff`
   tone：`success`
   badge：`Online`、`Shadow`、`Sync`、`Alert`
3. 快照 3
   眉标：`RISK`
   标题：`Change review`
   tone：`warning`
   badge：`Queued`、`Hold`、`Owner`、`Done`
4. 快照 4
   眉标：`CALM`
   标题：`Archive sweep`
   tone：`neutral`
   badge：`Pinned`、`Calm`、`Watch`、`Live`
5. 最终稳定帧
   眉标：`TRIAGE`
   标题：`Release lanes`
   tone：`accent`
   badge：`Review`、`Ready`、`Risk`、`Archive`

底部 preview 在整条轨道中始终固定：

1. `compact`
   眉标：`SET`
   标题：`Compact`
   badge：`Ready`、`Muted`
2. `read only`
   眉标：`ARCHIVE`
   标题：`Read only`
   badge：`Pinned`、`Review`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 242`
- 主控件：`196 x 118`
- 底部 preview 行：`216 x 84`
- 单个 preview：`104 x 84`
- 页面风格：浅色 page panel、低噪音边框、轻量 tone 强化，不回退到 showcase 式说明页

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `TRIAGE` | 是 | 否 | 否 |
| `QUEUE` | 是 | 否 | 否 |
| `RISK` | 是 | 否 | 否 |
| `CALM` | 是 | 否 | 否 |
| 最终稳定帧回到 `TRIAGE` | 是 | 否 | 否 |
| `SET / Compact` | 否 | 是 | 否 |
| `ARCHIVE / Read only` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_badge_group.c` 当前覆盖 `8` 条用例：

1. `set_snapshots clamps and clears pressed state`
   覆盖 `set_snapshots()` 的快照数量 clamp、空快照清理与 `pressed` 复位。
2. `snapshot and setters clear pressed state`
   覆盖 `set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 的 `pressed` 清理与参数落位。
3. `touch same-target release and cancel behavior`
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交、`DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交，以及 `ACTION_CANCEL` 清理。
4. `keyboard click listener`
   覆盖主控件键盘 `Enter` 触发 click。
5. `compact mode clears pressed and keeps click behavior`
   覆盖 `compact` 模式切换先清残留 `pressed`，恢复后仍保留主控件 click 语义。
6. `read only and disabled guards clear pressed state`
   覆盖 `read only` / disabled 在新输入到来时清理高亮、拒绝 `touch / key` 提交，并在恢复后重新允许交互。
7. `static preview consumes input and keeps state`
   通过 `badge_group_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`snapshots`、`font`、`meta_font`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`neutral_color`、`snapshot_count`、`current_snapshot`、`compact_mode`、`read_only_mode`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。
8. `internal helpers cover focus tone text and width`
   覆盖 tone 颜色选择、focus index、防御式文本长度与 pill 宽度估算等内部 helper。

补充说明：

- 主控件继续保留 same-target release 与键盘 click 语义，但不再让底部 preview 参与主区焦点收尾。
- 底部 `compact / read only` preview 统一通过 `egui_view_badge_group_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 当前 README 与单测口径统一使用 `read_only_mode` 命名，不再沿用历史 `locked_mode` 表述。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `TRIAGE` 和底部 preview 固定状态，请求首帧并等待 `BADGE_GROUP_RECORD_FRAME_WAIT = 170`
2. 切到 `QUEUE`，等待 `BADGE_GROUP_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `RISK`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 切到 `CALM`，等待 `90`
7. 请求第四组主区快照并继续等待 `170`
8. 回到默认 `TRIAGE`，等待 `90`
9. 请求最终稳定帧，并继续等待 `BADGE_GROUP_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区 `TRIAGE / QUEUE / RISK / CALM` 四组 reference 快照与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 底部 preview 全程静态，不再轮换 `compact` 快照，也不再承担点击桥接职责。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/badge_group
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge_group
```

## 10. 验收重点

- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `TRIAGE / QUEUE / RISK / CALM` 四组快照必须能从截图中稳定区分，且最终稳定帧显式回到默认 `TRIAGE`。
- `set_snapshots()`、`snapshot / compact / read only / disabled` 切换链路，以及 `touch / key` guard 不能残留 `pressed`。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_badge_group/default`
- 已归档复核结果：
  - 共捕获 `10` 帧
  - 主区 RGB 差分边界：`(46, 87) - (434, 251)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`4`
  - 按 `y >= 304` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `TRIAGE`

## 12. 与现有控件的边界

- 相比 `notification_badge`：这里不是单个计数泡，而是一组可混合 tone 的 badge 集群。
- 相比 `chips`：这里不是交互筛选条，不强调选中、取消和筛选结果。
- 相比 `tag_cloud`：这里不是权重词云，不做自由散点布局。
- 相比 `card_panel`：这里更轻、更扁平，重点在 badge 组合和 focus summary。

## 13. 本轮保留与删减

- 保留的主区状态：`TRIAGE`、`QUEUE`、`RISK`、`CALM`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：focus badge 驱动 footer summary、主控件 same-target release 与键盘 click、`snapshot / compact / read_only / disabled` 切换清理 `pressed`、static preview 输入抑制
- 删减的旧桥接与装饰：`compact` preview 轮换、preview 点击收尾、页面 `guide / status` chrome、旧双列包裹壳与额外录制桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `badge_group` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_badge_group/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `10` 帧 / `4` 组主区状态 / `y >= 304` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge_group`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1845 colors=195`
  - `web/demos/HelloCustomWidgets_display_badge_group` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `10` 帧
  - 主区 RGB 差分边界：`(46, 87) - (434, 251)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`4`
  - 按 `y >= 304` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `TRIAGE / QUEUE / RISK / CALM` 四组 reference 快照，最终稳定帧已回到默认 `TRIAGE`，底部 `compact / read only` preview 全程静态
