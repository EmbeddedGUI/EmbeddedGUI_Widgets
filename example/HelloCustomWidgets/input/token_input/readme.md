# token_input 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级参考：常见 `TokenInput / Tag editor / recipient field`
- 对应组件语义：`TokenInput`

## 1. 为什么需要这个控件
`token_input` 用来表达“在一个输入槽里连续提交多个离散值”的表单语义，适合收件人、标签、过滤条件和设备分组编辑。它既不是自由文本框，也不是纯展示型 `chips`，而是一个带输入位、token 提交、删除和焦点移动的多值编辑控件。

## 2. 为什么现有控件不够用
- `textinput` 只负责单段文本编辑，不表达多 token 提交与删除。
- `chips` 更偏展示或轻点击，不承担输入位与编辑闭环。
- `auto_suggest_box` 关注建议列表，不覆盖“已提交 token + 当前 draft”混合语义。
- 当前 reference 主线仍需要一版更接近 Fluent / WPF UI `TokenInput` 的标准页面。

## 3. 目标场景与示例概览
- 主区域展示标准 `token_input`，覆盖输入焦点、draft 提交、token 删除与主 snapshot 切换。
- 底部左侧展示 `compact` 静态 preview，只保留紧凑摘要。
- 底部右侧展示 `read only` 静态 preview，只表达静态结果。
- 页面结构收敛为：标题 -> 主 `token_input` -> `compact / read only` 双 preview。

目录：
`example/HelloCustomWidgets/input/token_input/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 180`
- 主控件：`196 x 92`
- 底部容器：`216 x 48`
- 单个 preview：`104 x 48`

视觉约束：
- 使用低噪音浅色 page panel、白色 surface 和轻边框，不保留旧版 guide、状态文案与外部标签。
- token 胶囊保持浅填充和轻描边，不做 showcase 式多色 badge 堆叠。
- 焦点强调只在主控件真实获得 focus、控件可用且非 `read only` 时绘制。
- `compact` 只压缩密度，不改变 `TokenInput` 的核心语义。
- `read only` 仅保留静态结果，不再承担输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 | 初始状态 | 用途 |
| --- | --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 180` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Token Input` | 页面标题 |
| `editor_primary` | `egui_view_token_input_t` | `196 x 92` | `Alice / Ops / QA` | 主交互控件 |
| `editor_compact` | `egui_view_token_input_t` | `104 x 48` | compact | 紧凑静态 preview |
| `editor_read_only` | `egui_view_token_input_t` | `104 x 48` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 token 列表 | 是 | 是 | 是 |
| 输入焦点 | 是 | 否 | 否 |
| draft 录入与提交 | 是 | 否 | 否 |
| token remove | 是 | 否 | 否 |
| overflow 摘要 | 是 | 是 | 是 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 7. 交互语义

### 7.1 主控件
- 触摸 token 区域会把当前 part 切到对应 token 或输入位。
- 触摸输入位后可继续录入 draft。
- `Enter / Comma / Space` 提交当前 draft 为新 token。
- `Backspace / Delete` 删除当前 token，或在输入位删除 draft / 最后一个 token。
- `Left / Right / Home / End / Tab` 在 token 与输入位之间循环。
- 输入位因为 overflow 暂时隐藏时，会保留 draft，并在空间恢复后回到输入位。

### 7.2 非拖拽 release 语义
`token_input` 不是拖拽控件，删除动作必须满足同目标释放：
- `DOWN(remove A) -> UP(remove A)` 才允许删除 token。
- `DOWN(remove A) -> MOVE(remove B) -> UP(remove B)` 不删除。
- `DOWN(remove A) -> MOVE(remove B) -> MOVE(remove A) -> UP(remove A)` 才允许删除。
- `ACTION_MOVE` 只改变按下高亮，不改写原始按下目标。

### 7.3 静态 preview
- `compact` 与 `read only` preview 统一通过 `egui_view_token_input_override_static_preview_api()` 覆盖为静态 API。
- preview 吞掉 touch / key 输入，并在入口先清理残留 `pressed_part / pressed_remove / is_pressed`。
- 点击 preview 只负责清掉主控件 focus，不再承担 token 选择、提交或删除职责。

