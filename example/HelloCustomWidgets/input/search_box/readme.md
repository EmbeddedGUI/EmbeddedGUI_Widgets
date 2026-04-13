# search_box 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 次级补充参考：`WPF UI` / `WinUI TextBox + search affordance`
- 对应组件名：`Searchbox`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`、`cleared`
- 删除效果：页面级 guide、建议列表、筛选器、Acrylic、复杂 hover、结果面板和场景化说明卡
- EGUI 适配说明：基于 SDK `textinput` 扩展搜索图标、清空按钮和 static preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`search_box` 用来表达“当前表面内搜索入口”的轻量语义，适合资源页、模板库、设置页和命令面板顶部的快速检索入口。它比普通 `text_box` 多一个稳定可辨识的搜索 affordance，也比 `auto_suggest_box` 更收敛，不强依赖展开候选列表。

## 2. 为什么现有控件不够用

- `text_box` 只覆盖最小文本输入，不带搜索图标和清空入口。
- `auto_suggest_box` 更偏建议选择输入，本轮不需要展开列表。
- `number_box`、`password_box`、`rich_edit_box` 都不是当前这种搜索入口语义。
- `text_box` README 已明确把“带内置搜索图标的派生控件”留作后续扩展，因此这里补齐 `search_box` reference 是合理增量。

## 3. 目标场景与示例概览

- 主区域展示一个标准 `search_box`，用于页内搜索词输入。
- 左下 `compact` preview 展示窄宽度搜索入口。
- 右下 `read only` preview 展示静态锁定搜索词。
- 页面只保留标题、主 `search_box` 和底部 `compact / read only` 双 preview。

目录：
- `example/HelloCustomWidgets/input/search_box/`

## 4. 视觉与布局规格

- 页面尺寸：`480 x 480`
- 根布局：`224 x 136`
- 页面结构：标题 -> 主 `search_box` -> `compact / read only` 双 preview
- 主搜索框尺寸：`196 x 40`
- 底部 preview 行：`216 x 32`
- 单个 preview：`104 x 32`

视觉约束：
- 标准态保持白底轻边框，与 Fluent 2 搜索入口一致。
- 左侧常驻搜索图标，作为最小可辨识 affordance。
- 右侧只在有内容时出现清空按钮，并保持同目标 release 才提交。
- `compact` 只收紧 padding 和图标槽位，不改变搜索语义。
- `read only` 保留搜索图标和静态文本，但不暴露清空动作。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 136` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Search Box` | 页面标题 |
| `box_primary` | `egui_view_search_box_t` | `196 x 40` | focused | 主搜索框 |
| `box_compact` | `egui_view_search_box_t` | `104 x 32` | static preview | 紧凑预览 |
| `box_read_only` | `egui_view_search_box_t` | `104 x 32` | static preview | 只读预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主搜索框 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Roadmap` | `Assets` | `Assets` |
| 键盘编辑 | `Roadmap -> Roadmaps` | 否 | 否 |
| `Enter` submit | 同步结果到 compact | 否 | 否 |
| 清空按钮 | 有内容时显示，点击后清空 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| preview 点击清焦点 | 否 | 是 | 是 |

## 7. 交互语义

- 主控件继续复用 SDK `textinput` 的 `touch -> focus -> key edit` 链路。
- 搜索框左侧搜索图标固定显示，右侧清空按钮只在非空且 enabled 时出现。
- 清空按钮遵循 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不清空
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才清空
- `compact / read only` preview 统一通过 `egui_view_search_box_override_static_preview_api()` 吞掉 `touch / key`。
- 页面层只在 preview `ACTION_DOWN` 时清主搜索框焦点，不在 preview 上追加业务交互。

## 8. 本轮收口内容

- 新增 `egui_view_search_box.h/.c`，作为 `textinput` 的搜索入口包装层。
- 在 custom 层补齐：
  - `standard / compact / read only` 三套样式 helper
  - 搜索图标与清空按钮绘制
  - 清空按钮 same-target release
  - static preview 输入吞掉
- 新增 `test.c` reference 页面，只保留主搜索框与双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、清空按钮、键盘提交、只读保护与静态 preview。

## 9. 录制动作设计

`egui_port_get_recording_action()` 链路：
1. 还原主搜索框、`compact` 和 `read only` 默认状态，并给主搜索框 request focus。
2. 截默认态。
3. 发送 `Backspace`，把 `Roadmap` 改成 `Roadma`。
4. 发送 `S`，得到编辑后的搜索词。
5. 发送 `Enter`，将结果同步到 `compact` preview。
6. 点击主搜索框清空按钮，验证 clear affordance。
7. 点击 `compact` preview 清理主搜索框焦点。
8. 截最终收尾帧。

## 10. 编译、检查与验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=input/search_box PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/search_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/search_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter search_box
```

验收重点：
- 主搜索框与底部双 preview 必须完整可见，不黑屏、不白屏、不裁切。
- 左侧搜索图标必须稳定可辨识，右侧清空按钮只在非空时出现。
- 清空按钮必须满足 same-target release，不能在移出后误清空。
- 主控件必须像标准搜索入口，而不是普通 button 或建议下拉框。
- `compact / read only` preview 必须保持静态 reference，对输入只做吞掉和焦点收尾。

## 11. 已知限制

- 当前只覆盖单行 `Searchbox` reference，不扩展筛选器、历史建议列表或异步搜索结果。
- 当前 `Enter` 只用于提交文本，不接入真实搜索数据源。
- 不做语音入口、最近搜索和富结果模板。

## 12. 与现有控件的差异边界

- 相比 `text_box`：这里补的是搜索图标、清空按钮和搜索入口语义。
- 相比 `auto_suggest_box`：这里不做展开候选列表和建议项选择。
- 相比 `command_bar_flyout`：这里是搜索输入，不是命令面板。

## 13. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Searchbox`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `focused`
  - `cleared`

## 14. EGUI 适配时的简化点与约束

- 继续复用 SDK `textinput` 的文本编辑、光标与提交逻辑。
- 搜索图标、清空按钮与 same-target release 收口在 custom 层，不改 SDK。
- 页面保持最小 reference 结构，先确保控件级闭环，再决定是否补充搜索建议和过滤能力。
