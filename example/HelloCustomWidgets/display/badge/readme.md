# Badge 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Badge`
- 本次保留语义：单个文本 badge 的 `filled / outline / subtle` 外观、可选 leading icon、`compact / read_only` 静态 preview
- 本次删除内容：成组 badge 布局、dismiss、容器级轮换、故事化装饰和额外交互
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_badge`，只承担单个 badge 的外观和文本语义，不修改 SDK

## 1. 为什么需要这个控件？
`Badge` 用来表达“系统生成的、附着在内容上的短状态或描述”，例如审核状态、发布标记、预览标签和归档标识。当前仓库虽然已经有 `info_badge`、`badge_group` 和 `tag`，但还缺少一个对齐 Fluent 2 `Badge` 语义的单个文本 badge 控件，因此需要补齐。

## 2. 为什么现有控件不够用？
- `info_badge` 更偏计数、图标和 attention dot，不承载短文本状态。
- `badge_group` 更偏多 badge 汇总和摘要，不是单个附着式状态标签。
- `tag` 表达的是用户已选、可撤销的值，不适合承载系统生成的不可编辑状态。

## 3. 目标场景与示例概览
- 主面板：一个单个 `badge`，在录制轨道中轮播 `filled / outline / subtle` 三个主线外观。
- 底部保留两个静态 preview：
  - `compact`
  - `read_only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 190`
- 主面板：`196 x 84`
- 主控件：`126 x 28`
- 底部容器：`216 x 62`
- 单个 preview 面板：`104 x 62`
- preview 控件：`88 x 24`

视觉原则：
- 页面继续保持浅灰背景、白底卡片和低噪音边框。
- `Badge` 只承担短文本状态和轻量图标，不扩成成组容器。
- `subtle` 和 `read_only` 继续压低视觉权重，避免压过主体内容。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 190` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 84` | 主展示面板 |
| `primary_badge` | `egui_view_badge_t` | `126 x 28` | 主 badge |
| `compact_badge` | `egui_view_badge_t` | `88 x 24` | 紧凑静态 preview |
| `read_only_badge` | `egui_view_badge_t` | `88 x 24` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 文本标签 | 是 | 是 | 是 |
| leading icon | 可选 | 可选 | 否 |
| `filled / outline / subtle` | 轮播展示 | `outline` | 弱化后的 `read_only` |
| `compact_mode` | 关闭 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| 接收焦点 / 交互 | 否 | 否 | 否 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `Badge` 本身是非交互的 display 控件，不承担 dismiss、点击提交或键盘激活。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，不让对照块接管交互。
- 录制轨道中的主控件切换通过程序化 snapshot 完成，不依赖用户输入。

## 8. 本轮收口内容
- 新增 `egui_view_badge.h/.c`
- 提供：
  - `apply_filled_style()`
  - `apply_outline_style()`
  - `apply_subtle_style()`
  - `apply_read_only_style()`
  - `set_text()`
  - `set_icon()`
  - `set_font()`
  - `set_icon_font()`
  - `set_palette()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
  - `get_icon_region()`
  - `override_static_preview_api()`
- demo 页面接入主 badge 和两个静态 preview
- 单测覆盖样式 helper、setter、icon region、模式切换和静态 preview 输入抑制

## 9. 录制动作设计
1. 还原初始 `filled` 快照
2. 抓取第一帧
3. 切到 `outline` 快照
4. 抓取第二帧
5. 切到 `subtle` 快照
6. 抓取第三帧
7. 恢复初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge
```

验收重点：
- 主控件、compact preview、read_only preview 都必须完整可见，不黑屏、不白屏、不裁切。
- `filled / outline / subtle` 三个快照之间的层级差异必须清晰，但整体保持低噪音。
- `read_only` preview 需要明显弱化，且不残留 icon 强强调。

## 11. 已知限制
- 当前只覆盖单个 `Badge`，不实现成组布局、dismiss 或容器级状态汇总。
- 当前不提供可点击 badge、可筛选 badge 或复杂 token-like 交互，这些语义仍由 `tag` / `badge_group` 承担。
- 当前 leading icon 仅支持单个图标字形，不扩展自定义图片或多段内容。
