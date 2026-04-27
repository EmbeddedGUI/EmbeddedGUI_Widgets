# Line 控件

## 为什么需要这个控件

`Line` 是 WPF `Shape` 体系中的基础图形控件，用于表达一条由 `X1 / Y1 / X2 / Y2` 定义的线段，并通过 `Stroke` 与 `StrokeThickness` 控制视觉。它适合做低噪声分隔、坐标标注、趋势提示和结构化图形里的基础线段。

## 为什么现有控件不够用

`divider` 是分隔控件，语义固定为水平或垂直 separator；`arc` 覆盖弧线，`rectangle` 与 `ellipse` 覆盖闭合 shape。`Line` 补齐的是独立线段 shape 的端点和描边语义，能够验证水平、垂直、对角线、强调、紧凑和只读状态。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：水平线段，蓝色 stroke。
2. Accent：对角线段，更粗 stroke。
3. Compact：垂直细线。
4. Read only：只读 muted 水平线。

底部静态 preview 固定展示 compact 与 read only 两种线段状态。

## 视觉与布局规格

- 主体使用细 stroke、浅色背景和低噪声 caption。
- 标准 stroke width 为 2。
- accent stroke width 为 3，并使用对角端点表达非水平线段。
- compact/read only stroke width 为 1。
- read only 模式降低 stroke 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认水平线段 | X1/Y1/X2/Y2、stroke、thickness |
| accent | 强调对角线段 | diagonal endpoints、thicker stroke |
| compact | 小尺寸垂直线段 | vertical endpoints、thin stroke |
| read only | 只读展示 | muted stroke |

## 录制动作设计

录制动作只切换主区线段样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/line PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe line
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/line --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/line
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_line
```

截图必须确认线段 stroke、thickness、端点方向、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Line`
- WPF `Shape.Stroke`、`Shape.StrokeThickness`
- WPF `Line.X1`、`Line.Y1`、`Line.X2`、`Line.Y2`
- Fluent 2 的低噪声形状、边框和 disabled 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Line`

## 保留的核心状态与删掉的装饰效果

保留 endpoint、stroke、stroke thickness、horizontal、vertical、diagonal、standard、accent、compact 和 read only。删除复杂 path geometry、dash pattern、line cap/join 样式、transform、动画和渐变。

## EGUI 适配时的简化点与限制

- 当前版本以绘制方式表达 line shape，不承载子控件。
- X1/Y1/X2/Y2 以 view 内 0 到 100 的百分比端点表达。
- Stroke dash、custom cap、gradient stroke 和 geometry transform 暂不实现。
- 静态 preview 消费 touch/key 输入并保持状态不变。
