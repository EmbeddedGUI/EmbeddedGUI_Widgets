# PathIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`PathIcon`
- 本次保留语义：只读单色矢量图标显示、预解析 path 数据切换、静态 preview 输入抑制
- 删减内容：完整 SVG path string 解析、多色分层渲染、动画描边、SDK 改动
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_path_icon`，接受 `MOVE_TO / LINE_TO / QUAD_TO / CUBIC_TO / CLOSE` 预解析命令数组，在 view 的 `on_draw` 中做有限步数 flatten，并在 SDK 现有 polygon / polyline canvas 能力上完成绘制，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`PathIcon` 用来承载“图标来源是矢量路径、颜色来自前景色、缩放由控件尺寸决定”的单色图标语义。它和 `BitmapIcon`、`ImageIcon`、`FontIcon` 互补，适合表达没有位图资源、也不依赖字体字形的轻量图标场景。

## 2. 为什么现有控件不够用？
- `bitmap_icon` 依赖 alpha-only 位图资源，资源准备成本更高，缩放本质上仍是位图重采样。
- `image_icon` 面向完整图片缩略图，不适合表达纯单色几何图标。
- `font_icon` / `symbol_icon` 依赖字体与字形映射，不适合承载任意业务自定义矢量路径。
- 直接使用 SDK canvas API 虽然能画折线和多边形，但缺少 `PathIcon` 这层统一的默认数据回退、样式 helper 和静态 preview 行为收口。

## 3. 目标场景与示例概览
- 主区域保留一个主 `PathIcon`，录制时切换 `Bookmark -> Heart -> Send -> Bookmark`
- 底部保留两个静态 preview，分别展示 `Subtle / Heart` 与 `Accent / Send`
- 页面只保留标题、主展示面板和底部双 preview，不承担额外交互职责
- 目录：`example/HelloCustomWidgets/display/path_icon/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 212`
- 主面板：`196 x 116`
- 主图标：`56 x 56`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`
- 单个 preview 图标：`30 x 30`

视觉原则：
- 保持 Fluent 主线中的浅灰背景与白色表面
- `PathIcon` 自身只承担单色路径缩放与填充，不额外承担边框、容器、阴影职责
- 主状态通过路径数据与前景色切换表达差异，底部 preview 保持静态 reference 对照

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 212` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 116` | 主展示面板 |
| `primary_heading_label` | `egui_view_label_t` | `176 x 12` | 主场景标题 |
| `primary_icon` | `egui_view_path_icon_t` | `56 x 56` | 主路径图标 |
| `primary_name_label` | `egui_view_label_t` | `176 x 14` | 当前路径名称 |
| `primary_note_label` | `egui_view_label_t` | `176 x 18` | 当前说明文案 |
| `subtle_icon` | `egui_view_path_icon_t` | `30 x 30` | `Heart` 静态 preview |
| `accent_icon` | `egui_view_path_icon_t` | `30 x 30` | `Send` 静态 preview |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Subtle | Accent |
| --- | --- | --- | --- |
| 默认 `Bookmark` | 是 | 否 | 否 |
| `Heart` | 录制切换 | 是 | 否 |
| `Send` | 录制切换 | 否 | 是 |
| `set_data(NULL)` 回退默认路径 | 是 | 是 | 是 |
| `apply_standard/subtle/accent_style()` | 是 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `PathIcon` 本轮只保留只读显示语义，不承担 click、toggle 或导航职责
- 主控件录制时通过 `set_data()` 与 `set_palette()` 切换路径数据和前景色
- 底部 preview 通过静态 preview API 吞掉 `touch / key`，避免误接管 pressed 或 click

## 8. 本轮收口内容
- 新增 `egui_view_path_icon.h/.c`，实现 `PathIcon` 轻量数据结构与自绘逻辑
- 提供三组内置路径：`bookmark / heart / send`
- 补齐 `standard / subtle / accent` 样式 helper、`set_data()`、`get_data()`、内置路径 getter、`set_palette()` 与静态 preview API
- demo 页面保留一个主 `PathIcon` 与两个静态 preview，录制轨道覆盖三组路径快照
- 单测覆盖默认初始化、样式 helper、路径切换 / 默认回退、palette setter 与静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Bookmark` 初始态
2. 抓取初始帧
3. 切换到 `Heart`
4. 抓取第二帧
5. 切换到 `Send`
6. 抓取第三帧
7. 回到 `Bookmark`
8. 抓取最终收尾帧

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/path_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/path_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/path_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_path_icon
```

验收重点：
- 主图标与底部 preview 必须完整可见，不黑屏、不白屏、不裁切
- `Bookmark / Heart / Send` 三组快照必须稳定可区分，并能看出 path 数据切换
- 曲线路径在主图标尺寸下不能出现明显断裂、空洞或整块缺失
- 底部两个 preview 必须保持静态 reference，不响应真实输入

## 11. 已知限制
- 当前只覆盖最小 `PathIcon` 语义，不支持完整 SVG/XAML path 字符串解析
- 当前 filled path 依赖 SDK `egui_canvas_draw_polygon_fill()`，单个 contour 最终顶点数会压缩到 `16` 以内
- 曲线路径使用有限步数 flatten，目标是 reference 图标稳定可读，不追求高精度矢量拟合
- 多 contour 会逐个填充，复杂镂空与布尔运算不在本轮范围内
