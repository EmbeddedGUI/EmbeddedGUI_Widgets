# Glyphs 控件

## 为什么需要这个控件

`Glyphs` 是 WPF 中用于绘制固定字形运行的低层文本元素。它不像 `TextBlock` 那样承担段落、换行和文本布局语义，而是更直接地描述 `UnicodeString`、字体大小、填充色和绘制原点，适合静态标识、编号、紧凑仪表文本和对字形位置有明确要求的 reference 预览。

## 为什么现有控件不够用

`text_block` 面向段落文本，`label_control` 面向表单标签，`access_text` 面向访问键标记。`Glyphs` 补齐的是固定字形运行语义：文本内容是只读图形层的一部分，重点是 `UnicodeString`、`FontRenderingEmSize`、`Fill`、`OriginX` 和 `OriginY`，而不是编辑、选择、换行或 access key。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Standard：默认 `UnicodeString` 字形运行。
2. Accent：强调色字形运行与不同 em size。
3. Compact：紧凑编号字形运行。
4. Read only：muted 只读字形运行。

底部静态 preview 固定展示 compact 与 read only 两种 glyph run 状态。

## 视觉与布局规格

- 主体使用深色 / 蓝色 / teal fill、浅色 origin marker 和低噪声 caption。
- standard 使用 16px 字体，accent 使用 14px 字体。
- compact/read only 使用 10px 字体。
- read only 模式降低 fill 与 origin marker 对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认字形运行 | UnicodeString、Fill、Origin |
| accent | 强调字形运行 | FontRenderingEmSize、accent Fill |
| compact | 紧凑字形运行 | compact text、small em size |
| read only | 只读展示 | muted fill、static preview |

## 录制动作设计

录制动作只切换主区 glyph run 样式并请求截图，不模拟点击或拖拽。底部 preview 始终静态，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/glyphs PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe glyphs
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/glyphs --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/glyphs
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_glyphs
```

截图必须确认 glyph text、fill、em size、origin marker、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、裁切或重叠。

## 参考设计体系与开源母本

- WPF `Glyphs`
- WPF `Glyphs.UnicodeString`
- WPF `Glyphs.FontRenderingEmSize`
- WPF `Glyphs.Fill`
- WPF `Glyphs.OriginX`
- WPF `Glyphs.OriginY`
- Fluent 2 的低噪声文本、描边和 disabled/read only 视觉层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `Glyphs`

## 保留的核心状态与删掉的装饰效果

保留 `UnicodeString`、`FontRenderingEmSize`、`Fill`、`OriginX`、`OriginY`、standard、accent、compact 和 read only。删除 glyph indices、caret stops、bidi level、style simulations、font URI 解析、复杂 shaping、selection、editing 和动画。

## EGUI 适配时的简化点与限制

- 当前版本用 EGUI 内置字体指针承载 WPF `FontUri` / `FontRenderingEmSize` 的演示语义。
- `OriginX` / `OriginY` 使用 view 内 0 到 100 的百分比表达。
- 绘制使用 `egui_canvas_draw_text_in_rect`，不实现复杂 glyph shaping。
- 静态 preview 消费 touch/key 输入并保持状态不变。
