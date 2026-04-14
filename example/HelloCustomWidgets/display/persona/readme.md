# Persona 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Persona`
- 本次保留语义：`avatar + name + secondary/tertiary/quaternary text + presence + tone + compact + read only`
- 本次删除内容：命令按钮、可编辑头像、复杂列表容器、业务侧数据绑定
- EGUI 适配说明：在 `HelloCustomWidgets` 的 `custom` 层新增轻量 `egui_view_persona`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Persona` 用来表达“这是一个带身份信息的单人条目”，比单独的 `person_picture` 更完整，也比 `persona_group` 更聚焦。它适合出现在审批责任人、评论作者、协作成员概览、只读资料卡等场景里。

## 2. 为什么现有控件不够用
- `person_picture` 只负责头像与 presence 点，不承接多行身份文本。
- `persona_group` 面向群组聚合与重叠头像，不适合作为单人信息条目。
- `text_block`、`rich_text_block` 只能显示文本，缺少头像、tone 和状态点语义。

## 3. 目标场景与示例概览
- 主控件展示标准 `Persona`，覆盖 `available / busy / offline` 三种状态。
- 底部左侧 preview 展示 `compact` 静态对照。
- 底部右侧 preview 展示 `read only` 静态对照。
- 页面结构统一为：标题 -> 主 `persona` -> `compact / read only`。

目标目录：`example/HelloCustomWidgets/display/persona/`

## 4. 视觉与布局规格
- 根容器：`224 x 238`
- 主控件：`196 x 104`
- 底部对照行：`216 x 78`
- 单个 preview：`104 x 78`

视觉约束：
- 延续 Fluent 2 的浅色低噪音面板。
- 左侧头像区使用轻量 section 底色，不做重阴影卡片。
- `tone` 只用于头像与细条强调，不放大为整块高饱和背景。
- `read only` 通过整体验色弱化，而不是改变布局结构。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 238` | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `persona_primary` | `egui_view_persona_t` | `196 x 104` | 主参考控件 |
| `persona_compact` | `egui_view_persona_t` | `104 x 78` | `compact` 静态 preview |
| `persona_read_only` | `egui_view_persona_t` | `104 x 78` | `read only` 静态 preview |

## 6. 状态矩阵
| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `available` | Accent tone，展示四行文本 |
| 主控件 | `busy` | Success tone，展示显式 initials |
| 主控件 | `offline` | Neutral tone，presence 退化为 ring |
| `compact` preview | `away` | 紧凑布局，仅保留两行文本 |
| `read only` preview | `do not disturb` | 紧凑 + 只读，对照 muted palette |

## 7. 交互语义
- `Persona` 本轮定义为显示型控件，不承接点击、导航或焦点职责。
- `display_name` 与 `initials` 共同形成头像文本回退链路：
  1. 优先使用显式 `initials`
  2. 否则从 `display_name` 推导两位 initials
  3. 仍为空时回退到 `person` glyph
- `presence` 仅负责绘制头像右下角状态点，不引入额外交互。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`。

## 8. 本轮收口内容
- 新增 `egui_view_persona.h/.c`
- 补齐 `set_display_name()`、`set_secondary_text()`、`set_tertiary_text()`、`set_quaternary_text()`、`set_initials()`、`set_status()`、`set_tone()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 与静态 preview API
- demo 页面接入主轨道与双 preview 对照
- 单测覆盖默认初始化、initials 推导、颜色/region helper、静态 preview 输入抑制

## 9. 录制动作设计
1. 还原默认 `available` 轨道
2. 抓取第一帧
3. 切到 `busy`
4. 抓取第二帧
5. 切到 `offline`
6. 抓取第三帧
7. 回到初始状态收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

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

验收重点：
- 主控件与两个 preview 必须完整可见，无黑白屏、无裁切。
- `available / busy / offline / do not disturb` 的状态差异要能明确辨认。
- `compact` 必须减少文本密度，但仍保留单人身份条目语义。
- `read only` 必须明显弱化，同时仍保持可读。

## 11. 已知限制
- 本轮不接入真实头像图资源，重点放在 `Persona` 的结构语义与文本层级。
- initials 推导当前按 ASCII 规则处理，优先覆盖仓库内现有英文命名场景。
- 该控件仍停留在 `HelloCustomWidgets` 的 reference 层，不下沉到 `src/widget/`。
