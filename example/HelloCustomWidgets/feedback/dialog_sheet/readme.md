# dialog_sheet 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`ContentDialog`
- 本次保留状态：`accent`、`success`、`warning`、`error`、`compact`、`read only`
- 删除效果：页面级 guide / 状态文案 / section label / preview label、系统级模糊、复杂阴影、动画进出场、拖拽手势关闭
- EGUI 适配说明：保留低噪音遮罩、sheet 化卡片、hero area、tag、footer summary 和 primary / secondary action 语义，在 `480 x 480` 下优先保证主卡与底部双预览的可读性

## 1. 为什么需要这个控件

`dialog_sheet` 用来表达轻量弹层确认语义，适合设置页、发布页、同步页里的二次确认场景。它不是全屏对话框，也不是页内横幅，而是更接近 `Fluent 2 / WPF UI ContentDialog` 的低噪音 sheet 收口。

## 2. 为什么现有控件不够用

- `message_bar` 偏页内单条反馈，不承担明确的动作确认
- `toast_stack` 偏临时通知叠卡，不适合做主次动作确认
- `card_panel` 偏信息结构展示，不包含遮罩语义和动作收口
- 当前主线仍需要一版统一到 `Fluent 2 / WPF UI` 的浅色 dialog reference

因此这里继续保留 `dialog_sheet`，但示例页必须收敛为当前 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `dialog_sheet`，覆盖 `warning / error / accent / success`
- 左下 `compact` 预览展示更窄尺寸下的单动作 sheet
- 右下 `read only` 预览展示锁定后的弱化态
- 示例页只保留标题、主 `dialog_sheet` 和底部 `compact / read only` 双预览，不再保留外部说明 chrome

目录：

- `example/HelloCustomWidgets/feedback/dialog_sheet/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 258`
- 页面结构：标题 -> 主 `dialog_sheet` -> `compact / read only` 双预览
- 主卡区域：`196 x 132`
- 底部双预览容器：`216 x 86`
- `compact` 预览：`104 x 86`
- `read only` 预览：`104 x 86`
- 视觉规则：
  - 使用浅灰 page panel + 低对比 overlay，避免回到旧 HMI / showcase 风格
  - sheet 卡片保留 handle、hero area、eyebrow、title、body、tag、footer summary 和 action row
  - `compact` 只保留最必要的结构，不再依赖外部标签解释
  - `read only` 保留完整结构，但弱化 tone 和动作能力

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 258` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Dialog Sheet` | 页面标题 |
| `sheet_primary` | `egui_view_dialog_sheet_t` | `196 x 132` | `warning` | 标准主卡 |
| `sheet_compact` | `egui_view_dialog_sheet_t` | `104 x 86` | `warning compact` | 紧凑预览 |
| `sheet_locked` | `egui_view_dialog_sheet_t` | `104 x 86` | `read only` | 锁定预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `warning / primary focus` | `warning / primary focus` | `neutral / locked` |
| 切换 1 | `error / secondary focus` | 保持 | 保持 |
| 切换 2 | `accent / primary focus` | 保持 | 保持 |
| 切换 3 | `success / primary focus` | 保持 | 保持 |
| 紧凑切换 | 保持 | `warning -> accent` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 弱化 tone、禁止触摸和键盘切换 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与 `compact` 快照
2. 请求第一页截图
3. 程序化切换主卡到 `error`
4. 请求第二页截图
5. 程序化切换主卡到 `accent`
6. 请求第三页截图
7. 程序化切换主卡到 `success`
8. 请求第四页截图
9. 程序化切换 `compact` 到第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- overlay、sheet、hero area、footer tag 和 action row 都必须完整可见
- `warning / error / accent / success` 四态差异要清晰，但整体不能回到高噪音 showcase 风格
- 页面中不再出现 guide、状态回显、section divider、`Compact` / `Read-only` 外部标签
- `compact` 和 `read only` 必须在同一套中性浅色 palette 下维持可辨识差异
- 底部预览不再承担交互职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前版本是固定尺寸 reference 实现，还未覆盖更长正文和更长动作文案
- 当前不做真实 modal 动画、遮罩渐变和关闭行为
- 当前不做真实图标资源，仅保留 tone glyph
- 后续若需要沉入框架层，再单独评估 `src/widget/` 抽象和 UI Designer 接入

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `message_bar`：这里强调遮罩下的确认层，而不是页内反馈条
- 相比 `toast_stack`：这里强调主次动作收口，而不是通知叠卡
- 相比 `card_panel`：这里保留 modal sheet 语义和 footer action row
- 相比旧版 showcase 弹层：这里回到标准 reference 结构，不再保留额外场景化页面壳

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`ContentDialog`
- 本次保留状态：
  - `warning`
  - `error`
  - `accent`
  - `success`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、section label 和 preview label
- 不做系统级模糊和 Acrylic
- 不做复杂阴影扩散和动效进出场
- 不做真实 close 交互和拖拽下拉关闭
- 不做图标资源、复选框、辅助链接等扩展内容

## 14. EGUI 适配时的简化点与约束

- 用固定尺寸 sheet 和低对比 overlay 表达对话层，优先保证 `480 x 480` 下可审阅
- 以 `hero + title + body + footer + actions` 五段式结构表达语义，不引入额外页面 chrome
- `compact` 和 `read only` 固定放到底部双列，方便与主卡直接对照
- 先完成示例级 `dialog_sheet` reference，再决定是否沉入通用框架控件
