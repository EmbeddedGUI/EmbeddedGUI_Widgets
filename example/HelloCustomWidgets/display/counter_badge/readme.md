# CounterBadge 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`CounterBadge`
- 当前保留形态：主区 `Inbox queue / 7`、`Escalation queue / 99+`、`Quiet watch / dot` 三组 reference 快照，底部 `compact / read only` 双静态 preview
- 当前保留交互：主区保留程序化 `count / overflow / dot` 切换；`count / max_display / dot / compact / read_only / palette` setter 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `panel / heading / summary / note`、底部 preview 包装和说明文案、录制末尾回切默认态的额外桥接帧
- EGUI 适配说明：复用 SDK `notification_badge` 的计数格式化与基础背景绘制，在 custom 层补齐 `CounterBadge` 的 palette、`compact / read only / dot` 语义和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`counter_badge` 用来表达“某个入口上还有多少条待处理消息或提醒”。它强调的是极小面积里的数字提醒，不负责承载完整状态卡片、宿主头像或操作入口，只负责把数量语义稳定地挂在列表、卡片角落或导航入口旁边。

## 2. 为什么现有控件不够用

- `badge` 更偏短文本状态标签，不是附着式数字提醒。
- `info_badge` 更强调 `count / icon / attention dot` 的信息提示，不直接对齐 `CounterBadge` 的轻量宿主角标语义。
- `presence_badge` 表达的是在线状态，不承载数量，也没有 `99+` 这类 overflow 计数语义。

## 3. 当前页面结构

- 标题：`CounterBadge`
- 主区：一个主 `counter_badge` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构：标题 -> 主 `counter_badge` -> 状态 label -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/display/counter_badge/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

1. 默认态
   文案：`Inbox queue / 7`
   表现：单数字 `7`
2. 快照 2
   文案：`Escalation queue / 99+`
   表现：`128` 在 `max_display=99` 下显示为 `99+`
3. 快照 3
   文案：`Quiet watch / dot`
   表现：dot mode
4. 最终稳定帧
   文案：`Inbox queue / 7`
   表现：恢复默认计数状态

底部 preview 在整条轨道中始终固定：

1. `compact`
   `count=4`
   `compact_mode=1`
2. `read only`
   `count=12`
   `compact_mode=1`
   `read_only_mode=1`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主 `counter_badge`：`48 x 24`
- 主状态 label：`224 x 12`
- 底部 preview 行：`48 x 16`
- 左侧 preview：`18 x 16`
- 右侧 preview：`22 x 16`
- 页面风格：浅色 page panel、低噪音红蓝提醒、白色 outline、`99+` 不裁切、dot mode 保持最小提醒信号

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `Inbox queue / 7` | 是 | 否 | 否 |
| `Escalation queue / 99+` | 是 | 否 | 否 |
| `Quiet watch / dot` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `count=4` + compact | 否 | 是 | 否 |
| `count=12` + read only | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_counter_badge.c` 当前覆盖 `4` 条用例：

1. `init uses defaults`
   覆盖默认 `count`、`max_display`、`dot_mode`、`compact_mode`、`read_only_mode` 和默认 palette。
2. `setters clear pressed state and update palette`
   覆盖 `set_count()`、`set_max_display()`、`set_dot_mode()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed` 状态的清理和非法值回退。
3. `helpers compute regions and modes`
   覆盖 `get_badge_region()`、overflow / dot 尺寸变化和 `egui_view_counter_badge_mix_disabled()`。
4. `static preview consumes input and keeps state`
   通过 `counter_badge_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`font`、`content_style`、`icon`、`icon_font`、`badge_color`、`text_color`、`outline_color`、`on_click_listener`、`api`、`alpha`、`count`、`max_display`、`compact_mode`、`read_only_mode`、`dot_mode`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `counter_badge` 是 display-first 的只读数量提醒控件，重点在 `count / overflow / dot` 语义，不承担 click 提交职责。
- 底部 `compact / read only` preview 统一通过 `egui_view_counter_badge_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Inbox queue / 7` 和底部 preview 固定状态，请求首帧并等待 `COUNTER_BADGE_RECORD_FRAME_WAIT = 170`
2. 切到 `Escalation queue / 99+`，等待 `COUNTER_BADGE_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Quiet watch / dot`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Inbox queue / 7`，等待 `90`
7. 请求最终稳定帧，并继续等待 `COUNTER_BADGE_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧 `primary_panel`、`heading / summary / note` 与底部 preview 包装文案。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/counter_badge PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/counter_badge --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/counter_badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_counter_badge
```

## 10. 验收重点

- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Inbox queue / 7`、`Escalation queue / 99+`、`Quiet watch / dot` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `set_count()`、`set_max_display()`、`set_dot_mode()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 不能残留 `pressed`。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_counter_badge/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(179, 161) - (301, 219)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 220` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Inbox queue / 7`

## 12. 与现有控件的边界

- 相比 `badge`：这里表达的是附着式数字提醒，不是短文本状态标签。
- 相比 `info_badge`：这里聚焦 `CounterBadge` 的数字和 overflow 语义，不扩展 icon 或 attention dot 组合展示。
- 相比 `presence_badge`：这里表达数量提醒，不表达在线状态。

## 13. 本轮保留与删减

- 保留的主区状态：`Inbox queue / 7`、`Escalation queue / 99+`、`Quiet watch / dot`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`count / overflow / dot` 语义、默认 palette、`compact / read_only` 预览、`count / max_display / dot / compact / read_only / palette` setter 清理 `pressed`、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `panel / heading / summary / note`、底部 preview 包装和说明文案、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/counter_badge PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `counter_badge` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/counter_badge --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_counter_badge/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 220` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_counter_badge`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1159 colors=114`
  - `web/demos/HelloCustomWidgets_display_counter_badge` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(179, 161) - (301, 219)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 220` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Inbox queue / 7`、`Escalation queue / 99+`、`Quiet watch / dot` 三组 reference 快照，最终稳定帧已回到默认态，底部 `compact / read only` preview 全程静态
