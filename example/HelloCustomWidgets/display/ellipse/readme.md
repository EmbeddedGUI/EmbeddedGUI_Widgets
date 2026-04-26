# Ellipse 控件

## 为什么需要这个控件

`Ellipse` 是 WPF `Shape` 体系中的基础图形控件，用于表达椭圆和圆形的填充、描边与 stroke thickness。它适合做图例标记、状态点、头像占位、轻量形状提示和图形标注，重点是 shape 语义，而不是 layout 容器。

## 为什么现有控件不够用

`rectangle` 覆盖矩形 shape，`arc` 覆盖弧线/进度弧，`badge` 和 `presence_badge` 带有特定业务语义。`Ellipse` 补齐的是独立椭圆 shape 的 `Fill / Stroke / StrokeThickness` 语义，并验证 oval、circle、compact 和 read only 低对比状态。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：白底椭圆、蓝色描边。
2. Accent：浅蓝填充圆形。
3. Compact：紧凑尺寸、细描边椭圆。
4. Read only：只读 muted 椭圆。

底部静态 preview 固定展示 compact 与 read only 两种椭圆状态。

## 视觉与布局规格

- 主体使用浅色 fill、细 stroke 和低噪声轮廓。
- 标准 stroke width 为 2。
- accent 使用 circle mode，在可用区域内取短边绘制圆。
- compact/read only stroke width 为 1。
- read only 模式降低 fill 与 stroke 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认椭圆 shape | fill、stroke、oval |
| accent | 强调圆形 shape | accent fill、circle mode |
| compact | 小尺寸椭圆 | thin stroke、compact fill |
| read only | 只读展示 | muted fill、muted stroke |

## 录制动作设计

录制动作只切换主区椭圆样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/ellipse PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe ellipse
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/ellipse --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/ellipse
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_ellipse
```

截图必须确认椭圆 fill、stroke、circle mode、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Ellipse`
- WPF `Shape.Fill`、`Shape.Stroke`、`Shape.StrokeThickness`
- Fluent 2 的低噪声形状、边框和 disabled 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Ellipse`

## 保留的核心状态与删掉的装饰效果

保留 fill、stroke、stroke thickness、oval、circle、standard、accent、compact 和 read only。删除复杂路径几何、dash pattern、transform、动画、渐变和真实 layout child 承载。

## EGUI 适配时的简化点与限制

- 当前版本以绘制方式表达椭圆 shape，不承载子控件。
- 圆形模式使用当前 view 的短边作为直径。
- Stroke dash、gradient fill 和 geometry transform 暂不实现。
- 静态 preview 消费 touch/key 输入并保持状态不变。
