# TextBlock 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI / WinUI`
- 对应组件：`TextBlock`
- 当前保留语义：单段多行文本、`standard / subtle / accent` 三套样式 helper、`compact / read_only` 静态 preview、静态 preview 输入不变性
- 当前移除内容：旧 `primary_panel`、`heading / summary / note`、底部 `compact_panel / read_only_panel` 包装与说明文案、录制末尾回切默认态恢复帧、滚动阅读区、编辑模式、文本选择、超链接、边框壳与 SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_text_block`，底层继续复用 SDK `textblock` 的自动换行与测量能力，但默认关闭滚动、编辑和边框，只保留 display-only `TextBlock` 语义

## 1. 为什么需要这个控件
`TextBlock` 是最基础的展示型文本块，用来承载说明语句、摘要、辅助提示和弱化说明。它的价值不在富文本，而在于把“单段可换行文字”收口成稳定、轻量、可复用的 Fluent reference 组件。

## 2. 为什么现有控件不够用
- `label` 更适合短文本，遇到多行正文时缺少统一换行和样式 helper。
- SDK `textblock` 更偏基础能力验证，默认还带滚动、边框和编辑扩展，不等于 reference 语义里的 display-only `TextBlock`。
- `rich_text_block` 已经承接多段层级文本，这里需要的是更轻的“单段文本”边界，而不是再次上升为段落组控件。

## 3. 当前页面结构
- 标题：`TextBlock`
- 主区：一个主 `text_block` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/text_block/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照：

1. 默认态
   文案：`A TextBlock keeps one paragraph readable / without acting like an editor.`
   样式：`standard`
   状态：`Standard / body`
   颜色：`#51606F`
2. 快照 2
   文案：`Subtle copy can wrap across two lines / while staying quieter than body text.`
   样式：`subtle`
   状态：`Subtle / note`
   颜色：`#6B7A89`
3. 快照 3
   文案：`Accent text can call out one action / without turning the block into a card.`
   样式：`accent`
   状态：`Accent / emphasis`
   颜色：`#0F6CBD`

底部 preview 在整条轨道中始终固定：

1. `compact`
   文案：`Compact copy / for tight rows.`
   `compact_mode=1`
   `subtle` 样式
   `max_lines=2`
2. `read_only`
   文案：`Review copy / before publish.`
   `compact_mode=1`
   `read_only_mode=1`
   `standard` 样式
   `max_lines=2`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主 `text_block`：`176 x 34`
- 主状态 label：`224 x 12`
- 底部 preview 行：`184 x 34`
- 单个 preview：`88 x 34`
- 页面结构：标题 -> 主 `text_block` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、低噪音文字层级、强调态只通过文字色而不是额外容器表达，底部 preview 不再承担说明文案

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 单段自动换行 | 是 | 是 | 是 |
| `standard` 样式 | 是 | API 保留 | 是 |
| `subtle` 样式 | 是 | 是 | API 保留 |
| `accent` 样式 | 是 | API 保留 | API 保留 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 接收焦点 / 交互 | 否 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Subtle / note`
4. 抓取第二组主区快照
5. 切到 `Accent / emphasis`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切默认态后额外补一帧。
- 页面层不再保留旧 `primary_panel`、`heading / summary / note`、底部 preview 包装容器与说明文案。
- 底部 preview 统一通过 `egui_view_text_block_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_text_block.c` 当前覆盖四部分：

1. 主控件初始化默认值
   覆盖默认字体、空文本、自动换行、`line_space`、`compact_mode`、`read_only_mode` 和 display-only 默认项。
2. 样式 helper、palette 与模式切换
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()`、`set_palette()`、`set_text()`、`set_font()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed` 状态的清理和非法值钳制。
3. display-only 默认项保持
   覆盖 `set_font()`、`set_text()`、`set_max_lines()` 与样式切换后仍保持无滚动、无边框的 display-only 行为。
4. 静态 preview 不变性断言
   通过 `text_block_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / on_click_listener / api / text / font / standard_color / subtle_color / accent_color / resolved_text_color / configured_text_alpha / resolved_text_alpha / line_space / max_lines / content_line_count / layout_width / align_type / is_auto_wrap_enabled / is_scroll_enabled / is_border_enabled / style / compact_mode / read_only_mode / enable / is_focused / is_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/text_block PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/text_block --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/text_block
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_text_block
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/text_block PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `text_block` suite `4 / 4`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`，本轮无额外变更
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/text_block --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/text_block`，输出 `web/demos/HelloCustomWidgets_display_text_block`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_text_block`，结果 `PASS status=Running canvas=480x480 ratio=0.1159 colors=83`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_text_block/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(64, 158) - (298, 234)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 235` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前只覆盖单段 `TextBlock` 显示语义，不实现多段层级；多段场景由 `rich_text_block` 承接。
- 当前不支持滚动、编辑、文本选择和超链接点击。
- 当前 wrapper 只收口 Fluent reference 方向的 display-only 样式，不扩展为通用阅读器控件。
- 底部 `compact / read_only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `label`：这里承载的是稳定的单段换行文本，而不是短标题或短标签。
- 相比 SDK `textblock`：这里收口的是 display-only `TextBlock` 语义，不保留滚动、编辑和边框扩展。
- 相比 `rich_text_block`：这里不承担多段层级文本，只聚焦单段正文。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_text_block` custom view，不修改 SDK。
- 主区保留 `Standard / body`、`Subtle / note`、`Accent / emphasis` 三组 reference 快照。
- 底部 preview 通过 `egui_view_text_block_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制不再保留旧 panel 级说明 chrome。
