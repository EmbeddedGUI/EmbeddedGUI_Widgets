# HeaderedContentControl

## 1. 为什么需要这个控件

`HeaderedContentControl` 用来表达 WPF 中“标题 + 单一内容”的承载语义。它比普通内容槽多一个稳定的 header 区域，适合设置项、属性摘要、分组说明和表单段落等需要先给出标题再呈现内容的界面。

## 2. 为什么现有控件不够用

`ContentControl` 和 `ContentPresenter` 只处理单一 content 或模板承载，不负责标题区与内容区的关系。直接用两个 Label 拼装会让 header 间距、边框、只读态和 compact 态在各示例中反复分叉，因此这里单独保留一个 WPF 主线参考控件。

## 3. 目标场景与示例概览

示例页面包含一个主控件和两个底部静态 preview：

- 主控件录制 `Header / centered content`、`Accent / leading content`、`Compact / stacked`、`Read only / muted header` 四种状态。
- 底部 preview 固定展示 compact 与 read only，不参与点击状态变化。
- 每轮录制最终回到默认状态，便于 runtime 截图对比。

## 4. 视觉与布局规格

- 根页面使用浅灰圆角面板。
- 控件主体为白色或浅灰 surface，圆角 6 到 8，边框 1 像素。
- header 区使用低透明度浅蓝或浅灰底色，并在标题下方保留 1 像素分隔线。
- accent 只保留一条短横线，避免变成装饰性卡片。
- content 默认在 header 下方剩余区域居中，也支持 leading 和 top-left。

## 5. 控件清单与状态矩阵

| 文件 | 作用 |
| --- | --- |
| `egui_view_headered_content_control.h` | 控件结构体、API 与样式函数声明 |
| `egui_view_headered_content_control.c` | 绘制、header/content 布局、静态 preview 输入拦截 |
| `test.c` | HelloCustomWidgets 录制示例 |
| `example/HelloUnitTest/test/test_headered_content_control.c` | 初始化、布局、样式和静态 preview 单测 |

状态矩阵：

| 状态 | header | content | 视觉 |
| --- | --- | --- | --- |
| Standard | 左上 | 居中 | 白底、浅蓝 header 区、蓝色 accent |
| Accent | 左上 | 左侧垂直居中 | 更明显的 header 引导 |
| Compact | 左上 | 左上 | 更小 padding 和 gap |
| Read only | 左上 | 居中 | 灰化 header、边框和文字 |

## 6. 录制动作设计

录制脚本按固定快照切换：

1. 默认 `Header / centered content`。
2. 切到 accent leading 状态并截图。
3. 切到 compact stacked 状态并截图。
4. 切到 read only muted 状态并截图。
5. 回到默认状态并截图。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/headered_content_control PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 与 `output\main.exe headered_content_control` 必须通过。
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout` 必须通过。
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/headered_content_control --track reference --timeout 10 --keep-screenshots` 必须通过。
- 截图中 header、content、边框、accent、compact 和 read only preview 必须完整可见，不允许黑屏、白屏、裁切和文字重叠。

## 8. 参考设计体系与开源母本

- Fluent 2 的浅色、低噪音、标准层级与留白原则。
- WPF `HeaderedContentControl` 的 header + content 单子项承载模型。
- WinUI / WPF UI 常见设置区域的 header 与 content 分层表达。

## 9. 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `HeaderedContentControl`

## 10. 保留的核心状态与删掉的装饰效果

保留：

- header 与 content 两个命名槽位。
- header gap、header 对齐、content 对齐。
- standard、accent、compact、read only 四类主线状态。
- 静态 preview 的输入消费与状态保持。

删掉：

- 复杂阴影、渐变和大面积装饰色。
- 多 content 列表行为；该职责应由 ItemsControl / ListView 类控件承担。
- 业务场景文案和行业化图形。

## 11. EGUI 适配时的简化点与限制

- 当前版本只支持一个 header view 和一个 content view。
- header 固定在内容区顶部，content 在 header 下方剩余区域内按 alignment 布局。
- 不实现模板选择器和数据绑定，示例通过显式 setter 更新子控件。
- preview 控件覆盖 touch/key API，只消费输入，不触发点击状态。
