# Rectangle 控件

## 为什么需要这个控件

`Rectangle` 是 WPF `Shape` 体系中的基础图形控件，用于表达矩形填充、描边和圆角。它适合做图例、状态块、形状标注和低噪声区域提示，重点是 shape 语义，而不是 layout 容器。

## 为什么现有控件不够用

`border` 负责容器边框与 child 承载，`card_panel` 表达内容卡片，`arc` 只覆盖弧形 shape。`Rectangle` 补齐的是独立矩形 shape 的 `Fill / Stroke / RadiusX / RadiusY` 语义，能够验证填充、描边、圆角、紧凑和只读低对比状态。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：白底矩形、蓝色描边和圆角。
2. Accent：浅蓝填充与更大的圆角。
3. Compact：紧凑尺寸、细描边和底部 guide。
4. Read only：只读 muted 矩形。

底部静态 preview 固定展示 compact 与 read only 两种矩形状态。

## 视觉与布局规格

- 主体使用浅色 fill、细 stroke 和低噪声圆角。
- 标准 stroke width 为 2，corner radius 为 8。
- accent corner radius 为 12。
- compact/read only stroke width 为 1，corner radius 为 5。
- read only 模式降低 fill、stroke 与辅助线对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认矩形 shape | fill、stroke、corner radius |
| accent | 强调矩形 shape | accent fill、larger radius |
| compact | 小尺寸矩形 | thin stroke、compact guide |
| read only | 只读展示 | muted fill、muted stroke |

## 录制动作设计

录制动作只切换主区矩形样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/rectangle PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe rectangle
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/rectangle --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/rectangle
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_rectangle
```

截图必须确认矩形 fill、stroke、corner radius、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Rectangle`
- WPF `Shape.Fill`、`Shape.Stroke`、`Shape.StrokeThickness`
- Fluent 2 的低噪声形状、边框和 disabled 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Rectangle`

## 保留的核心状态与删掉的装饰效果

保留 fill、stroke、corner radius、standard、accent、compact 和 read only。删除复杂路径几何、dash pattern、transform、动画、渐变和真实 layout child 承载。

## EGUI 适配时的简化点与限制

- 当前版本以绘制方式表达矩形 shape，不承载子控件。
- RadiusX / RadiusY 简化为统一 `corner_radius`。
- Stroke dash、gradient fill 和 geometry transform 暂不实现。
- 静态 preview 消费 touch/key 输入并保持状态不变。
