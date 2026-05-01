# AccessText 控件

## 为什么需要这个控件

`AccessText` 用于呈现带访问键提示的文本。WPF 中常见的 `_Save`、`E_xport` 这类标记需要解析成普通显示文本，并在键盘 cue 可见时给访问键字符绘制下划线。它补齐的是文本本身的 access key 语义，而不是 `Label` 的目标控件关联语义。

## 为什么现有控件不够用

已有 `text_block` 只负责静态文本，`label_control` 负责表单 caption 和 target hint，但它刻意不解析 `_` access key 标记。`access_text` 独立保留 marker 解析、escaped underscore 和 cue underline，便于其它 caption 或命令文本复用这个语义。

## 目标场景与示例概览

示例主区按录制顺序覆盖：

1. `_Save changes`：首个 marker 解析为 `Save changes`，并给 `S` 绘制键盘 cue 下划线。
2. `E_xport report`：accent 状态下显示 `x` 助记字符。
3. `File__name field`：双下划线转义为普通 `_`，不产生访问键。
4. `Muted _field`：示例侧用 muted palette 降低对比度并隐藏 keyboard cue。

底部静态 preview 固定展示 secondary 与 muted 两种 APP 配置结果。

## 视觉与布局规格

- 主体使用浅色 surface、细边框、低对比底线和短 cue rail。
- 控件内部只保留默认文本留白与圆角；小尺寸 preview 通过 APP 设置 view size 与 font。
- muted 外观通过 APP 设置 palette 与 keyboard cue visible。
- 访问键下划线按实际字符宽度绘制，超出可见区域时不强制绘制。
- 文本过长时使用 `...` 省略，避免压到边框或 preview。

## 控件清单与状态矩阵

| 状态 | 语义 | 保留内容 |
| --- | --- | --- |
| standard | 默认访问键文本 | marker 解析、cue underline |
| accent | 强调访问键文本 | 浅蓝 surface、accent cue |
| secondary | 小尺寸文本 | APP 配置 view size、font、palette |
| muted | 降噪说明文本 | APP 配置 muted palette、隐藏 cue |

## 录制动作设计

录制动作只切换主区状态并请求截图，不模拟真实键盘焦点跳转。`AccessText` 在本仓库中是显示控件，触控和键盘输入只清理瞬时 pressed 状态，不提交业务动作。

## 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=display/access_text PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe access_text
python scripts\checks\check_touch_release_semantics.py --scope custom --category display
python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub display/access_text --track reference --timeout 10 --keep-screenshots
python scripts\code_compile_check.py --custom-widgets --category display --bits64
python scripts\code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts\web\wasm_build_demos.py --app HelloCustomWidgets --app-sub display/access_text
python scripts\web\web_smoke_check.py --web-root web --manifest web\demos\demos.json --demo HelloCustomWidgets_display_access_text
```

截图必须确认文本、转义下划线、访问键下划线、secondary preview 和 muted preview 完整可见，不出现黑屏、白屏、文字裁切或重叠。

## 参考设计体系与开源母本

- WPF `AccessText`
- WPF access key marker 约定
- Fluent 2 的低噪声文本、边框和 keyboard cue 表达

## 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `AccessText`

## 保留的核心状态与删掉的装饰效果

保留 marker 解析、escaped underscore、访问键字符索引、keyboard cue 可见性。小尺寸和 muted 变体由 APP 通过尺寸、字体、palette 与 cue visible 按需配置；删除控件内置尺寸/只读状态、强装饰化标签、复杂快捷键气泡、真实焦点跳转和命令执行逻辑。

## EGUI 适配时的简化点与限制

- 只解析第一个单下划线 marker，后续单下划线按普通字符处理。
- 双下划线 `__` 转义成普通 `_`。
- 不接管全局 Alt 键或焦点系统。
- 不承载任意 inline 文本元素。
- 不实现本地化访问键冲突检测。
