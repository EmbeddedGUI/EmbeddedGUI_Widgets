# teaching_tip 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI TeachingTip`
- 对应组件名：`TeachingTip`
- 保留状态：anchored target、callout surface、top / bottom placement、compact / read-only 对照
- 删除效果：页面级 guide / 状态桥接 / preview label、系统级 popup 定位、复杂阴影、Acrylic、真实图标资源、入场动画
- EGUI 适配说明：保留“目标锚点 + 提示气泡 + 轻量动作行”的核心语义，收敛成页内稳定锚定的 reference 版本

## 1. 为什么需要这个控件

`teaching_tip` 用来表达“围绕某个页面目标做上下文引导”的提示语义，适合首次引导、功能提示、快捷键提示和发布前提醒等场景。

## 2. 为什么现有控件不够用

- `message_bar` 是页内横向反馈条，不围绕具体目标锚定
- `dialog_sheet` 是收口型对话层，不是贴近控件的上下文提示
- `toast_stack` 偏临时通知，不承担目标指引语义
- `menu_flyout` 是命令面板，不是教学提示

因此这里继续保留 `teaching_tip`，但页面壳必须回到当前统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `TeachingTip`，保留 target、primary、secondary、close 与键盘焦点闭环
- 左下 `compact` 预览展示缩小版 coachmark
- 右下 `read only` 预览展示只读弱化版提示
- 示例页只保留标题、主卡和底部 `compact / read only` 双预览，不再保留 guide、状态桥接或外部预览标签

目录：

- `example/HelloCustomWidgets/feedback/teaching_tip/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 252`
- 页面结构：标题 -> 主 `teaching_tip` -> `compact / read only` 双预览
- 主卡尺寸：`196 x 132`
- 底部双预览容器：`216 x 80`
- `compact` / `read only` 预览：`104 x 80`
- 视觉规则：
  - 使用浅色 page panel 和低噪音边框
  - target 与 bubble 的锚定关系必须明确
  - 标准态保留 action row 和 close affordance，但不再依赖外部状态文案解释
  - `compact` 与 `read only` 在同一套中性浅色 palette 下做差异化表达

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Teaching Tip` | 页面标题 |
| `tip_primary` | `egui_view_teaching_tip_t` | `196 x 132` | accent / bottom / open | 标准 teaching tip |
| `tip_compact` | `egui_view_teaching_tip_t` | `104 x 80` | compact | 紧凑预览 |
| `tip_locked` | `egui_view_teaching_tip_t` | `104 x 80` | read only | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `accent / bottom / open` | `accent / compact` | `neutral / read only` |
| 切换 1 | `accent / top / open` | 保持 | 保持 |
| 切换 2 | `warning / bottom / open` | 保持 | 保持 |
| 关闭态 | `neutral / closed helper` | 保持 | 保持 |
| 重开态 | `accent / bottom / reopen` | 保持 | 保持 |
| 紧凑切换 | 保持 | `accent -> success` | 保持 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 请求默认 `bottom placement` 截图
2. 点击主卡 target
3. 点击主卡 primary
4. 程序化切换到 `top placement`
5. 点击主卡 secondary
6. 程序化切换到 `warning` teaching tip
7. 点击主卡 primary
8. 点击 close 进入关闭态
9. 点击 target 重新展开
10. 通过 `Right / Right / Escape` 回放键盘焦点迁移
11. 程序化切换底部 `compact` 到第二组快照
12. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- target 与 bubble 的锚定关系必须清楚
- `top / bottom placement` 必须能从截图中直接辨认
- 关闭态与重开态必须稳定，不依赖外部状态桥接
- 页面中不再出现 guide、状态桥接、section divider、`Compact` / `Read-only` 外部标签
- `compact` 与 `read only` 仅作对照展示，不再承担切换职责

## 9. 已知限制与下一轮迭代计划

- 当前版本是页内固定锚点 reference，不做真实 popup 跟随
- 当前不做自动避让和屏幕边缘翻转策略
- 当前不做多段正文换行和真实图标资源
- 当前录制仍通过控件内部导航 helper 回放键盘路径

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `message_bar`：这里是围绕目标的上下文提示，不是横向反馈条
- 相比 `dialog_sheet`：这里强调 anchored callout，不是居中确认层
- 相比 `toast_stack`：这里强调教学引导，不是短暂消息
- 相比 `menu_flyout`：这里不呈现命令列表，核心是提示与动作收口

## 11. 参考设计系统与开源母本

- `Fluent 2`：提供低噪音教学提示语义
- `WPF UI`：提供 Windows Fluent 风格参考
- `WinUI TeachingTip`：补充 placement 与 target 锚定关系

## 12. 对应组件名与保留核心状态

- 对应组件名：`TeachingTip`
- 本次保留的核心状态：
  - target pill
  - open callout surface
  - top / bottom placement
  - primary / secondary / close 动作位
  - compact 对照态
  - read-only 对照态

## 13. 相比参考原型删掉的效果或装饰

- 不做页面级 guide、状态桥接和 preview label
- 不做系统级 popup 定位与边界翻转
- 不做复杂阴影和入场动画
- 不做 Acrylic 与真实图标资源
- 不做多目标链式引导

## 14. EGUI 适配时的简化点与约束

- 固定在 `480 x 480` 下优化，优先保证锚定关系和文本可读性
- 用固定 target / bubble 组合表达 reference 语义，不增加额外页面 chrome
- `compact` 与 `read only` 固定放在底部双列预览
- 先完成 `HelloCustomWidgets` 版本，后续再决定是否沉淀到框架层
