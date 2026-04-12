# BitmapIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`BitmapIcon`
- 本次保留语义：只读单色位图图标显示、alpha-only 遮罩着色、内置资源切换、静态 preview 输入抑制
- 删减内容：额外交互容器、复杂装饰背景、资源生成链依赖、SDK 改动
- EGUI 适配说明：在 `custom` 层基于 SDK `egui_view_image` 做轻封装，内置三张 16x16 手写 `alpha-only` `egui_image_std` 资源，并通过 `egui_view_image_set_image_color()` 提供前景色着色能力，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`BitmapIcon` 用来承载“形状来自位图遮罩、颜色来自前景色”的单色图标语义，例如文档、邮件、告警这类简化信息符号。它和 `ImageIcon` 的边界很清楚：前者强调单色遮罩与前景色着色，后者强调完整图片内容。

## 2. 为什么现有控件不够用？
- `image_icon` 适合完整位图缩略图，不适合由前景色统一着色的单色图标资源。
- `font_icon` / `symbol_icon` 依赖字形字体，不适合表达“位图来源固定、颜色由宿主决定”的掩码图标。
- 直接使用 SDK `egui_view_image` 能显示 alpha-only 图片，但缺少 `BitmapIcon` 这层默认资源回退、样式 helper 和静态 preview 输入抑制。

## 3. 目标场景与示例概览
- 主区域保留一个主 `BitmapIcon`，录制时切换 `Document -> Mail -> Alert -> Document`
- 底部保留两个静态 preview，分别展示 `Subtle / Mail` 与 `Accent / Alert`
- 页面只保留标题、主面板和底部双 preview，不承担额外交互职责
- 目录：`example/HelloCustomWidgets/display/bitmap_icon/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主面板：`196 x 112`
- 主图标：`52 x 52`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`
- 单个 preview 图标：`28 x 28`

视觉原则：
- 保持 Fluent 主线里的浅灰背景与白色表面
- `BitmapIcon` 自身只负责把 alpha-only 位图缩放到工作区并着色，不承载边框和阴影
- 主状态通过资源切换与前景色切换表达差异，底部 preview 保持静态 reference 对照

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 112` | 主展示面板 |
| `primary_heading_label` | `egui_view_label_t` | `176 x 12` | 主场景标题 |
| `primary_icon` | `egui_view_bitmap_icon_t` | `52 x 52` | 主位图图标 |
| `primary_name_label` | `egui_view_label_t` | `176 x 12` | 当前资源名 |
| `primary_note_label` | `egui_view_label_t` | `176 x 18` | 当前说明文案 |
| `subtle_icon` | `egui_view_bitmap_icon_t` | `28 x 28` | `Mail` 静态 preview |
| `accent_icon` | `egui_view_bitmap_icon_t` | `28 x 28` | `Alert` 静态 preview |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Subtle | Accent |
| --- | --- | --- | --- |
| 默认 `Document` 资源 | 是 | 否 | 否 |
| `Mail` 资源 | 录制切换 | 是 | 否 |
| `Alert` 资源 | 录制切换 | 否 | 是 |
| `set_image(NULL)` 回退默认图 | 是 | 是 | 是 |
| `apply_standard/subtle/accent_style()` | 是 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `BitmapIcon` 本轮只保留只读显示语义，不承担 click、toggle 或导航职责
- 主控件录制时通过 `set_image()` 和 `set_palette()` 切换资源与前景色
- 底部 preview 通过静态 preview API 吞掉 `touch / key`，避免误接管 pressed 或 click

## 8. 本轮收口内容
- 新增 `egui_view_bitmap_icon.h/.c`，基于 SDK `egui_view_image` 封装 `BitmapIcon`
- 新增三张内置 alpha-only 位图资源：`document / mail / alert`
- 补齐 `standard / subtle / accent` 样式 helper、`set_image()`、`get_image()`、内置资源 getter、`set_palette()` 与静态 preview API
- demo 页面保留一个主 `BitmapIcon` 与两个静态 preview，录制轨道覆盖三组资源快照
- 单测覆盖默认初始化、样式 helper、资源切换 / 回退、palette setter 与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Document` 初始态
2. 抓取初始帧
3. 切到 `Mail`
4. 抓取第二帧
5. 切到 `Alert`
6. 抓取第三帧
7. 回到 `Document`
8. 抓取最终收尾帧

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/bitmap_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/bitmap_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/bitmap_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_bitmap_icon
```

验收重点：
- 主图标与底部 preview 必须完整可见，不黑屏、不白屏、不裁切
- `Document / Mail / Alert` 三组快照必须稳定可区分，并能看出“位图源 + 前景色”双重切换
- 底部两个 preview 必须保持静态 reference，不响应真实输入

## 11. 已知限制
- 当前只覆盖最小 `BitmapIcon` 语义，不扩展动画、可编辑画刷或复杂多色位图
- 内置资源是为了 reference 页面和默认回退准备的轻量 alpha-only 掩码，不等同于通用图标库
- 当前所有默认资源统一按 `resize` 模式绘制，不额外做等比或居中裁切策略切换
- 当前着色语义完全依赖前景色与 alpha-only 位图组合，不支持多通道分层着色
