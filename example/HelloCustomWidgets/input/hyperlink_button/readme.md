# HyperlinkButton 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`HyperlinkButton`
- 本次保留状态：`standard`、`inline`、`disabled`、`focused`
- 删除效果：页面级说明性大段文案、额外标签、showcase 式装饰容器
- EGUI 适配说明：复用 SDK `button` 的点击和 same-target release 语义，只在 custom 层补齐轻量链接视觉、`Space / Enter` 键盘闭环与静态 preview 输入吞掉

## 1. 为什么需要这个控件？
`HyperlinkButton` 适合承载“跳转到文档、详情、设置项”的轻量动作。仓库当前已有 `Button`、`SplitButton`、`DropDownButton` 等命令型控件，但还缺少一个更贴近 Fluent / WPF UI 的文本链接型 reference 页面，因此需要新增。

## 2. 为什么现有控件不够用？
- `button` 的默认语义偏主操作，视觉重量更高，不适合内容区内的轻量跳转。
- `toggle_button` 和 `switch` 表达的是状态切换，不是一次性导航动作。
- `drop_down_button`、`split_button` 带有额外菜单或分段动作，不是最小链接语义。

## 3. 目标场景与示例概览
- 主区域展示一条可交互的 `HyperlinkButton`，保留真实触摸和 `Space / Enter` 键盘触发。
- 下方左侧 `inline` preview 展示更弱、更紧凑的次级链接。
- 下方右侧 `disabled` preview 展示禁用态对照，不承担真实输入职责。
- 页面仅保留标题、一个主链接面板和底部双 preview。

目录：
- `example/HelloCustomWidgets/input/hyperlink_button/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 170`
- 主面板：`196 x 92`
- 主链接：`164 x 24`
- 底部 preview 行：`200 x 24`
- 单个 preview：`96 x 24`

视觉约束：
- 主链接使用 Fluent 蓝色文本和细 underline，不绘制厚重实体按钮背景。
- `pressed` 和 `focused` 只保留轻量浅色底和细边框，不回到主按钮化视觉。
- `inline` 保持相同链接语义，但降低尺寸和强调强度。
- `disabled` 使用灰色文本，不承担交互语义。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 170` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `HyperlinkButton` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 92` | enabled | 主展示面板 |
| `panel_heading_label` | `egui_view_label_t` | `172 x 12` | enabled | 场景标题 |
| `primary_link_button` | `egui_view_button_t` | `164 x 24` | enabled | 主链接 |
| `note_label` | `egui_view_label_t` | `172 x 20` | enabled | 场景说明 |
| `inline_preview_button` | `egui_view_button_t` | `96 x 24` | static preview | 紧凑预览 |
| `disabled_preview_button` | `egui_view_button_t` | `96 x 24` | static preview | 禁用态预览 |

## 6. 状态矩阵

| 状态 / 区域 | 主链接 | Inline | Disabled |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 点击提交 | 是 | 否 | 否 |
| same-target release | 是 | 否 | 否 |
| 键盘 `Space / Enter` | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focused | 是 | 否 | 否 |

## 7. 交互语义
- 主链接继续复用 SDK `button` 的默认触摸闭环：只有 `DOWN(A) -> UP(A)` 才提交。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交；回到原命中区后 `UP(A)` 才提交。
- `Space / Enter`：按下进入 `pressed`，抬起时触发 click listener。
- 主链接点击后切换到下一组文案快照，用于 runtime 录制。
- 底部两个 preview 通过 `hcw_hyperlink_button_override_static_preview_api()` 统一吞掉 `touch / key`，并清理残留 `pressed`。

## 8. 本轮收口内容
- 新增 `egui_view_hyperlink_button.h/.c`，作为 SDK `button` 的 `HyperlinkButton` reference 包装层。
- 在包装层补齐：
  - `standard / inline / disabled` 三套样式 helper
  - 自定义 `on_draw` 细 underline
  - `set_text()`、`set_font()` 的 `pressed` 清理
  - `Space / Enter` 键盘 click 闭环
  - 静态 preview 输入吞掉
- 新增 `test.c` reference 页面，保留主链接和底部双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、setter 清理、same-target release、键盘点击、禁用态和静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 轨道：
1. 还原默认快照并给主链接 request focus。
2. 截默认态。
3. 触摸点击主链接，切到第二组文案。
4. 截触摸后状态。
5. 发送 `Space`，切到第三组文案。
6. 截键盘切换状态。
7. 发送 `Enter`，切回第一组文案。
8. 截最终收尾帧。

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/hyperlink_button PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/hyperlink_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/hyperlink_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_hyperlink_button
```

验收重点：
- 主链接和底部双 preview 必须完整可见，不黑屏、不白屏、不裁切。
- 细 underline、浅色 `pressed` 背景和 focused ring 必须能辨识，但不能回到厚重按钮化。
- `Space / Enter` 必须稳定触发 click listener。
- `inline / disabled` preview 必须保持静态 reference，对输入只做吞掉和焦点收尾。

## 11. 已知限制
- 当前只覆盖最小 `HyperlinkButton` reference，不扩展访问历史、外链图标或 visited 态。
- 不做 hover 动画和系统主题联动。
- 页面不承担复杂内容排版，只验证控件级视觉和交互闭环。
