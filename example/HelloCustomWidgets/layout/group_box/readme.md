# GroupBox

## 1. 为什么需要这个控件

`GroupBox` 用来表达 WPF 中带 header 的分组内容容器语义：一个边框分组框包含标题和一块内容区域。它适合设置组、表单分组、属性摘要和只读分组信息。

## 2. 为什么现有控件不够用

`Border` 只有基础边框，`HeaderedContentControl` 表达 header/content 槽位但不是分组边框语义。`GroupBox` 需要保留 header 贴合 framed surface 的分组视觉和内容区域，补齐 WPF 主线中常见的分组容器。

## 3. 目标场景与示例概览

示例页面包含一个主控件和两个底部静态 preview：

- 主控件录制 `Standard / centered content`、`Accent / leading content`、`Compact / framed group`、`Read only / muted group` 四种状态。
- 底部 preview 固定展示 compact 与 read only，作为静态对照。
- 录制最终回到默认 standard 状态，便于截图比较。

## 4. 视觉与布局规格

- 控件主体使用浅色 surface、1 像素低对比边框和 6 到 8 圆角。
- header 使用浅色胶囊背景贴合左上区域，并保留适度 header indent。
- content 区域使用低对比浅色面，强调分组内容范围。
- compact 减小 padding、header gap、header indent 和圆角。
- read only 使用灰化 surface、header、content、border 和 accent。

## 5. 控件清单与状态矩阵

| 文件 | 作用 |
| --- | --- |
| `egui_view_group_box.h` | 控件结构体和 API 声明 |
| `egui_view_group_box.c` | header/content 管理、布局、绘制、样式和静态 preview 输入拦截 |
| `test.c` | HelloCustomWidgets 录制示例 |
| `example/HelloUnitTest/test/test_group_box.c` | 初始化、布局、样式和 preview 单测 |

状态矩阵：

| 状态 | 内容对齐 | 视觉 |
| --- | --- | --- |
| Standard | center | 白底、浅蓝 header、浅色 content region、蓝色 accent |
| Accent | left center | 浅蓝边框与 header surface |
| Compact | top left | 更小 padding、绿色 accent |
| Read only | center | 灰化 header、content 与文字 |

## 6. 录制动作设计

录制脚本按固定快照切换：

1. 默认 standard group box。
2. 切到 accent leading content 并截图。
3. 切到 compact framed group 并截图。
4. 切到 read only muted group 并截图。
5. 回到默认状态并截图。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/group_box PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe group_box` 必须通过。
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout` 必须通过。
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/group_box --track reference --timeout 10 --keep-screenshots` 必须通过。
- 截图中 header、content 区、边框、accent、compact preview 和 read only preview 必须完整可见，不允许黑屏、白屏、裁切和文字重叠。

## 8. 参考设计体系与开源母本

- Fluent 2 的浅色、低噪音、标准层级与合理留白原则。
- WPF `GroupBox` 的 header + framed content 基础语义。
- WPF UI / WinUI 中常见设置分组、表单分组和属性分组的视觉语言。

## 9. 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `GroupBox`

## 10. 保留的核心状态与删掉的装饰效果

保留：

- header child 管理。
- content child 管理。
- framed group surface、header capsule 和 content region。
- padding、header gap、header indent、header alignment、content alignment。
- standard、accent、compact、read only 四类主线状态。
- 静态 preview 的输入消费与状态保持。

删掉：

- 表单校验、可展开分组、选择状态和复杂键盘导航。
- 数据绑定、模板选择器和嵌套表单生成。
- 强阴影、渐变、业务化色块和场景化图形。

## 11. EGUI 适配时的简化点与限制

- 当前版本直接承载 EGUI header 与 content child view，不实现数据源绑定。
- header 胶囊和 content region 背景由父控件绘制，child 仍负责自身文本或图形。
- content 只保留一个 child；多内容组合应在外部先用 layout 容器组织。
- preview 控件覆盖 touch/key API，只消费输入，不触发点击状态。
