# radio_buttons 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 官方语义参考：`WinUI RadioButtons`
- 对应组件：`RadioButtons`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留标准组内互斥选择；底部 `compact / read only` 仅作为静态 preview 对照
- 当前移除内容：旧 preview 快照轮换、键盘驱动录制切换、说明性面板文案和额外收尾态

## 1. 为什么需要这个控件
`radio_buttons` 用于表达“一组互斥选项里始终只保留一个当前值”的标准表单语义，适合通知渠道、同步频率、主题模式这类单选但需要整组呈现的配置场景。它强调组内切换和最终选中结果，而不是单个布尔开关。

## 2. 为什么现有控件不够用
- `radio_button` 更偏单项控件，reference 页面仍需要外层手工组装和整组键盘行为收口。
- `segmented_control` 更偏导航式切换，不等价于表单里的 `RadioButtons` 组选项。
- 如果继续在页面层手工拼装，会把 same-target release、键盘组内切换、静态 preview 输入抑制和布局约束分散到 demo 代码里，难以复用和验证。
- 当前主线仍需要一版与 `Fluent 2 / WPF UI RadioButtons` 语义对齐的 `RadioButtons` reference。

## 3. 当前页面结构
- 标题：`RadioButtons`
- 主区：1 个真实承载整组互斥选择的 `radio_buttons`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Auto / Manual`，选中 `Auto`
- 右侧 preview：`read only`，固定显示 `Desktop / Tablet`，选中 `Tablet`

目录：
- `example/HelloCustomWidgets/input/radio_buttons/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段对 preview 发送输入，也不再依赖 preview 做清焦桥接：

1. 默认态
   `Email / Push / SMS`
   选中：`Email`
2. 快照 2
   `Daily / Weekly / Monthly`
   选中：`Weekly`
3. 快照 3
   `Auto / Light / Dark`
   选中：`Dark`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Auto / Manual`
   选中：`Auto`
2. `read only`
   `Desktop / Tablet`
   选中：`Tablet`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 196`
- 主区：`196 x 90`
- 底部 preview 行：`216 x 60`
- 单个 preview：`104 x 60`
- 页面结构：标题 -> 主 `radio_buttons` -> 底部 `compact / read only`
- 风格约束：浅色 `page panel`、低噪音描边、标准 `RadioButtons` 组项比例和轻量 focus ring，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Email / Push / SMS`，选中 `Email` | `Auto / Manual`，选中 `Auto` | `Desktop / Tablet`，选中 `Tablet` |
| 快照 2 | `Daily / Weekly / Monthly`，选中 `Weekly` | 保持不变 | 保持不变 |
| 快照 3 | `Auto / Light / Dark`，选中 `Dark` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Auto / Light / Dark`，选中 `Dark` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Daily / Weekly / Monthly`
4. 抓取第二组主区快照
5. 切到 `Auto / Light / Dark`
6. 抓取第三组主区快照
7. 保持最终状态不变并等待稳定
8. 抓取最终稳定帧

说明：
- 录制阶段不再用键盘驱动主区切换
- 录制阶段不再轮换底部 preview 文本或选中项
- 底部 preview 统一通过 `egui_view_radio_buttons_override_static_preview_api()` 吞掉 `touch / key`
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `RADIO_BUTTONS_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_radio_buttons.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 item clamp、`set_items()` / `set_current_index()` / `set_font()` / `set_compact_mode()` / `set_read_only_mode()` / `set_palette()` 的 `pressed` 清理、focus 默认选中、组内键盘导航、same-target release、`ACTION_CANCEL` 与 read-only 守卫。
2. 静态 preview 不变性断言
   通过 `radio_buttons_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`on_selection_changed`、`items`、`font`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`item_count`、`current_index`、`compact_mode`、`read_only_mode`、`pressed_index`、`alpha`

同时要求：
- `g_listener_count == 0`
- `g_listener_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE`
- `is_pressed == false`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_buttons PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_buttons --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_buttons
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_buttons
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Email / Push / SMS`、`Daily / Weekly / Monthly`、`Auto / Light / Dark` 3 组可识别状态。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `on_selection_changed / items / font / surface_color / border_color / text_color / muted_text_color / accent_color / item_count / current_index / compact_mode / read_only_mode / pressed_index / alpha / region_screen`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_radio_buttons/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界收敛到 `(54, 142) - (425, 231)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 232` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `radio_button`：这里强调整组互斥选择和组内键盘导航，而不是单项控件拼装。
- 相比 `check_box`：这里不支持多选，始终只保留一个选中项。
- 相比 `segmented_control`：这里保留标准表单字段语义，而不是胶囊式切换器。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Email / Push / SMS`
  - `Daily / Weekly / Monthly`
  - `Auto / Light / Dark`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 旧 preview 快照轮换
  - 键盘驱动录制切换
  - 说明性面板文案
  - 额外收尾态与页面层桥接逻辑

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/radio_buttons PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `radio_buttons` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_buttons --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_radio_buttons/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_buttons`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_buttons`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1495 colors=121`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界为 `(54, 142) - (425, 231)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 232` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Email / Push / SMS`、`Daily / Weekly / Monthly` 与 `Auto / Light / Dark` 三组 reference 快照，最终稳定帧保持 `Auto / Light / Dark`，底部 `compact / read only` preview 全程静态
