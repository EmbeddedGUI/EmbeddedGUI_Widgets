# data_list_panel 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`ListView` / `ItemsView`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、Acrylic、复杂 hover、虚拟滚动、桌面级列头拖拽
- EGUI 适配说明：保留数据行卡片、焦点行、title / summary / footer 语义与 `compact / read only` 对照，在 `480 x 480` 页面里优先保证信息密度与留白稳定；`selection / snapshot / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件

`data_list_panel` 用来表达标准化的数据行列表，适合任务队列、资产审阅、归档清单和同步记录等需要在一张卡片里快速浏览多条结构化条目的场景。

## 2. 为什么现有控件不够用

- `list` 只有基础列表能力，不强调 Fluent 风格的数据行结构和焦点行反馈
- `table` 更像网格数据，不适合低噪音列表卡片语义
- `settings_panel` 强调设置行与尾部控件，不是数据记录列表
- `master_detail` 强调双栏联动，而不是单卡片内的数据行浏览

因此这里继续保留 `data_list_panel`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `data_list_panel`，覆盖 `sync queue`、`asset review`、`archive sweep` 三组关键状态
- 左下 `compact` 预览展示小尺寸数据列表的高密度布局
- 右下 `read only` 预览展示弱化后的静态对照态
- 示例页只保留标题、主 `data_list_panel` 和底部 `compact / read only` 双预览，不再保留外部 guide、状态回显和标签点击

目录：

- `example/HelloCustomWidgets/layout/data_list_panel/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 236`
- 页面结构：标题 -> 主 `data_list_panel` -> `compact / read only` 双预览
- 主卡区域：`196 x 116`
- 底部双预览容器：`216 x 80`
- `compact` 预览：`104 x 80`
- `read only` 预览：`104 x 80`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音列表卡片
  - 主卡保留 eyebrow、title、summary、rows、footer 五层结构
  - 数据行只保留 `glyph + title + value` 的最小表达，不再外接状态文案桥接
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 236` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Data List` | 页面标题 |
| `panel_primary` | `egui_view_data_list_panel_t` | `196 x 116` | `sync queue` | 标准数据列表主卡 |
| `panel_compact` | `egui_view_data_list_panel_t` | `104 x 80` | `compact` | 紧凑预览 |
| `panel_read_only` | `egui_view_data_list_panel_t` | `104 x 80` | `read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `sync queue + accent row` | `compact recent` | `read only locked` |
| 轮换 1 | `sync queue + success row` | 保持 | 保持 |
| 轮换 2 | `asset review + warning row` | 保持 | 保持 |
| 轮换 3 | `archive sweep + warning row` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `compact recent -> compact review` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、内容可见但不可交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主状态与 `compact` 状态
2. 请求第一页截图
3. 程序化切换主卡到 `success` 行
4. 请求第二页截图
5. 程序化切换主卡到 `asset review` 的 `warning` 行
6. 请求第三页截图
7. 程序化切换主卡到 `archive sweep`
8. 请求第四页截图
9. 程序化切换 `compact` 到第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡和底部双预览必须完整可见，不能被裁切
- 行内 `glyph`、标题、右侧数值和 footer 之间要保留稳定留白
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签
- `selection / snapshot / compact / read only / view disabled` 切换后不能残留 row 的 `pressed` 高亮或下压位移渲染
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖更长数据行和滚动大列表
- 当前 value 只承载简短计数，不做复杂列头、排序和过滤
- 当前数据仍由静态 snapshot 驱动，不接入真实数据源
- 若后续要沉入框架层，再单独评估数据源接口、滚动容器和列表虚拟化能力

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `list`：这里强调标准数据行结构和焦点行视觉反馈
- 相比 `table`：这里不是网格表，而是卡片内的数据清单
- 相比 `settings_panel`：这里没有设置项尾部控件语义，重点是数据记录
- 相比 `master_detail`：这里不拆分详情面板，焦点信息通过列表自身表达

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`ListView` / `ItemsView`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做 Acrylic、复杂阴影、hover reveal 和虚拟滚动
- 不做列头拖拽、排序箭头和桌面级大列表行为
- 不做页面外部状态桥接，只保留控件自身的 row 与 snapshot 语义

## 14. EGUI 适配时的简化点与约束

- 使用固定 `snapshot + item` 数组驱动，先保证 reference 展示稳定
- 每组最多保留 3 行数据，优先保证 `480 x 480` 页面内的节奏稳定
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 主卡保留 row 切换与 snapshot 切换，录制时改为程序化触发
