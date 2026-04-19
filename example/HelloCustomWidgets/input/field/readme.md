# field 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React Field`
- 开源母本：`Fluent UI React`
- 对应组件：`Field`
- 当前保留形态：`Notification email`、`API token`、`Owner alias`、`compact`、`read only`
- 当前保留交互：主区保留 `info button / bubble` 的 `touch` same-target release、`Enter / Space` 开关与 `Escape` 关闭语义；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、底部 panel 包装与 heading、录制阶段真实点击 `info button`、preview 清焦桥接、额外收尾轨道，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用当前目录下的 `hcw_field` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 README、reference 录制说明、static preview 语义与验收记录

## 1. 为什么需要这个控件
`field` 用来把 `label`、必填标记、辅助说明、校验信息和轻量 info 入口稳定地组织在同一个字段上下文里。仓库已经有 `text_box`、`info_label` 和 `message_bar`，但还缺少一颗与 `Fluent 2 / Fluent UI React Field` 语义对齐的字段壳层 reference。

## 2. 为什么现有控件不够用
- `text_box` 只覆盖输入框本体，不承载字段级 `label / helper / validation / info`
- `info_label` 更像独立说明入口，不承担字段壳层布局
- `message_bar` 面向整条反馈条，不适合贴在字段底部做低噪音校验提示
- 仓库里当前 `input/field` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Field`
- 主区：一个保留字段壳层、info 入口与气泡语义的标准 `field`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Alias / core-api`
- 右侧 preview：`read only`，固定显示 `Region / North Europe`

目录：
- `example/HelloCustomWidgets/input/field/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Notification email`
   `name@company.com`
   `Used for build alerts only.`
2. 快照 2
   `API token`
   `staging-reader`
   `Required before handoff.`
   `Why required` bubble 打开
3. 快照 3
   `Owner alias`
   `team@example.com`
   `Used by audit routing.`
   `Enter a valid alias before saving.`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Alias / core-api`
2. `read only`
   `Region / North Europe`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 220`
- 主控件：`196 x 126`
- 底部 preview 行：`216 x 54`
- 单个 preview：`104 x 54`
- 页面结构：标题 -> 主 `field` -> 底部 `compact / read only`
- 风格约束：保持浅色 page panel、白色主表面、低噪音边框与字段级辅助文字层级；主区只保留 `label + field box + helper / validation + info button / bubble` 的最小完整语义；`compact / read only` 只做静态 reference 对照

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Notification email` + helper | `Alias / core-api` | `Region / North Europe` |
| 快照 2 | `API token` + required + warning + open bubble | 保持不变 | 保持不变 |
| 快照 3 | `Owner alias` + error | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到默认 `Notification email` + helper | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_field.c` 当前覆盖 `5` 条用例：

1. 样式 helper 与 setter 清理 `pressed_part / is_pressed`。
   覆盖 `apply_compact_style()`、`apply_read_only_style()`、`set_label()`、`set_field_text()`、`set_placeholder()`、`set_helper_text()`、`set_validation_text()`、`set_validation_state()`、`set_required()`、`set_info_title()`、`set_info_body()`、`set_font()`、`set_meta_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`，并补充验证 `set_open()` 后 `bubble` 区域按预期出现。
2. `touch` same-target release 与 cancel。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不开 bubble，`DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才打开 bubble，以及 `ACTION_CANCEL` 只清理 pressed，不改已有 open 状态。
3. 键盘开关与 `Escape` 关闭。
   覆盖 `Enter` 打开、`Space` 关闭、再次 `Enter` 打开与 `Escape` 关闭，且每次交互后不残留 `pressed_part / is_pressed`。
4. static preview 吞输入且保持状态不变。
   通过 `field_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`label`、`field_text`、`placeholder`、`helper_text`、`validation_text`、`info_title`、`info_body`、`font`、`meta_font`、`icon_font`、`on_open_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`error_color`、`bubble_surface_color`、`shadow_color`、`required`、`compact_mode`、`read_only_mode`、`open`、`validation_state`、`pressed_part`、`alpha`、`enable`、`is_focused` 与 `padding` 不变，并要求 `g_open_count == 0`、`g_open_state == 0xFF`。
5. `read only / !enable` guard 清理 pressed 状态。
   覆盖 `read only` 与 disabled 下的 `touch / key` 输入都被拒绝，同时清理残留 `pressed_part / is_pressed`。

说明：
- 主区真实交互继续保留 `info button / bubble` 的 `touch` same-target release、`Enter / Space` 开关与 `Escape` 关闭语义。
- 样式 helper、setter、palette / mode 切换、`read only / !enable` guard 都统一要求先清理残留 `pressed_part / is_pressed`，再处理后续状态。
- 底部 `compact / read only` preview 统一通过 `hcw_field_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不触发 open 变化。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 恢复主区默认 `Notification email`，同时恢复底部 `compact / read only` 固定状态并抓取首帧，等待 `FIELD_RECORD_FRAME_WAIT`。
2. 切到 `API token`，等待 `FIELD_RECORD_WAIT`。
3. 抓取 `API token` 主区快照，等待 `FIELD_RECORD_FRAME_WAIT`。
4. 切到 `Owner alias`，等待 `FIELD_RECORD_WAIT`。
5. 抓取 `Owner alias` 主区快照，等待 `FIELD_RECORD_FRAME_WAIT`。
6. 回到默认 `Notification email` 并等待 `FIELD_RECORD_FINAL_WAIT`。
7. 抓取最终稳定帧，继续等待 `FIELD_RECORD_FINAL_WAIT`。

说明：
- 录制阶段不再真实点击 `info button`，`warning + bubble` 状态直接通过 `hcw_field_set_open()` 程序化设定。
- 底部 preview 统一通过 `hcw_field_override_static_preview_api()` 吞掉 `touch / key` 且不改状态，只负责静态 reference 对照。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。
- `FIELD_DEFAULT_SNAPSHOT` 继续作为默认态入口，初始化阶段在 root view 挂载前后各重放一次默认态与 preview。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/field PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/field --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/field
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_field
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Notification email`、`API token` 与 `Owner alias` 三组 reference 状态必须能从截图中稳定区分。
- 主控件 `info button / bubble`、键盘开关与 setter 状态清理链路收口后不能残留 `pressed_part / is_pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_field` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_field/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(46, 108) - (434, 221)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 222` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `text_box`：这里聚焦字段壳层，不承担真实文本编辑。
- 相比 `info_label`：这里把说明入口放回字段上下文，而不是独立标签。
- 相比 `message_bar`：这里只保留字段级低噪音校验提示，不做整条反馈条。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Notification email`
  - `API token`
  - `Owner alias`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `info button / bubble` same-target release
  - `Enter / Space` 开关与 `Escape` 关闭
  - setter / 样式 helper / palette / mode 切换状态清理
  - static preview 对照
  - 最终稳定帧回到默认字段态
- 删减的旧桥接与旧装饰：
  - 页面级 guide
  - 底部 panel 包装与 heading
  - 录制阶段真实点击 `info button`
  - preview 清焦桥接
  - 额外收尾轨道与旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/field PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `field` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/field --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_field/default`
  - 共捕获 `9` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/field`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_field`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1678 colors=80`
- 截图复核结论：
  - 主区覆盖默认 `Notification email`、`API token` 与 `Owner alias` 三组 reference 状态
  - 最终稳定帧回到默认 `Notification email`
  - 主区 RGB 差分边界收敛到 `(46, 108) - (434, 221)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 222` 裁切后全程保持单哈希静态
