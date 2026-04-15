# combo_box 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ComboBox`
- 开源母本：`WPF UI`
- 对应组件名：`ComboBox`
- 本轮保留语义：`standard / compact / read only / expanded / focused`
- 本轮移除内容：页面级 guide、状态说明文案、preview 快照切换、旧录制轨道里的额外收尾态
- EGUI 适配说明：继续复用 SDK `combobox` 的基础展开与提交语义，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`combo_box` 用来表达在固定候选项里选择一个当前值的标准表单语义，适合工作区切换、模式选择、时间范围、布局密度和默认模板等离散单选场景。它比命令按钮更像输入控件，也比建议输入更强调“当前已选值”。

## 2. 为什么现有控件不够用

- `auto_suggest_box` 更强调建议列表与搜索/匹配语义，不适合作为纯粹的当前值选择器。
- `drop_down_button` 与 `split_button` 更接近命令入口，不承担表单字段语义。
- 仓库里当前 `combo_box` 页面虽然已经 reference 化，但录制轨道、静态 preview 单测和 README 仍保留旧 workflow，没有真正收口到当前 static preview 模板。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `combo_box` -> 底部 `compact / read only` 双静态 preview。
- 主区只保留真实 `ComboBox` 的触摸展开/选择与键盘 `Down / End / Space / Enter` 闭环。
- 底部 `compact` preview 固定显示 `Auto`，不再承担快照切换职责。
- 底部 `read only` preview 固定显示 `Tablet`，作为只读静态对照。
- 两个 preview 都通过 `hcw_combo_box_override_static_preview_api()` 收口：
  - 吞掉新增 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed` 和 `expanded`
  - 不改 `items / current_index / current_text / region_screen / palette / metrics`

目标目录：`example/HelloCustomWidgets/input/combo_box/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主控件：`196 x 34`
- 底部对照行：`216 x 28`
- `compact` preview：`104 x 28`
- `read only` preview：`104 x 28`

视觉约束：

- 保持浅色 page panel、白色字段面和轻边框，不回退到 showcase 式重装饰外壳。
- 展开列表只保留单层白底与低噪音强调，不叠加厚阴影或说明卡片。
- focus 维持轻量边框强调，不做 glow。
- `compact` 只压缩尺寸和可见项数量，不改变 `ComboBox` 语义。
- `read only` 保留当前值展示，但不承担真实输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Combo Box` | 页面标题 |
| `control_primary` | `egui_view_combobox_t` | `196 x 34` | `Work` / collapsed | 主控件 |
| `control_compact` | `egui_view_combobox_t` | `104 x 28` | `Auto` / collapsed | 紧凑静态 preview |
| `control_read_only` | `egui_view_combobox_t` | `104 x 28` | `Tablet` / collapsed | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Work` / collapsed | 默认态 |
| 主控件 | `Work` / expanded | 第一次 `Down` 展开 |
| 主控件 | `Travel` / expanded | 展开后第二次 `Down` 导航 |
| 主控件 | `Compact` / collapsed | 第二组主快照提交结果与最终稳定帧 |
| `compact` preview | `Auto` / collapsed | 全程静态对照 |
| `read only` preview | `Tablet` / collapsed | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` same-target release、`Down / Home / End / Space / Enter / Escape` 键盘闭环。
- `set_items()`、`set_current_index()`、`apply_standard_style()`、`apply_compact_style()` 与 `apply_read_only_style()` 必须在切换时清理残留 `pressed / expanded / pressed_index`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例必须验证：
  - `items / current_index / current_text / font / icon_font / expand_icon / collapse_icon` 不变
  - `region_screen / palette / collapsed_height / item_height / icon_text_gap / max_visible_items` 不变
  - `g_selected_count == 0` 且 `g_last_selected == 0xFF`
  - `is_pressed`、`pressed_index` 与 `pressed_is_header` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Work` 折叠态，同时恢复底部 `compact / read only` 静态 preview，并输出首帧。
2. 发送 `Down`，展开主控件。
3. 输出展开后的主区截图。
4. 再发送一次 `Down`，把当前项导航到 `Travel`。
5. 输出导航后的主区截图。
6. 切换到第二组主快照 `Balanced`，再发送 `Enter -> End -> Space`，提交 `Compact`。
7. 输出提交后的主区截图。
8. 保持 `Compact` 折叠态不变，作为尾帧稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生状态变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持静态一致。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/combo_box PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/combo_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/combo_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_combo_box
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Work collapsed / Work expanded / Travel expanded / Compact collapsed` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `items / current_index / current_text / region_screen / palette / metrics`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_combo_box/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前版本只覆盖固定候选项选择，不做自由输入和过滤。
- 不做多列 item 模板、分组标题或异步数据源。
- 页面保持最小 reference 结构，不额外承载 placeholder、leading icon 或搜索联想。

## 13. 与现有控件的边界

- 相比 `auto_suggest_box`：这里强调当前值与固定候选，不强调搜索建议。
- 相比 `drop_down_button` / `split_button`：这里是表单字段，不是命令入口。
- 相比 SDK `combobox` 示例：这里强调 Fluent reference 页面和静态 preview 收口，而不是基础 API 演示。

## 14. EGUI 适配时的简化点与约束

- 继续复用 SDK `combobox` 的基础绘制和展开/提交语义，避免重复实现底层列表状态机。
- 样式、键盘闭环和静态 preview 语义全部收口在 custom 层，不改 `sdk/EmbeddedGUI`。
- 主控件保留最小必要的真实展开和提交闭环，preview 不再承担切换或收尾职责。
- 先完成 reference 级 `ComboBox` 收口，再决定是否补更复杂的 item 模板或附带文本字段版本。
