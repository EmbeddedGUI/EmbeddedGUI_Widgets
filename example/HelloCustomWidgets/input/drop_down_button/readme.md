# drop_down_button 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`DropDownButton`
- 当前保留形态：`Sort`、`Layout`、`Theme`、`Filter`、`compact`、`read only`
- 当前保留交互：主区保留真实 `touch`、`same-target release` 与 `Enter` 键闭环；`snapshot` 切换、`font / meta_font / palette / compact / read only` setter 与 `disabled` guard 统一清理 `pressed`；底部 `compact / read only` preview 继续作为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、旧 preview 快照轮换、preview 清焦桥接、额外收尾态、真实 flyout 定位、多级菜单与 showcase 化包装，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `egui_view_drop_down_button` 包装层与 `egui_view_drop_down_button_override_static_preview_api()`，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件?
`drop_down_button` 用来表达“整个按钮本身就是下拉入口”的标准命令触发语义。它适合排序、布局、主题和筛选这类场景：用户点击整个按钮，进入一组可选动作，而不是像 `split_button` 那样在主动作段和菜单段之间做双入口分工。

## 2. 为什么现有控件不够用
- `split_button` 强调“主动作 + 菜单段”的双入口，不等于整个按钮统一展开。
- `toggle_split_button` 还携带 `checked` 语义，不适合纯下拉入口。
- `menu_flyout` 是独立弹出菜单容器，不是页内按钮控件。
- `command_bar` 承担的是工具栏语义，不是单个下拉按钮语义。
- 当前 reference 主线仍需要一个与 `Fluent 2 / WPF UI` 语义对齐的 `DropDownButton` 示例。

## 3. 当前页面结构
- 标题：`Drop Down Button`
- 主区：一个保留真实点击与键盘闭环的 `drop_down_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick`
- 右侧 preview：`read only`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/drop_down_button/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 快照，最终稳定帧继续保持 `Filter`，不会回到默认 `Sort`；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   `Sort`
2. 快照 2
   `Layout`
3. 快照 3
   `Theme`
4. 快照 4
   `Filter`
5. 最终稳定帧
   保持 `Filter`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Quick`
2. `read only`
   `Locked`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 160`
- 主控件：`196 x 76`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 页面结构：标题 -> 主 `drop_down_button` -> 底部 `compact / read only`
- 风格约束：保持浅色 page panel、低噪音边框和 Fluent 风格的单入口按钮表面；主区只保留 `eyebrow + glyph + title + helper` 的最小完整语义；`compact / read only` 只做静态 reference 对照，不承担额外焦点桥接

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Sort` | `Quick` | `Locked` |
| 快照 2 | `Layout` | 保持不变 | 保持不变 |
| 快照 3 | `Theme` | 保持不变 | 保持不变 |
| 快照 4 | `Filter` | 保持不变 | 保持不变 |
| 最终稳定帧 | 保持 `Filter` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_drop_down_button.c` 当前覆盖 `10` 条用例：

1. `snapshot switching clears pressed state`。
   覆盖 `set_current_snapshot()`、`set_snapshots()` 对 `is_pressed` 的清理，以及无效 snapshot index 的回落。
2. `setters clear pressed state`。
   覆盖 `set_font()`、`set_meta_font()`、`set_palette()` 更新时的 `is_pressed` 清理。
3. `touch click listener`。
   覆盖主区标准 `touch` 点击提交与点击计数。
4. `keyboard enter click listener`。
   覆盖 `Enter` 的 `ACTION_DOWN -> ACTION_UP` 提交闭环。
5. `same-target release requires return to origin`。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到 `A` 后 `UP(A)` 才提交。
6. `touch cancel and key guard clear pressed state`。
   覆盖 `ACTION_CANCEL` 与无关键盘输入对 `is_pressed` 的清理。
7. `compact_mode clears pressed and keeps click behavior`。
   覆盖 `compact_mode` 切换时清理 `is_pressed`，并验证恢复点击后仍可提交。
8. `read_only clears pressed and ignores input`。
   覆盖 `read_only_mode` 切换后触摸 / 键盘输入被吞掉且不残留 `pressed`。
