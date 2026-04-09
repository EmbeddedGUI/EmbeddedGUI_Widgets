# annotated_scroll_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AnnotatedScrollBar`
- 补充对照控件：`scroll_bar`、`pips_pager`
- 对应组件名：`AnnotatedScrollBar`
- 本次保留状态：`standard`、`keyboard step`、`marker jump`、`rail drag`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态栏、外部 preview 标签、双列 preview 包裹壳、过重 bubble 装饰、过强 rail chrome
- EGUI 适配说明：继续复用仓库内 `annotated_scroll_bar` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和绘制强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`annotated_scroll_bar` 用来表达长内容中的语义分段导航。它适合年表、版本里程碑、审计记录、归档列表这类“需要按 section 快速跳转，但又不是分页切换”的界面。

## 2. 为什么现有控件不够用

- `scroll_bar` 只表达比例和当前位置，不表达 marker 与注释语义。
- `pips_pager` 是离散页切换，不适合连续长列表定位。
- `breadcrumb_bar` 负责路径层级，不负责纵向 rail 导航。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `AnnotatedScrollBar` 语义的 custom widget。

## 3. 目标场景与示例概览

- 主控件展示标准 `AnnotatedScrollBar`，覆盖 `Gallery rail`、`Release rail`、`Incident rail` 三组主 snapshot。
- 底部左侧展示 `compact` 静态对照，只保留最小摘要和 rail。
- 底部右侧展示 `read only` 静态对照，保留相同结构但冻结交互。
- 页面结构统一收口为：标题 -> 主 `annotated_scroll_bar` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和额外页面 chrome 已移除，底部两张 preview 直接挂在同一行容器下。

目标目录：`example/HelloCustomWidgets/navigation/annotated_scroll_bar/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 260`
- 主控件尺寸：`196 x 150`
- 底部对照行尺寸：`216 x 68`
- `compact` 预览：`104 x 68`
- `read only` 预览：`104 x 68`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白色主卡、低噪音浅边框。
  - 主控件继续保留 title、helper、summary、bubble、marker label 和 rail 语义，但统一压轻边框、填充和 focus。
  - `compact` 与 `read only` 直接依赖控件模式区分，不再依赖外部标签或包裹卡片。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `annotated_scroll_bar_primary` | `egui_view_annotated_scroll_bar_t` | `196 x 150` | `Gallery rail` | 主 `AnnotatedScrollBar` |
| `annotated_scroll_bar_compact` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Compact / Focus` | 底部 compact 静态对照 |
| `annotated_scroll_bar_read_only` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Read only / Ship` | 底部 read only 静态对照 |
| `primary_snapshots` | `annotated_scroll_bar_snapshot_t[3]` | - | `Gallery / Release / Incident` | 主控件录制轨道 |
| `compact_snapshots` | `annotated_scroll_bar_snapshot_t[2]` | - | `Mix / Docs` | compact 预览程序化切换 |
| `read_only_snapshot` | `annotated_scroll_bar_snapshot_t` | - | `Static preview` | read only 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Gallery rail` | 默认主 snapshot，保留完整 title / helper / bubble / marker label |
| 主控件 | `Down` | 验证小步进和 active marker 更新 |
| 主控件 | `+` | 验证大步进跳转 |
| 主控件 | `End` | 验证尾部定位和 rail 终点表现 |
| 主控件 | `Release rail` | 程序化切换主 snapshot，验证 summary / bubble / rail 同步 |
| `compact` | `Mix` | 默认 compact 对照 |
| `compact` | `Docs` | 第二条 compact 轨道，只做程序化切换 |
| `read only` | `Static preview` | 固定只读轨道，禁 touch、禁 focus、禁键盘 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 请求默认截图。
3. 发送 `Down`，验证小步进后的 section 更新。
4. 请求第二张截图。
5. 发送 `+`，验证大步进跳转。
6. 请求第三张截图。
7. 发送 `End`，验证尾部状态。
8. 请求第四张截图。
9. 程序化切换到下一组主 snapshot。
10. 请求第五张截图。
11. 程序化切换 `compact` 到第二组 snapshot。
12. 请求最终截图。

## 8. 编译、touch、runtime、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件的 summary、bubble、marker label、indicator 和按钮都必须完整可见。
- 底部 `compact / read only` 需要在同一底色体系下保持清晰层级，但不能重新长出外部壳。
- `read only` 预览只能做静态展示，不能响应 touch、focus 或键盘。
- 触摸释放语义必须继续满足“按下与抬起命中同一目标才提交”。
- unit test 中已有的键盘导航、marker 跳转、rail drag 与只读/紧凑忽略输入语义不能回归。

## 9. 已知限制与后续方向

- 当前仍使用固定 snapshot 数据，不接真实滚动容器。
- 当前不实现桌面端 hover tooltip 和复杂 reveal 动画。
- marker label 仍采用轻量避让规则，不做复杂碰撞排版。
- 当前优先验证 reference 语义、布局和交互闭环，不联动外部内容区。

## 10. 与现有控件的边界

- 相比 `scroll_bar`：这里强调 marker 与注释，不强调 viewport 比例。
- 相比 `pips_pager`：这里是连续长列表定位，不是离散分页。
- 相比 `breadcrumb_bar`：这里是纵向 rail 导航，不是路径层级。
- 相比旧版 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部叙事壳层。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AnnotatedScrollBar`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`AnnotatedScrollBar`
- 本次保留核心状态：
  - `standard`
  - `keyboard step`
  - `marker jump`
  - `rail drag`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰

- 删除页面级 `guide`、状态栏、preview 标签和旧的双列 preview 包裹结构。
- 删除让 preview 参与交互的点击与焦点职责。
- 删除过重的 bubble、connector、focus ring 和 rail 装饰强度。
- 删除与 reference 无关的外部说明壳层和场景化叙事。

## 14. EGUI 适配时的简化点与约束

- 使用固定 `snapshot + marker` 数据保证录制稳定。
- 用控件内部 summary 与 bubble 承载当前 section 和 offset，不依赖外部状态栏。
- 底部 `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
