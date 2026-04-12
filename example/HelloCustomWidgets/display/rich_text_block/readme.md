# RichTextBlock 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件名：`RichTextBlock`
- 本次保留语义：段落级文本块、lead / emphasis / accent / caption 四类段落样式、静态 preview 输入抑制
- 删减内容：完整 inline 对象树、超链接交互、选择复制、编辑能力、SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_rich_text_block`，以“段落数组 + 样式枚举”收口 `RichTextBlock` 的展示语义；底层仍复用 SDK `textblock` 的测量能力，不改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`RichTextBlock` 的价值不是“再来一个多行文本控件”，而是把一组属于同一语义块的段落文本组织成统一展示对象。它适合承载发行说明、状态摘要、说明性段落和弱化的 caption，让页面不必为了几段不同层级的文字再拆多个零散 `label` / `textblock`。

## 2. 为什么现有控件不够用？
- `textblock` 解决的是单段文本的换行、滚动和边框，不负责段落之间的样式层级。
- `label` 适合短文本，不适合承接多段正文与 caption 的组合。
- 在页面层手工拼多个 `textblock` 会让 lead / accent / caption 的规则分散到页面代码里，缺少统一 API。

## 3. 目标场景与示例概览
- 主面板轮播三种参考态：
  - `lead + body + caption`
  - `body + accent callout + caption`
  - `emphasis + body + caption`
- 底部保留两个静态 preview：
  - `compact`
  - `read_only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 272`
- 主面板：`196 x 142`
- 主控件：`176 x 86`
- 底部容器：`216 x 92`
- 单个 preview：`104 x 92`
- preview 控件：`84 x 42`

视觉原则：
- 保持 Fluent 主线中的浅灰页面背景、白色承载面和低噪音边界。
- `accent` 段落在块内用轻量强调底和竖条表达，不升级成独立卡片。
- `caption` 段落只承担补充信息，不与正文抢层级。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 272` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 142` | 主展示面板 |
| `primary_widget` | `egui_view_rich_text_block_t` | `176 x 86` | 主富文本块 |
| `compact_widget` | `egui_view_rich_text_block_t` | `84 x 42` | 紧凑静态 preview |
| `read_only_widget` | `egui_view_rich_text_block_t` | `84 x 42` | 只读弱化静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `BODY` 段落 | 是 | 是 | 是 |
| `EMPHASIS` 段落 | 是 | 可选 | 可选 |
| `ACCENT` 段落 | 是 | 可选 | 是 |
| `CAPTION` 段落 | 是 | 是 | 是 |
| `compact_mode` | 可选 | 开启 | 开启 |
| `read_only_mode` | 可选 | 关闭 | 开启 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `RichTextBlock` 本轮定义为显示型控件，不承接编辑、选择、超链接点击或焦点流转。
- 输入语义只保留在静态 preview 收口：`override_static_preview_api()` 会吞掉 `touch / key`，避免 reference 页面中的预览块接管输入。
- 正文、强调、accent、caption 的差异只体现在字体层级、颜色与轻量 callout 表达，不引入额外页面状态机。

## 8. 本轮收口内容
- 新增 `egui_view_rich_text_block.h/.c`
- 提供：
  - `set_paragraphs()`
  - `set_font()`
  - `set_emphasis_font()`
  - `set_caption_font()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
  - `set_palette()`
  - `override_static_preview_api()`
- demo 页面接入三组主状态与两组静态 preview
- 单测覆盖默认值、段落数量 clamp、样式解析、布局测量与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Lead paragraph`
2. 抓取初始帧
3. 切到 `Accent callout`
4. 抓取第二帧
5. 切到 `Compact editorial`
6. 抓取第三帧
7. 回到初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/rich_text_block PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

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

验收重点：
- 主控件与两个 preview 必须完整可见，不黑屏、不白屏、不裁切。
- lead / body / accent / caption 的层级必须能在截图中明确分辨。
- `read_only` preview 需要明显弱化，但仍保留段落块语义。

## 11. 已知限制
- 当前不实现完整 inline 树和超链接点击，只保留段落级富文本参考语义。
- 当前 `accent` 强调以轻量 callout 盒子表达，不追求完整 WinUI 文本格式功能。
- 当前控件是 display-only，不承担滚动容器职责；需要滚动时应由更外层容器承接。
