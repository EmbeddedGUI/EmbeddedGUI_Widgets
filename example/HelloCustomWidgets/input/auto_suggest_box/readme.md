# auto_suggest_box 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 AutoSuggestBox`
- 开源母本：`WPF UI`
- 对应组件名：`AutoSuggestBox`
- 本轮保留语义：`standard / compact / read only / expanded / focused`
- 本轮移除内容：页面级 guide、状态文案、preview 标签点击桥接、旧录制轨道里的 `compact` 快照切换与 preview 收尾动作
- EGUI 适配说明：继续复用仓库内 `autocomplete -> combobox` 基础实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`auto_suggest_box` 用来表达“在输入语义下展示候选建议，并允许用户快速确认一个结果”的标准场景，适合成员搜索、命令查找、模板选择和最近项匹配。它比纯 `combo_box` 更强调建议列表，也比完整文本框更轻。

## 2. 为什么现有控件不够用

- `combo_box` 更强调固定候选里的当前值选择，不强调建议语义。
- `text_box` 更偏自由输入，不提供标准的建议结果列表表达。
- `menu_flyout` 与 `command_bar` 更偏命令容器，而不是输入字段。
- 仓库里旧版 `auto_suggest_box` 页面虽然已经完成 reference 化，但录制轨道、静态 preview 单测和 README 仍停留在旧口径，没有真正收口到当前 static preview 工作流。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `auto_suggest_box` -> 底部 `compact / read only` 双静态 preview。
- 主区只负责展示真实 `AutoSuggestBox` 的建议展开、键盘导航和提交结果。
- 底部 `compact` preview 固定显示紧凑态建议框，不再承担录制轨道切换职责。
- 底部 `read only` preview 固定显示只读对照态，不再承担任何交互职责。
- 两个 preview 都通过 `hcw_auto_suggest_box_override_static_preview_api()` 收口：
  - 吞掉新增 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed / expanded`
  - 不改 `suggestions / current_index / current_text / region_screen / palette`
  - 不触发 `on_selected`

目标目录：`example/HelloCustomWidgets/input/auto_suggest_box/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主控件：`196 x 34`
- 底部对照行：`216 x 28`
- `compact` preview：`104 x 28`
- `read only` preview：`104 x 28`

视觉约束：

- 使用浅色 page panel、白色字段面和轻边框，不回退到 showcase 式重装饰。
- 展开列表只保留低噪音高亮和单层白底，不引入额外装饰 chrome。
- 焦点强调保留轻量边框，不做 glow。
- `compact` 只缩减尺寸与可见项数量，不改变语义。
- `read only` 保留当前值显示，但不再接管交互。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `AutoSuggest Box` | 页面标题 |
| `control_primary` | `egui_view_autocomplete_t` | `196 x 34` | `Alicia Gomez` | 主建议框 |
| `control_compact` | `egui_view_autocomplete_t` | `104 x 28` | `Recent` | 紧凑静态 preview |
| `control_read_only` | `egui_view_autocomplete_t` | `104 x 28` | `Recent` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Alicia Gomez` | 默认折叠态 |
| 主控件 | people suggestions expanded | 默认建议列表展开 |
| 主控件 | `Allen Park` highlighted | 键盘向下导航后的展开态 |
| 主控件 | `Deploy Worker` committed | 切到命令集后 `Enter + End + Space` 提交结果 |
| `compact` preview | `Recent` | 全程静态对照 |
| `read only` preview | `Recent` | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` 展开/提交与 `Down / Home / End / Enter / Space / Escape` 键盘闭环。
- 包装层 `set_suggestions()`、`set_current_index()` 与样式 helper 必须在切换时清理残留 `pressed / expanded`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例必须验证：
  - `items / item_count / current_index / current_text` 不变
  - `collapsed_height / item_height / max_visible_items / colors / icons / font` 不变
  - `region_screen` 不变
  - `pressed_index / pressed_is_header / is_pressed` 被清理
  - `on_selected` 不触发

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Alicia Gomez`，同时恢复底部 `compact / read only` 静态 preview，并输出首帧。
2. 发送一次 `Down`，展开 people suggestions。
3. 输出展开态截图。
4. 再发送一次 `Down`，把高亮移动到 `Allen Park`。
5. 输出键盘导航态截图。
6. 切换到命令 suggestions，并发送 `Enter + End + Space`，提交 `Deploy Worker`。
7. 输出命令提交结果截图。
8. 保持 `Deploy Worker` 提交结果不变，作为尾帧稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生状态变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持静态一致。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/auto_suggest_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_auto_suggest_box
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `默认 / 展开 / 高亮导航 / 命令提交` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `suggestions / current_index / current_text / region_screen / palette`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_auto_suggest_box/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前版本仍使用固定 suggestions 数组，不做动态过滤。
- 当前不支持输入中高亮匹配字串。
- 当前不做图标化建议项、分组标题或异步结果刷新。

## 13. 与现有控件的边界

- 相比 `combo_box`：这里强调建议输入与候选结果，不是固定当前值字段。
- 相比 `text_box`：这里保留建议列表语义，不强调自由文本编辑能力。
- 相比 `menu_flyout / command_bar`：这里是输入字段，不是命令容器。

## 14. EGUI 适配时的简化点与约束

- 直接复用 `autocomplete / combobox` 基础结构，避免重复实现底层列表逻辑。
- 通过 wrapper 统一收口尺寸、palette、状态清理与静态 preview 语义。
- 主区录制保留最小必要键盘闭环，preview 不再承担轨道切换或点击桥接职责。
- 先完成 reference 级 `AutoSuggestBox` 收口，再决定是否继续补 placeholder、leading icon 或更复杂过滤逻辑。
