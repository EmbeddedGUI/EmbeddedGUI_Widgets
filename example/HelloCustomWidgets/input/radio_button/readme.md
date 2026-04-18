# radio_button 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`RadioButton`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留标准互斥选择；底部 `compact / read only` 仅作为静态 preview 对照
- 当前移除内容：旧 preview 快照轮换、preview 清焦桥接、键盘驱动录制切换、额外收尾态、说明文案和场景化装饰

## 1. 为什么需要这个控件
`radio_button` 用于表达“多个候选项中只能选一个”的标准表单语义，适合通知方式、同步策略、频率模式和终端形态这类互斥配置。它强调组内互斥和最终选择结果，而不是单个布尔开关。

## 2. 为什么现有控件不够用
- `check_box` 面向独立布尔字段，不承接组内互斥选择。
- `toggle_button` 更接近命令式按钮，不适合作为表单里的单选项。
- `segmented_control` 强调分段切换和导航表现，不等价于标准 `RadioButton`。
- 当前主线仍需要一版与 `Fluent 2 / WPF UI RadioButton` 语义对齐的 `RadioButton` reference。

## 3. 当前页面结构
- 标题：`Radio Button`
- 主区：3 个互斥 `radio_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Auto / Manual`，选中 `Auto`
- 右侧 preview：`read only`，固定显示 `Desktop / Tablet`，选中 `Tablet`

目录：
- `example/HelloCustomWidgets/input/radio_button/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段对 preview 发送输入，也不再依赖 preview 做清焦桥接：

1. 默认态
   `Home / Alerts / Privacy`
   选中：`Home`
2. 快照 2
   `Email / Push / SMS`
   选中：`Push`
3. 快照 3
   `Daily / Weekly / Monthly`
   选中：`Monthly`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Auto / Manual`
   选中：`Auto`
2. `read only`
   `Desktop / Tablet`
   选中：`Tablet`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主区列：`196 x 98`
- 主区单项：`196 x 30`
- 底部 preview 行：`216 x 52`
- 单个 preview 列：`104 x 52`
- 单个 preview 项：`104 x 24`
- 页面结构：标题 -> 主区 3 个 `radio_button` -> 底部 `compact / read only`
- 风格约束：浅色 `page panel`、低噪音描边、标准 `RadioButton` 比例和轻量 focus ring，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Home / Alerts / Privacy`，选中 `Home` | `Auto / Manual`，选中 `Auto` | `Desktop / Tablet`，选中 `Tablet` |
| 快照 2 | `Email / Push / SMS`，选中 `Push` | 保持不变 | 保持不变 |
| 快照 3 | `Daily / Weekly / Monthly`，选中 `Monthly` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Daily / Weekly / Monthly`，选中 `Monthly` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Email / Push / SMS`
4. 抓取第二组主区快照
5. 切到 `Daily / Weekly / Monthly`
6. 抓取第三组主区快照
7. 保持最终状态不变并等待稳定
8. 抓取最终稳定帧

说明：
- 录制阶段不再用键盘驱动主区切换
- 录制阶段不再轮换底部 preview 文本或选中项
- 底部 preview 统一通过 `hcw_radio_button_override_static_preview_api()` 吞掉 `touch / key`
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `RADIO_BUTTON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_radio_button.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖样式 helper、`set_checked()`、文本 / 字体 / 图标相关 setter 清理 `pressed`、same-target release、`Enter / Space` 键盘提交以及禁用态守卫。
2. 静态 preview 不变性断言
   通过 `radio_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`group`、`is_checked`、`alpha`、`circle_color`、`dot_color`、`text`、`font`、`text_color`、`text_gap`、`mark_style`、`mark_icon`、`icon_font`

同时要求：
- `g_changed_count == 0`
- `g_last_index == -1`
- `is_pressed == false`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_button PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Home / Alerts / Privacy`、`Email / Push / SMS`、`Daily / Weekly / Monthly` 3 组可识别状态。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `group / is_checked / alpha / circle_color / dot_color / text / font / text_color / text_gap / mark_style / mark_icon / icon_font / region_screen`。
- README、录制轨道、单测断言和验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_radio_button/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界收敛到 `(44, 106) - (435, 240)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 241` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `check_box`：这里是互斥组，只允许一个选项保持选中。
- 相比 `segmented_control`：这里保留标准表单字段语义，而不是胶囊式切换器。
- 相比 `toggle_button`：这里强调字段选择结果，而不是命令式按钮反馈。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Home / Alerts / Privacy`
  - `Email / Push / SMS`
  - `Daily / Weekly / Monthly`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 旧 preview 快照轮换
  - preview 清焦桥接
  - 键盘驱动录制切换与额外收尾态
  - 说明文案、场景化装饰和复杂设置行包装

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/radio_button PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `radio_button` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_radio_button/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1632 colors=136`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界为 `(44, 106) - (435, 240)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 241` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Home / Alerts / Privacy`、`Email / Push / SMS` 与 `Daily / Weekly / Monthly` 三组 reference 快照，最终稳定帧保持 `Daily / Weekly / Monthly`，底部 `compact / read only` preview 全程静态
