# card_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件语义：`Card`
- 当前保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前删除内容：preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与结构化信息卡无关的额外交互录制
- EGUI 适配范围：只维护 `HelloCustomWidgets` 中的 `reference widget` 版本，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`card_panel` 用来承载一张结构化信息卡：顶部 badge、标题、正文摘要、右侧 summary slot、底部 detail strip 与可选 action pill。在 Fluent / WPF UI 语义里，它适合放在设置页、概览页和详情页中，用一张低噪音卡片承载“主信息 + 次级摘要 + 轻量动作”。

## 2. 为什么现有控件不够用
- `card` 更偏通用容器，不负责固定的信息层级。
- `layer_stack` 强调叠层和景深，不适合作为标准信息卡。
- `message_bar` 和 `toast_stack` 属于反馈控件，不是常驻摘要卡。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `Card` 语义的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `card_panel`，录制轨道覆盖 `OVERVIEW / SYNC / DEPLOY / ARCHIVE` 四组 snapshot。
- 底部左侧展示 `compact` 静态 preview，验证小尺寸下标题、摘要、summary slot 与 footer 仍能稳定阅读。
- 底部右侧展示 `read only` 静态 preview，验证弱化 tone、隐藏动作与输入抑制后的只读语义。
- 页面结构固定为：标题 -> 主 `card_panel` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_card_panel_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/display/card_panel/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 252`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 90`
- `compact` preview：`104 x 90`
- `read only` preview：`104 x 90`

视觉约束：
- 使用浅灰 `page panel`、白底卡片和低噪音浅边框。
- 顶部 tone strap 只保留轻量强调，不再承担高对比装饰。
- 右侧 summary slot 维持 `value + label` 语义，但视觉权重低于标题和正文。
- `compact` 继续保留结构化卡片层级，不退化成简单摘要块。
- `read only` 除了色调弱化，还要作为真正静态 preview，不再承接录制桥接逻辑。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 252` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Card Panel` | 页面标题 |
| `panel_primary` | `egui_view_card_panel_t` | `196 x 122` | `OVERVIEW` | 标准结构化卡片 |
| `panel_compact` | `egui_view_card_panel_t` | `104 x 90` | `TASK / static` | 紧凑静态对照 |
| `panel_read_only` | `egui_view_card_panel_t` | `104 x 90` | `ARCHIVE / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `OVERVIEW` | 默认 `accent` 状态 |
| 主控件 | `SYNC` | `warning` 状态 |
| 主控件 | `DEPLOY` | `success` 状态 |
| 主控件 | `ARCHIVE` | `neutral` 状态 |
| `compact` | `TASK` | 紧凑静态 preview |
| `read only` | `ARCHIVE` | 只读静态 preview，隐藏 action 并弱化 tone |

## 7. 交互语义要求
- 主控件保持标准 clickable 语义，继续遵守 same-target release。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()` 必须先清理残留 `pressed` 再刷新。
- `read only / disabled` 的 `touch / key guard` 必须在拒绝输入前清理残留 `pressed`。
- 静态 preview 必须吞掉 `touch / key`，不能修改 `current_snapshot`，也不能触发 click。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/display/card_panel/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview snapshot 切换
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件四组 snapshot 切换与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `OVERVIEW`，并同步两个静态 preview 状态。
2. 请求默认截图。
3. 切到 `SYNC`。
4. 请求第二张截图。
5. 切到 `DEPLOY`。
6. 请求第三张截图。
7. 切到 `ARCHIVE`。
8. 请求第四张截图。
9. 恢复默认 `OVERVIEW` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、交互、runtime 与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/card_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/card_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/card_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_card_panel
```

验收重点：
- 主控件四张关键截图必须能清晰区分 `OVERVIEW / SYNC / DEPLOY / ARCHIVE`。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或 snapshot 切换产生额外变化。
- 页面不能出现黑白屏、裁切、pressed 残留或底部 preview 脏态。
- preview 不响应触摸或键盘输入，也不改变 `current_snapshot`。

## 11. 已知限制
- 当前版本仍使用固定 snapshot 数据和固定 summary slot，不覆盖超长标题、超长摘要和超长 `value`。
- 当前不做真实图标、hover、完整 focus ring 和桌面级键盘导航细节。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部卡片容器系统。
