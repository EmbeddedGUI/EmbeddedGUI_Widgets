# master_detail 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`MasterDetail`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、Acrylic、复杂 hover、系统级转场动画
- EGUI 适配说明：保留 master 列表驱动 detail 面板的核心语义，在 `480 x 480` 页面里优先保证双栏结构与 `compact / read only` 对照稳定；`selection / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义，底部 preview 统一通过 `egui_view_master_detail_override_static_preview_api()` 固定为静态 reference

## 1. 为什么需要这个控件

`master_detail` 用于在同一块页面区域里同时呈现“列表选择”和“详情阅读”两层信息。它适合文件库、成员列表、审核队列和内容分组等需要先选中条目，再在同屏查看对应摘要的场景。

## 2. 为什么现有控件不够用

- `nav_panel` 解决的是导航入口，不强调右侧详情阅读
- `settings_panel` 强调设置项行和尾部控件，不是 master-detail 双栏结构
- `card_panel` 更像单卡片信息摘要，没有左侧列表驱动
- `list`、`table` 只能列出条目，缺少同屏 detail 面板语义

因此这里继续保留 `master_detail`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `master_detail`，覆盖 `files`、`review`、`members`、`archive` 四组关键状态
- 左下 `compact` 预览展示小尺寸 master-detail 的缩窄布局
- 右下 `read only` 预览展示不可交互的静态对照态
- 示例页只保留标题、主 `master_detail` 和底部 `compact / read only` 双预览，不再保留外部 guide、状态回显和标签点击
- 底部两个 preview 统一通过 `egui_view_master_detail_override_static_preview_api()` 吞掉 `touch / key`，点击 preview 时只清主控件 `panel_primary` 的 focus

目录：

- `example/HelloCustomWidgets/layout/master_detail/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 208`
- 页面结构：标题 -> 主 `master_detail` -> `compact / read only` 双预览
- 主卡区域：`196 x 96`
- 底部双预览容器：`216 x 72`
- `compact` 预览：`104 x 72`
- `read only` 预览：`104 x 72`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音 master-detail 容器
  - 主卡保留左侧列表、中央分隔与右侧 detail pane 的标准层级
  - `compact` 预览压缩 master rail 和 detail 文本，不再依赖外部标签说明
  - `read only` 通过控件自身弱化态表达，不再依赖页面状态桥接

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 208` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Master Detail` | 页面标题 |
| `panel_primary` | `egui_view_master_detail_t` | `196 x 96` | `files` | 标准 master-detail 主卡 |
| `panel_compact` | `egui_view_master_detail_t` | `104 x 72` | `files compact` | 紧凑预览 |
| `panel_read_only` | `egui_view_master_detail_t` | `104 x 72` | `members read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `files + accent` | `files compact` | `members read only` |
| 轮换 1 | `review + warning` | 保持 | 保持 |
| 轮换 2 | `members + success` | 保持 | 保持 |
| 轮换 3 | `archive + neutral` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `files -> review` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、内容可见但不可交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主状态与 `compact` 状态，并给主控件请求 focus
2. 请求第一页截图
3. 程序化切换主卡到 `review`
4. 请求第二页截图
5. 程序化切换主卡到 `members`
6. 请求第三页截图
7. 程序化切换主卡到 `archive`
8. 请求第四页截图
9. 程序化切换 `compact` 到 `review`
10. 请求第五张截图
11. 再次给主控件请求 focus
12. 点击 `compact` preview，只验证 focus 收尾
13. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/master_detail --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡与底部双预览必须完整可见，不能被裁切
- 主卡左侧列表、中央分隔和右侧 detail pane 之间要保留稳定留白
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签
- `selection / compact / read only / view disabled` 切换后不能残留 master row 的 `pressed` 高亮或下压位移渲染
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染
- 底部预览必须统一通过 `egui_view_master_detail_override_static_preview_api()` 吞掉 `touch / key`，且不能改当前选中项
- 点击底部 preview 时只允许清主控件 `panel_primary` 的 focus，最终收尾帧不能出现焦点残留、黑白屏或异常重排

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖更长列表和长文本详情
- 当前 detail 正文仍是静态快照，不接入真实数据源
- 当前不做 hover、focus ring 与系统级转场动画
- 若后续要沉入框架层，再单独评估数据绑定、滚动与详情区域模型

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `nav_panel`：这里强调列表驱动的详情阅读，不承担页面导航容器职责
- 相比 `settings_panel`：这里没有设置项 value cell、switch、chevron 语义
- 相比 `card_panel`：这里不是单卡摘要，而是双栏联动结构
- 相比 `list` / `table`：这里强调“当前选中项 + detail pane”的同步关系

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`MasterDetail`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做 Acrylic、复杂阴影、hover reveal 和系统级转场
- 不做完整页面容器联动，只保留控件自身的 selection 与 detail 语义
- 不做桌面级长文本与复杂数据源驱动

## 14. EGUI 适配时的简化点与约束

- 使用固定 item 数组驱动，先保证 reference 展示稳定
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 主卡保留 selection 切换，录制时改为程序化触发
- 底部 `compact / read only` 统一通过 `egui_view_master_detail_override_static_preview_api()` 固定为静态对照
- 先完成示例级 `master_detail`，后续再决定是否沉入通用框架控件
