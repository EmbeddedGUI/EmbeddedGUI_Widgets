# color_picker 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ColorPicker`
- 开源母本：`WPF UI`
- 对应组件名：`ColorPicker`
- 本轮保留语义：`tone palette / hue rail / compact / read only / focused`
- 本轮移除内容：页面级 guide、状态说明文案、preview 快照轮换、额外收尾态、弹出式高级编辑器、透明度通道、eyedropper
- EGUI 适配说明：继续保留标准 `ColorPicker` 的 `tone palette + hue rail + 当前色预览` 语义，本轮只收口 `reference` 页面结构、static preview 轨道、单测断言和验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件?
`color_picker` 用来表达标准颜色选择语义，适合主题色、状态色、卡片强调色、图标前景色等离散但可导出 tone 的场景。它补上了当前 `HelloCustomWidgets` 里“同一 hue 下继续选择明暗与饱和度”的标准输入能力。

## 2. 为什么现有控件不够用

- `swatch_picker` 只覆盖离散色板切换，不支持 `tone palette + hue rail` 的组合语义。
- `slider` / `xy_pad` 虽然能做连续输入，但不直接表达颜色选择，也没有当前色预览和 `hex` 摘要。
- `number_box` / `text_box` 可以承载数值或文本，却不适合作为主颜色选择入口。
- 当前仓库里的 `color_picker` 页面虽然已经 reference 化，但录制轨道、静态 preview 单测和 README 仍停留在旧 workflow，没有对齐到当前 static preview 模板。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `color_picker` -> 底部 `compact / read only` 双静态 preview。
- 主区保留真实 `ColorPicker` 的键盘 `tone / hue` 调整能力。
- 底部 `compact` preview 固定显示 `Mint` 配色，不再承担快照轮换职责。
- 底部 `read only` preview 固定显示 `Locked` 配色，作为只读静态对照。
- 两个 preview 统一通过 `egui_view_color_picker_override_static_preview_api()` 收口：
  - 吞掉 `touch / dispatch_key_event()`
  - 收到输入后立刻清理残留 `pressed`
  - 不改 `label / helper / selection / hex / region_screen / palette`

目标目录：`example/HelloCustomWidgets/input/color_picker/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 204`
- 主控件：`196 x 112`
- 底部对照行：`216 x 52`
- `compact` preview：`104 x 52`
- `read only` preview：`104 x 52`

视觉约束：

- 使用浅色 page panel、低噪音边框和单层白底主卡，不回退到 showcase 式装饰。
- 主区保留 `tone palette + hue rail + swatch + hex` 的最小完整语义，不额外叠加外部说明卡。
- `compact` 与 `read only` 只做静态 reference 对照，整条 runtime 轨道中必须保持不变。
- focus 仅在主控件内部的 `palette / hue rail` 间切换，不叠加夸张 glow 或复杂阴影。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 204` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Color Picker` | 页面标题 |
| `picker_primary` | `egui_view_color_picker_t` | `196 x 112` | `Accent color` | 主控件 |
| `picker_compact` | `egui_view_color_picker_t` | `104 x 52` | `Mint` / compact | 静态 preview |
| `picker_read_only` | `egui_view_color_picker_t` | `104 x 52` | `Locked` / read only | 静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Accent color` / palette focused | 默认状态 |
| 主控件 | `Accent color` / tone adjusted | `Right` 后 |
| 主控件 | `Accent color` / hue adjusted | `Tab + Down` 后 |
| 主控件 | `Signal color` / palette focused | 第二组主快照 |
| `compact` preview | `Mint` / compact | 全程静态对照 |
| `read only` preview | `Locked` / read only | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` palette/hue 命中、`Tab / Left / Right / Up / Down / Home / End / Escape` 键盘闭环。
- `set_selection()`、`set_current_part()`、`set_palette()`、`set_compact_mode()` 与 `set_read_only_mode()` 必须清理残留 `pressed_part / is_pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直连旧的 `on_key_event()`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 固定断言覆盖：
  - `font / meta_font / label / helper` 不变
  - `region_screen / alpha / surface_color / border_color / text_color / muted_text_color / accent_color / selected_color` 不变
  - `hue / saturation / value / current_part / compact_mode / read_only_mode / hex_text` 不变
  - `g_changed_count == 0`，`g_changed_hue / saturation / value == 0xFF`，`g_changed_part == EGUI_VIEW_COLOR_PICKER_PART_NONE`
  - `is_pressed` 与 `pressed_part` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Accent color` 状态，并同步底部 `compact / read only` 静态 preview，输出首帧。
2. 对主控件发送 `Right`，把当前 tone 向高饱和方向推进一格。
3. 输出 tone 调整后的主区截图。
4. 对主控件发送 `Tab + Down`，切到 hue rail 并切换下一条色相。
5. 输出 hue 调整后的主区截图。
6. 程序化切换到第二组主快照 `Signal color`。
7. 输出第二组主快照截图。
8. 保持第二组主快照不变，做最终稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持单一静态对照。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/color_picker PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/color_picker --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/color_picker
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_color_picker
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Accent default / Accent tone adjusted / Accent hue adjusted / Signal color` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `selection / hex / label / helper / palette / region_screen`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_color_picker/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前只覆盖 `hue + tone`，不包含 `alpha channel`。
- 不做文本输入式 `#RRGGBB` 编辑。
- `compact` 与 `read only` 当前只作为静态 reference 对照，不承载真实交互。

## 13. 与现有控件的边界

- 相比 `swatch_picker`：这里保留 `tone palette + hue rail`，不是离散色板列表。
- 相比 `xy_pad`：这里强调颜色语义、当前色预览与 `hex` 摘要，不是二维参数输入。
- 相比 `slider` / `number_box`：这里直接表达颜色选择，不是单轴数值调节或文本输入。

## 14. EGUI 适配时的简化点与约束

- 继续复用 custom 层现有 `color_picker` 绘制与输入语义，不改 `sdk/EmbeddedGUI`。
- 主控件只保留最小必要的 `tone palette + hue rail` 闭环与 focus 切换。
- preview 固定放在底部双列，并统一挂接 static preview API，避免继续承担场景切换或收尾逻辑。
- 先完成 reference 级 `ColorPicker` 收口，再决定是否补充 `alpha`、文本输入或 eyedropper。
