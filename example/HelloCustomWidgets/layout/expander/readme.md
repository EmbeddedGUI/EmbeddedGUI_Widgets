# expander 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`Expander` / `CardExpander`
- 本次保留状态：`expanded`、`collapsed`、`compact`、`read only`、`accent`、`success`、`warning`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、Acrylic、投影、Reveal/Hover 光效、系统级转场动画
- EGUI 适配说明：保留“标题行 disclosure + 展开 body”的核心语义，在 `480 x 480` 页面里优先保证主卡节奏与 `compact / read only` 对照稳定；`current / expanded / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件

`expander` 用来表达“点击标题行后展开说明内容，再次点击可收起”的标准 disclosure 结构。它适合设置说明、同步规则、帮助段落和审核说明等需要按需展开正文的页面内信息组织。

## 2. 为什么现有控件不够用

- `settings_panel` 强调设置行和 trailing value / switch，不负责正文展开
- `tree_view` 强调层级导航，不是单层 disclosure
- `master_detail` / `split_view` 强调双栏联动，不适合内联展开说明
- `card_panel` 只能展示固定摘要，缺少收放状态

因此这里继续保留 `expander`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `expander`，覆盖 `expanded / collapsed / warning` 等关键状态
- 左下 `compact` 预览展示紧凑版 disclosure 结构
- 右下 `read only` 预览展示不可交互的静态对照态
- 示例页只保留标题、主 `expander` 和底部 `compact / read only` 双预览，不再保留外部 guide、状态回显和标签点击

目录：

- `example/HelloCustomWidgets/layout/expander/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 226`
- 页面结构：标题 -> 主 `expander` -> `compact / read only` 双预览
- 主卡区域：`196 x 110`
- 底部双预览容器：`216 x 76`
- `compact` 预览：`104 x 76`
- `read only` 预览：`104 x 76`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音 disclosure 卡片
  - 主卡保留 header row、body text 与 footer chip 的标准层级
  - `compact` 版本压缩正文，只保留一行 body + footer
  - `read only` 通过控件自身弱化态表达，不依赖外部标签说明

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 226` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Expander` | 页面标题 |
| `panel_primary` | `egui_view_expander_t` | `196 x 110` | `workspace expanded` | 标准 expander 主卡 |
| `panel_compact` | `egui_view_expander_t` | `104 x 76` | `mode compact` | 紧凑预览 |
| `panel_read_only` | `egui_view_expander_t` | `104 x 76` | `audit read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `workspace expanded` | `mode expanded` | `audit expanded` |
| 轮换 1 | `sync expanded` | 保持 | 保持 |
| 轮换 2 | `sync collapsed` | 保持 | 保持 |
| 轮换 3 | `release expanded` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `backup expanded` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、内容可见但不可交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主状态与 `compact` 状态
2. 请求第一页截图
3. 程序化切换主卡到 `sync expanded`
4. 请求第二页截图
5. 程序化切换主卡到 `sync collapsed`
6. 请求第三页截图
7. 程序化切换主卡到 `release expanded`
8. 请求第四页截图
9. 程序化切换 `compact` 到 `backup expanded`
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡与底部双预览必须完整可见，不能被裁切
- header、body、footer 之间要保留稳定留白，collapsed 态不能留下异常空洞
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签
- `current / expanded / compact / read only / view disabled` 切换后不能残留 header 的 `pressed` 高亮或 disclosure chevron 下压位移
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖更长正文和多层嵌套 expander
- 当前不做动画，只保留结构状态切换
- 当前仍由静态 item 数据驱动，不接真实数据绑定
- 若后续要沉入框架层，再单独评估数据模型与过渡动画

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `settings_panel`：这里强调 inline disclosure，而不是设置行 trailing controls
- 相比 `tree_view`：这里没有多层级树结构，只有单层 accordion
- 相比 `master_detail` / `split_view`：这里不是双栏结构，而是页内纵向展开
- 相比 `card_panel`：这里自带展开 / 收起状态，不是固定摘要卡

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Expander` / `CardExpander`
- 本次保留状态：
  - `expanded`
  - `collapsed`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做桌面级阴影、Acrylic 和复杂 hover / reveal / transition 动效
- 不做多重嵌套 expander，仅保留单层 accordion 语义
- 不做真实图标资源和页面外部状态桥接

## 14. EGUI 适配时的简化点与约束

- 使用固定 item 数组驱动，先保证 reference 展示稳定
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 主卡保留展开 / 收起切换，录制时改为程序化触发
- 先完成示例级 `expander`，后续再决定是否沉入通用框架控件
