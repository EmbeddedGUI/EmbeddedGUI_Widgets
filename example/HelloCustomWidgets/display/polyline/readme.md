# Polyline 控件

## 为什么需要这个控件

`Polyline` 是 WPF `Shape` 体系里的基础开口多段线控件，用 `Points` 定义一组连续顶点，并通过 `Stroke` 与 `StrokeThickness` 控制线段视觉。它适合表达趋势折线、路径草图、流程连线、坐标提示和轻量结构化线段。

## 为什么现有控件不够用

`line` 只表达单条线段，`polygon` 表达闭合填充图形，`divider` 是固定分隔语义。`Polyline` 补齐的是开口多段线 shape：多个顶点连续连接，但最后一个顶点不回连到第一个顶点，也不做 fill。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：zigzag 多段线。
2. Accent：trend 多段线，更粗 stroke。
3. Compact：小尺寸 step 多段线。
4. Read only：muted 多段线。

底部静态 preview 固定展示 compact 与 read only 两种 polyline 状态。

## 视觉与布局规格

- 主体使用蓝色 / teal stroke、浅色背景和低噪声 caption。
- 标准 stroke width 为 2。
- accent stroke width 为 3，用更长点集表达趋势线。
- compact/read only stroke width 为 1。
- read only 模式降低 stroke 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认 zigzag 开口多段线 | Points、Stroke、Thickness |
| accent | 强调趋势线 | longer points、thicker stroke |
| compact | 小尺寸 step 线 | compact points、thin stroke |
| read only | 只读展示 | muted stroke |

## 录制动作设计

录制动作只切换主区 polyline 样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/polyline PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe polyline
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/polyline --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/polyline
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_polyline
```

截图必须确认 polyline stroke、thickness、开口端点、折线方向、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Polyline`
- WPF `Shape.Stroke`
- WPF `Shape.StrokeThickness`
- WPF `Polyline.Points`
- Fluent 2 的低噪声形状、描边和 disabled/read only 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Polyline`

## 保留的核心状态与删掉的装饰效果

保留 `Points`、`Stroke`、`StrokeThickness`、open multi-segment path、zigzag、step、trend、standard、accent、compact 和 read only。删除 fill、闭合 polygon、dash pattern、自定义 line cap/join、gradient stroke、geometry transform 和动画。

## EGUI 适配时的简化点与限制

- 当前版本以 view 内 0 到 100 的百分比顶点表达 `Points`。
- 点集最多保留 8 个顶点，满足示例层 reference 验证，不暴露动态集合容器。
- 使用连续 `egui_canvas_draw_line` 调用绘制开口多段线，不回连最后一个顶点。
- 静态 preview 消费 touch/key 输入并保持状态不变。
