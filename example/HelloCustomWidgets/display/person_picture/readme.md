# PersonPicture 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件名：`PersonPicture`
- 本次保留语义：头像圆形承载、`display name -> initials` 回退、显式 `initials`、`fallback glyph`、可选 `image`、`presence` 状态点、`tone` 语义色、静态 preview 输入抑制
- 删减内容：联系人数据源绑定、系统级状态桥接、复杂徽章系统、SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_person_picture`，支持外部传入 `egui_image_t *` 并在控件内部做圆形遮罩；本轮 demo 以 initials 与 fallback glyph 为主，不内置额外人像资源

## 1. 为什么需要这个控件？
`PersonPicture` 用来表达“这里代表一个人或一个联系人槽位”的标准头像语义。它和单纯的 `image_icon` 不同，核心价值不是展示任意图片，而是在图片缺失时仍然能稳定回退到 initials 或 person glyph，并附带 presence 点来承载在线状态。

## 2. 为什么现有控件不够用？
- `image_icon` 只负责图片本身，没有 `display name / initials / fallback glyph` 的回退链路。
- `font_icon`、`symbol_icon` 只能展示图标，不能表达头像的 tone 和 presence 语义。
- `persona_group` 面向群组与聚合展示，不适合作为单个头像的基础控件。

## 3. 目标场景与示例概览
- 主面板演示三种主状态：
  - 通过 `display_name` 自动解析 initials
  - 通过显式 `initials` 固定头像文案
  - 在没有 name / initials / image 时回退到 `person` glyph
- 底部两个静态 preview 分别保留：
  - `compact` 小尺寸头像
  - `read_only` 静态弱化态

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 256`
- 主面板：`196 x 136`
- 主头像：`72 x 72`
- 底部容器：`216 x 84`
- 单个 preview：`104 x 84`
- preview 头像：`38 x 38`

视觉原则：
- 使用 Fluent 主线中的浅灰背景、白色承载面和低噪音描边。
- 头像控件本身只负责头像绘制，不承担额外卡片、阴影或列表语义。
- presence 点保持小而明确，避免抢占头像主体。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 256` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 136` | 主展示面板 |
| `primary_picture` | `egui_view_person_picture_t` | `72 x 72` | 主头像控件 |
| `compact_picture` | `egui_view_person_picture_t` | `38 x 38` | 小尺寸静态 preview |
| `read_only_picture` | `egui_view_person_picture_t` | `38 x 38` | 只读弱化态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `display_name -> initials` | 是 | 是 | 是 |
| 显式 `initials` | 是 | 是 | 是 |
| `fallback_glyph` | 是 | 是 | 是 |
| `set_image()` 圆形遮罩 | 支持 | 支持 | 支持 |
| `presence` | 是 | 是 | 是 |
| `tone` | 是 | 是 | 是 |
| `compact_mode` | 可选 | 开启 | 开启 |
| `read_only_mode` | 可选 | 关闭 | 开启 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `PersonPicture` 本轮定义为显示型控件，不承担点击、导航或焦点切换职责。
- `set_display_name()` 与 `set_initials()` 共同形成回退链路：
  - 有 `image` 时优先画 image
  - 无 image 时优先画显式或推导出的 initials
  - 若仍无文本，则回退到 `fallback glyph`
- `presence` 仅负责绘制右下角状态点，不引入额外交互。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，避免在 reference 页面里误接管输入。

## 8. 本轮收口内容
- 新增 `egui_view_person_picture.h/.c`
- 补齐 `set_display_name()`、`set_initials()`、`set_fallback_glyph()`、`set_image()`、`set_presence()`、`set_tone()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 与静态 preview API
- demo 页面接入主状态切换、compact preview 与 read-only preview
- 单测覆盖默认初始化、initials 解析、setter 收口、辅助 region 计算与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Derived initials`
2. 抓取初始帧
3. 切到 `Manual initials`
4. 抓取第二帧
5. 切到 `Fallback glyph`
6. 抓取第三帧
7. 回到初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/person_picture PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/person_picture --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/person_picture
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_person_picture
```

验收重点：
- 主头像与两个 preview 必须完整可见，不黑屏、不白屏、不裁切。
- initials、fallback glyph 和 presence 点都必须清晰可辨。
- `read_only` preview 必须明显弱化，但仍保留头像语义。

## 11. 已知限制
- 本轮不内置新的 portrait 资源，demo 重点放在 initials / glyph 回退链路。
- `set_image()` 已保留并支持圆形遮罩，但需要业务层自行提供稳定的 `egui_image_t *`。
- initials 解析当前按 ASCII 规则处理，优先覆盖 reference demo 与现有英文命名场景。
