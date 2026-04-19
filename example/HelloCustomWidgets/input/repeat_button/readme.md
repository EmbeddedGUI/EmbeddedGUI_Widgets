# RepeatButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI RepeatButton`
- 参考开源库：`WPF UI`
- 对应组件：`RepeatButton`
- 当前保留形态：`standard`、`compact`、`disabled`
- 当前保留交互：主区保留 immediate click、press-and-hold repeat、移出/移回恢复与 `Space / Enter` 键盘 repeat；底部 `compact / disabled` 统一收口为静态 preview 对照
- 当前移除内容：旧主面板说明文案、preview 说明标签、preview 快照轮换、录制阶段真实 touch / key 长按与额外收尾态
- EGUI 适配说明：继续使用当前目录下的 `egui_view_repeat_button` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下完成 reference 页面、README、录制轨道和单测收口

## 1. 为什么需要这个控件
`repeat_button` 用于表达“按下立即触发一次，继续按住则持续重复触发”的标准命令语义，适合音量步进、数值加减、滚动微调和列表连续翻页这类离散但连续可重复的操作入口。

仓库里已有普通 `button`，但还缺少一个明确承接 press-and-hold repeat 语义、带独立 reference 页面、README、单测和 web 链路的 `RepeatButton`。

## 2. 为什么现有控件不够用
- `button` 只表达单次点击，不表达按住连发。
- `toggle_button` 表达状态切换，不适合连续步进。
- `slider` 适合连续拖拽，不适合离散重复命令。
- 如果继续在 demo 页用普通 `button` 手工拼 repeat timer，会把触摸移出/移回、键盘长按、静态 preview 输入吞掉和 attach / detach timer 恢复语义分散在页面层，难以复用和验证。

## 3. 当前页面结构
- 标题：`RepeatButton`
- 主区：1 个保留真实 repeat 语义的 `repeat_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Fast`
- 右侧 preview：`disabled`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/repeat_button/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段真实驱动长按，也不再让底部 preview 参与轮换：

1. 默认态
   `Volume 12`
2. 快照 2
   `Volume 15`
3. 快照 3
   `Volume 18`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Fast`
2. `disabled`
   `Locked`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主控件：`160 x 40`
- 底部 preview 行：`200 x 32`
- 单个 preview：`96 x 32`
- 页面结构：标题 -> 主 `repeat_button` -> 底部 `compact / disabled`
- 风格约束：浅色圆角 page panel、低噪音布局、标准命令按钮比例和轻量 focus ring，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Volume 12` | `Fast` | `Locked` |
| 快照 2 | `Volume 15` | 保持不变 | 保持不变 |
| 快照 3 | `Volume 18` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Volume 18` | 保持不变 | 保持不变 |
| 立即 click + repeat | 是 | 否 | 否 |
| 移出 / 移回命中区恢复 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Volume 15`
4. 抓取第二组主区快照
5. 切到 `Volume 18`
6. 抓取第三组主区快照
7. 保持最终状态不变并等待稳定
8. 抓取最终稳定帧

说明：
- 主区仍然保留真实 touch 和 keyboard repeat 行为，供运行时手动交互。
- runtime 录制阶段不再真实执行触摸长按或 `Space / Enter` 长按。
- 底部 preview 统一通过 `egui_view_repeat_button_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `REPEAT_BUTTON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_repeat_button.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 immediate click、press-and-hold repeat、移出 / 移回命中区恢复、`Space / Enter` repeat、未处理 key 清理、disabled guard 与 attach / detach timer restore。
2. 静态 preview 不变性断言
   通过 `repeat_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`text`、`icon`、`font`、`icon_font`、`background`、`color`、`alpha`、`icon_text_gap`、`on_click_listener`、`initial_delay_ms`、`repeat_interval_ms`

同时要求：
- `g_click_count == 0`
- `is_pressed == false`
- `touch_active == false`
- `key_active == false`
- timer 已停止

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/repeat_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/repeat_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_repeat_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Volume 12`、`Volume 15`、`Volume 18` 3 组可识别状态。
- 主区真实交互仍需保留 immediate click、press-and-hold repeat、移出 / 移回命中区恢复与 `Space / Enter` repeat 语义。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `region_screen / text / icon / font / icon_font / background / color / alpha / icon_text_gap / on_click_listener / initial_delay_ms / repeat_interval_ms`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_repeat_button/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主按钮实际占位边界收敛到 `(82, 173) - (397, 228)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 245` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `button`：这里强调按下立即触发并支持按住重复，而不是一次性点击。
- 相比 `toggle_button`：这里没有持久选中态。
- 相比 `slider`：这里不承担连续拖拽输入。
- 相比 `number_box`：这里不负责文本输入和解析。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Volume 12`
  - `Volume 15`
  - `Volume 18`
  - `compact`
  - `disabled`
- 删减的装饰或桥接：
  - 旧主面板说明文案
  - preview 说明标签
  - preview 快照轮换
  - 录制阶段真实 touch / key 长按
  - 额外收尾态

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/repeat_button PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `repeat_button` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/repeat_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_repeat_button/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/repeat_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_repeat_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0977 colors=139`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主按钮实际占位边界为 `(82, 173) - (397, 228)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 245` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Volume 12`、`Volume 15` 与 `Volume 18` 三组 reference 快照，最终稳定帧保持 `Volume 18`，底部 `compact / disabled` preview 全程静态
