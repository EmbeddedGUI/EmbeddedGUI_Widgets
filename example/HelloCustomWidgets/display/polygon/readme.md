# Polygon 控件

## 为什么需要这个控件

`Polygon` 是 WPF `Shape` 体系里的基础闭合图形控件，用 `Points` 定义多个顶点，并通过 `Fill`、`Stroke` 和 `StrokeThickness` 控制闭合区域与描边。它适合表达轻量几何标识、状态面片、流程节点、坐标图形和需要闭合轮廓的静态提示。

## 为什么现有控件不够用

`rectangle` 和 `ellipse` 覆盖固定闭合 shape，`line` 覆盖单条线段，`divider` 只表达分隔语义。`Polygon` 补齐的是任意闭合点集语义，能够验证三角形、菱形、五边形、填充、描边、描边厚度、compact 与 read only 等 WPF Shape 状态。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：菱形 polygon，浅蓝 fill 与蓝色 stroke。
2. Accent：五边形 polygon，更粗 stroke。
3. Compact：三角形 polygon，低噪声 teal stroke。
4. Read only：muted 菱形 polygon。

底部静态 preview 固定展示 compact 与 read only 两种 polygon 状态。

## 视觉与布局规格

- 主体使用浅色 fill、低噪声 stroke 和简短 caption。
- 标准 stroke width 为 2。
- accent stroke width 为 3，并使用五边形点集表达更复杂闭合轮廓。
- compact/read only stroke width 为 1。
- read only 模式降低 fill 与 stroke 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认闭合菱形 | Points、Fill、Stroke、Thickness |
| accent | 强调五边形 | pentagon points、thicker stroke |
| compact | 小尺寸三角形 | triangle points、thin stroke |
| read only | 只读展示 | muted fill and stroke |

## 录制动作设计

录制动作只切换主区 polygon 样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/polygon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe polygon
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/polygon --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/polygon
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_polygon
```

截图必须确认 polygon fill、stroke、thickness、顶点轮廓、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Polygon`
- WPF `Shape.Fill`
- WPF `Shape.Stroke`
- WPF `Shape.StrokeThickness`
- WPF `Polygon.Points`
- Fluent 2 的低噪声形状、描边和 disabled/read only 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Polygon`

## 保留的核心状态与删掉的装饰效果

保留 `Points`、`Fill`、`Stroke`、`StrokeThickness`、triangle、diamond、pentagon、standard、accent、compact 和 read only。删除复杂 path geometry、dash pattern、自定义 line join、gradient fill、geometry transform、动画和文本叙事装饰。

## EGUI 适配时的简化点与限制

- 当前版本以 view 内 0 到 100 的百分比顶点表达 `Points`。
- 点集最多保留 8 个顶点，满足示例层 reference 验证，不暴露动态集合容器。
- Fill 与 stroke 使用 solid color，不实现 gradient、dash、transform 或复杂 geometry。
- 静态 preview 消费 touch/key 输入并保持状态不变。