## 8. 本轮交互收口内容
- 新增统一的 `token_input_clear_pressed_state()`。
- 补齐 setter 链路的 pressed 清理：
  - `set_font()`
  - `set_meta_font()`
  - `set_palette()`
  - `set_placeholder()`
  - `set_tokens()`
  - `add_token()`
  - `remove_token()`
  - `clear_draft()`
  - `set_current_part()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
- 主控件的 `touch / key guard` 在 `read only / disabled` 下会先清残留 pressed 再返回。
- `ACTION_UP / ACTION_CANCEL` 收口到统一清理逻辑，避免交互结束后残留 pressed 污染。
- `compact / read only` preview 全部走静态 API，统一吞掉 touch / key 输入。
- 输入位被 overflow 暂时隐藏时，保留 draft 与 `restore_input_focus`，待布局恢复后再回到输入位；显式切到 token 时会取消这类待恢复标记。
- 主控件外框 accent 只在真实 focus 下绘制，底部 preview 不再出现误导性的 focus ring。

## 9. 交互测试覆盖
`example/HelloUnitTest/test/test_token_input.c` 本轮覆盖了以下回归：
- draft 录入并通过 `Enter / Comma / Space` 提交
- `Backspace / Delete` 对焦点 token、draft 和尾 token 的删除逻辑
- 触摸 remove 图标删除 token
- 同目标 release 语义
- setter 清理 pressed
- `ACTION_CANCEL` 清理 pressed 且不 notify
- `read only / disabled` guard 清理 pressed
- overflow 下隐藏输入位后的 draft 保留与焦点恢复
- static preview 吞输入且不改动 `token_count / current_part / draft / restore_input_focus`

## 10. 录制动作设计
`egui_port_get_recording_action()` 当前录制链路：
1. 应用主控件默认 snapshot、`compact` snapshot 和 `read only` 对照。
2. 抓取初始截图。
3. 触摸主控件输入位，进入输入焦点。
4. 抓取焦点截图。
5. 通过 `N / E / T / Enter` 提交新 token。
6. 抓取新增 token 后截图。
7. 点击新 token 的 remove 图标。
8. 抓取删除结果截图。
9. 程序化切换主 snapshot 到第二组 token。
10. 抓取主 snapshot 切换截图。
11. 程序化切换 `compact` 静态 preview 到第二组摘要。
12. 抓取 preview 切换截图。
13. 点击 `compact` preview，验证静态 preview 只清主控件 focus。
14. 抓取最终截图。

## 11. 编译、检查与验收命令
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=input/token_input PORT=pc
make all APP=HelloCustomWidgets APP_SUB=input/token_input PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/token_input --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- draft、token 文本、remove affordance 和 overflow 摘要必须可辨认。
- 删除动作必须满足同目标 release，不能出现跨 token 误删除。
- 输入位隐藏再恢复后，draft 与焦点恢复链路必须稳定。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 12. 已知限制
- 当前不做拖拽重排、批量粘贴解析和 IME 候选联动。
- 当前仍以固定 snapshot 和固定录制脚本为主，不接动态建议源。
- overflow 摘要只做静态压缩显示，不展开完整 token 面板。

## 13. 与现有控件的差异边界
- 相比 `textinput`：这里是多 token 编辑器，不是单值文本框。
- 相比 `chips`：这里保留输入位与编辑闭环，不是纯展示 chip 列表。
- 相比 `auto_suggest_box`：这里强调“已提交 token + 当前 draft”的混合状态，而不是建议下拉。

## 14. EGUI 适配说明
- 继续复用仓库内现有 `token_input` 基础实现，优先把示例收口到统一的 reference 页面与交互语义。
- 通过静态 preview API 避免底部对照区再维护第二套交互逻辑。
- 当前先保证 release 语义、pressed 清理、隐藏输入恢复和 runtime 渲染稳定，再评估是否继续上升为框架层公共控件。
