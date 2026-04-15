# button 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 Button`
- 开源母本：`WPF UI`
- 对应组件名：`Button`
- 本轮保留语义：`standard / compact / disabled / focused`
- 本轮移除内容：页面级 guide、状态说明文案、preview 清焦桥接、旧录制轨道里的 `compact` preview 快照切换
- EGUI 适配说明：继续复用 SDK `button` 的基础点击语义，本轮只收口 `reference` 页面结构、键盘闭环、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`button` 是最基本的命令触发控件，用来表达一次性动作，例如部署、同步、确认和派发。`HelloCustomWidgets` 需要一个与 `Fluent 2 / WPF UI` 主线一致的 `Button` reference 页面，作为其它输入控件的最小语义基准。

## 2. 为什么现有控件不够用

- `toggle_button` 表达的是状态切换，不是一次性命令提交。
- `split_button` 与 `drop_down_button` 带有分裂动作或菜单，不是最小 `Button` 语义。
- SDK 虽然已有基础 `button`，但仓库内当前 `input/button` 的 demo 轨道、静态 preview 单测和 README 仍停留在旧 workflow，没有真正收口到当前 static preview 模板。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `button` -> 底部 `compact / disabled` 双静态 preview。
- 主区只保留真实 `Button` 的点击、`Space` 和 `Enter` 键盘触发。
- 底部 `compact` preview 固定显示紧凑次级动作 `Open`，不再承担切换轨道或清焦职责。
- 底部 `disabled` preview 固定显示禁用态 `Queued`，作为纯静态对照。
- 两个 preview 都通过 `hcw_button_override_static_preview_api()` 收口：
  - 吞掉新增 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed`
  - 不改 `text / icon / icon_font / icon_text_gap / region_screen / background / color`

目标目录：`example/HelloCustomWidgets/input/button/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主按钮：`140 x 40`
- 底部对照行：`200 x 32`
- `compact` preview：`96 x 32`
- `disabled` preview：`96 x 32`

视觉约束：

- 主按钮使用 Fluent 风格的蓝色主动作视觉，不叠加 showcase 式装饰容器。
- `compact` preview 只压缩尺寸和间距，保留按钮语义。
- `disabled` preview 使用浅灰低对比视觉，并明确处于禁用态。
- 焦点态只保留轻量 ring，不引入厚阴影、说明标签或额外帮助文案。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 128` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Button` | 页面标题 |
| `button_primary` | `egui_view_button_t` | `140 x 40` | `Deploy` | 主按钮 |
| `button_compact` | `egui_view_button_t` | `96 x 32` | `Open` | 紧凑静态 preview |
| `button_disabled` | `egui_view_button_t` | `96 x 32` | `Queued` | 禁用态静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主按钮 | `Deploy` | 默认态 |
| 主按钮 | `Sync` | 触摸点击后 |
| 主按钮 | `Confirm` | `Space` 触发后 |
| 主按钮 | `Dispatch` | `Enter` 触发后与最终稳定帧 |
| `compact` preview | `Open` | 全程静态对照 |
| `disabled` preview | `Queued` | 全程静态对照 |

## 7. 交互语义与单测口径

- 主按钮继续保留真实 `touch` same-target release、`Space` 和 `Enter` 键盘 click 闭环。
- `set_text()`、`set_icon()`、`set_icon_font()`、`set_icon_text_gap()` 与样式 helper 必须在切换时清理残留 `pressed`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例必须验证：
  - `text / icon / icon_font / icon_text_gap` 不变
  - `region_screen / background / color / alpha` 不变
  - `g_click_count == 0`
  - `is_pressed` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主按钮默认 `Deploy`，同时恢复底部 `compact / disabled` 静态 preview，并输出首帧。
2. 触摸点击主按钮，切到 `Sync`。
3. 输出触摸后的主区截图。
4. 发送 `Space`，切到 `Confirm`。
5. 输出键盘 `Space` 后截图。
6. 发送 `Enter`，切到 `Dispatch`。
7. 输出键盘 `Enter` 后截图。
8. 保持 `Dispatch` 不变，作为尾帧稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生状态变化。底部 `compact / disabled` preview 在整条 reference 轨道里必须保持静态一致。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/button PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_button
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Deploy / Sync / Confirm / Dispatch` 四组可识别状态。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `text / icon / icon_font / icon_text_gap / region_screen / background / color`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_button/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前只覆盖单个命令按钮，不扩展按钮组、图文复合工具栏或菜单按钮。
- 当前不做 hover 动画、主题切换或强调态阴影。
- 页面保持最小 reference 结构，不额外承载说明性标签。

## 13. 与现有控件的边界

- 相比 `toggle_button`：这里是一次性命令触发，不是状态开关。
- 相比 `split_button`：这里没有主次动作分裂。
- 相比 `drop_down_button`：这里没有展开菜单语义。

## 14. EGUI 适配时的简化点与约束

- 继续复用 SDK `button` 的图标文本排版和 same-target release 触摸语义。
- Fluent 风格样式、键盘闭环和静态 preview 语义全部收口在 custom 层，不改 `sdk/EmbeddedGUI`。
- 主按钮保留最小必要的真实触摸和键盘闭环，preview 不再承担清焦、切换或收尾职责。
- 先完成 reference 级 `Button` 收口，再决定是否补充按钮组、强调按钮或工具栏场景。
