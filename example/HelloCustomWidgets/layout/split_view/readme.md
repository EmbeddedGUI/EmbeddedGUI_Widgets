# split_view 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`SplitView`
- 本次保留状态：`standard`、`compact`、`read only`、`pane open`、`pane compact`、`accent`、`warning`、`neutral`
- 删除效果：页面级 guide / 状态文案 / standard label / section label / preview label、复杂阴影、场景化说明文案、hover/focus ring
- EGUI 适配说明：保留侧栏列表、pane 展开/收起、detail 面板和当前选择项，在 `480 x 480` 页面内优先保证结构稳定和主副卡对照阅读；`item / pane / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件

`split_view` 用来承载“侧栏导航 + 内容面板”的双栏布局，并让 pane 的展开/收起和当前选择项成为同一个控件语义。它适合出现在设置中心、资料库、审阅页和分栏工作区里。

## 2. 为什么现有控件不够用

- `nav_panel` 负责导航语义，但不承担 detail 面板
- `data_list_panel` 更偏列表选择，不等同于 `SplitView` 的双栏 pane 结构
- `master_detail` 偏主从内容阅读，不等同于可收起侧栏
- 旧 showcase 页面大量依赖外部状态说明和标签驱动，不适合继续作为 Fluent 主线

因此这里继续保留 `split_view`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `split_view`，覆盖 `pane open / pane compact / warning item / neutral item` 等关键状态
- 左下 `compact` 预览展示小尺寸 split view 在压缩布局中的表现
- 右下 `read only` 预览展示只读成员库态
- 示例页只保留标题、主 `split_view` 和底部 `compact / read only` 双预览，不再保留外部 guide 和状态回显

目录：

- `example/HelloCustomWidgets/layout/split_view/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 224`
- 页面结构：标题 -> 主 `split_view` -> `compact / read only` 双预览
- 主卡区域：`196 x 104`
- 底部双预览容器：`216 x 74`
- `compact` 预览：`104 x 74`
- `read only` 预览：`104 x 74`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音 pane 容器
  - 侧栏 rail、内容面板、detail block 和 footer 维持清晰层级，不做场景化装饰
  - pane 的展开/收起只通过控件本身表达，不依赖外部状态文字
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Split View` | 页面标题 |
| `panel_primary` | `egui_view_split_view_t` | `196 x 104` | `overview open` | 标准 split view |
| `panel_compact` | `egui_view_split_view_t` | `104 x 74` | `overview compact` | 紧凑预览 |
| `panel_read_only` | `egui_view_split_view_t` | `104 x 74` | `members read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `overview + pane open` | `overview + pane compact` | `members + read only` |
| 轮换 1 | `overview + pane compact` | 保持 | 保持 |
| 轮换 2 | `review + warning` | 保持 | 保持 |
| 轮换 3 | `archive + neutral` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `review + pane open` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、内容可见但不可交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主状态与 `compact` 状态
2. 请求第一页截图
3. 程序化切换主卡到 `pane compact`
4. 请求第二页截图
5. 程序化切换主卡到 `review` 选择项
6. 请求第三页截图
7. 程序化切换主卡到 `archive` 选择项
8. 请求第四页截图
9. 程序化切换 `compact` 到 `review + pane open`
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡和底部双预览必须完整可见，不能被裁切
- rail、detail 区、正文和 footer 之间要保留稳定留白
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签
- `item / pane / compact / read only / view disabled` 切换后不能残留 rail row 或 toggle 的 `pressed` 高亮与下压位移渲染
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖更长列表和更复杂的多级 pane
- 当前不做 hover、焦点环、键盘辅助文案等桌面细节
- 当前 detail 文本仍是静态快照，不接入真实数据源
- 若后续要沉入框架层，再单独评估与导航容器、内容区域模型的衔接

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `nav_panel`：这里不仅有导航 rail，还有 detail 面板
- 相比 `data_list_panel`：这里强调 pane 展开/收起，而不只是列表选择
- 相比 `master_detail`：这里更靠近 `SplitView` 的收起侧栏语义
- 相比 `settings_panel`：这里是导航/内容双栏结构，而不是设置项分组卡

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`SplitView`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `pane open`
  - `pane compact`
  - `accent`
  - `warning`
  - `neutral`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section label 和 preview label
- 不做复杂阴影、场景化插画和装饰性背景层
- 不做 hover、pressed、focus ring 等完整桌面交互细节
- 不做多级导航树和复杂联动动画
- 不做页面外部状态桥接，只保留控件自身的 pane 与 selection 语义

## 14. EGUI 适配时的简化点与约束

- 使用固定 item 数组驱动，先保证 reference 展示稳定
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 主卡保留 pane 展开/收起与选择项切换，录制时改为程序化触发
- 先完成示例级 `split_view`，后续再决定是否沉入通用框架控件
