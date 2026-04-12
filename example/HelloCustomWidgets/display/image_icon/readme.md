# ImageIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`ImageIcon`
- 本次保留语义：只读图片图标显示、紧凑尺寸约束、可切换图片源、静态 preview 输入抑制
- 删减内容：额外场景化容器、复杂图片装饰、资源生成链依赖、SDK 改动
- EGUI 适配说明：在 `custom` 层基于 SDK `egui_view_image` 做轻封装，内置三张 16x16 手写纯 `RGB565` `egui_image_std` 资源，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`ImageIcon` 用来承载照片缩略图、品牌图块或状态插图等“图片型图标”语义。它和 `SymbolIcon` 的差异很明确：前者依赖位图内容表达，后者依赖字体字形表达。

## 2. 为什么现有控件不够用？
- `symbol_icon` 只适合单色或低细节符号，不适合带场景信息的图片缩略图。
- 现有按钮、卡片类控件虽然能嵌图，但语义核心是交互容器，不是只读图标元素。
- 直接使用 SDK `egui_view_image` 能显示图片，但缺少 `ImageIcon` 这层默认图、静态 preview 和 reference 页面约束。

## 3. 目标场景与示例概览
- 主区域保留一个主 `ImageIcon`，录制时切换 `thumbnail -> warm -> fresh -> thumbnail`
- 底部保留两个静态 preview，分别展示 `Warm` 与 `Fresh`
- 页面只保留标题、主面板和底部双 preview，不承担额外交互职责
- 目录：`example/HelloCustomWidgets/display/image_icon/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 266`
- 主面板：`196 x 136`
- 主图片：`72 x 72`
- 底部容器：`216 x 88`
- 单个 preview：`104 x 88`
- 单个 preview 图片：`40 x 40`

视觉原则：
- 保持 Fluent 主线里的浅灰背景和白色表面
- `ImageIcon` 自身只负责把图片缩放到工作区，不承载边框和阴影
- 三张图片都使用纯 `RGB565` 内置位图，只切换颜色场景和构图，不依赖额外 alpha 通道

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 208` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 112` | 主展示面板 |
| `primary_heading_label` | `egui_view_label_t` | `176 x 12` | 主场景标题 |
| `primary_icon` | `egui_view_image_icon_t` | `72 x 72` | 主图片图标 |
| `primary_name_label` | `egui_view_label_t` | `176 x 14` | 当前图片名 |
| `primary_note_label` | `egui_view_label_t` | `176 x 18` | 当前说明文案 |
| `warm_icon` | `egui_view_image_icon_t` | `40 x 40` | 暖色静态 preview |
| `fresh_icon` | `egui_view_image_icon_t` | `40 x 40` | 清爽静态 preview |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Warm | Fresh |
| --- | --- | --- | --- |
| 默认内置图 | 是 | 否 | 否 |
| 暖色图片源 | 录制切换 | 是 | 否 |
| 清爽图片源 | 录制切换 | 否 | 是 |
| `set_image(NULL)` 回退默认图 | 是 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `ImageIcon` 本轮只保留只读显示语义，不承担 click、toggle 或导航职责
- 主控件录制时通过 `set_image()` 切换图片源
- 底部 preview 通过静态 preview API 吞掉 `touch / key`，避免误接管 pressed 或 click

## 8. 本轮收口内容
- 新增 `egui_view_image_icon.h/.c`，基于 SDK `egui_view_image` 封装 `ImageIcon`
- 新增三张内置图片资源：`default / warm / fresh`
- 补齐 `set_image()`、默认图回退、内置图片 getter 与静态 preview API
- demo 页面保留一个主 `ImageIcon` 与两个静态 preview，录制轨道覆盖三组图片快照
- 单测覆盖默认初始化、图片切换 / 回退与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Thumbnail` 初始态
2. 抓取初始帧
3. 切到 `Warm tone`
4. 抓取第二帧
5. 切到 `Fresh tone`
6. 抓取第三帧
7. 回到 `Thumbnail`
8. 抓取最终收尾帧

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/image_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_icon
```

验收重点：
- 主图片与底部 preview 必须完整可见，不黑屏、不白屏、不裁切
- `Thumbnail / Warm tone / Fresh tone` 三组快照必须稳定可区分
- 底部两个 preview 必须保持静态 reference，不响应真实输入

## 11. 已知限制
- 当前只覆盖最小 `ImageIcon` 语义，不扩展复杂图片裁剪、动画和远程资源
- 内置图片是为了 reference 页面和默认回退准备的轻量资源，不等同于通用图片库
- 如果传入自定义图片资源，控件仍按 `resize` 方式绘制，不额外做比例策略切换
- 当前默认资源为了 PC runtime / web 稳定性采用纯 `RGB565` 路径，不额外启用 alpha 缩放语义
