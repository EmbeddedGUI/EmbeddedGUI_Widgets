# Tag 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Tag`
- 本次保留语义：单个 tag 的 `text + secondary text + dismiss`、`compact / read only` 预览、same-target release、键盘 dismiss 与静态 preview 输入抑制
- 本次删除内容：`interaction tag`、avatar/media slot、tag group、换行布局管理、额外动画和复杂外观变体
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_tag`，专注单个 tag 的 Fluent 语义，不修改 SDK

## 1. 为什么需要这个控件？
`Tag` 用来表达“用户已经选中、但仍可轻量撤销的一项内容”，常见于筛选器、收件人、已分配状态和轻量分类场景。当前仓库里虽然有 `badge_group`、`info_badge` 和 `token_input`，但还缺少一个对齐 Fluent 2 `Tag` 语义的单个 chip 组件，因此需要补齐。

## 2. 为什么现有控件不够用？
- `badge_group` 更偏成组展示与摘要，不承担单个 tag 的 dismiss 交互。
- `info_badge` 用来表达系统状态或数量，不适合承载“用户可移除的一项值”。
- `token_input` 是输入容器，职责是编辑和提交 token，不是单个 tag 的轻量展示与撤销。

## 3. 目标场景与示例概览
- 主面板：一个可 dismiss 的标准 `tag`，展示主文本和次级文本。
- 底部保留两个静态 preview：
  - `compact`
  - `read_only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 190`
- 主面板：`196 x 82`
- 主控件：`140 x 28`
- 底部容器：`216 x 62`
- 单个 preview 面板：`104 x 62`
- preview 控件：`84 x 24`

视觉原则：
- 页面继续保持浅灰背景、白底卡片和低噪音边框。
- `Tag` 本身只承担 pill 外观和 dismiss affordance，不扩成整组布局容器。
- `read_only` 只保留弱化后的展示语义，不再暴露 dismiss affordance。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 190` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 82` | 主展示面板 |
| `primary_tag` | `egui_view_tag_t` | `140 x 28` | 主 dismissible tag |
| `compact_tag` | `egui_view_tag_t` | `84 x 24` | 紧凑静态 preview |
| `read_only_tag` | `egui_view_tag_t` | `84 x 24` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 主文本 | 是 | 是 | 是 |
| 次级文本 | 是 | 是 | 是 |
| dismiss affordance | 是 | 是 | 否 |
| same-target release | 是 | 否 | 否 |
| `Delete / Backspace / Enter / Space` dismiss | 是 | 否 | 否 |
| `compact_mode` | 可选 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- dismiss 只在 `DOWN` 命中 dismiss 区域，并且 `UP` 仍落在同一目标时才提交。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交；回到 `A` 后再 `UP(A)` 才提交。
- 键盘保留 `Delete / Backspace / Enter / Space` 触发 dismiss，用于聚焦在整颗 tag 上时的轻量撤销。
- `read_only` 或 `disabled` 时，dismiss 输入全部失效。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，不让对照块接管交互。

## 8. 本轮收口内容
- 新增 `egui_view_tag.h/.c`
- 提供：
  - `apply_standard_style()`
  - `apply_compact_style()`
  - `apply_read_only_style()`
  - `set_text()`
  - `set_secondary_text()`
  - `set_font()`
  - `set_secondary_font()`
  - `set_icon_font()`
  - `set_palette()`
  - `set_dismissible()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
  - `set_on_dismiss_listener()`
  - `get_dismiss_region()`
  - `override_static_preview_api()`
- demo 页面接入主 dismissible tag 和两个静态 preview
- 单测覆盖样式 helper、setter、dismiss region、same-target release、键盘 dismiss、只读/禁用 guard 与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原初始 `Assigned / Today`
2. 抓取初始帧
3. 点击 dismiss，切到下一条样例
4. 抓取第二帧
5. 用 `Delete` 再次 dismiss
6. 抓取第三帧
7. 保持最终状态收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/tag PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/tag --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/tag
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_tag
```

验收重点：
- 主控件、compact preview、read_only preview 都必须完整可见，不黑屏、不白屏、不裁切。
- dismiss affordance 在主控件上清晰可见，且不会在 `read_only` preview 里残留。
- 次级文本必须保持从属层级，不抢主文本视觉权重。

## 11. 已知限制
- 当前只覆盖单个 `Tag`，不实现 `interaction tag`、avatar/media slot 和整组 tag layout。
- 当前不提供自动换行、溢出折叠和文本省略策略，示例通过给足宽度保持可读性。
- 当前 dismiss 的唯一动作是外部 listener 回调，不负责容器级排序或数据源管理。
