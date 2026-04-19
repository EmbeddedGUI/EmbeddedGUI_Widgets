# switch 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI ToggleSwitch`
- 开源母本：`WPF UI`
- 对应组件名：`ToggleSwitch`
- 当前保留形态：`checked + done`、`unchecked + done`、`unchecked + done/cross`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release`、`Enter / Space` 键盘闭环与 `ToggleSwitch` 二态切换语义；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：旧 preview 快照轮换、preview 点击清焦桥接、额外收尾态、说明文案、外部标签、section divider、showcase 化装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `egui_view_switch` 包装层与 SDK `switch` 基础绘制，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`switch` 用来表达“状态切换后立即生效”的设置项开关，适合通知、同步、自动化与权限类场景。它强调轨道与 thumb 的二态反馈，比命令按钮更贴近设置页里的标准开关语义。

## 2. 为什么现有控件不够用
- `toggle_button` 更像页内命令按钮，不是设置行里的拨杆。
- `check_box` 更适合表单字段和清单勾选，不强调轨道式开关反馈。
- `slider` 面向连续值调节，不表达离散的 on/off 切换。
- 当前 `reference` 主线仍需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleSwitch` 页面与验收闭环。

## 3. 当前页面结构
- 标题：`Toggle Switch`
- 主区：一个保留真实 `ToggleSwitch` 语义的 `switch`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `checked + done`
- 右侧 preview：`read only`，固定显示 `checked + done`

目录：
- `example/HelloCustomWidgets/input/switch/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `checked + done`
2. 快照 2
   `unchecked + done`
3. 快照 3 / 最终稳定帧
   `unchecked + done/cross`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `checked + done`
2. `read only`
   `checked + done`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 132`
- 主控件：`108 x 44`
- 底部 preview 行：`160 x 32`
- 单个 preview：`76 x 32`
- 页面结构：标题 -> 主 `switch` -> 底部 `compact / read only`
- 风格约束：保持浅色 `page panel`、低噪音描边和 Fluent 风格的标准 `ToggleSwitch` 比例；主区只保留轨道、thumb、icon 与 focus ring，不再叠加旧 demo 的解释性 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `checked + done` | `checked + done` | `checked + done` |
| 快照 2 | `unchecked + done` | 保持不变 | 保持不变 |
| 快照 3 / 最终稳定帧 | `unchecked + done/cross` | 保持不变 | 保持不变 |
| `same-target release` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_switch.c` 当前覆盖 `7` 条用例：

1. 样式 helper 更新 palette 并清理 `pressed`。
   覆盖 `apply_compact_style()` 与 `apply_read_only_style()` 的 `bk_color_on / bk_color_off / switch_color_on` 更新，以及进入 helper 前后的 pressed 清理。
2. setter 清理 `pressed` 并更新数据。
   覆盖 `set_checked()`、`set_state_icons()` 与 `set_icon_font()`，要求 setter 入口先清理残留 pressed，再更新 checked、图标与字体。
3. `touch` same-target release 只切换一次。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不切换，以及回到 `A` 后 `UP(A)` 才切换并回调一次。
4. `Enter / Space` 键盘切换。
   覆盖完整 `ACTION_DOWN -> ACTION_UP` 闭环下的 checked 状态切换与回调计数。
5. disabled 输入不切换。
   覆盖 disabled 后的 `Space` 与 `touch` 路径不再切换状态，同时清理残留 pressed。
6. 无关键盘输入清理 `pressed`。
   覆盖 `Tab` 这类未处理按键不会改写 checked 状态，但会清理残留 pressed。
7. static preview 吞输入且保持状态不变。
   通过 `switch_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`on_checked_changed`、`is_checked`、`icon_on`、`icon_off`、`icon_font`、`switch_color_on`、`switch_color_off`、`bk_color_on`、`bk_color_off`、`alpha` 不变，并要求 `g_checked_count == 0`、`g_last_checked == 0xFF`、`is_pressed == false`。

说明：
- 主控件继续保留标准 `ToggleSwitch` 语义：触摸 `DOWN(inside) -> UP(inside)` 才提交切换；`DOWN(inside) -> MOVE(outside) -> UP(outside)` 不提交；只有回到原命中区后 `UP(inside)` 才重新提交。
- `set_checked()`、`set_state_icons()`、`set_icon_font()`、样式 helper、`!enable` guard 和无关键盘输入都统一要求清理残留 `pressed`。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 底部 `compact / read only` preview 统一通过 `hcw_switch_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主控件默认 `checked + done`，同步底部双静态 preview，聚焦主开关并请求首帧截图，等待 `SWITCH_RECORD_FRAME_WAIT`。
2. 程序化切到 `unchecked + done`，等待 `SWITCH_RECORD_WAIT`。
3. 请求第二帧主区截图，等待 `SWITCH_RECORD_FRAME_WAIT`。
4. 程序化切到 `unchecked + done/cross`，等待 `SWITCH_RECORD_WAIT`。
5. 请求第三帧主区截图，等待 `SWITCH_RECORD_FRAME_WAIT`。
6. 保持最终状态不变并等待 `SWITCH_RECORD_FINAL_WAIT`。
7. 请求最终稳定帧，并继续等待 `SWITCH_RECORD_FINAL_WAIT`。

说明：
- 录制只允许主区发生变化。底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。
- `apply_primary_default_state()`、`apply_preview_states()`、`layout_page()`、`focus_primary_switch()` 与 `request_page_snapshot()` 共同负责统一页面恢复路径。
- runtime 录制阶段不再真实发送 `touch / key` 输入来驱动主区切换。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/switch PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/switch --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/switch
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_switch
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `checked + done`、`unchecked + done`、`unchecked + done/cross` 三组可识别状态。
- 主区真实交互仍需保留 `same-target release`、`Enter / Space` 键盘闭环与 `ToggleSwitch` 二态切换语义。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `icon_on / icon_off / on_checked_changed / bk_color_on / bk_color_off / switch_color_on / switch_color_off / is_checked / icon_font / region_screen / alpha`。
- WASM demo 必须能以 `HelloCustomWidgets_input_switch` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_switch/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态
  - 主区 RGB 差分边界收敛到 `(132, 168) - (347, 233)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 234` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `toggle_button`：这里是设置页开关，不是页内命令按钮。
- 相比 `check_box`：这里强调轨道与 thumb 的二态反馈，而不是表单勾选。
- 相比 `slider`：这里是离散开关，不提供连续值调节。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `checked + done`
  - `unchecked + done`
  - `unchecked + done/cross`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `same-target release`
  - `Enter / Space` 键盘闭环
  - setter / 样式 helper 状态清理
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 旧 preview 快照轮换
  - preview 点击清焦桥接
  - 额外收尾态、说明文案、外部标签、section divider 与 showcase 化装饰
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/switch PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `switch` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/switch --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_switch/default`
  - 共捕获 `8` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/switch`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_switch`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1007 colors=137`
- 截图复核结论：
  - 主区覆盖默认 `checked + done`、`unchecked + done` 与 `unchecked + done/cross` 三组 reference 状态
  - 最终稳定帧保持 `unchecked + done/cross`
  - 主区 RGB 差分边界收敛到 `(132, 168) - (347, 233)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 234` 裁切后全程保持单哈希静态
