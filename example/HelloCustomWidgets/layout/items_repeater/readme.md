# ItemsRepeater 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 / ItemsRepeater`
- 对应组件：`ItemsRepeater`
- 当前保留形态：`Wrap repeater`、`Stack repeater`、`Strip repeater`、`Compact`、`Read only`
- 当前保留交互：主区保留真实 `same-target release`、`Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击清主控件焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_items_repeater`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`items_repeater` 用来表达“同一批数据按统一 item template 重复排布，但布局本身保持轻量、可嵌入、可按场景切换”的语义，适合最近项摘要、标签集合、轻量资源卡片和多状态条目带这类需要重复呈现同构内容、但又不希望直接上完整 `ListView` 或页面级列表控件的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏带容器语义的 `ListView` 页面，不是轻量的 repeater 宿主。
- `wrap_panel`、`uniform_grid`、`virtualizing_wrap_panel` 更偏布局容器，不负责统一 item template、当前项和提交语义。
- `virtualizing_stack_panel` 解决的是纵向窗口化列表，不覆盖非虚拟化 repeater 的模板化重复布局。
- `scroll_presenter` 更偏滚动承载层，不表达重复项模板和当前项语义。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `items_repeater` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Wrap repeater`
  - `Stack repeater`
  - `Strip repeater`
- 录制最终稳定帧显式回到默认 `Wrap repeater`。
- 底部左侧是 `Compact` 静态 preview，只负责对照窄尺寸下的重复项密度。
- 底部右侧是 `Read only` 静态 preview，只负责对照冻结后的弱化层级。
- 两个 preview 统一通过 `egui_view_items_repeater_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_item / layout_mode`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/items_repeater/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Wrap repeater`
2. 快照 2
   `Stack repeater`
3. 快照 3
   `Strip repeater`
4. 最终稳定帧
   回到默认 `Wrap repeater`

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部 preview 行：`216 x 78`
- 单个 preview：`104 x 78`
- 页面结构：标题 -> 主 `items_repeater` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量 `item / meta / tone` 层级；主区突出同一模板在 `wrap / stack / strip` 三种 preset 下的重复布局变化；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Wrap repeater` | `Compact wrap` | `Read only stack` |
| 快照 2 | `Stack repeater` | 保持不变 | 保持不变 |
| 快照 3 | `Strip repeater` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Wrap repeater` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_items_repeater.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认项回落、空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()`、`set_current_item()` 的 pressed 清理与状态更新。
3. `get_item_region()`、`activate_current_item()` 与 listener 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
5. 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter` 行为，含跨 snapshot 的 `Tab` 切换。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode` 不变并清理 pressed；恢复后继续验证 `Home / End + Enter`。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode` 不变，同时不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Wrap repeater`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Stack repeater`，等待 `ITEMS_REPEATER_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Strip repeater`，等待 `ITEMS_REPEATER_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Wrap repeater`，同时重放底部 preview 固定状态，等待 `ITEMS_REPEATER_RECORD_FINAL_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `ITEMS_REPEATER_RECORD_FINAL_WAIT`。

说明：
- 主区继续保留真实 same-target release 与键盘导航语义，供手动复核和单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/items_repeater PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_repeater --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/items_repeater
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_items_repeater
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Wrap repeater`、`Stack repeater`、`Strip repeater` `3` 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 same-target release、键盘导航与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_items_repeater` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_items_repeater/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(56, 96) - (419, 260)`
  - 以 `y >= 261` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Wrap repeater`

## 12. 与现有控件的边界
- 相比 `wrap_panel`：这里强调模板化重复项与当前项语义，不只是流式布局。
- 相比 `uniform_grid`：这里允许不同宽度权重的 item 模板复用，不只强调规则网格。
- 相比 `data_list_panel`：这里是轻量 repeater，而不是页面级列表容器。
- 相比 `virtualizing_stack_panel`：这里保持非虚拟化模板布局，不引入窗口化列表轨道。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Wrap repeater`
  - `Stack repeater`
  - `Strip repeater`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview 点击清主控件焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/items_repeater PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 当前环境下直接执行 `X:\output\main.exe` 超时且重定向日志为空，本轮按本地 unit 日志复核总计 `845 / 845`，其中 `items_repeater` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_repeater --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_items_repeater/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/items_repeater`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_items_repeater`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.186 colors=322`
- 截图复核结论：
  - 主区覆盖 `Wrap repeater / Stack repeater / Strip repeater` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Wrap repeater`
  - 主区差分边界收敛到 `(56, 96) - (419, 260)`
  - 以 `y >= 261` 裁切后，底部 `Compact / Read only` preview 全程保持单哈希静态
