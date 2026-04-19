# rich_edit_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 对应组件：`RichEditBox`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留轻量富文本编辑面板、document snapshot 和 preset 语义；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、preview 清焦桥接、录制阶段真实键盘输入、preset 实录切换、preview dismiss 轨道和场景化说明块

## 1. 为什么需要这个控件
`rich_edit_box` 用来表达“同一个输入面板里既能编辑文本，又能保留轻量格式层级”的语义。它适合承载交接摘要、发布说明、回顾记录和诊断备注这类需要标题、正文和段落风格共存的输入场景。

## 2. 为什么现有控件不够用
- `text_box` 只覆盖最小单行文本输入，不承担段落样式和 preset 语义。
- `rich_text_block` 只负责展示，不承担真实编辑、焦点和键盘输入链路。
- `token_input` 和 `shortcut_recorder` 面向结构化录入，不适合连续富文本编辑。
- 主线仓库仍需要一版与 `Fluent 2 / WinUI RichEditBox` 语义对齐的 `RichEditBox` reference 页面、单测和 web 验收闭环。

## 3. 当前页面结构
- 标题：`Rich Edit Box`
- 主区：一个标准 `rich_edit_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact note / Hold draft.`
- 右侧 preview：`read only`，固定显示 `Read only draft / Callout stays muted.`

目录：
- `example/HelloCustomWidgets/input/rich_edit_box/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化 document snapshot，不再在录制阶段发送真实键盘事件、preset 切换事件或 preview 点击：

1. 快照 1
   header：`OPS`
   title：`Shift briefing`
   preset：`Body`
   text：`Line check complete.`
2. 快照 2
   header：`QA`
   title：`Bug scrub`
   preset：`Callout`
   text：`Investigate focus ring drift.`
3. 快照 3
   header：`REL`
   title：`Launch checklist`
   preset：`Checklist`
   text：`Ship build\nVerify smoke`

底部 preview 在整条轨道中始终固定：
1. `compact`
   header：`UI`
   title：`Compact note`
   text：`Hold draft.`
2. `read only`
   header：`LOCK`
   title：`Read only draft`
   footer：`Preview only`
   text：`Callout stays muted.`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 270`
- 主控件：`196 x 146`
- 底部 preview 行：`216 x 82`
- 单个 preview：`104 x 82`
- 页面结构：标题 -> 主 `rich_edit_box` -> 底部 `compact / read only`
- 风格约束：浅灰 page panel、白色主表面、低噪音边框、弱阴影、轻量 focus ring 和稳定的段落层级，不回退到 showcase 式说明卡片

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Shift briefing / Body` | `Compact note` | `Read only draft` |
| 快照 2 | `Bug scrub / Callout` | 保持不变 | 保持不变 |
| 快照 3 | `Launch checklist / Checklist` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `Shift briefing / Body` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Bug scrub`
4. 抓取第二组主区快照
5. 切到 `Launch checklist`
6. 抓取第三组主区快照
7. 回到默认 `Shift briefing`
8. 抓取最终稳定帧

说明：
- 录制阶段不再发送 `R / E / V / I / E / W`
- 录制阶段不再发送 `Tab / Right / Enter`
- 录制阶段不再点击底部 `compact` preview
- 底部 preview 统一通过 `egui_view_rich_edit_box_override_static_preview_api()` 吞掉 `touch / key` 且不改状态
- preview 只负责静态 reference 对照，不再承担清焦或页面状态桥接职责
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `RICH_EDIT_BOX_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_rich_edit_box.inc` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖 document / preset 切换、same-target release、键盘导航、preset apply、文本编辑，以及 `read only / !enable` guard。
2. 静态 preview 不变性断言
   通过 `rich_edit_box_preview_snapshot_t`、`rich_edit_box_capture_preview_snapshot()`、`rich_edit_box_assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`documents`、`font`、`meta_font`、`on_action`、`surface_color`、`border_color`、`editor_color`、`text_color`、`muted_text_color`、`accent_color`、`shadow_color`、`editor_text`、`document_count`、`current_document`、`current_part`、`current_preset`、`applied_preset`、`compact_mode`、`read_only_mode`、`pressed_part`、`text_len`、`alpha`、`enable`、`is_focused`、`padding`

同时要求：
- `is_pressed == false`
- `rich_edit_box_action_count == 0`
- `rich_edit_box_action_document == 0xFF`
- `rich_edit_box_action_preset == 0xFF`
- `rich_edit_box_action_style == 0xFF`
- `rich_edit_box_action_text_length == 0xFF`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/rich_edit_box PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rich_edit_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/rich_edit_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_rich_edit_box
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Shift briefing / Bug scrub / Launch checklist` 三组 reference 快照必须能从截图中稳定区分。
- 主控件的 document / preset / 编辑链路收口后不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_rich_edit_box/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(54, 73) - (425, 274)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 275` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `text_box`：这里保留多段落样式和 preset 语义，而不是最小文本输入。
- 相比 `rich_text_block`：这里保留真实编辑、焦点和键盘链路，而不只是展示。
- 相比 `field`：这里聚焦富文本编辑面板本身，不承载标签、必填标记和帮助信息容器。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Shift briefing / Body`
  - `Bug scrub / Callout`
  - `Launch checklist / Checklist`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 录制阶段 `R / E / V / I / E / W` 键盘输入
  - 录制阶段 `Tab / Right / Enter` 真实导航链
  - 点击底部 `compact` preview 的旧轨道
  - 让 preview 承担清焦或页面状态桥接职责的旧链路

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/rich_edit_box PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `rich_edit_box` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rich_edit_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_rich_edit_box/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/rich_edit_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_rich_edit_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2059 colors=238`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(54, 73) - (425, 274)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 275` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Shift briefing / Body`、`Bug scrub / Callout` 与 `Launch checklist / Checklist` 三组 reference 快照，最终稳定帧已显式回到默认态，底部 `compact / read only` preview 全程静态
