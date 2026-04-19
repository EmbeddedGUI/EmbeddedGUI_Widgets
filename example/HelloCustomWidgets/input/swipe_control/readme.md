# swipe_control 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI SwipeControl`
- 对应组件名：`SwipeControl`
- 当前保留形态：`Inbox surface`、`start action reveal`、`end action reveal`、`Planner surface`、`Review surface`、`compact`、`read only`
- 当前保留交互：主区保留真实拖拽 reveal、surface 点击关闭、action 点击提交、`same-target release` 非拖拽语义，以及 `Left / Right / Tab / Escape` 键盘闭环；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：`compact` preview 状态切换、preview touch 清焦桥接、旧版 guide / 状态回显 / section divider / 外部 preview 标签，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `egui_view_swipe_control` 包装层，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`swipe_control` 用来表达“列表行默认保持整洁，只在用户明确侧滑或键盘切换后暴露上下文操作”的标准交互语义。它适合消息、待办、审批、工作队列这类单行内容密度高、但又需要快速处理动作的场景。

## 2. 为什么现有控件不够用
- `settings_panel`、`data_list_panel` 更偏向静态信息行，不承担 reveal action 语义。
- `split_button`、`toggle_split_button` 是按钮级复合入口，不是整行内容的侧滑暴露。
- 普通 `card` 或 `button` 无法同时表达 `surface / start action / end action` 三段状态。

因此保留 `swipe_control`，但示例页只保留 Fluent / WPF UI 主线需要的最小 reference 结构。

## 3. 当前页面结构
- 标题：`Swipe Control`
- 主区：一个标准 `swipe_control`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Mail / Pocket`
- 右侧 preview：`read only`，固定显示 `Lock / Locked row`
- 页面结构：标题 -> 主 `swipe_control` -> `compact / read only` 双 preview

目录：
- `example/HelloCustomWidgets/input/swipe_control/`

## 4. 主区 reference 快照
主区录制轨道保留 `5` 组 reference 状态，最终稳定帧回到默认 `Inbox` track；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   标题：`Inbox`
   状态：默认 `surface`
2. 快照 2
   标题：`Inbox`
   状态：`start action reveal`
3. 快照 3
   标题：`Inbox`
   状态：`end action reveal`
4. 快照 4
   标题：`Planner`
   状态：主区 `surface`
5. 快照 5
   标题：`Review`
   状态：主区 `surface`
6. 最终稳定帧
   标题：`Inbox`
   状态：默认 `surface`

底部 preview 在整条轨道中始终固定：

1. `compact`
   状态：`Mail / Pocket`
2. `read only`
   状态：`Lock / Locked row`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 222`
- 主控件：`196 x 118`
- 底部预览容器：`216 x 64`
- 单个 preview：`104 x 64`
- 页面结构：标题 -> 主 `swipe_control` -> `compact / read only` 双 preview
视觉约束：
- 使用低噪音浅色 panel 和白色 row shell，不保留旧版 guide、状态回显、外部 preview 标签。
- 主控件保留 `title + helper + row surface + action rail` 四段层级。
- 焦点 ring 只在控件真实获得 focus 且非 `read only` 时绘制，避免静态 preview 误导焦点语义。

## 6. 状态矩阵

| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 surface | 是 | 是 | 是 |
| start action reveal | 是 | 否 | 否 |
| end action reveal | 是 | 否 | 否 |
| `Planner` surface | 是 | 否 | 否 |
| `Review` surface | 是 | 否 | 否 |
| 最终稳定帧回到默认 `Inbox` surface | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |
| 键盘 focus / navigation | 是 | 否 | 否 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_swipe_control.c` 当前覆盖 `11` 条用例：

1. 默认状态。
   覆盖默认 `item / reveal_state / current_part` 是否正确初始化。
2. setter 清理 `pressed`。
   覆盖 `set_title()`、`set_helper()`、`set_item()`、`set_actions()`、`set_reveal_state()`、`set_current_part()`、`set_palette()`、`set_compact_mode()` 与 `set_read_only_mode()` 的残留 `pressed_part / dragging / is_pressed` 清理。
3. 键盘 reveal 闭环。
   覆盖 `Right` 打开 `start action`、`Left` 切到 `end action`、`Escape` 回到 `surface`。
4. `Tab` part 循环。
   覆盖 `surface -> start action -> end action -> surface` 的键盘轮转。
5. 触摸拖拽 reveal。
   覆盖 `surface` 上的左右拖拽分别打开 `start / end action reveal`。
6. 可见 action 的 region 暴露。
   覆盖 reveal 前后 action region 是否按可见性正确暴露。
7. surface 点击关闭 reveal。
   覆盖已 reveal 时 `DOWN(surface) -> UP(surface)` 回到默认 `surface`。
8. 非拖拽 `same-target release` 语义。
   覆盖 `DOWN(A) -> UP(B)` 不提交，只有同目标释放才 close / notify。
9. `ACTION_CANCEL` 清理 pressed 且不 notify。
   覆盖取消触摸时 `pressed_part / dragging / is_pressed` 全部清空，且不改状态。
10. `compact / read only / disabled` guard 清理残留 pressed。
   覆盖三类 guard 下的 `touch / key` 输入先清 pressed 再返回，不改 reveal。
11. static preview 吞输入且清理 pressed。
   覆盖 preview 侧 `touch / key` 输入不会改写 `reveal_state / current_part`。

说明：
- 主控件保留真实 `SwipeControl` 交互：`Right` 打开 `start action`，`Left` 打开 `end action`，`Escape` 关闭 reveal 回到 `surface`，`Tab` 在当前可见 part 间循环。
- `ACTION_DOWN(surface)` 会请求 focus。非拖拽点击遵循同目标释放语义：`DOWN(surface) -> UP(surface)` 才允许关闭或提交；`DOWN(action) -> UP(action)` 才允许提交 action；`DOWN(A) -> UP(B)` 不提交。
- `swipe_control` 保留连续拖拽 reveal 语义，这是该控件必须保留的例外能力：`DOWN(surface) -> MOVE(...) -> UP(...)` 可以根据拖拽方向提交 reveal 状态；该例外已保持在 touch release 检查脚本的 allowlist 语义内，不应按普通点击控件削平。
- `compact` 与 `read only` preview 通过 `egui_view_swipe_control_override_static_preview_api()` 统一覆盖为静态 API；preview 吞掉 `touch / key` 输入，并在入口先清理残留 `pressed_part / dragging / is_pressed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主控件默认 `Inbox` track、同步底部双静态 preview、聚焦主控件并请求首帧截图，等待 `SWIPE_CONTROL_RECORD_FRAME_WAIT`。
2. 通过 `Right` 打开 `start action`，等待 `SWIPE_CONTROL_RECORD_WAIT`。
3. 请求 `start action reveal` 帧，等待 `SWIPE_CONTROL_RECORD_FRAME_WAIT`。
4. 通过 `Left` 切到 `end action`，等待 `SWIPE_CONTROL_RECORD_WAIT`。
5. 请求 `end action reveal` 帧，等待 `SWIPE_CONTROL_RECORD_FRAME_WAIT`。
6. 程序化切到 `Planner` track，并保持主控件 focus，等待 `SWIPE_CONTROL_RECORD_WAIT`。
7. 请求 `Planner` 主状态，等待 `SWIPE_CONTROL_RECORD_FRAME_WAIT`。
8. 程序化切到 `Review` track，并保持主控件 focus，等待 `SWIPE_CONTROL_RECORD_WAIT`。
9. 请求 `Review` 主状态，等待 `SWIPE_CONTROL_RECORD_FRAME_WAIT`。
10. 显式恢复默认 `Inbox` track 并恢复主控件 focus，等待 `SWIPE_CONTROL_RECORD_FINAL_WAIT`。
11. 请求最终稳定帧，并继续等待 `SWIPE_CONTROL_RECORD_FINAL_WAIT`。

说明：
- 录制阶段保留主区 `Right / Left` 键盘 reveal，以覆盖 `SwipeControl` 的核心标准语义。
- `Planner / Review` 两组 row snapshot 改为只在主区程序化切换，不再让 `compact` preview 参与轨道。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留 `SWIPE_CONTROL_RECORD_WAIT`、`SWIPE_CONTROL_RECORD_FRAME_WAIT`、`SWIPE_CONTROL_RECORD_FINAL_WAIT`、`apply_primary_default_state()`、`apply_preview_states()`、`focus_primary_swipe_control()` 与显式布局重放路径。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 reveal、row snapshot 和最终稳定帧口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

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

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- reveal 前后 surface 与 action rail 层级必须清晰。
- 默认 `Inbox`、`start action reveal`、`end action reveal`、`Planner` 与 `Review` 五组 reference 状态必须能从截图中稳定区分。
- 主控件交互结束后不能残留 pressed 污染。
- 最终稳定帧必须显式回到默认 `Inbox` track。
- 底部 preview 必须保持静态 reference，不出现误触发 reveal、误焦点 ring 或残留高亮。
- WASM demo 必须能以 `HelloCustomWidgets_input_swipe_control` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_swipe_control/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6,7] / [8,9]`
  - 主区 RGB 差分边界收敛到 `(53, 110) - (427, 267)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 268` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `settings_panel` / `data_list_panel`：这里的核心是 reveal action，不是静态信息行。
- 相比 `split_button` / `toggle_split_button`：这里的入口是整行 surface，不是按钮本体。
- 相比普通 `card`：这里明确保留 `surface / start action / end action` 三段状态语义。

## 13. 本轮保留与删减
- 保留的主区状态：
  - 默认 `Inbox` surface
  - `start action reveal`
  - `end action reveal`
  - `Planner`
  - `Review`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 真实拖拽 reveal
  - surface 点击关闭
  - action 点击提交
  - `same-target release` 非拖拽语义
  - `Left / Right / Tab / Escape` 键盘闭环
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - `compact` preview 状态切换
  - preview touch 清焦桥接
  - 旧版 guide、状态回显、section divider 和外部 preview 标签
  - 多 action stack、真实图标资源和批量操作工具栏
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `swipe_control` suite `11 / 11`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/swipe_control --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_swipe_control/default`
  - 共捕获 `13` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/swipe_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_swipe_control`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1693 colors=234`
- 截图复核结论：
  - 主区覆盖默认 `Inbox` surface、`start action reveal`、`end action reveal`、`Planner` surface 与 `Review` surface 五组 reference 状态
  - 最终稳定帧已显式回到默认 `Inbox` track
  - 主区 RGB 差分边界收敛到 `(53, 110) - (427, 267)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 268` 裁切后全程保持单哈希静态
