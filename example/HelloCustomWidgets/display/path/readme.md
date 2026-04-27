# Path 控件

## 为什么需要这个控件

`Path` 是 WPF `Shape` 体系中表达通用几何的基础控件。它通过一组 path geometry 命令描述线段、二次曲线、三次曲线和闭合轮廓，并由 `Fill`、`Stroke` 与 `StrokeThickness` 控制视觉。它适合承载图标轮廓、简化插画、曲线轨迹和需要自定义轮廓的轻量形状。

## 为什么现有控件不够用

`line` 只覆盖单线段，`polyline` 覆盖开放折线，`polygon` 覆盖闭合直线轮廓，`path_icon` 是图标语义。`Path` 补齐的是通用 Shape：同一个控件既可以绘制闭合填充几何，也可以绘制开放曲线路径。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：闭合 shield geometry，包含 fill 与 stroke。
2. Accent：开放 cubic curve path，更粗 stroke。
3. Compact：小尺寸 line path。
4. Read only：muted bookmark silhouette。

底部静态 preview 固定展示 compact 与 read only 两种 path 状态。

## 视觉与布局规格

- 主体使用浅蓝填充、蓝色 / teal stroke 和低噪声 caption。
- 标准 stroke width 为 2。
- accent stroke width 为 3，强调开放曲线路径。
- compact/read only stroke width 为 1。
- read only 模式降低 fill 与 stroke 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 闭合几何 | path data、Fill、Stroke、Thickness |
| accent | 开放曲线 | cubic path、thicker stroke |
| compact | 线段路径 | line commands、thin stroke |
| read only | 只读轮廓 | muted fill、muted stroke |

## 录制动作设计

录制动作只切换主区 path 样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/path PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe path
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/path --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/path
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_path
```

截图必须确认 path fill、stroke、stroke thickness、闭合轮廓、开放曲线、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Path`
- WPF `Shape.Fill`
- WPF `Shape.Stroke`
- WPF `Shape.StrokeThickness`
- WPF `Path.Data`
- Fluent 2 的低噪声形状、描边和 disabled/read only 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Path`

## 保留的核心状态与删掉的装饰效果

保留 path data、move / line / quadratic / cubic / close 命令、fill、stroke、stroke thickness、standard、accent、compact 和 read only。删除 complex geometry collection、arc segment、dash pattern、自定义 line cap/join、gradient fill/stroke、geometry transform、clip 和动画。

## EGUI 适配时的简化点与限制

- 当前版本使用固定 viewport 的命令数据描述 path geometry。
- 绘制前把二次 / 三次曲线扁平化为有限点集。
- 闭合路径使用 polygon fill + polygon stroke；开放路径使用 polyline stroke。
- 静态 preview 消费 touch/key 输入并保持状态不变。
