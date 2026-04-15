# RichTextBlock 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`RichTextBlock`
- 当前保留语义：段落级显示块、`BODY / EMPHASIS / ACCENT / CAPTION` 四类段落样式、`compact / read_only` 静态 preview、静态 preview 输入抑制
- 当前移除内容：旧主 `panel / heading / summary / note`、底部 preview 标题和包装文案、录制末尾回切默认态恢复帧、完整 inline 对象树、超链接交互、选择复制、编辑能力
- EGUI 适配说明：在 custom 层提供轻量 `egui_view_rich_text_block`，复用 SDK `textblock` 的字体测量能力，在不修改 `sdk/EmbeddedGUI` 的前提下收口 `RichTextBlock` 的段落样式、palette 与静态 preview API

## 1. 为什么需要这个控件
`rich_text_block` 的价值不在于“再提供一个多行文本控件”，而在于把属于同一语义块的多段文本收口成统一显示对象。它适合承载发布说明、策略提示、编辑摘要和补充 caption，避免页面为了几段不同层级的文案拆成多个零散 `label` / `text_block`。

## 2. 为什么现有控件不够用
- `text_block` 更适合单段正文换行，不负责段落之间的样式层级。
- `label` 适合短文本，不适合承接正文、强调段和 caption 的组合。
- 页面层手工拼多个文本控件会把 `EMPHASIS / ACCENT / CAPTION` 规则分散到页面代码里，缺少统一 API。

## 3. 当前页面结构
- 标题：`RichTextBlock`
- 主区：一个主 `rich_text_block` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/rich_text_block/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照：

1. 默认态
   文案：`Release note / emphasis + body + caption`
   结构：`EMPHASIS + BODY + CAPTION`
2. 快照 2
   文案：`Policy callout / body + accent + caption`
   结构：`BODY + ACCENT + CAPTION`
3. 快照 3
   文案：`Editorial brief / emphasis + body + caption`
   结构：`EMPHASIS + BODY + CAPTION`

底部 preview 在整条轨道中始终固定：

1. `compact`
   段落：`BODY + CAPTION`
   `compact_mode=1`
2. `read_only`
   段落：`BODY + CAPTION`
   `compact_mode=1`
   `read_only_mode=1`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 204`
- 标题：`224 x 18`
- 主 `rich_text_block`：`176 x 96`
- 主状态 label：`224 x 12`
- 底部 preview 行：`176 x 42`
- 单个 preview：`84 x 42`
- 页面结构：标题 -> 主 `rich_text_block` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、低噪音段落层级、`ACCENT` 段落只保留轻量 callout、`read_only` 通过更弱的文字对比度表达静态对照

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `BODY` 段落 | 是 | 是 | 是 |
| `EMPHASIS` 段落 | 是 | API 保留 | API 保留 |
| `ACCENT` 段落 | 是 | API 保留 | API 保留 |
| `CAPTION` 段落 | 是 | 是 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 接收焦点 / 交互 | 否 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Policy callout / body + accent + caption`
4. 抓取第二组主区快照
5. 切到 `Editorial brief / emphasis + body + caption`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切 `Release note / emphasis + body + caption` 后额外补一帧。
- 页面层不再保留旧 `primary_panel`、`heading / summary / note`、底部 preview 标题与说明文案。
- 底部 preview 统一通过 `egui_view_rich_text_block_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_rich_text_block.c` 当前覆盖四部分：

1. 主控件初始化默认值
   覆盖默认 `paragraph_count`、字体、`compact_mode`、`read_only_mode`、`paragraph_gap` 和 `line_space`。
2. setter 与 palette 守卫
   覆盖 `set_paragraphs()`、`set_font()`、`set_emphasis_font()`、`set_caption_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()` 对 `pressed` 状态的清理、非法值钳制和 helper 返回值。
3. helper 几何与段落测量
   覆盖 `prepare_layout()`、`measure_content_height()`、强调字体解析、`ACCENT` callout 盒子区域与颜色解析。
4. 静态 preview 不变性断言
   通过 `rich_text_block_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / paragraphs / font / emphasis_font / caption_font / surface_color / border_color / text_color / muted_text_color / accent_color / on_click_listener / api / alpha / line_space / paragraph_gap / paragraph_count / compact_mode / read_only_mode / enable / is_focused / is_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/rich_text_block PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/rich_text_block --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/rich_text_block
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_rich_text_block
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/rich_text_block PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `rich_text_block` suite `4 / 4`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`，本轮无额外变更
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/rich_text_block --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/rich_text_block`，输出 `web/demos/HelloCustomWidgets_display_rich_text_block`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_rich_text_block`，结果 `PASS status=Running canvas=480x480 ratio=0.1597 colors=87`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_rich_text_block/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(64, 116) - (416, 286)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 286` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前只覆盖段落级 `RichTextBlock` 显示语义，不实现完整 inline 树、超链接点击、选择复制和编辑能力。
- `ACCENT` 先收口为轻量 callout 盒子，不继续扩展完整 WinUI 富文本格式系统。
- 当前控件是 display-only，不承担滚动容器职责；需要滚动时应由外层容器承接。
- 底部 `compact / read_only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `text_block`：这里表达的是多段落层级文本块，而不是单段正文换行。
- 相比 `label`：这里承载的是正文、强调段和 caption 的组合，而不是短文本标题。
- 相比 `info_label`：这里收口的是段落排版语义，不承担状态标题加说明标签的固定模板。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_rich_text_block` custom view，不修改 SDK。
- 主区保留 `Release note / emphasis + body + caption`、`Policy callout / body + accent + caption`、`Editorial brief / emphasis + body + caption` 三组 reference 快照。
- 底部 preview 通过 `egui_view_rich_text_block_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制不再保留旧 panel 级说明 chrome。
