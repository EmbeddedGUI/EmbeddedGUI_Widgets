# CounterBadge 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`CounterBadge`
- 当前保留语义：`count / overflow / dot`、`compact / read_only` 静态 preview、静态 preview 输入抑制
- 当前移除内容：旧主 `panel / heading / summary / note`、底部 preview 包装和说明文案
- EGUI 适配说明：复用 SDK `notification_badge` 的计数格式化与基础背景绘制，在 custom 层补齐 `CounterBadge` 的 palette、`compact / read_only / dot` 语义和静态 preview API，不修改 `sdk/EmbeddedGUI`

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
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/counter_badge/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照和最终稳定帧：

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
2. `read_only`
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
- 页面结构：标题 -> 主 `counter_badge` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、低噪音红蓝提醒、白色 outline、`99+` 不裁切、dot mode 保持最小提醒信号

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 默认显示 | `Inbox queue / 7` | `count=4` | `count=12` |
| 快照 2 | `Escalation queue / 99+` | 保持不变 | 保持不变 |
| 快照 3 | `Quiet watch / dot` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Inbox queue / 7` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Escalation queue / 99+`
4. 抓取第二组主区快照
5. 切到 `Quiet watch / dot`
6. 抓取第三组主区快照
7. 回到默认 `Inbox queue / 7`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧 `primary_panel`、`heading / summary / note` 与底部 preview 包装文案。
- 底部 preview 统一通过 `egui_view_counter_badge_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `COUNTER_BADGE_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_counter_badge.c` 当前覆盖四部分：

1. 主控件初始化默认值
   覆盖默认 `count`、`max_display`、`dot_mode`、`compact_mode`、`read_only_mode` 和默认 palette。
2. setter 与 palette 守卫
   覆盖 `set_count()`、`set_max_display()`、`set_dot_mode()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed` 状态的清理和非法值回退。
3. helper 几何与禁用态混色
   覆盖 `get_badge_region()`、overflow/dot 尺寸变化和 `egui_view_counter_badge_mix_disabled()`。
4. 静态 preview 不变性断言
   通过 `counter_badge_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / font / content_style / icon / icon_font / badge_color / text_color / outline_color / on_click_listener / api / alpha / count / max_display / compact_mode / read_only_mode / dot_mode / enable / is_focused / is_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/counter_badge PORT=pc

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

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/counter_badge PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `counter_badge` suite `4 / 4`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_counter_badge/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_counter_badge`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1159 colors=114`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_counter_badge/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(179, 161) - (301, 219)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 220` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `counter_badge` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应默认、`Escalation queue / 99+` 与 `Quiet watch / dot` 三组主区快照；最终稳定帧已显式回到默认 `Inbox queue / 7`。
- 按 `y >= 220` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read_only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖独立 `CounterBadge`，不负责与头像、卡片或命令入口做宿主绑定。
- dot mode 先收口为纯色实心点，不继续扩展动画或宿主吸附逻辑。
- 底部 `compact / read_only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `badge`：这里表达的是附着式数字提醒，不是短文本状态标签。
- 相比 `info_badge`：这里聚焦 `CounterBadge` 的数字和 overflow 语义，不扩展 icon 或 attention dot 组合展示。
- 相比 `presence_badge`：这里表达数量提醒，不表达在线状态。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_counter_badge` custom view，不修改 SDK。
- 主区保留 `Inbox queue / 7`、`Escalation queue / 99+`、`Quiet watch / dot` 三组 reference 快照。
- 底部 preview 通过 `egui_view_counter_badge_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制不再保留旧 panel 级说明 chrome。
