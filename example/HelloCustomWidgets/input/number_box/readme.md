# number_box 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`NumberBox`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留标准步进数值输入；底部 `compact / read only` 仅作为静态 preview 对照
- 当前移除内容：页面级 guide、状态说明、preview 清焦桥接、录制阶段 preview 状态切换、文本编辑态、错误提示、额外装饰动画

## 1. 为什么需要这个控件
`number_box` 用于在表单、属性面板和设置页里输入离散数值，例如间距、字号、数量、延时或阈值。它需要比 `slider` 更精确，比 `text_box` 更低风险，也比滚轮式的 `number_picker` 更接近标准桌面表单里的数值输入语义。

## 2. 为什么现有控件不够用
- `slider` 更偏向连续拖动，不适合表达明确步长和离散值。
- `text_box` 是通用文本输入，不自带范围、步进和增减按钮语义。
- `number_picker` 更偏滚轮选择，不是标准表单中的紧凑数值框。
- 当前主线仍需要一个与 `Fluent 2 / WPF UI` 语义对齐的 `NumberBox` reference。

## 3. 当前页面结构
- 标题：`Number Box`
- 主区：一个可真实承载数值切换的 `number_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `12 ms`
- 右侧 preview：`read only`，固定显示 `16 px`

目录：
- `example/HelloCustomWidgets/input/number_box/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段对 preview 发送输入，也不再切换 preview 自身状态：

1. 默认态
   `Spacing = 24 px`
2. 快照 2
   `Spacing = 28 px`
3. 快照 3
   `Spacing = 32 px`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `12 ms`
2. `read only`
   `16 px`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 154`
- 主控件：`196 x 70`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 页面结构：标题 -> 主 `number_box` -> 底部 `compact / read only`
- 风格约束：浅灰 page panel、白色主表面、低噪音边框、清晰的 `- / +` 步进按钮和稳定的数值文本层级

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `24 px` | `12 ms` | `16 px` |
| 快照 2 | `28 px` | 保持不变 | 保持不变 |
| 快照 3 | `32 px` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `24 px` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `28 px`
4. 抓取第二组主区快照
5. 切到 `32 px`
6. 抓取第三组主区快照
7. 回到默认 `24 px`
8. 抓取最终稳定帧

说明：
- 录制阶段不再切换 `compact` preview 到 `14 ms`
- 页面层不再负责 preview 清主控件 focus
- 底部 preview 统一通过 `egui_view_number_box_override_static_preview_api()` 吞掉 `touch / key` 且不改状态
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `NUMBER_BOX_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_number_box.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `range / value / step` clamp、`same-target release`、`compact / read only / disabled` 输入抑制、`touch cancel`、setter 清理 `pressed`
2. 静态 preview 不变性断言
   通过 `number_box_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`on_value_changed`、`font`、`meta_font`、`label`、`suffix`、`helper`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`value`、`min_value`、`max_value`、`step`、`compact_mode`、`read_only_mode`、`pressed_part`、`alpha`、`enable`

同时要求：
- `is_pressed == false`
- `changed_count == 0`
- `changed_value == -1`

验收命令：
```bash
make all APP=HelloCustomWidgets APP_SUB=input/number_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/number_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/number_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_number_box
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Spacing = 24 px`、`Spacing = 28 px` 与 `Spacing = 32 px` 三组 reference 快照必须能从截图中稳定区分。
- 主区 `- / +` 步进按钮、`same-target release` 和 setter 清理链路收口后不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_number_box/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(221, 181) - (235, 189)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 190` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `slider`：这里表达离散数值输入，不是连续拖动。
- 相比 `text_box`：这里保留范围、步长与步进按钮语义。
- 相比 `number_picker`：这里强调标准表单数值框，而不是滚轮式选择。
- 相比 `stepper`：这里表达单个字段的数值输入，不表达流程步骤。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Spacing = 24 px`
  - `Spacing = 28 px`
  - `Spacing = 32 px`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 清焦桥接
  - 录制阶段 preview 状态切换
  - 文本编辑态、错误提示与额外装饰动画

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/number_box PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `number_box` suite `12 / 12`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/number_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_number_box/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/number_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_number_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1175 colors=97`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(221, 181) - (235, 189)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 190` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Spacing = 24 px`、`Spacing = 28 px` 与 `Spacing = 32 px` 三组 reference 快照，最终稳定帧已显式回到默认 `Spacing = 24 px`，底部 `compact / read only` preview 全程静态
