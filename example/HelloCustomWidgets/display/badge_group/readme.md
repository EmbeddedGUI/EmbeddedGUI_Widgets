# badge_group 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`Badge`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`mixed group`
- 删除效果：页面级 guide / 状态文案 / section label / preview label、真实图标、复杂阴影、拖拽排序、hover/focus ring、长列表滚动、桌面级动画
- EGUI 适配说明：保留多 badge 组合、tone 混合、focus badge 和 footer summary，在 `480 x 480` 页面内优先保证结构稳定和对照阅读

## 1. 为什么需要这个控件

`badge_group` 用来展示一组语义相关的 badge，并让其中一个 focus badge 驱动整张卡片的 tone 和 footer summary。它适合出现在概览页、审阅页和状态面板里，表达“这一组标签是同一条信息的多个维度”。

## 2. 为什么现有控件不够用

- `notification_badge` 只解决单个角标或计数，不解决多 badge 并列展示
- `chips` 偏交互筛选和选中态，不适合作为静态信息组合
- `tag_cloud` 强调权重分布和散点云布局，不强调 focus badge 与 summary
- `card_panel` 更偏结构化信息卡，不适合做轻量 badge 集群

因此这里继续保留 `badge_group`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `badge_group`，覆盖 `accent / success / warning / neutral` 四组 snapshot
- 左下 `compact` 预览展示小尺寸 badge 组合在单行里的压缩布局
- 右下 `read only` 预览展示 tone 弱化后的被动展示态
- 示例页只保留标题、主 `badge_group` 和底部 `compact / read only` 双预览，不再保留外部 guide 和状态回显

目录：

- `example/HelloCustomWidgets/display/badge_group/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 242`
- 页面结构：标题 -> 主 `badge_group` -> `compact / read only` 双预览
- 主卡区域：`196 x 118`
- 底部双预览容器：`216 x 84`
- `compact` 预览：`104 x 84`
- `read only` 预览：`104 x 84`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音卡片
  - focus badge 的 tone 只在控件内部驱动 eyebrow、summary 和 badge 强调
  - 主体 badge 允许 filled / outlined 混排，但整体不做过强装饰
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 242` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Badge Group` | 页面标题 |
| `group_primary` | `egui_view_badge_group_t` | `196 x 118` | `accent` | 标准 badge 组合卡 |
| `group_compact` | `egui_view_badge_group_t` | `104 x 84` | `accent compact` | 紧凑预览 |
| `group_locked` | `egui_view_badge_group_t` | `104 x 84` | `neutral locked` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `accent + mixed badges` | `accent compact` | `neutral locked` |
| 轮换 1 | `success focus` | 保持 | 保持 |
| 轮换 2 | `warning focus` | 保持 | 保持 |
| 轮换 3 | `neutral focus` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `accent -> warning` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、对比降低、无交互强调 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与 `compact` 快照
2. 请求第一页截图
3. 程序化切换主卡到 `success`
4. 请求第二页截图
5. 程序化切换主卡到 `warning`
6. 请求第三页截图
7. 程序化切换主卡到 `neutral`
8. 请求第四页截图
9. 程序化切换 `compact` 到第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡和底部双预览必须完整可见，不能被裁切
- 主卡的 badge 组合最多两行，不能压到 footer summary
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、section divider、`Compact` / `Read only` 外部标签
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖超长文案和超过 6 个 badge 的数据
- 当前不做真实图标、可关闭按钮、键盘导航和 hover/focus ring
- 当前 badge 宽度估算基于简化字符宽度，不是完整文本测量系统
- 若后续要沉入框架层，再单独评估 `src/widget/` 抽象和更通用的数据接口

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `notification_badge`：这里不是单个计数泡，而是一组可混合 tone 的 badge 集群
- 相比 `chips`：这里不是交互筛选条，不强调选中、取消和筛选结果
- 相比 `tag_cloud`：这里不是权重词云，不做自由散点布局和字号权重编码
- 相比 `card_panel`：这里更轻、更扁平，重点在 badge 组合与 focus summary，而不是结构化信息卡

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Badge`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`
  - `mixed group`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、section label 和 preview label
- 不做真实图标、头像、关闭动作和上下文菜单
- 不做真实阴影层叠、hover 动效和高频过渡动画
- 不做动态换行列表、拖拽排序和可滚动 badge 池
- 不做复杂页面联动，只保留 focus badge 驱动整体 summary 的核心语义

## 14. EGUI 适配时的简化点与约束

- 用固定 snapshot + item 数组驱动，先保证 reference 展示稳定
- 每个 snapshot 最多 6 个 badge，先满足示例级展示，不追求无限扩展
- `compact` 与 `read only` 固定放底部双列，便于与主卡直接对照
- 先完成示例级 `badge_group`，后续再决定是否抽象为通用框架控件
