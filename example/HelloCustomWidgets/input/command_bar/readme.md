# command_bar 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CommandBar`
- 开源母本：`WPF UI`
- 对应组件名：`CommandBar`
- 当前保留形态：`Edit / Save`、`Review / Block`、`Layout / Overflow`、`Publish / Stage`、`compact`、`disabled`
- 当前保留交互：主区保留真实 `Left / Right / Tab / Home / End` 键盘闭环与 `touch` same-target release；底部 `compact / disabled` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 快照轮换、preview 点击清焦桥接、旧录制轨道里的额外收尾态，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `input/command_bar`，底层仍复用仓库内现有 `egui_view_command_bar` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件?
`command_bar` 用来承载页面内长期驻留的一组高频命令，强调主命令、当前 scope 和 overflow 入口的分层表达。它适合编辑、审核、布局和发布等场景，不等同于一次性弹出菜单，也不等同于页面导航。

## 2. 为什么现有控件不够用
- `menu_flyout` 解决的是弹出式菜单，不是常驻命令栏。
- `nav_panel`、`breadcrumb_bar`、`tab_strip` 解决的是导航，不承担主操作语义。
- `split_button`、`drop_down_button` 只表达单个入口，不表达一整组页面命令。
- 仓库里当前 `input/command_bar` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Command Bar`
- 主区：一个保留真实命令导航与选择闭环的 `command_bar`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick`
- 右侧 preview：`disabled`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/command_bar/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：
1. 默认态
   `Edit / Save`
2. 切到 `Review` 主快照后 `Right`
   `Review / Block`
3. 切到 `Layout` 主快照后 `End`
   `Layout / Overflow`
4. 切到 `Publish` 主快照后 `Right`
   `Publish / Stage`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Quick / Save`
2. `disabled`
   `Locked / Save`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 192`
