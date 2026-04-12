# AnimatedIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件名：`AnimatedIcon`
- 本次保留语义：状态驱动动画、内置 source 切换、fallback glyph、静态 preview 输入抑制
- 删减内容：Lottie 文件解析、任意 marker 图谱导入、系统级动画首选项桥接、SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_animated_icon`，先收口 `AnimatedBackVisualSource` 与 `AnimatedChevronDownSmallVisualSource` 两个内置 source，使用状态字符串驱动短时计时器动画；当 source 不可用时回退到静态 glyph，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`AnimatedIcon` 用来表达“图标不是静态资源，而是根据控件状态在几个关键姿态之间过渡”的语义。它适合承接按钮、导航项、折叠入口等轻量状态反馈，不必为了一个小图标引入整块复杂动画视图。

## 2. 为什么现有控件不够用？
- `symbol_icon`、`font_icon`、`path_icon` 只能直接切换最终图形，没有状态过渡。
- `animated_image` 面向帧序列图片，不适合表达按状态驱动的单色图标。
- 直接在业务页里手写定时器和 canvas 绘制会把 source、状态、fallback 与静态 preview 逻辑散落到页面代码里，缺少统一收口。

## 3. 目标场景与示例概览
- 主区域展示一个 `AnimatedBackVisualSource`，录制时依次切到 `Normal -> PointerOver -> Pressed -> Normal`
- 底部左侧展示一个静态 `AnimatedChevronDownSmallVisualSource / Pressed`
- 底部右侧展示 `source = NULL` 时的 fallback glyph 行为
- 页面结构只保留标题、主展示面板和底部双 preview

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 220`
- 主面板：`196 x 118`
- 主图标：`60 x 60`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`
- 单个 preview 图标：`28 x 28`

视觉原则：
- 保持 Fluent 主线中的浅灰背景、白色表面和低噪音留白
- `AnimatedIcon` 自身只承担单色图标动画，不承担外部容器、描边或阴影语义
- 主区域通过状态切换体现动画，底部 preview 只保留静态 reference 对照

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 220` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 118` | 主展示面板 |
| `primary_icon` | `egui_view_animated_icon_t` | `60 x 60` | 主动画图标 |
| `primary_state_label` | `egui_view_label_t` | `176 x 12` | 当前状态标签 |
| `chevron_icon` | `egui_view_animated_icon_t` | `28 x 28` | `Chevron / Pressed` 静态 preview |
| `fallback_icon` | `egui_view_animated_icon_t` | `28 x 28` | `Fallback / Settings` 静态 preview |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Chevron preview | Fallback preview |
| --- | --- | --- | --- |
| `Normal` | 是 | 否 | 是 |
| `PointerOver` | 是 | 否 | 否 |
| `Pressed` | 是 | 是 | 否 |
| `NormalToPressed` 等过渡字符串 | 是 | 否 | 否 |
| `source = NULL` 使用 fallback glyph | 否 | 否 | 是 |
| `set_animation_enabled(0)` 直接落最终帧 | 支持 | 支持 | 支持 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `AnimatedIcon` 本轮保留图标级状态语义，不直接承担按钮点击或导航职责
- `set_state("Normal" / "PointerOver" / "Pressed")` 会根据当前 source 在关键姿态间过渡
- 兼容常见过渡字符串：`NormalToPointerOver`、`NormalToPressed`、`PointerOverToNormal`、`PointerOverToPressed`、`PressedToNormal`、`PressedToPointerOver`
- `source = NULL` 时走 fallback glyph，便于在缺失动画源时仍保持图标可见
- 底部 preview 通过静态 preview API 吞掉 `touch / key`，避免误接管输入

## 8. 本轮收口内容
- 新增 `egui_view_animated_icon.h/.c`
- 内置两个 source：`AnimatedBackVisualSource`、`AnimatedChevronDownSmallVisualSource`
- 补齐 `set_source()`、`set_state()`、`set_fallback_glyph()`、`set_animation_enabled()`、`set_icon_font()`、`set_palette()` 与静态 preview API
- demo 页面展示主 `Back` 动画、静态 `Chevron` 和 fallback glyph
- 单测覆盖默认初始化、状态解析、source / fallback / animation setter、attach 后定时器动画与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Back / Normal`
2. 抓取初始帧
3. 切到 `PointerOver`，等待动画完成
4. 抓取第二帧
5. 切到 `Pressed`，等待动画完成
6. 抓取第三帧
7. 回到 `Normal`，等待动画完成
8. 抓取最终收尾帧

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/animated_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/animated_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/animated_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_animated_icon
```

验收重点：
- 主图标与底部 preview 必须完整可见，不黑屏、不白屏、不裁切
- `Normal / PointerOver / Pressed` 三个关键姿态必须清晰可分辨
- `Chevron` preview 要稳定显示为静态最终帧
- `Fallback` preview 在没有 source 的情况下仍要显示有效 glyph

## 11. 已知限制
- 当前不解析 Lottie / JSON 动画资源，只支持内置 source
- 当前只覆盖常见 `Normal / PointerOver / Pressed` 状态族及对应过渡字符串
- 当前动画由固定步长定时器推进，目标是 reference 演示稳定，不追求复杂 easing
- 当前 fallback 只支持静态 glyph，不支持完整 `IconSource` 对象树
