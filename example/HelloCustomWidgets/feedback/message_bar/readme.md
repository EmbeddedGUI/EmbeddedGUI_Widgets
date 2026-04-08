# message_bar 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名称：`MessageBar / InfoBar`
- 本次保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 删除效果：页面级 guide / 状态栏 / section label / 预览标签、Acrylic、系统级阴影、复杂关闭动效、场景化页面壳层
- EGUI 适配说明：保留 severity accent、leading glyph、标题 / 正文 / 动作层级和只读弱化态；录制态通过程序化 snapshot 切换覆盖主要状态

## 1. 为什么需要这个控件？

`message_bar` 用来表达页面级或容器级的轻量反馈消息，覆盖 `info / success / warning / error` 四种常见状态，适合设置页、表单页、同步页和后台管理页顶部的提示场景。

## 2. 为什么现有控件不够用？

- `toast_stack` 更偏叠卡式临时通知，不是单条页内反馈条
- `dialog_sheet` 是阻塞式弹层，不适合轻量常驻提示
- `badge_group` 只能表达汇总提醒，不承载正文与动作
- 当前主线仍需要一版贴近 Fluent / WPF UI `MessageBar / InfoBar` 语义的 reference custom widget

## 3. 目标场景与示例概览

- 主卡展示标准 `message_bar`，覆盖 `Info / Success / Warning / Error` 四态
- 左下预览展示 `Compact` 紧凑态，保留标题、正文和动作按钮
- 右下预览展示 `Read only` 弱化态，隐藏关闭与动作能力
- 示例页结构收敛为标题、主 `message_bar` 和 compact / read-only 双预览，不再保留 guide、状态栏和 section label

目标目录：`example/HelloCustomWidgets/feedback/message_bar/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主卡片：`196 x 96`
- 底部双预览容器：`216 x 82`
- `Compact` / `Read only` 预览：`104 x 82`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音 message bar
  - 左侧 severity accent 与 leading glyph 保留清晰层级，但整体回到中性浅色 Fluent / WPF UI 语法
  - 主卡保留标题、正文、动作按钮和关闭位
  - `Compact` 与 `Read only` 直接通过控件模式表达，不再依赖外围标签

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 214` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Message Bar` | 页面标题 |
| `bar_primary` | `egui_view_message_bar_t` | `196 x 96` | `Info` | 标准主卡 |
| `bar_compact` | `egui_view_message_bar_t` | `104 x 82` | `Warning` | 紧凑预览 |
| `bar_locked` | `egui_view_message_bar_t` | `104 x 82` | `Read only` | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 | `Info` | `Warning` | `Info pinned` |
| 切换 1 | `Success` | 保持 | 保持 |
| 切换 2 | `Warning` | 保持 | 保持 |
| 切换 3 | `Error` | 保持 | 保持 |
| 紧凑切换 | 保持 | `Warning -> Error` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 弱化 accent、隐藏 action / close |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与紧凑快照
2. 稳定后请求默认截图
3. 程序化切到 `Success`
4. 请求第二张截图
5. 程序化切到 `Warning`
6. 请求第三张截图
7. 程序化切到 `Error`
8. 请求第四张截图
9. 程序化切到 `Compact` 第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/message_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/message_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡与底部双预览必须完整可见，不能裁切
- `Info / Success / Warning / Error` 四态需要一眼可分，但整体不能回到高饱和 showcase 风格
- 标题、正文、动作按钮与关闭位之间的留白必须稳定
- `Compact` 在小尺寸下仍需保留 message bar 的基本层级
- 页面不再出现 guide、状态栏、section label、preview label 这类外部 chrome

## 9. 已知限制与下一轮迭代计划

- 当前版本使用固定 snapshot 数据，不接真实业务状态流
- 关闭位当前只保留视觉语义，不单独演示关闭动作
- 不做多动作按钮、展开正文和复杂布局换行
- 当前示例优先验证 reference 语义与布局稳定性，不联动其它反馈容器

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `toast_stack`：这里强调单条页内反馈，而不是多条叠卡通知
- 相比 `dialog_sheet`：这里是轻量反馈，不阻塞页面
- 相比 `badge_group`：这里承载标题、正文、动作和关闭语义
- 相比旧版 showcase 告警条：这里回到标准 Fluent reference 结构，不保留叙事式页面壳层

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名称，以及本次保留的核心状态

- 对应组件名称：`MessageBar / InfoBar`
- 本次保留：
  - `info`
  - `success`
  - `warning`
  - `error`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、section label 与外部预览标签
- 不做 Acrylic、系统级阴影与复杂关闭动效
- 不做真实图标资源和多动作按钮组合
- 不做可展开正文与更复杂的桌面端交互细节

## 14. EGUI 适配时的简化点与约束

- 使用固定尺寸和固定 snapshot 保证 `480 x 480` 下可审阅性
- 使用 Montserrat 内置字体，不引入额外字体资源
- 通过中性浅色边框和克制的 severity accent 保持 Fluent / WPF UI 主线
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再评估
