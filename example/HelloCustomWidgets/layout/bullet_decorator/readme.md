# BulletDecorator 控件

## 为什么需要这个控件

`BulletDecorator` 用于表达 WPF 中典型的 bullet + child 布局：左侧是项目符号槽，右侧是内容槽。它适合说明列表项、设置项、步骤项或小型提示项，重点是稳定的 slot 布局，而不是可点击列表控件。

## 为什么现有控件不够用

`text_block` 只能显示文本，`items_control` / `list` 负责多 item 容器，`border` 只提供外框。`BulletDecorator` 补齐的是单个 list item 的 bullet slot 与 content slot 装饰布局语义，能够明确验证 dot、square、numbered marker 与 content 的对齐关系。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. Dot bullet：圆点 bullet 与内容槽。
2. Square bullet：方形 bullet 与 accent surface。
3. Numbered bullet：文本 bullet `1.` 与 compact 布局。
4. Read only：只读 muted numbered item。

底部静态 preview 固定展示 compact dot 与 read only numbered 两种状态。

## 视觉与布局规格

- 主体使用浅色 surface、细边框与低噪声分隔线。
- 默认 bullet slot 宽度 24，content gap 6，bullet size 8。
- compact 模式 bullet slot 宽度 20，content gap 4，bullet size 6。
- 文本过长时使用 `...` 省略，避免压到边框或 preview。
- read only 模式降低 bullet、文字和边框对比度。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认 bullet + content item | dot bullet、content slot |
| accent | 强调 list item | square bullet、浅蓝 surface |
| compact | 小尺寸 item | numbered bullet、压缩 slot |
| read only | 只读说明 item | muted bullet、muted content |

## 录制动作设计

录制动作只切换主区状态并请求截图，不模拟点击或选择。`BulletDecorator` 在本仓库中是布局/显示控件，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/bullet_decorator PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe bullet_decorator
python scripts\checks\check_touch_release_semantics.py --scope custom --category layout
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub layout/bullet_decorator --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category layout --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/bullet_decorator
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_layout_bullet_decorator
```

截图必须确认 bullet slot、content slot、dot、square、numbered bullet、compact preview 和 read only preview 完整可见，不出现黑屏、白屏、文字裁切或重叠。

## 参考设计体系与开源母本

- WPF `BulletDecorator`
- Fluent 2 的低噪声列表项、文本和边框层级

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `BulletDecorator`

## 保留的核心状态与删掉的装饰效果

保留 bullet slot、content slot、dot bullet、square bullet、text bullet、compact 和 read only。删除复杂图标库、真实 item collection、selection、拖拽、命令执行和多行富文本。

## EGUI 适配时的简化点与限制

- 当前版本以绘制方式表达 bullet/content slot，不承载任意 child view。
- 文本 bullet 只作为短字符串绘制，不做自动编号。
- 不实现列表选择、键盘导航或虚拟化。
- 不实现多行换行，窄区域使用省略。
