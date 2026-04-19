# info_label 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`InfoLabel`
- 当前保留形态：主区 `Project policy`、`Export guidance`、`Reading help` 三组 reference 快照，底部 `compact / read only` 双静态 preview
- 当前保留交互：主区保留程序化 `closed accent / open warning / open neutral` 切换；`text / title / body / font / palette / compact / read_only / open` setter 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主面板包装、底部 preview panel / heading、录制阶段真实 icon click、额外故事化说明与高噪音页面壳层、录制末尾额外桥接帧
- EGUI 适配说明：继续使用 `HelloCustomWidgets` custom 层的轻量 `wrapper`，在 custom view 内收口 `label + info button + anchored bubble`、`compact / read only` 与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`info_label` 用来在正文标签旁边提供一个低打扰的信息入口。它适合出现在设置项、表单字段、只读说明和摘要行附近，让用户按需展开一段短说明，而不是被 `TeachingTip` 或块级反馈容器打断。

## 2. 为什么现有控件不够用

- `tool_tip` 更偏悬停提示，不承担标签旁常驻解释入口的语义。
- `teaching_tip` 层级更高、信息量更大，不适合这里的轻量说明。
- `text_block` 只能静态展示文本，无法表达按需展开的解释气泡。
- `info_badge` 表达的是状态或数量提醒，不表达“标签解释”。

## 3. 当前页面结构

- 标题：`InfoLabel`
- 主区：一个主 `info_label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构：标题 -> 主 `info_label` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/display/info_label/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

1. 默认态
   文案：`Project policy`
   `Versioning`
   `Keep release notes aligned with the approved branch.`
   表现：`closed accent`
2. 快照 2
   文案：`Export guidance`
   `Sensitive content`
   `Mask personal data before sharing outside the tenant.`
   表现：`open warning`
3. 快照 3
   文案：`Reading help`
   `Reference note`
   `Use the compact preview when the layout has limited width.`
   表现：`open neutral`
4. 最终稳定帧
   文案：`Project policy`
   `Versioning`
   `Keep release notes aligned with the approved branch.`
   表现：恢复默认 `closed accent`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact help`
   `Inline note`
   `Compact mode keeps the bubble short.`
2. `read only`
   `Audit note`
   `Read only`
   `Static preview keeps input disabled.`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 194`
- 标题：`224 x 18`
- 主控件：`196 x 96`
- 底部 preview 行：`176 x 54`
- 单个 preview：`84 x 54`
- 页面风格：浅色 page panel、主区保留 `closed / open` 与 palette 语义变化，底部 preview 只做静态 reference，对比集中在 bubble 开合、文案和颜色层级本身

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `Project policy / closed accent` | 是 | 否 | 否 |
| `Export guidance / open warning` | 是 | 否 | 否 |
| `Reading help / open neutral` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `Compact help` | 否 | 是 | 否 |
| `Audit note` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_info_label.c` 当前覆盖 `4` 条用例：

1. `style helpers and setters clear pressed state`
   覆盖 `apply_compact_style()`、`apply_read_only_style()`、`set_text()`、`set_info_title()`、`set_info_body()`、`set_font()`、`set_meta_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 与 `set_open()` 附近的状态清理。
2. `touch same target release and cancel behavior`
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交、回到同一 target 后 `UP(A)` 才提交，以及 `CANCEL` 不触发额外 notify。
3. `keyboard toggle and escape close`
   覆盖 `Enter / Space` 开关与 `Esc` 关闭闭环。
4. `static preview consumes input and keeps state`
   通过 `info_label_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`label`、`info_title`、`info_body`、`font`、`meta_font`、`icon_font`、`on_open_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`bubble_surface_color`、`shadow_color`、`compact_mode`、`read_only_mode`、`open`、`pressed_part`、`icon_region`、`bubble_region`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `info_label` 是 display-first 的标签解释控件，重点在按需展开的说明入口和 bubble 层级，不承担复杂 popup 管理。
- 底部 `compact / read only` preview 统一通过 `hcw_info_label_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Project policy / closed accent` 和底部 preview 固定状态，请求首帧并等待 `INFO_LABEL_RECORD_FRAME_WAIT = 170`
2. 切到 `Export guidance / open warning`，等待 `INFO_LABEL_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Reading help / open neutral`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Project policy / closed accent`，继续等待 `INFO_LABEL_RECORD_FINAL_WAIT = 280`
7. 请求最终稳定帧，并继续等待 `INFO_LABEL_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 录制阶段不再依赖真实 icon click 来驱动主区展开。
- 页面层不再保留旧 preview panel、heading 和额外说明文案。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/info_label PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_label --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_label
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_label
```

## 10. 验收重点

- 主区 `info_label` 与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Project policy`、`Export guidance`、`Reading help` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `apply_compact_style()`、`apply_read_only_style()`、`set_text()`、`set_info_title()`、`set_info_body()`、`set_font()`、`set_meta_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 不能残留 `pressed`。
- 主控件需要保持 same-target release 与 `Enter / Space / Esc` 键盘闭环；底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_info_label/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(44, 121) - (436, 265)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Project policy / closed accent`

## 12. 与现有控件的边界

- 相比 `tool_tip`：这里强调标签旁解释入口，而不是悬停提示。
- 相比 `teaching_tip`：这里不承担教学卡片或高层级引导。
- 相比 `text_block`：这里保留按需展开的轻交互语义。
- 相比 `info_badge`：这里表达的是解释信息，而不是状态角标。

## 13. 本轮保留与删减

- 保留的主区状态：`Project policy / closed accent`、`Export guidance / open warning`、`Reading help / open neutral`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`label + info button + anchored bubble`、`compact / read only`、same-target release、`Enter / Space / Esc` 键盘闭环、static preview 输入抑制
- 删减的旧桥接与装饰：主面板包装、底部 preview panel / heading、录制阶段真实 icon click、额外故事化说明与高噪音页面壳层、录制末尾额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/info_label PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `info_label` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_label --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_info_label/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 265` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_label`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1479 colors=136`
  - `web/demos/HelloCustomWidgets_display_info_label` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(44, 121) - (436, 265)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Project policy`、`Export guidance`、`Reading help` 三组 reference 快照，最终稳定帧已回到默认态，底部 `compact / read only` preview 全程静态
