# ResizeGrip 控件

## 为什么需要这个控件

`ResizeGrip` 表达 WPF 中窗口或面板右下角的可调整大小角标。它不是列表、按钮或拖拽容器，而是一个明确的 resize affordance：用户看到角标后能理解当前 surface 可被调整尺寸。

## 为什么现有控件不够用

`border` 只能表达外框，`grid_splitter` 负责分隔线拖拽，`thumb` 类语义在本仓库没有独立 reference。`ResizeGrip` 补齐的是角落 grip 的视觉提示语义，重点验证角落锚定、对角点阵、disabled/read only 低对比状态和静态 preview。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：右下角标准 resize grip。
2. Accent：左下角 accent grip，用于验证镜像角落。
3. Compact：小尺寸 grip。
4. Disabled：不可调整尺寸的 muted grip。

底部静态 preview 固定展示 compact 与 read only 两种角标。

## 视觉与布局规格

- 使用浅色 surface、细边框和低对比对角辅助线。
- 标准 grip 使用 34 像素 grip 区、4 像素圆点和 5 像素间距。
- compact grip 使用 24 像素 grip 区、3 像素圆点和 4 像素间距。
- corner 支持 bottom right 与 bottom left 两种锚定。
- disabled/read only 降低圆点、边框和辅助线对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认 resize corner | bottom right、对角点阵 |
| accent | 强调或镜像角落 | bottom left、accent dots |
| compact | 小尺寸 surface | compact metrics |
| disabled | 不可调整尺寸 | muted dots 与低对比边框 |
| read only | 静态展示角标 | 静态 preview 消费输入 |

## 录制动作设计

录制动作只切换主区状态并请求截图，不模拟真实拖拽。`ResizeGrip` 在本仓库中只表达角标提示，不实现窗口尺寸变更、捕获鼠标或布局重排。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/resize_grip PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe resize_grip
python scripts\checks\check_touch_release_semantics.py --scope custom --category layout
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub layout/resize_grip --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category layout --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/resize_grip
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_layout_resize_grip
```

截图必须确认角标 surface、对角点阵、bottom right / bottom left、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `ResizeGrip`
- Fluent 2 的低噪声窗口角标、边框和 disabled 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `ResizeGrip`

## 保留的核心状态与删掉的装饰效果

保留 corner affordance、diagonal dots、standard、accent、compact、disabled 和 read only。删除真实窗口 resize、拖拽捕获、多方向 thumb、复杂动画和平台窗口集成。

## EGUI 适配时的简化点与限制

- 当前版本以绘制方式表达 resize grip，不承载真实窗口 resize 行为。
- 只实现 bottom right 与 bottom left 两种角落锚定。
- 静态 preview 消费 touch/key 输入并保持状态不变。
- 不实现拖拽生命周期、hit test resize zone 或系统光标切换。
