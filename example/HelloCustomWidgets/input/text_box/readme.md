# text_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`TextBox`
- 当前保留形态：`Node 01`、`Node 02`、`empty + Display name`、`compact`、`read only`
- 当前保留交互：主区保留标准单行文本输入、光标移动与键盘提交语义；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、preview 清焦桥接、录制阶段真实键盘轨道、preview 文本同步、showcase 式场景化装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `egui_view_text_box` 包装层与 SDK `textinput` 基础输入能力，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`text_box` 用于承载最基础的单行文本输入，例如显示名称、标签、备注标题和轻量配置字段。主线仓库需要一版与 `Fluent 2 / WPF UI TextBox` 语义对齐的 `TextBox` reference，用来补齐最小文本输入能力。

## 2. 为什么现有控件不够用
- `password_box` 面向密文字段，保留遮罩与 reveal 语义，不适合作为普通文本输入。
- `number_box` 面向离散数值输入，不承担自由文本编辑。
- `search_box` 带固定搜索图标和 clear affordance，语义比普通文本框更具体。
- SDK 虽然已有基础 `textinput`，但当前仓库仍需要完整的 `input/text_box` 包装页、单测和 web 验收闭环。

## 3. 当前页面结构
- 标题：`Text Box`
- 主区：一个标准 `text_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Queued`
- 右侧 preview：`read only`，固定显示 `Managed`

目录：
- `example/HelloCustomWidgets/input/text_box/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 快照，最终稳定帧回到默认 `Node 01`；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   placeholder：`Display name`
   文本：`Node 01`
2. 快照 2
   placeholder：`Display name`
   文本：`Node 02`
3. 快照 3
   placeholder：`Display name`
   文本：空
4. 最终稳定帧
   placeholder：`Display name`
   文本：`Node 01`

底部 preview 在整条轨道中始终固定：

1. `compact`
   placeholder：`Compact`
   文本：`Queued`
2. `read only`
   placeholder：`Read only`
   文本：`Managed`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主控件：`196 x 40`
- 底部 preview 行：`216 x 32`
- 单个 preview：`104 x 32`
- 页面结构：标题 -> 主 `text_box` -> 底部 `compact / read only`
- 风格约束：浅灰 `page panel`、白色主表面、低噪音边框、轻量焦点描边和稳定的文本层级，不回退到 showcase 式说明卡片

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Node 01` | `Queued` | `Managed` |
| 快照 2 | `Node 02` | 保持不变 | 保持不变 |
| 快照 3 | 空文本，仅显示 placeholder | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到默认 `Node 01` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_text_box.c` 当前覆盖 `6` 条用例：

1. 样式 helper 应用预期状态。
   覆盖 `apply_standard_style()`、`apply_compact_style()` 与 `apply_read_only_style()` 对 `background`、`padding`、`text_color`、`cursor_color` 与 enable 状态的设置。
2. 键盘编辑与提交。
   覆盖 `Backspace -> 2 -> Enter`，把 `Node 01` 编辑为 `Node 02`，并校验 `on_submit` 回调结果。
3. 导航键与光标移动。
   覆盖 `Left / Delete / Home / Shift+Z`，校验中间位删除、游标移动与头部插入字符后的文本结果。
4. setter 与 `max_length`。
   覆盖 `set_max_length()`、`set_placeholder()` 与 `set_text()`，要求超长输入按上限截断为 `abcd`。
5. `read only` 样式拦截键盘输入。
   覆盖只读样式后的 `A` 键输入不会改写文本内容。
6. static preview 吞输入且保持状态不变。
   通过 `text_box_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`on_submit`、`font`、`placeholder`、`text_color`、`text_alpha`、`placeholder_color`、`placeholder_alpha`、`cursor_color`、`text`、`text_len`、`cursor_pos`、`max_length`、`cursor_visible`、`align_type`、`scroll_offset_x`、`alpha`、`enable`、`is_focused` 与四向 `padding` 不变，并要求 `is_pressed == false`、`submit_count == 0`、`submitted_text == ""`。

说明：
- 主区当前 README 口径与实现一致：保留标准单行文本编辑、光标移动与 `Enter` 提交语义；录制阶段只导出程序化 reference 快照，不再使用真实键盘输入驱动截图。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留 `TEXT_BOX_DEFAULT_SNAPSHOT`、`TEXT_BOX_RECORD_WAIT`、`TEXT_BOX_RECORD_FRAME_WAIT`、`TEXT_BOX_RECORD_FINAL_WAIT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview。
- 底部 `compact / read only` preview 统一通过 `hcw_text_box_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主区默认 `Node 01` 与底部双静态 preview，请求首帧截图，等待 `TEXT_BOX_RECORD_FRAME_WAIT`。
2. 程序化切到 `Node 02`，等待 `TEXT_BOX_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `TEXT_BOX_RECORD_FRAME_WAIT`。
4. 程序化切到空文本态，等待 `TEXT_BOX_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `TEXT_BOX_RECORD_FRAME_WAIT`。
6. 程序化回到默认 `Node 01`，等待 `TEXT_BOX_RECORD_FINAL_WAIT`。
7. 请求最终稳定帧，并继续等待 `TEXT_BOX_RECORD_FINAL_WAIT`。

说明：
- 录制阶段不再发送 `Backspace`、数字键或 `Enter`。
- 录制阶段不再点击 `compact` preview，也不再依赖 preview 帮主区清焦。
- `apply_primary_default_state()`、`apply_primary_snapshot()`、`apply_preview_states()`、`layout_page()` 与 `request_page_snapshot()` 共同负责统一页面恢复路径。
- 底部 `compact / read only` preview 在整条 `reference` 轨道里必须保持单一静态对照。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/text_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/text_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/text_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_text_box
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Node 01`、`Node 02` 与空文本 placeholder 三组 reference 快照必须能从截图中稳定区分。
- 最终稳定帧必须显式回到默认 `Node 01`。
- 主区文本编辑、光标移动和提交链路收口后不能残留旧 preview 清焦桥接。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_text_box` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_text_box/default`
- 本轮复核结果：
  - 共捕获 `7` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,4,5,6] / [2] / [3]`
  - 主区 RGB 差分边界收敛到 `(45, 196) - (117, 205)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 206` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `password_box`：这里展示明文文本输入，不承担 reveal / hide。
- 相比 `number_box`：这里不做数值范围、步进和后缀约束。
- 相比 `search_box`：这里不包含搜索图标、clear affordance 和搜索入口语义。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Node 01`
  - `Node 02`
  - 空文本 placeholder
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 标准单行文本编辑
  - 光标移动与 `Enter` 提交
  - setter / `max_length` 口径
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide
  - preview 清焦桥接
  - 录制阶段真实键盘轨道
  - preview 文本同步与 showcase 式场景化装饰
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/text_box PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `text_box` suite `6 / 6`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/text_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_text_box/default`
  - 共捕获 `7` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/text_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_text_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0977 colors=103`
- 截图复核结论：
  - 主区覆盖默认 `Node 01`、`Node 02` 与空文本 placeholder 三组 reference 快照
  - 最终稳定帧已显式回到默认 `Node 01`
  - 主区 RGB 差分边界收敛到 `(45, 196) - (117, 205)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 206` 裁切后全程保持单哈希静态
