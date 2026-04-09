# color_picker 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`ColorPicker`
- 本次保留状态：`tone palette`、`hue rail`、`compact`、`read only`、`keyboard focus`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、标签点击切换、弹出式高级编辑器、透明度通道、桌面 hover 动画
- EGUI 适配说明：保留标准颜色选择器的 tone palette + hue rail + 当前色预览语义，在 `480 x 480` 下优先保证色块可辨识、hex 文本可读与 `compact / read only` 对照稳定

## 1. 为什么需要这个控件

`color_picker` 用于表达标准颜色选择语义，比如主题色、状态色、卡片强调色和图标前景色等。它补足了当前 `HelloCustomWidgets` 里“可派生色调 + 连续色相切换”的标准颜色选择器能力。

## 2. 为什么现有控件不够用

- `swatch_picker` 只覆盖离散色样切换，不支持从同一 hue 派生明暗 / 饱和度变化
- `slider` / `xy_pad` 有连续输入能力，但不具备颜色语义和即时色彩预览
- `number_box` / `textinput` 可以录数值或文本，但不适合直接做颜色选择体验
- 当前主线需要一版更接近 `Fluent 2 / WPF UI ColorPicker` 的标准轻量色彩选择器

因此这里继续保留 `color_picker`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `color_picker`，包含 label、preview swatch、hex 文本、tone palette 与 hue rail
- 左下 `compact` 预览展示紧凑颜色选择卡
- 右下 `read only` 预览展示只读配色卡
- 示例页只保留标题、主 `color_picker` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和标签点击
- 录制动作改为程序化切换主 preset、键盘 tone/hue 切换和 `compact` 静态快照轮换

目录：

- `example/HelloCustomWidgets/input/color_picker/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 204`
- 页面结构：标题 -> 主 `color_picker` -> `compact / read only` 双预览
- 主色彩选择器：`196 x 112`
- 底部双预览容器：`216 x 52`
- `compact` 预览：`104 x 52`
- `read only` 预览：`104 x 52`
- 视觉规则：
  - 使用浅灰白 page panel + 白底轻边框 palette
  - 主卡保留标准颜色选择器语义，不再叠页面级说明与状态桥接
  - `compact` 与 `read only` 作为静态对照，不承担交互职责
  - accent、边框和文本色统一向 Fluent / WPF UI 低噪音浅色体系收口

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 204` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Color Picker` | 页面标题 |
| `picker_primary` | `egui_view_color_picker_t` | `196 x 112` | `Ocean` preset | 标准颜色选择器 |
| `picker_compact` | `egui_view_color_picker_t` | `104 x 52` | `Mint` preset | 紧凑静态预览 |
| `picker_read_only` | `egui_view_color_picker_t` | `104 x 52` | `Locked` preset | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主色彩选择器 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Ocean` 配色 | `Mint` 配色 | `Locked` 配色 |
| `Right` | 提升当前 tone 饱和度 | 不响应 | 不响应 |
| `Tab` / `Down` | 切到 hue rail 并切换色相 | 不响应 | 不响应 |
| 主 preset 轮换 | `Ocean` -> `Coral` | 保持 | 保持 |
| 紧凑 preset 轮换 | 保持 | `Mint` -> `Sun` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 保留静态只读颜色卡 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主 preset、`compact` preset 和只读预览
2. 请求第一页默认截图
3. 发送 `Right`，提升当前 tone
4. 请求第二页截图
5. 发送 `Tab` + `Down`，切到 hue rail 并切换色相
6. 请求第三页截图
7. 程序化切换主 preset 到 `Coral`
8. 请求第四页截图
9. 程序化切换 `compact` 预览到 `Sun`
10. 请求最终对照截图

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/color_picker PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/color_picker --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主色板、hue rail、preview swatch 与 hex 文本都必须完整可见
- 色块之间要能清楚区分，不能因边框或留白不足混在一起
- 主卡必须仍然像标准颜色选择器，而不是场景化装饰卡片
- `compact` 与 `read only` 必须是静态对照，不再承担标签切换职责
- 页面中不再出现 guide、状态文案、standard label、section divider 和外部 preview label

## 9. 已知限制与后续方向

- 当前只覆盖 hue + tone，不含 alpha channel
- 当前没有加入文本输入式 `#RRGGBB` 编辑
- 当前 `compact` 与 `read only` 仅作为静态对照，不承载真实交互
- 若后续要沉入框架层，再补数值输入、透明度、最近使用色与 eyedropper 语义

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `swatch_picker`：核心在派生 tone palette + hue rail，不是离散命名色样列表
- 相比 `xy_pad`：核心在颜色语义、即时色预览和 hex 摘要，不是二维参数控制
- 相比 `slider` / `range_band_editor`：核心在综合色彩选择，而不是单轴数值变化
- 相比 `number_box` / `textinput`：本控件直接表达颜色选择，而不是文本或数值输入

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`ColorPicker`
- 本次保留状态：
  - `tone palette`
  - `hue rail`
  - `compact`
  - `read only`
  - `keyboard focus`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做标签点击轮换和外部状态桥接
- 不做弹出式高级编辑器、透明度轨道与最近使用色面板
- 不做 eyedropper、系统主题联动、hover 光效和复杂阴影层级

## 14. EGUI 适配时的简化点与约束

- 使用固定 preset 与离散键盘步进，优先保证 `480 x 480` 页面里的可审阅性
- 只保留单层 tone palette + hue rail，不引入浮层面板
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 先完成示例级颜色选择器，再决定是否上升到框架公共控件
