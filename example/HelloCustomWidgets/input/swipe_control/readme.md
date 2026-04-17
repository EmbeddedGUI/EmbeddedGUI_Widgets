# swipe_control 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI SwipeControl`
- 对应组件名：`SwipeControl`

## 1. 为什么需要这个控件
`swipe_control` 用来表达“列表行默认保持整洁，只在用户明确侧滑或键盘切换后暴露上下文操作”的标准交互语义。它适合消息、待办、审批、工作队列这类单行内容密度高、但又需要快速处理动作的场景。

## 2. 为什么现有控件不够用
- `settings_panel`、`data_list_panel` 更偏向静态信息行，不承担 reveal action 语义。
- `split_button`、`toggle_split_button` 是按钮级复合入口，不是整行内容的侧滑暴露。
- 普通 `card` 或 `button` 无法同时表达 `surface / start action / end action` 三段状态。

因此保留 `swipe_control`，但示例页只保留 Fluent / WPF UI 主线需要的最小 reference 结构。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `swipe_control`，覆盖 `Inbox / Planner / Review` 三组 row snapshot。
- 底部左侧展示 `compact` 静态对照。
- 底部右侧展示 `read only` 静态对照。
- 主控件保留真实交互：
  - 触摸侧滑 reveal
  - 触摸 surface 关闭 reveal
  - 触摸 action 点击提交
  - 键盘 `Left / Right / Tab / Escape`
- 底部两个 preview 改为静态 reference，对输入统一吞掉，只承担对照职责。

目录：
`example/HelloCustomWidgets/input/swipe_control/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 222`
- 页面结构：标题 -> 主 `swipe_control` -> `compact / read only` 双 preview
- 主控件：`196 x 118`
- 底部预览容器：`216 x 64`
- 单个 preview：`104 x 64`

视觉约束：
- 使用低噪音浅色 panel 和白色 row shell，不保留旧版 guide、状态回显、外部 preview 标签。
- 主控件保留 `title + helper + row surface + action rail` 四段层级。
- 焦点 ring 只在控件真实获得 focus 且非 `read only` 时绘制，避免静态 preview 误导焦点语义。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 | 初始状态 | 用途 |
| --- | --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 222` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Swipe Control` | 页面标题 |
| `swipe_control_primary` | `egui_view_swipe_control_t` | `196 x 118` | `Inbox` | 主交互控件 |
| `swipe_control_compact` | `egui_view_swipe_control_t` | `104 x 64` | compact | 紧凑静态对照 |
| `swipe_control_read_only` | `egui_view_swipe_control_t` | `104 x 64` | compact + read only | 只读静态对照 |

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 surface | 是 | 是 | 是 |
| start action reveal | 是 | 否 | 否 |
| end action reveal | 是 | 否 | 否 |
| surface tap close | 是 | 否 | 否 |
| action click | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 键盘 focus / navigation | 是 | 否 | 否 |

## 7. 交互语义
### 7.1 主控件
- `Right` 打开 `start action`。
- `Left` 打开 `end action`。
- `Escape` 关闭 reveal，回到 `surface`。
- `Tab` 在当前可见 part 之间循环。
- `ACTION_DOWN(surface)` 会请求 focus。
- 非拖拽点击遵循同目标释放语义：
  - `DOWN(surface) -> UP(surface)` 才允许关闭或提交。
  - `DOWN(action) -> UP(action)` 才允许提交 action。
  - `DOWN(A) -> UP(B)` 不提交。

### 7.2 连续手势例外
`swipe_control` 保留连续拖拽 reveal 语义，这是该控件必须保留的例外能力：
- `DOWN(surface) -> MOVE(...) -> UP(...)` 可以根据拖拽方向提交 reveal 状态。
- 该例外已保持在 touch release 检查脚本的 allowlist 语义内，不应按普通点击控件削平。

### 7.3 静态 preview
- `compact` 与 `read only` preview 通过 `egui_view_swipe_control_override_static_preview_api()` 统一覆盖为静态 API。
- preview 吞掉 touch / key 输入，并在入口先清理残留 `pressed_part / dragging / is_pressed`。
- 点击 preview 只负责清掉主控件 focus，不再承担任何 reveal 或 action 行为。

