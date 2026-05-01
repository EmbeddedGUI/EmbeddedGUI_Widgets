# persona 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI / Fluent UI React`
- 对应组件：`Persona`
- 当前保留语义：`avatar + display_name + secondary/tertiary/quaternary text + status + tone + palette + static preview 输入抑制`
- 当前移除内容：旧主 panel / heading / note、底部 preview panel / heading / body、场景化叙事文案、录制阶段额外恢复帧、SDK 改动
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_persona`，在控件内部完成头像 initials 回退、状态点绘制、默认布局与静态 preview API，不修改 `sdk/EmbeddedGUI`。尺寸、字体、palette 与 enable 状态由 APP 按需配置。

## 1. 为什么需要这个控件
`persona` 用来表达“这是一个带身份信息的单人条目”，它不仅要显示头像，还要表达姓名、辅助文本层级、在线状态与视觉 tone。相比只显示头像的 `person_picture`，这里强调的是完整的单人身份条目语义。

## 2. 为什么现有控件不够用
- `person_picture` 只覆盖头像和 presence 点，不承接多行身份文本。
- `persona_group` 面向群组聚合和重叠头像，不适合作为单人信息条目。
- `text_block`、`rich_text_block` 只能显示文本，缺少头像、tone 和状态点语义。

## 3. 当前页面结构
- 标题：`Persona`
- 主区：一个主 `persona` 和一个当前状态 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`secondary`
- 右侧 preview：`muted`

目录：
- `example/HelloCustomWidgets/display/persona/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   name：`Lena Marsh`
   secondary：`Principal designer`
   tertiary：`Focuses on spacing audit`
   quaternary：`Available for review`
   label：`Lena Marsh / available`
2. 快照 2
   name：`Aria Rowan`
   initials：`AR`
   secondary：`Release manager`
   label：`Aria Rowan / busy`
3. 快照 3
   name：`Mina Brooks`
   initials：`MB`
   secondary：`Archive owner`
   label：`Mina Brooks / offline`

底部 preview 在整条轨道中始终固定：

1. `secondary`
   name：`Maya Yu`
   secondary：`Research lead`
   status：`away`
2. `muted`
   name：`Jin Park`
   secondary：`Compliance archive`
   tertiary：`Muted audit trail`
   initials：`JP`
   status：`do not disturb`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 238`
- 标题：`224 x 18`
- 主 `persona`：`196 x 104`
- 主状态 label：`224 x 12`
- 底部 preview 行：`216 x 78`
- 单个 preview：`104 x 78`
- 页面结构：标题 -> 主 `persona` -> 状态 label -> 底部 `secondary / muted`
- 风格约束：浅色 page panel、左侧头像 section 只保留轻量承载与 tone indicator，主区只展示 reference 身份层级变化，底部 preview 只做静态对照

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Secondary preview | Muted preview |
| --- | --- | --- | --- |
| 四行文本层级 | 是 | 由可用高度决定 | 由可用高度决定 |
| 两行文本层级 | 是 | 是 | 是 |
| 显式 `initials` | 是 | 否 | 是 |
| `tone` | 是 | 是 | 是 |
| `status` | 是 | 是 | 是 |
| 小尺寸 / 小字体 | APP 配置 | 是 | 是 |
| 弱化视觉 / 禁用输入 | APP 配置 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Aria Rowan / busy`
4. 抓取第二组主区快照
5. 切到 `Mina Brooks / offline`
6. 抓取第三组主区快照
7. 回到默认 `Lena Marsh / available`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧主 panel、heading、note 与底部 preview 包装文案。
- 底部 preview 统一通过 `egui_view_persona_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `PERSONA_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_persona.c` 当前覆盖四部分：

1. 主控件初始化与默认语义
   覆盖默认 `display_name / secondary / tertiary / quaternary / initials`、默认 `tone / status`、默认字体与默认 icon font 分档规则
2. setter 与 initials 回退守卫
   覆盖 `set_display_name()`、`set_secondary_text()`、`set_tertiary_text()`、`set_quaternary_text()`、`set_initials()`、`set_status()`、`set_tone()`、`set_font()`、`set_meta_font()`、`set_palette()` 对 `pressed` 状态的清理和非法值回退
3. helper 与 region/color 计算
   覆盖 `resolve_initials()`、`get_avatar_region()`、`get_presence_region()`、`tone_color()`、`status_color()` 与 disabled 混色 helper
4. 静态 preview 不变性断言
   通过 `persona_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`display_name`、`secondary_text`、`tertiary_text`、`quaternary_text`、`initials`、`font`、`meta_font`、`surface_color`、`border_color`、`section_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`neutral_color`、`on_click_listener`、`api`、`alpha`、`tone`、`status`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/persona
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_persona
```

## 10. 当前验收结果（2026-05-01）
- `HelloCustomWidgets` 单控件 PC 编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/persona PORT=pc COMPILE_DEBUG= COMPILE_OPT_LEVEL=-O0`
- `HelloUnitTest`：`PASS`，`make all APP=HelloUnitTest PORT=pc_test COMPILE_DEBUG= COMPILE_OPT_LEVEL=-O0` 与 `output\main.exe`，总计 `1049 / 1049`
- 单控件 runtime：`PASS`，`8 frames captured -> runtime_check_output/HelloCustomWidgets_display_persona/default`
- `touch release semantics`：`PASS`，`custom_audited=32 custom_skipped_allowlist=0`
- wasm 编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/persona PORT=emscripten COMPILE_DEBUG= COMPILE_OPT_LEVEL=-O0`
- `docs encoding`：`PASS`，`172` 个文档文件编码检查通过
- `git diff --check`：`PASS`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_persona/default`

复核结果：
- 总帧数：`8`
- 主区保持三组 reference 快照：`Lena Marsh / available`、`Aria Rowan / busy`、`Mina Brooks / offline`
- 底部 preview 由页面侧尺寸、字体、palette 与 enable 状态配置，不依赖控件内部模式 API

目标：
- 主区三组 reference 快照可见
- 底部 preview 全程静态
- 未出现黑屏、白屏或主体缺失

结论：
- 主区变化严格收敛在 `persona` 主体，页面 chrome 在整条轨道中保持静态。
- `secondary / muted` preview 在录制轨道中始终静态一致，且状态差异全部来自 APP 侧配置。

## 12. 已知限制
- 当前 demo 不接入真实头像图资源，重点放在 initials 回退、文本层级与状态点语义。
- initials 解析当前按 ASCII 规则处理，优先覆盖 reference demo 与现有英文命名场景。
- 主区状态 label 属于页面级辅助说明，不属于 `persona` 控件本体 API。
- 底部 `secondary / muted` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `person_picture`：这里承载完整单人条目和多行身份文本，而不是只画头像。
- 相比 `persona_group`：这里聚焦单个 persona，不处理多人聚合、重叠头像和 overflow。
- 相比 `text_block`、`rich_text_block`：这里不仅展示文本，还要表达头像、tone 和 presence 语义。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_persona` custom view，不修改 SDK。
- 主区保留 `Lena Marsh / available`、`Aria Rowan / busy`、`Mina Brooks / offline` 三组 reference 快照。
- 底部 preview 通过 `egui_view_persona_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展头像资源或宿主容器示例。
