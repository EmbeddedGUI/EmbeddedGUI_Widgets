# ItemsControl

## 1. 为什么需要这个控件

`ItemsControl` 用来表达 WPF 中最基础的 items 集合承载语义：一个控件持有多个 item 子视图，并用统一的 item host 规则进行排列。它适合选项摘要、轻量 tag 集合、状态条目和简单分组内容。

## 2. 为什么现有控件不够用

`ItemsRepeater` 更接近数据驱动重复器，`ListView` 关注可选择列表和列表行为。这里的 `ItemsControl` 保留更基础的“多个 item child + 简单面板布局”能力，用来填补 WPF 主线中 items host 的基础层级。

## 3. 目标场景与示例概览

示例页面包含一个主控件和两个底部静态 preview：

- 主控件录制 `Vertical / item host`、`Horizontal / strip`、`Wrap / compact chips`、`Read only / muted list` 四种状态。
- 底部 preview 固定展示 wrap 与 read only，作为静态对照。
- 录制最终回到默认 vertical 状态，便于截图比较。

## 4. 视觉与布局规格

- 控件主体使用浅色 surface、1 像素低对比边框和 6 到 8 圆角。
- 每个 item 子视图后方绘制浅色 item slot，不给 item 增加强阴影或装饰。
- vertical 模式纵向排列，horizontal 模式横向 strip，wrap 模式按可用宽度换行。
- compact 减小 padding、gap、圆角和 item 背景厚度。
- read only 使用灰化 surface、border、item slot 和 accent。

## 5. 控件清单与状态矩阵

| 文件 | 作用 |
| --- | --- |
| `egui_view_items_control.h` | 控件结构体、布局常量和 API 声明 |
| `egui_view_items_control.c` | items 管理、三种布局、绘制和静态 preview 输入拦截 |
| `test.c` | HelloCustomWidgets 录制示例 |
| `example/HelloUnitTest/test/test_items_control.c` | 初始化、布局、样式和 preview 单测 |

状态矩阵：

| 状态 | 布局 | 视觉 |
| --- | --- | --- |
| Standard | vertical | 白底、浅蓝 item slot、蓝色 accent |
| Strip | horizontal | 横向条目排列，浅蓝边框 |
| Wrap | wrap | compact chip 样式，绿色 accent |
| Read only | vertical | 灰化 item slot 与文字 |

## 6. 录制动作设计

录制脚本按固定快照切换：

1. 默认 vertical items host。
2. 切到 horizontal strip 并截图。
3. 切到 wrap compact chips 并截图。
4. 切到 read only muted list 并截图。
5. 回到默认状态并截图。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/items_control PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe items_control` 必须通过。
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout` 必须通过。
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_control --track reference --timeout 10 --keep-screenshots` 必须通过。
- 截图中 items host、item slot、边框、accent、wrap preview 和 read only preview 必须完整可见，不允许黑屏、白屏、裁切和文字重叠。

## 8. 参考设计体系与开源母本

- Fluent 2 的浅色、低噪音、标准层级与合理留白原则。
- WPF `ItemsControl` 的 items collection + items panel 基础语义。
- WPF UI / WinUI 中常见轻量集合、chip group 和设置摘要的视觉语言。

## 9. 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `ItemsControl`

## 10. 保留的核心状态与删掉的装饰效果

保留：

- 多 item child 管理。
- vertical、horizontal、wrap 三种基础布局。
- padding、item gap、item alignment。
- standard、strip、wrap、read only 四类主线状态。
- 静态 preview 的输入消费与状态保持。

删掉：

- 虚拟化、数据绑定和模板选择器。
- item selection、滚动和复杂键盘导航；这些属于 ListView / ItemsRepeater 等控件。
- 强阴影、渐变、业务化色块和场景化图形。

## 11. EGUI 适配时的简化点与限制

- 当前版本直接承载 EGUI child view，不实现数据源绑定。
- wrap 布局只按当前 child 尺寸从左到右换行，不做复杂行对齐。
- item slot 背景由父控件绘制，child 仍负责自身文本或图形。
- preview 控件覆盖 touch/key API，只消费输入，不触发点击状态。
