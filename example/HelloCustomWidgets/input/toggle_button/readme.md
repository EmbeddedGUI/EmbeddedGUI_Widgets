# toggle_button 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI ToggleButton`
- 开源母本：`WPF UI`
- 对应组件名：`ToggleButton`
- 当前保留形态：`Alerts / On`、`Visible / Off`、`Favorite / On`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release`、`Enter / Space` 键盘闭环与持久 `checked` 语义；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 `guide`、旧 preview 快照轮换、preview 点击清焦桥接、额外收尾态、外部标签、section divider、showcase 化装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用 custom 层现有 `egui_view_toggle_button` 绘制与输入语义，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`toggle_button` 用来表达“按钮本身就是一个持久开关命令”的场景，例如提醒开关、可见性切换、收藏态和固定态。它不是设置页拨杆，也不是带菜单入口的复合命令，而是最小可用的 checked command。

## 2. 为什么现有控件不够用
- `switch` 更像设置项里的开关拨杆，不强调命令按钮语义。
- `button` 只有瞬时动作，不保留 checked 状态。
- `split_button` 与 `toggle_split_button` 都带入了额外菜单或分段入口，信息密度更高。
- 当前 `reference` 主线仍需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleButton`。

## 3. 当前页面结构
- 标题：`Toggle Button`
- 主区：一个保留真实 toggle 语义的 `toggle_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Visible / On`
- 右侧 preview：`read only`，固定显示 `Pinned / On`

目录：
- `example/HelloCustomWidgets/input/toggle_button/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Alerts / On`
2. 快照 2
   `Visible / Off`
3. 快照 3 / 最终稳定帧
   `Favorite / On`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Visible / On`
2. `read only`
   `Pinned / On`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 142`
- 主控件：`196 x 52`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 页面结构：标题 -> 主 `toggle_button` -> 底部 `compact / read only`
- 风格约束：保持浅色 `page panel`、低噪音描边和 Fluent 风格的单按钮 checked 表达；主区只保留图标 + 文本 + on/off 填充色，不再叠加旧 demo 的解释性 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Alerts / On` | `Visible / On` | `Pinned / On` |
| 快照 2 | `Visible / Off` | 保持不变 | 保持不变 |
| 快照 3 / 最终稳定帧 | `Favorite / On` | 保持不变 | 保持不变 |
| `same-target release` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_toggle_button.c` 当前覆盖 `10` 条用例：

1. `touch` 提交切换。
   覆盖主按钮 `DOWN(inside) -> UP(inside)` 会切换 `is_toggled`，并触发 `on_toggled` 回调。
2. `Enter / Space` 键盘切换。
   覆盖键盘完整 `ACTION_DOWN -> ACTION_UP` 闭环下的切换与回调计数。
3. disabled 忽略输入。
   覆盖 disabled 后的 `touch / key` 路径不再切换状态，也不触发新回调。
4. setter 与样式 helper 清理 `pressed`。
   覆盖 `hcw_toggle_button_set_toggled()`、`apply_compact_style()` 与 `apply_read_only_style()`，要求进入 setter / helper 后先清理残留 pressed。
5. 样式 helper 更新 palette。
   覆盖 `standard / compact / read only` 三套 `corner_radius`、`on_color` 与 `off_color`。
6. `touch cancel` 与 disabled guard 清理 `pressed`。
   覆盖 `ACTION_CANCEL` 的首次清理、重复 cancel 返回 `0`，以及触摸按下后切到 disabled 时 `UP` 不再提交。
7. `same-target release` 必须回到原命中区。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后再 `UP(A)` 才提交切换。
8. 无关键盘输入不切换。
   覆盖 `Tab` 这类未处理按键不会改写 `is_toggled`，同时会清理残留 `pressed`。
9. disabled 键盘 guard 清理 `pressed`。
   覆盖已禁用状态下的 `Enter` 输入拒绝与 pressed 清理。
10. static preview 吞输入且保持状态不变。
    通过 `toggle_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`on_toggled`、`is_toggled`、`icon`、`text`、`font`、`icon_font`、`text_color`、`on_color`、`off_color`、`corner_radius`、`icon_text_gap`、`alpha` 不变，并要求 `toggled_count == 0`、`is_pressed == false`。

说明：
- 主控件继续保留标准 toggle 语义：触摸 `DOWN(inside) -> UP(inside)` 才提交切换；只有回到原命中区后 `UP(inside)` 才重新提交。
- `set_toggled()`、样式 helper、`touch cancel`、`!enable` guard 和无关键盘输入都统一要求清理残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 底部 `compact / read only` preview 统一通过 `hcw_toggle_button_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主控件默认 `Alerts / On`，同步底部 `Visible / Pinned` 静态 preview，聚焦主按钮并请求首帧截图，等待 `TOGGLE_BUTTON_RECORD_FRAME_WAIT`。
2. 程序化切到 `Visible / Off`，等待 `TOGGLE_BUTTON_RECORD_WAIT`。
3. 请求第二帧主区截图，等待 `TOGGLE_BUTTON_RECORD_FRAME_WAIT`。
4. 程序化切到 `Favorite / On`，等待 `TOGGLE_BUTTON_RECORD_WAIT`。
5. 请求第三帧主区截图，等待 `TOGGLE_BUTTON_RECORD_FRAME_WAIT`。
6. 保持 `Favorite / On` 不变并等待 `TOGGLE_BUTTON_RECORD_FINAL_WAIT`。
7. 请求最终稳定帧，并继续等待 `TOGGLE_BUTTON_RECORD_FINAL_WAIT`。

说明：
- 录制只允许主区发生变化。底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。
- `apply_primary_default_state()`、`apply_preview_states()`、`layout_page()`、`focus_primary_button()` 与 `request_page_snapshot()` 共同负责统一页面恢复路径。
- runtime 录制阶段不再真实发送 `touch / key` 输入来驱动主区切换。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toggle_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toggle_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Alerts`、`Visible`、`Favorite` 三组可识别状态。
- 主区真实交互仍需保留 `same-target release`、`Enter / Space` 键盘闭环与持久 `checked` 语义。
- 底部 `Visible / Pinned` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `icon / text / on_toggled / on_color / off_color / corner_radius / icon_text_gap / is_toggled / region_screen / alpha`。
- WASM demo 必须能以 `HelloCustomWidgets_input_toggle_button` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_toggle_button/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态
  - 主区 RGB 差分边界收敛到 `(44, 160) - (435, 237)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 238` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `switch`：这里是页内命令按钮，不是设置拨杆。
- 相比 `button`：这里保留 checked 状态，而不是一次性点击动作。
- 相比 `split_button`：这里没有下拉入口。
- 相比 `toggle_split_button`：这里只保留单入口切换，不保留复合分段。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Alerts / On`
  - `Visible / Off`
  - `Favorite / On`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `same-target release`
  - `Enter / Space` 键盘闭环
  - setter / 样式 helper 状态清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 `guide`
  - 旧 preview 快照轮换
  - preview 点击清焦桥接
  - 额外收尾态、外部标签、section divider 与 showcase 化装饰
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/toggle_button PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `toggle_button` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_toggle_button/default`
  - 共捕获 `8` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toggle_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toggle_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1083 colors=105`
- 截图复核结论：
  - 主区覆盖默认 `Alerts / On`、`Visible / Off` 与 `Favorite / On` 三组 reference 状态
  - 最终稳定帧保持 `Favorite / On`
  - 主区 RGB 差分边界收敛到 `(44, 160) - (435, 237)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 238` 裁切后全程保持单哈希静态
