# persona_group 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`AvatarGroup`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`live`、`busy`、`away`、`idle`
- 删除效果：页面级 guide / 状态文案 / standard label / section label / preview label、真实头像图片、复杂阴影、装饰性叠层、hover/focus ring
- EGUI 适配说明：保留团队快照、焦点成员、presence 语义和 overflow 气泡，在 `480 x 480` 页面内优先保证成员关系清晰和对照阅读

## 1. 为什么需要这个控件

`persona_group` 用来展示一组协作成员，并让当前焦点成员驱动整张卡片的阅读重心。它适合出现在审阅链、责任人概览、协作面板和团队摘要里，表达“这一组成员属于同一条工作流”。

## 2. 为什么现有控件不够用

- `avatar_stack` 更偏装饰性叠层头像，不适合作为主线 reference 控件
- `badge_group` 表达的是状态标签，不适合承载成员身份、角色和 presence
- `card_panel` 更偏结构化信息卡，不适合做轻量成员群组
- 旧 showcase 方案大量依赖场景化说明文字和页面壳，不适合继续作为 Fluent 主线

因此这里继续保留 `persona_group`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `persona_group`，覆盖 `accent / warning / success / neutral` 焦点成员组合
- 左下 `compact` 预览展示小尺寸成员群组在单行卡片里的压缩布局
- 右下 `read only` 预览展示归档团队的被动展示态
- 示例页只保留标题、主 `persona_group` 和底部 `compact / read only` 双预览，不再保留外部 guide 和状态回显

目录：

- `example/HelloCustomWidgets/display/persona_group/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 230`
- 页面结构：标题 -> 主 `persona_group` -> `compact / read only` 双预览
- 主卡区域：`196 x 114`
- 底部双预览容器：`216 x 76`
- `compact` 预览：`104 x 76`
- `read only` 预览：`104 x 76`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音卡片
  - 头像、presence 点和 footer summary 都保持轻量表达，不做装饰性堆叠
  - 主卡允许焦点成员切换，但页面本身不再依赖外部状态文字解释
  - `compact` 与 `read only` 直接通过控件模式表达，不依赖外部标签说明

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 230` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Persona Group` | 页面标题 |
| `group_primary` | `egui_view_persona_group_t` | `196 x 114` | `design snapshot` | 标准成员群组 |
| `group_compact` | `egui_view_persona_group_t` | `104 x 76` | `team compact` | 紧凑预览 |
| `group_readonly` | `egui_view_persona_group_t` | `104 x 76` | `archive read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `design + accent focus` | `team compact` | `archive muted` |
| 轮换 1 | `design + warning focus` | 保持 | 保持 |
| 轮换 2 | `ops + success focus` | 保持 | 保持 |
| 轮换 3 | `archive + neutral focus` | 保持 | 保持 |
| 紧凑轮换 | 保持 | `team -> ops` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化、成员可见但不可交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与 `compact` 快照
2. 请求第一页截图
3. 程序化切换主卡焦点到 `warning` 成员
4. 请求第二页截图
5. 程序化切换主卡到 `ops` 快照
6. 请求第三页截图
7. 程序化切换主卡到 `archive` 快照
8. 请求第四页截图
9. 程序化切换 `compact` 到第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡和底部双预览必须完整可见，不能被裁切
- 头像、presence 点、标题、角色和 footer summary 之间要保留稳定留白
- 主卡与双预览都必须维持 `Fluent 2 / WPF UI` 低噪音浅色语义
- 页面中不再出现 guide、状态回显、standard label、section divider、`Compact` / `Read only` 外部标签
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前是固定尺寸 reference 实现，未覆盖真实头像图片和超长成员名
- 当前不做 hover、焦点环、键盘提示文案等桌面细节
- 当前 overflow 只保留简化的 `+n` 气泡，不接入真实更多成员面板
- 若后续要沉入框架层，再单独评估与头像资源系统、团队数据模型的衔接

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `avatar_stack`：这里不是装饰性头像叠层，而是标准成员群组
- 相比 `badge_group`：这里表达的是人员关系，不是状态标签
- 相比 `card_panel`：这里更轻、更扁平，重点在成员焦点与 presence，而不是结构化信息卡
- 相比 `notification_badge`：这里不是单个提醒泡，而是一组有主次关系的协作成员

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`AvatarGroup`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`
  - `live`
  - `busy`
  - `away`
  - `idle`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、standard label、section label 和 preview label
- 不做真实头像图片、阴影层叠和装饰性背景形状
- 不做 hover、pressed、focus ring 等完整桌面交互细节
- 不做额外成员详情弹层和复杂上下文菜单
- 不做场景化页面联动，只保留成员焦点与 presence 的核心语义

## 14. EGUI 适配时的简化点与约束

- 使用固定 snapshot + item 数组驱动，先保证 reference 展示稳定
- 每个 snapshot 最多 4 个成员，并通过 `+n` 处理 overflow
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照
- 先完成示例级 `persona_group`，后续再决定是否沉入通用框架控件