## 8. 本轮收口内容
- 为 setter 链路补齐 pressed 清理：
  - `set_title()`
  - `set_helper()`
  - `set_palette()`
  - `set_item()`
  - `set_actions()`
  - `set_current_part()`
  - `set_reveal_state()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
- 为 guard 链路补齐 pressed 清理：
  - `compact`
  - `read only`
  - `disabled`
  - `item == NULL`
  - static preview
- 把 `ACTION_UP / ACTION_CANCEL` 收口到统一清理逻辑，避免交互结束后残留 pressed 高亮。
- 非拖拽 surface release 现在要求同目标 `UP(surface)` 才 close / notify，修正了 `DOWN(surface) -> UP(outside)` 也提交的旧问题。
- 主控件 focus ring 只在真实 focus 下绘制，避免 preview 误显示焦点。

## 9. 交互测试覆盖
`example/HelloUnitTest/test/test_swipe_control.c` 本轮覆盖了以下回归：
- 默认状态与键盘 reveal 切换
- `Tab` part 循环
- 触摸拖拽 reveal start / end
- surface tap close
- 非拖拽 release 必须同目标提交
- `ACTION_CANCEL` 清理 pressed 且不 notify
- setter 切换清理 pressed
- `compact / read only / disabled` guard 清理残留 pressed
- static preview 吞输入且不改变 reveal / current part

## 10. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 应用主控件和 `compact` 默认 snapshot。
2. 抓首帧默认 surface。
3. 通过 `Right` 打开 `start action`。
4. 抓取 reveal start 帧。
5. 通过 `Left` 切到 `end action`。
6. 抓取 reveal end 帧。
7. 程序化切到 `Planner` track。
8. 抓取第二组主状态。
9. 程序化切换 `compact` track。
10. 抓取最终收尾帧并保留 `read only` 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`dismiss_primary_swipe_control()`、键盘录制入口、`compact` 对照切换与 `request_page_snapshot()` 都先走显式布局路径，再进入稳定抓帧。

## 11. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/swipe_control --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/swipe_control
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_swipe_control
```

验收重点：
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- reveal 前后 surface 与 action rail 层级必须清晰。
- 主控件交互结束后不能残留 pressed 污染。
- 底部 preview 必须保持静态 reference，不出现误触发 reveal、误焦点 ring 或残留高亮。

## 12. 已知限制
- 当前只做单行 `SwipeControl` reference，不接入真实列表容器。
- 不实现连续 easing 动画、多 action stack、真实图标资源和批量操作工具栏。
- action rail 仍以文本为主，不引入额外图标资源系统。

## 13. 与现有控件的差异边界
- 相比 `settings_panel` / `data_list_panel`：这里的核心是 reveal action，不是静态信息行。
- 相比 `split_button` / `toggle_split_button`：这里的入口是整行 surface，不是按钮本体。
- 相比普通 `card`：这里明确保留 `surface / start action / end action` 三段状态语义。

## 14. EGUI 适配说明
- 通过固定 row snapshot 驱动 reference，优先保证 `480 x 480` 下的稳定审阅。
- `compact` 与 `read only` preview 通过静态 API 收口，避免把对照控件做成第二套可交互实现。
- 交互实现优先保证状态清理、release 语义和 runtime 渲染稳定性，再保留 `SwipeControl` 必需的连续 reveal 例外。

## 15. 当前验收结果（2026-04-17）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `swipe_control` suite `11 / 11`
- catalog / 触摸语义 / 文档编码：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/swipe_control --track reference --timeout 10 --keep-screenshots`
  - `12` 帧输出到 `runtime_check_output/HelloCustomWidgets_input_swipe_control/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/swipe_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_swipe_control`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1693 colors=234`
- 截图复核结论：
  - 全帧 `12` 帧共出现 `5` 组唯一状态，对应默认 surface、start reveal、end reveal、`Planner` 主状态与最终 `compact` 对照
  - 按主区裁切后共出现 `4` 组唯一状态，覆盖主 `SwipeControl` 的四组 reference 状态
  - `compact` preview 在前 `8` 帧与后 `4` 帧分成 `2` 组哈希，对应默认对照与最终对照
  - `read only` preview `12` 帧保持单一哈希；从 `Planner` 主状态切到最终对照帧时，差分只落在 `compact` 预览的 `(29, 297) - (223, 381)`，确认 `read only` 对照未参与变化
