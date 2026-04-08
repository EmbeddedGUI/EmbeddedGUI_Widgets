# annotated_scroll_bar 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 参考平台控件：`WinUI AnnotatedScrollBar`
- 次级补充参考：`scroll_bar`、`pips_pager`
- 对应组件名称：`AnnotatedScrollBar`
- 本次保留状态：`standard`、`keyboard step`、`marker jump`、`compact`、`read only`
- 删除效果：页面级 guide / 状态栏 / section label / 预览标签、hover tooltip、重叠装饰文案、场景化外壳
- EGUI 适配说明：保留固定指示线、语义 marker、左侧摘要卡与紧凑预览；录制态改为程序化切换 snapshot，不再依赖隐藏 label 点击

## 1. 为什么需要这个控件？

`annotated_scroll_bar` 用来表达长内容中的语义分段导航。它适合照片年份浏览、版本说明、审计日志和阶段清单这类需要“按 section 快速跳转”的界面，而不是普通比例滚动条。

## 2. 为什么现有控件不够用？

- `scroll_bar` 只表达 viewport 比例和当前位置，不表达 section marker 与注释语义
- `pips_pager` 是离散分页，不适合连续长列表定位
- `chapter_strip` 更接近章节切换，不是纵向 rail + 固定指示线
- 当前主线仍需要一版贴近 Fluent / WPF UI `AnnotatedScrollBar` 语义的 reference custom widget

## 3. 目标场景与示例概览

- 主卡展示标准 `annotated_scroll_bar`，覆盖 `Gallery rail`、`Release rail`、`Incident rail` 三组快照
- 左下预览展示 `Compact` 紧凑态，保留最小摘要与 rail
- 右下预览展示 `Read only` 弱化态，保持同一控件结构但冻结交互
- 示例页结构收敛为标题、主 `annotated_scroll_bar` 和 compact / read-only 双预览，不再保留页面级 guide、状态栏和 section label

目标目录：`example/HelloCustomWidgets/navigation/annotated_scroll_bar/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 260`
- 主卡片：`196 x 150`
- 底部双预览容器：`216 x 68`
- `Compact` / `Read only` 预览：`104 x 68`
- 视觉规则：
  - 使用浅灰 page panel + 白底低噪音卡片
  - 主卡保留 title、helper、摘要区、注释气泡和 rail，不再额外叠加外部说明壳层
  - section marker 与固定 indicator line 保持清晰，但整体回到低饱和 Fluent / WPF UI palette
  - `Compact` 与 `Read only` 直接通过控件模式区分，不再依赖外围标签

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 260` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Annotated Scroll Bar` | 页面标题 |
| `annotated_scroll_bar_primary` | `egui_view_annotated_scroll_bar_t` | `196 x 150` | `Gallery rail` | 标准主卡 |
| `annotated_scroll_bar_compact` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Compact / Focus` | 紧凑预览 |
| `annotated_scroll_bar_locked` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Read only / Ship` | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 | `Gallery / 2021` | `Compact / Focus` | `Read only / Ship` |
| 键盘步进 | `Down / + / End` 改变 offset 与当前 section | 不响应 | 不响应 |
| marker jump | 主卡 marker 直接跳到目标段 | 不演示 | 不响应 |
| rail drag | 主卡沿 rail 连续拖动 | 不演示 | 不响应 |
| 快照切换 | 录制态程序化切到 `Release rail` | 录制态程序化切到另一组 compact 快照 | 固定冻结 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与紧凑快照
2. 稳定后请求默认截图
3. 发送 `Down`，验证小步推进后的 section 更新
4. 请求第二张截图
5. 发送 `+`，验证跨 section 大步跳转
6. 请求第三张截图
7. 发送 `End`，验证尾部边界
8. 请求第四张截图
9. 程序化切到下一组主快照
10. 请求第五张截图
11. 程序化切到下一组 compact 快照
12. 请求最终截图

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主卡摘要区、注释气泡和 rail 必须完整可见，不能因为压缩高度而裁切
- 主卡底部 offset 文案需要保持可读，不再依赖外部状态栏补充说明
- `Compact` 与 `Read only` 需要在同一 palette 下保持清晰层级差异
- 页面不再出现 guide、状态栏、section label、preview label 这类外部 chrome

## 9. 已知限制与下一轮迭代计划

- 当前仍使用固定 snapshot 数据，不接真实滚动容器
- 当前不实现桌面端 hover tooltip 与复杂 reveal 动画
- marker label 仍采用轻量避让，不做复杂碰撞排版
- 当前示例优先验证 reference 语义与布局稳定性，不联动外部内容区

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `scroll_bar`：这里强调语义 marker 与注释，不强调 viewport 比例
- 相比 `pips_pager`：这里是连续长列表定位，不是离散分页
- 相比 `chapter_strip`：这里是纵向 rail 导航，不是横向章节切换
- 相比旧版 showcase 式页面：这里回到标准 Fluent reference 结构，不保留额外叙事壳层

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 平台控件：`WinUI AnnotatedScrollBar`

## 12. 对应组件名称，以及本次保留的核心状态

- 对应组件名称：`AnnotatedScrollBar`
- 本次保留：
  - `standard`
  - `keyboard step`
  - `marker jump`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、section label 与外部预览标签
- 不做 pointer hover 才出现的 tooltip 层
- 不接真实系统滚动容器和复杂联动动画
- 不做多列 annotation、复杂标签避让和高装饰场景化页面

## 14. EGUI 适配时的简化点与约束

- 使用固定 snapshot + marker 数据保证录制稳定
- 用控件内部摘要区承载当前 section 与 offset，不再依赖外部状态同步
- compact 与 read-only 直接复用同一控件模式，减少额外页面壳层
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再评估
