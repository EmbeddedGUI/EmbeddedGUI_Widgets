# segmented_control 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`SegmentedControl`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`
- 删除效果：页面级 guide、状态回显、section divider、外部 preview 标签、场景化轮播入口、复杂 hover 动画、Acrylic 和图标化装饰段
- EGUI 适配说明：复用仓库已有 `segmented_control` 核心交互，在 `480 x 480` 页面里优先保证选中胶囊、焦点 ring 和底部双预览对照稳定

## 1. 为什么需要这个控件

`segmented_control` 用来表达同一组互斥选项之间的轻量切换，适合视图模式、时间范围、过滤级别、密度选择这类页内局部状态切换。它比 tab 更轻，比 radio 更紧凑，是 Fluent / WPF UI 主线里明确存在的标准输入控件。

## 2. 为什么现有控件不够用

- `tab_strip` 更偏整页导航，不适合页内局部过滤
- `toggle_button` 与 `split_button` 是单个动作控件，不是互斥选项组
- `radio_button` 强调表单语义，不适合横向胶囊式切换
- 当前 reference 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `SegmentedControl`

因此这里继续保留 `segmented_control`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `segmented_control`，保留真实触摸切换与焦点状态
- 左下 `compact` 预览展示紧凑尺寸下的轻量 reference
- 右下 `read only` 预览展示静态只读对照
- 主控件保留 `Left / Right / Up / Down / Home / End / Tab` 键盘闭环
- 示例页只保留标题、主 `segmented_control` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和外部标签

目录：

- `example/HelloCustomWidgets/input/segmented_control/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 112`
- 页面结构：标题 -> 主 `segmented_control` -> `compact / read only` 双预览
- 主控件：`196 x 38`
- 底部双预览容器：`216 x 30`
- `compact` 预览：`104 x 30`
- `read only` 预览：`104 x 30`
- 视觉规则：
  - 使用浅灰白 page panel + 白底低噪音表面
  - 主控件保留标准胶囊选中块、外边框和焦点 ring
  - `compact` 预览只收紧圆角、padding 和分段间距，不改变控件语义
  - `read only` 预览保留选中项表达，但不承担真实交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 112` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Segmented Control` | 页面标题 |
| `control_primary` | `egui_view_segmented_control_t` | `196 x 38` | `Overview / Team / Usage / Access` | 标准主控件 |
| `control_compact` | `egui_view_segmented_control_t` | `104 x 30` | `Day / Week` | 紧凑静态预览 |
| `control_read_only` | `egui_view_segmented_control_t` | `104 x 30` | `Off / Auto / Lock` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 真实触摸切换 | 是 | 否 | 否 |
| 键盘切换 | 是 | 否 | 否 |
| focus ring | 是 | 否 | 否 |
| snapshot 轮换 | 是 | 否 | 否 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主控件与 `compact` 预览状态，并请求主控件焦点
2. 请求第一页截图
3. 触摸主控件第三段，验证真实切换反馈
4. 请求第二页截图
5. 程序化切换主控件到 `Live / Pending / History` 组，并通过 `End` 跳到最后一项
6. 请求第三页截图
7. 程序化切换主控件到 `Day / Week / Month / Year`，同时把 `compact` 预览切到第二组静态对照，再通过 `Home` 回到首项
8. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/segmented_control PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/segmented_control --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部双预览必须完整可见，不能被裁切
- 选中胶囊、边框、文字和 focus ring 必须清晰可辨
- 主控件触摸与键盘切换必须都能工作，且不再依赖外部 guide / label 点击
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 的低噪音浅色 reference

## 9. 已知限制与后续方向

- 当前只保留纯文本分段，不接图标、badge 和计数器
- 当前 `compact` 与 `read only` 仅作为静态对照，不承担交互职责
- 当前只覆盖单行水平布局，不扩展到多行折行场景
- 若后续要沉入框架层，再单独评估与 `tab_strip`、`button group` 等语义边界

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `tab_strip`：本控件用于局部状态切换，不承担整页导航
- 相比 `radio_button`：本控件是横向胶囊分段，不是表单单选列
- 相比 `toggle_button`：本控件表达互斥分组，不是单一 on/off 动作
- 相比核心层 `src/widget/egui_view_segmented_control`：本目录负责 reference 页面与样式落地，不重复造核心控件

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`SegmentedControl`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `focused`
  - `touch switch`
  - `keyboard switch`

## 13. 相比参考原型删除了哪些效果或装饰

- 不做页面级 guide、状态回显、section divider 和外部 preview 标签
- 不做图标段、badge、计数器和桌面级 hover reveal
- 不做 Acrylic、复杂阴影和多层装饰描边
- 不做拖拽重排、溢出折叠和复杂自适应动画

## 14. EGUI 适配时的简化点与约束

- 直接复用核心层已有 `segmented_control`，避免与 `src/widget` 同名实现冲突
- 用固定 snapshot 驱动，优先保证 `480 x 480` 页面里的稳定 reference
- 底部 `compact` 与 `read only` 固定为静态对照，不再承担额外交互
- 先完成示例级审阅稳定性，再决定是否抽象到框架公共层
