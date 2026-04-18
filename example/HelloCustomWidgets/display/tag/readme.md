# Tag 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`Tag`
- 当前保留语义：主区 `text + secondary text + dismiss`、同目标 release 才提交的 dismiss、`Delete / Backspace / Enter / Space` 键盘 dismiss、`compact / read_only` 静态 preview、静态 preview 输入不变性
- 当前移除内容：旧 `primary_panel`、`heading / note`、底部 `compact_panel / read_only_panel` 包装与说明文案、录制阶段真实 click / key 切换、录制末尾回切默认态恢复帧
- EGUI 适配说明：继续复用当前目录下的 `egui_view_tag` custom view，在 custom 层收口单个 `Tag` 的 Fluent 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`tag` 用来表达“用户已经选中、但仍可轻量撤销的一项内容”。它常见于筛选器、分配状态、文件标签和收件人场景，强调的是单个 chip 的显示与 dismiss 语义，而不是整组 token 容器或状态面板。

## 2. 为什么现有控件不够用
- `badge` 更偏短文本状态标签，不承载 dismiss 交互。
- `info_badge` 强调系统提醒或数量提示，不适合承载可移除的内容项。
- `token_input` 是输入与提交容器，不是独立 `Tag` 的轻量显示语义。

## 3. 当前页面结构
- 标题：`Tag`
- 主区：一个主 `tag` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/tag/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照：

1. 默认态
   文案：`Assigned / Today`
   状态：`Assigned / standard`
   palette：标准中性色 + 蓝色 dismiss accent
2. 快照 2
   文案：`Needs review / 2 files`
   状态：`Needs review / warm`
   palette：暖色 review
3. 快照 3
   文案：`Pinned / Release`
   状态：`Pinned / calm`
   palette：绿色 calm

底部 preview 在整条轨道中始终固定：

1. `compact`
   文案：`Compact / Preview`
   模式：`compact_mode=1`
2. `read_only`
   文案：`System / Locked`
   模式：`compact_mode=1`、`read_only_mode=1`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 140`
- 标题：`224 x 18`
- 主 `tag`：`148 x 28`
- 主状态 label：`224 x 12`
- 底部 preview 行：`184 x 24`
- 单个 preview：`88 x 24`
- 页面结构：标题 -> 主 `tag` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、单个主 `tag` 居中、状态差异只通过主区文案与 palette 表达，底部 preview 不再承担额外说明文案

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| 主文本 | 是 | 是 | 是 |
| 次级文本 | 是 | 是 | 是 |
| dismiss affordance | 是 | 是 | 否 |
| 同目标 release 才提交 dismiss | 是 | 否 | 否 |
| `Delete / Backspace / Enter / Space` dismiss | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Needs review / 2 files`
4. 抓取第二组主区快照
5. 切到 `Pinned / Release`
6. 抓取第三组主区快照
7. 回到默认 `Assigned / standard`
8. 抓取最终稳定帧

说明：
- 录制阶段不再通过真实 click / key 驱动页面切换。
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 主区真实 dismiss 语义仍保留在 `on_primary_tag_dismiss()`，交互正确性改由单测覆盖。
- 底部 preview 统一通过 `egui_view_tag_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `TAG_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_tag.c` 当前覆盖七部分：

1. style helper 与 palette
   覆盖 `apply_standard_style()`、`apply_compact_style()`、`apply_read_only_style()` 对模式位和颜色的更新。
2. setter 更新
   覆盖 `set_text()`、`set_secondary_text()`、`set_font()`、`set_secondary_font()`、`set_icon_font()`、`set_dismissible()`、`set_palette()` 对 `pressed` 状态的清理。
3. dismiss 区域
   覆盖 `get_dismiss_region()` 在 `dismissible` 和 `read_only_mode` 下的可见性变化。
4. touch dismiss
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到 `A` 后 `UP(A)` 才提交。
5. keyboard dismiss
   覆盖 `Delete` 和 `Enter` 对 dismiss listener 的触发。
6. `read_only / disabled` guard
   覆盖只读和禁用时 touch / key 均不触发 dismiss。
7. 静态 preview 不变性断言
   通过 `tag_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / text / secondary_text / font / secondary_font / icon_font / on_dismiss / surface_color / border_color / text_color / secondary_color / accent_color / api / alpha / dismissible / compact_mode / read_only_mode / enable / is_focused / is_pressed / dismiss_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

验收命令：
```bash
make all APP=HelloCustomWidgets APP_SUB=display/tag PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/tag --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/tag
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_tag
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Assigned / standard`、`Needs review / warm`、`Pinned / calm` 三组 reference 快照必须能从截图中稳定区分。
- 主区 dismiss 语义必须继续保持同目标 release 才提交，且不能残留 `pressed` 污染。
- 底部 `compact / read_only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_display_tag/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(92, 162) - (387, 231)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 232` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `badge`：这里表达的是可撤销的内容项，不是纯状态标签。
- 相比 `info_badge`：这里不表达系统提醒或数量提示，重点是 `dismissible chip`。
- 相比 `token_input`：这里不承担输入和提交职责，只负责独立 `Tag` 的显示与 dismiss 语义。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Assigned / standard`
  - `Needs review / warm`
  - `Pinned / calm`
  - `compact`
  - `read_only`
- 删减的装饰或桥接：
  - 旧 `primary_panel`
  - `heading / note` 页面说明块
  - 底部 `compact_panel / read_only_panel` 包装容器与额外说明文案
  - 录制阶段真实 click / key 切换轨道

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/tag PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 本轮按本地 unit 日志复核总计 `845 / 845`，其中 `tag` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=21 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/tag --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_display_tag/default`
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - display `21 / 21` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/tag`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_tag`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1097 colors=115`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(92, 162) - (387, 231)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 232` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Assigned / standard`、`Needs review / warm` 与 `Pinned / calm` 三组 reference 快照，最终稳定帧已显式回到默认态，底部 `compact / read_only` preview 全程静态