- 主控件：`196 x 88`
- 底部 preview 行：`216 x 64`
- 单个 preview：`104 x 64`
- 页面结构：标题 -> 主 `command_bar` -> 底部 `compact / disabled`
- 风格约束：保持浅色 page panel、低噪音边框和单层命令栏容器；主区只保留 `eyebrow + title + scope + command rail + footer` 的最小完整语义；`compact / disabled` 只做静态 reference 对照；focus 仅在主控件内部命令项间切换，不做夸张 glow 或额外 preview 焦点桥接

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Edit / Save` | `Quick / Save` | `Locked / Save` |
| 快照 2 | `Review / Block` | 保持不变 | 保持不变 |
| 快照 3 | `Layout / Overflow` | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Publish / Stage` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_command_bar.c` 当前覆盖 `10` 条用例：

1. setter 清理 `pressed` 并 clamp。
   覆盖 `set_snapshots()`、`set_current_snapshot()`、`set_current_index()`、`set_compact_mode()`、`set_disabled_mode()` 对 `pressed_index / is_pressed` 的清理。
2. snapshot clamp 与默认 index 解析。
   覆盖 snapshot 数量裁剪、当前 snapshot 回退与默认可用命令解析。
3. snapshot 与 index guard + listener 通知。
   覆盖 `set_current_snapshot()`、`set_current_index()` 的边界、去重与通知路径。
4. 字体、模式、listener 与 palette。
   覆盖 `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_disabled_mode()`、`set_palette()`。
5. `touch` 选择与 hit-testing。
   覆盖内容区命中测试、same-target release 提交和 overflow 项选择。
6. `ACTION_CANCEL`、disabled mode 与 view-disabled guard。
   覆盖取消输入、禁用模式与 view disabled 下的按压态清理和输入拒绝。
7. compact mode 清理 `pressed` 并忽略输入。
   覆盖 compact 后的 `touch / key` 拒绝与模式恢复。
8. 键盘导航与 guard。
   覆盖 `Left / Right / Tab / Home / End` 的主区导航闭环，以及 disabled snapshot 下的导航拒绝。
9. 内部 helper 与 metrics。
   覆盖 tone color、默认索引、宽度测量、hit item、home/end 查找与 disabled tone 混色。
10. static preview 吞掉输入且保持状态不变。
   固定校验 `snapshots / font / meta_font / on_selection_changed / region_screen / alpha / surface_color / section_color / border_color / text_color / muted_text_color / accent_color / success_color / warning_color / danger_color / neutral_color / snapshot_count / current_snapshot / current_index / compact_mode / disabled_mode` 不变，并要求 `changed_count == 0`、`last_index == EGUI_VIEW_COMMAND_BAR_INDEX_NONE`，且 `is_pressed / pressed_index` 被清理。

说明：
- 主区真实交互继续保留 `touch` same-target release 与 `Left / Right / Tab / Home / End` 键盘导航。
- setter、模式切换、disabled guard 和 static preview 都统一要求先清理残留 `pressed_index / is_pressed`，再处理后续状态。
- 底部 `compact / disabled` preview 统一通过 `egui_view_command_bar_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不改 `snapshots / current_snapshot / current_index / region_screen / palette`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件导航闭环，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Edit / Save`，同时恢复底部 `compact / disabled` 固定状态并抓取首帧，等待 `COMMAND_BAR_RECORD_FRAME_WAIT`。
2. 切到 `Review` 主快照并发送 `Right`，把当前命令移动到 `Block`，等待 `COMMAND_BAR_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `COMMAND_BAR_RECORD_FRAME_WAIT`。
4. 切到 `Layout` 主快照并发送 `End`，把当前命令移动到 `Overflow`，等待 `COMMAND_BAR_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `COMMAND_BAR_RECORD_FRAME_WAIT`。
6. 切到 `Publish` 主快照并发送 `Right`，把当前命令移动到 `Stage`，等待 `COMMAND_BAR_RECORD_WAIT`。
7. 抓取第四组主区快照，等待 `COMMAND_BAR_RECORD_FRAME_WAIT`。
8. 保持 `Publish / Stage` 不变并导出最终稳定帧，等待 `COMMAND_BAR_RECORD_FINAL_WAIT`。

说明：
- 录制阶段只有主区状态会变化，底部 `compact / disabled` preview 在整条 reference 轨道中保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证默认态、`Review`、`Layout`、`Publish` 与最终稳定帧的布局口径一致。
- 初始化阶段在 root view 挂载前后统一回放默认态与 preview；录制入口和键盘动作继续通过 `focus_primary_bar()` 收敛焦点，再进入显式布局后的稳定抓帧路径。
- README 这里按当前 `test.c` 如实保留 `COMMAND_BAR_RECORD_WAIT / COMMAND_BAR_RECORD_FRAME_WAIT / COMMAND_BAR_RECORD_FINAL_WAIT` 三档等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar
```

## 10. 验收重点
- 主区和底部 `compact / disabled` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Edit / Save`、`Review / Block`、`Layout / Overflow` 与 `Publish / Stage` 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、键盘导航与 setter 状态清理链路收口后不能残留 `pressed_index / is_pressed` 污染。
- 底部 `compact / disabled` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_command_bar` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_command_bar/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 变化边界位于 `(51, 129) - (428, 248)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 按 `y >= 249` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `menu_flyout`：这里是常驻命令 rail，不是弹出式命令面板。
- 相比 `drop_down_button` / `split_button`：这里承载的是命令组，不是单个入口按钮。
- 相比 `nav_panel` / `breadcrumb_bar` / `tab_strip`：这里表达操作命令，不表达导航结构。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Edit / Save`
  - `Review / Block`
  - `Layout / Overflow`
  - `Publish / Stage`
- 保留的底部对照：
  - `compact`
  - `disabled`
- 保留的交互：
  - `touch` same-target release
  - 键盘 `Left / Right / Tab / Home / End`
  - setter / 模式切换状态清理
  - 静态 preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide 与状态说明
  - preview 快照轮换
  - preview 点击清焦桥接
  - 旧录制轨道中的额外收尾态

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `command_bar` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_command_bar/default`
  - 共捕获 `10` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/command_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_command_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1464 colors=197`
- 截图复核结论：
  - 主区覆盖默认 `Edit / Save`、`Review / Block`、`Layout / Overflow` 与 `Publish / Stage` 四组 reference 状态
  - 最终稳定帧保持 `Publish / Stage`
  - 主区 RGB 差分边界收敛到 `(51, 129) - (428, 248)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / disabled` preview 以 `y >= 249` 裁切后全程保持单哈希静态
