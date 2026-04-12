# TextBlock 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI / WinUI`
- 对应组件名：`TextBlock`
- 本次保留语义：单段多行文本、`standard / subtle / accent` 三套样式 helper、`compact / read_only` 与静态 preview 输入抑制
- 删减内容：滚动阅读区、编辑模式、文本选择、超链接、边框壳与 SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_text_block`，底层继续复用 SDK `textblock` 的自动换行与测量能力，但默认关闭滚动、编辑和边框，只保留 display-only `TextBlock` 语义

## 1. 为什么需要这个控件？
`TextBlock` 是最基础的展示型文本块，用来承载说明语句、摘要、辅助提示和弱化说明。它的价值不在富文本，而在于把“单段可换行文字”收口成稳定、轻量、可复用的 Fluent reference 组件。

## 2. 为什么现有控件不够用？
- `label` 更适合短文本，遇到多行正文时缺少统一换行和样式 helper。
- SDK `textblock` 更偏基础能力验证，默认还带滚动、边框和编辑扩展，不等于 reference 语义里的 display-only `TextBlock`。
- `rich_text_block` 已经承接多段层级文本，这里需要的是更轻的“单段文本”边界，而不是再次上升为段落组控件。

## 3. 目标场景与示例概览
- 主面板轮播三种参考态：
  - `standard body`
  - `subtle note`
  - `accent line`
- 底部保留两个静态 preview：
  - `compact`
  - `read_only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 246`
- 主面板：`196 x 118`
- 主控件：`176 x 56`
- 底部容器：`216 x 82`
- 单个 preview：`104 x 82`
- preview 控件：`84 x 34`

视觉原则：
- 页面保持浅灰背景、白色表面和低噪音圆角容器。
- `TextBlock` 自身不承担卡片和分组语义，重点是文字层级。
- `accent` 只是文字色强调，不升级成富文本块或 callout 卡片。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 246` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 118` | 主展示面板 |
| `primary_widget` | `egui_view_text_block_t` | `176 x 56` | 主文本块 |
| `compact_widget` | `egui_view_text_block_t` | `84 x 34` | 紧凑静态 preview |
| `read_only_widget` | `egui_view_text_block_t` | `84 x 34` | 只读弱化静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 单段自动换行 | 是 | 是 | 是 |
| `standard` 样式 | 是 | 可选 | 是 |
| `subtle` 样式 | 是 | 是 | 可选 |
| `accent` 样式 | 是 | 可选 | 可选 |
| `compact_mode` | 可选 | 开启 | 开启 |
| `read_only_mode` | 可选 | 关闭 | 开启 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `TextBlock` 本轮定义为 display-only 控件，不承接滚动、编辑、选择和超链接点击。
- 底层继续使用 SDK `textblock` 的换行能力，但 wrapper 默认关闭滚动与边框，避免偏离 reference 语义。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，不让底部对照块接管输入。

## 8. 本轮收口内容
- 新增 `egui_view_text_block.h/.c`
- 提供：
  - `apply_standard_style()`
  - `apply_subtle_style()`
  - `apply_accent_style()`
  - `set_text()`
  - `set_font()`
  - `set_palette()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
  - `override_static_preview_api()`
- demo 页面接入三组主状态与两组静态 preview
- 单测覆盖默认值、样式 helper、palette、模式切换与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Standard body`
2. 抓取初始帧
3. 切到 `Subtle note`
4. 抓取第二帧
5. 切到 `Accent line`
6. 抓取第三帧
7. 回到初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/text_block PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

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

验收重点：
- 主控件和两个 preview 必须完整可见，不黑屏、不白屏、不裁切。
- `standard / subtle / accent` 三种文字层级必须在截图里明确区分。
- `read_only` preview 需要明显弱化，但不能失去可读性。

## 11. 已知限制
- 当前只覆盖单段 `TextBlock` 语义，不实现多段层级；多段场景由 `rich_text_block` 承接。
- 当前不支持滚动、编辑、文本选择和超链接点击。
- 当前 wrapper 只收口 Fluent reference 方向的 display-only 样式，不扩展为通用阅读器控件。
