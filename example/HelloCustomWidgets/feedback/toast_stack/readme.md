# toast_stack 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名称：`Toast / Snackbar`
- 本次保留状态：`info`、`success`、`warning`、`error`、`compact`、`read only`
- 删除效果：页面级 guide / 状态栏 / section label / 预览标签、系统级阴影、Acrylic、自动入场退场动画、场景化页面壳层
- EGUI 适配说明：保留轻量叠卡、左侧 severity accent、标题 / 正文 / 动作 / meta 层级；录制态通过程序化 snapshot 切换覆盖主要状态

## 1. 为什么需要这个控件？

`toast_stack` 用来表达页内临时通知的叠卡语义，适合设置页、同步页和工作台首页展示最近 2 到 3 条轻量反馈。它不是全屏弹窗，也不是单条横幅，而是更贴近 Fluent 的轻量 toast / snackbar 组合。

## 2. 为什么现有控件不够用？

- `message_bar` 更偏单条页内反馈，不强调连续 toast 的前后层级
- `dialog_sheet` 是阻塞式弹层，不适合轻量临时消息
- `badge_group` 只能表达汇总提醒，不承载正文、动作和时间信息
- 当前主线仍需要一版贴近 Fluent / WPF UI 的叠卡式 `Toast` reference custom widget

## 3. 目标场景与示例概览

- 主卡展示标准 `toast_stack`，覆盖 `info / success / warning / error` 四态
- 左下预览展示 `Compact` 紧凑态，保留前卡与两层摘要
- 右下预览展示 `Read only` 弱化态，隐藏动作能力并冻结展示
- 示例页结构收敛为标题、主 `toast_stack` 和 compact / read-only 双预览，不再保留外部 guide、状态栏和 section label

目标目录：`example/HelloCustomWidgets/feedback/toast_stack/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主卡片：`196 x 108`
- 底部双预览容器：`216 x 83`
- `Compact` / `Read only` 预览：`104 x 83`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音 toast card
  - 前卡保留 severity strip、标题、正文、动作 pill 和 meta pill
  - 后两层卡只保留摘要标题与层级偏移，不叠加页面外说明
  - palette 统一回中性浅色 Fluent / WPF UI 语法，不保留额外彩色标签壳层

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toast Stack` | 页面标题 |
| `stack_primary` | `egui_view_toast_stack_t` | `196 x 108` | `Backup ready` | 标准主卡 |
| `stack_compact` | `egui_view_toast_stack_t` | `104 x 83` | `Quota alert` | 紧凑预览 |
| `stack_locked` | `egui_view_toast_stack_t` | `104 x 83` | `Review ready` | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 | `info` | `warning` | `success locked` |
| 切换 1 | `success` | 保持 | 保持 |
| 切换 2 | `warning` | 保持 | 保持 |
| 切换 3 | `error` | 保持 | 保持 |
| 紧凑切换 | 保持 | `warning -> error` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 弱化 accent、隐藏 action / close |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与紧凑快照
2. 稳定后请求默认截图
3. 程序化切到 `success`
4. 请求第二张截图
5. 程序化切到 `warning`
6. 请求第三张截图
7. 程序化切到 `error`
8. 请求第四张截图
9. 程序化切到 `Compact` 第二组快照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/toast_stack PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/toast_stack --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡、后两层卡和底部双预览都必须完整可见，不能裁切
- `info / success / warning / error` 四态差异要清晰，但整体不能回到高饱和 showcase 风格
- 标题、正文、action pill 和 meta pill 之间要保持稳定留白
- `Compact` 与 `Read only` 需要在同一 palette 下维持清晰层级差异
- 页面不再出现 guide、状态栏、section label、preview label 这类外部 chrome

## 9. 已知限制与下一轮迭代计划

- 当前仍使用固定 snapshot 数据，不接真实通知队列
- 当前不做自动弹入 / 弹出动画，也不做滑动关闭手势
- 当前不引入真实图标资源，只保留 severity strip 与圆点
- 当前示例优先验证 reference 语义与布局稳定性，不联动全局通知中心

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `message_bar`：这里强调多条 toast 的叠卡关系，而不是单条页内反馈
- 相比 `dialog_sheet`：这里是轻量反馈，不阻塞页面
- 相比 `badge_group`：这里承载正文、动作和时间信息，不只是数量提示
- 相比旧版 showcase 通知控件：这里回到标准 Fluent reference 结构，不保留额外叙事壳层

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名称，以及本次保留的核心状态

- 对应组件名称：`Toast / Snackbar`
- 本次保留：
  - `info`
  - `success`
  - `warning`
  - `error`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、section label 与外部预览标签
- 不做系统级阴影、Acrylic 和自动入场退场动画
- 不做真实图标资源与复杂手势关闭
- 不做通知队列计数、折叠、hover 和其它桌面端高装饰效果

## 14. EGUI 适配时的简化点与约束

- 使用固定叠卡偏移量与 snapshot 数据，优先保证 `480 x 480` 下可审阅性
- 用低噪音边框和少量色彩混合表达层级，避免回到 HMI / showcase 风格
- compact 与 read-only 直接放在底部双列，对照主卡状态变化
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再评估