9. `disabled ignores input and clears pressed state`。
   覆盖 `!enable` guard 下的输入抑制与 `pressed` 清理。
10. `static preview consumes input and keeps state`。
    通过 `drop_down_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`snapshots`、`font`、`meta_font`、`on_click_listener`、全部 palette 字段、`snapshot_count`、`current_snapshot`、`compact_mode`、`read_only_mode`、`alpha` 不变，并要求 `click_count == 0` 且 `is_pressed == false`。

说明：
- 主区非拖拽点击继续遵循 `same-target release`：`DOWN(A) -> UP(B)` 不提交，只有回到原命中目标释放才提交。
- `compact / read only / disabled` guard 与 static preview 入口都会先清理残留 `is_pressed`，再吞掉输入返回。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留 `DROP_DOWN_BUTTON_DEFAULT_SNAPSHOT`、`apply_primary_default_state()`、`apply_preview_states()` 与 `focus_primary_button()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主区默认 `Sort` 与底部双静态 preview，请求首帧截图，等待 `DROP_DOWN_BUTTON_RECORD_FRAME_WAIT`。
2. 对主区执行一次真实 `touch` 点击，切到 `Layout`，等待 `DROP_DOWN_BUTTON_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `DROP_DOWN_BUTTON_RECORD_FRAME_WAIT`。
4. 对主区注入一次 `Enter` 键闭环，切到 `Theme`，等待 `DROP_DOWN_BUTTON_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `DROP_DOWN_BUTTON_RECORD_FRAME_WAIT`。
6. 对主区再次执行一次真实 `touch` 点击，切到 `Filter`，等待 `DROP_DOWN_BUTTON_RECORD_WAIT`。
7. 请求第四组主区快照，等待 `DROP_DOWN_BUTTON_RECORD_FRAME_WAIT`。
8. 保持 `Filter` 不变并等待 `DROP_DOWN_BUTTON_RECORD_FINAL_WAIT`。
9. 请求最终稳定帧，并继续等待 `DROP_DOWN_BUTTON_RECORD_FINAL_WAIT`。

说明：
- `DROP_DOWN_BUTTON_DEFAULT_SNAPSHOT = 0`，但最终稳定帧不会回到默认 `Sort`，而是保持 `Filter`。
- 录制阶段只有主区状态会变化；底部 preview 统一通过 `egui_view_drop_down_button_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`。
- 静态 preview 收到输入时只负责清理残留 `is_pressed`，不会改动 `snapshots / current_snapshot / compact_mode / read_only_mode / region_screen / palette`。
- `request_page_snapshot()` 会统一执行 `layout + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照与最终稳定帧口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/drop_down_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_drop_down_button
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Sort`、`Layout`、`Theme` 与 `Filter` 四组 reference 状态必须能从截图中稳定区分。
- 最终稳定帧必须继续保持 `Filter`，不能回到默认 `Sort`。
- 主控件 `touch`、`same-target release`、`Enter` 键闭环，以及 setter / guard 清理链路收口后不能残留 `is_pressed` 污染。
- 底部 `Quick / Locked` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_drop_down_button` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_drop_down_button/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(50, 150) - (429, 257)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 258` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `split_button`：这里是整个按钮统一展开，没有主动作段 / 菜单段双入口。
- 相比 `toggle_split_button`：这里没有 `checked` 复合语义。
- 相比 `menu_flyout`：这里是按钮控件，不是独立弹出菜单容器。
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Sort`
  - `Layout`
  - `Theme`
  - `Filter`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 主区真实 `touch` 点击
  - `same-target release`
  - `Enter` 键闭环
  - `snapshot / setter / compact / read only / disabled` 共享 `pressed` 清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide 与状态说明文案
  - 旧 preview 快照轮换
  - preview 清焦桥接
  - 额外收尾态
  - 真实 flyout 定位
  - 多级菜单与 showcase 化包装
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `drop_down_button` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_drop_down_button/default`
  - 共捕获 `10` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/drop_down_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_drop_down_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.122 colors=135`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(50, 150) - (429, 257)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 258` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Sort`、`Layout`、`Theme` 与 `Filter` 四组 reference 状态，最终稳定帧保持 `Filter`，底部 `compact / read only` preview 全程静态
