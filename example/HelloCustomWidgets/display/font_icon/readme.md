# FontIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`FontIcon`
- 本次保留语义：只读字形图标显示、显式字形与字体句柄切换、颜色样式 helper、静态 preview 输入抑制
- 删减内容：动画、hover、额外容器装饰、场景化 showcase 叙事
- EGUI 适配说明：在 `custom` 层基于 SDK `egui_view_label` 做轻封装；demo 为了确定性复用 `Material Symbols` 子集，但控件本身接受任意 `egui_font_t *`

## 1. 为什么需要这个控件？
`FontIcon` 适合那些“图标就是一个字形”的场景，例如设置项、列表摘要、轻量状态提示。它和 `SymbolIcon` 的边界在于：`SymbolIcon` 更偏语义化符号控件，而 `FontIcon` 更强调调用方直接提供 `glyph + font handle`。

## 2. 为什么现有控件不够用？
- `symbol_icon` 已经收口成偏语义型的符号图标控件，不适合表达“任意字形 + 任意图标字体”的更通用入口。
- 直接使用 SDK `egui_view_label` 虽然能渲染字形，但缺少 `FontIcon` 这层默认回退、样式 helper 和静态 preview 输入抑制。
- `image_icon` 面向位图缩略图，不适合字体字形图标。

## 3. 目标场景与示例概览
- 主区域保留一个主 `FontIcon`，录制时依次切换 `Search / MS24 -> Favorite / MS20 -> Settings / MS16 -> Search / MS24`
- 底部保留两个静态 preview，对照 `MS16` 与 `MS20`
- 页面只保留标题、主面板和底部双 preview，不承担额外交互职责
- 目录：`example/HelloCustomWidgets/display/font_icon/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主面板：`196 x 112`
- 主图标槽：`48 x 48`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`
- 单个 preview 图标槽：`28 x 28`

视觉原则：
- 维持 Fluent 主线里的浅灰页面与白色表面
- `FontIcon` 自身只负责居中绘制字形，不承担额外边框和阴影
- 主状态通过不同颜色与不同字体句柄表达密度变化，preview 保持静态 reference 对照

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 112` | 主展示面板 |
| `primary_icon` | `egui_view_font_icon_t` | `48 x 48` | 主字形图标 |
| `primary_name_label` | `egui_view_label_t` | `176 x 12` | 当前字形与字体句柄 |
| `primary_note_label` | `egui_view_label_t` | `176 x 12` | 当前说明文案 |
| `compact_icon` | `egui_view_font_icon_t` | `28 x 28` | `MS16` 静态 preview |
| `default_icon` | `egui_view_font_icon_t` | `28 x 28` | `MS20` 静态 preview |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | MS16 | MS20 |
| --- | --- | --- | --- |
| 默认 `Search / MS24` | 是 | 否 | 否 |
| `Favorite / MS20` | 录制切换 | 否 | 是 |
| `Settings / MS16` | 录制切换 | 是 | 否 |
| `set_glyph(NULL)` 回退默认字形 | 是 | 是 | 是 |
| `set_icon_font(NULL)` 回退默认字体 | 是 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `FontIcon` 本轮只保留只读显示语义，不承担 click、toggle 或导航职责
- 主控件录制时通过 `set_glyph()`、`set_icon_font()` 和 `set_palette()` 切换状态
- 底部 preview 通过静态 preview API 吞掉 `touch / key`，避免误接管 pressed 或 click

## 8. 本轮收口内容
- 新增 `egui_view_font_icon.h/.c`，基于 SDK `egui_view_label` 封装 `FontIcon`
- 补齐 `standard / subtle / accent` 三套样式 helper
- 补齐 `set_glyph()`、`get_glyph()`、`set_icon_font()`、`set_palette()` 与静态 preview API
- demo 页面保留一个主 `FontIcon` 与两个静态 preview，录制轨道覆盖三组 `glyph + font` 快照
- 单测覆盖默认初始化、样式 helper、setter 回退与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Search / MS24` 初始态
2. 抓取初始帧
3. 切到 `Favorite / MS20`
4. 抓取第二帧
5. 切到 `Settings / MS16`
6. 抓取第三帧
7. 回到 `Search / MS24`
8. 抓取最终收尾帧

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/font_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/font_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/font_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_font_icon
```

验收重点：
- 主字形与底部 preview 必须完整可见，不黑屏、不白屏、不裁切
- 三组 `glyph + font` 快照必须稳定区分
- 底部两个 preview 必须保持静态 reference，不响应真实输入

## 11. 已知限制
- 当前 demo 为了稳定性只演示 `Material Symbols`，但控件接口不限定某个字体资源
- 当前只覆盖最小 `FontIcon` 语义，不扩展动画、状态叠层和组合容器
- 当前不提供图标枚举包装，调用方直接传入字形字符串和字体句柄
