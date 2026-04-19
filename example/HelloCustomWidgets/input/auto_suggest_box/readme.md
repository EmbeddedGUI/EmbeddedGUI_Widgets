# AutoSuggestBox 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`AutoSuggestBox`
- 当前保留形态：`Alicia Gomez`、people suggestions、`Allen Park`、`Deploy Worker`、`compact`、`read only`
- 当前保留交互：主区保留真实建议展开、键盘导航和提交；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态文案、preview 标签点击桥接、录制阶段 `compact` 快照切换与 preview 收尾动作，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `input/auto_suggest_box`，底层仍复用仓库内现有 `hcw_auto_suggest_box` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`auto_suggest_box` 用来表达“在输入语义下展示候选建议，并允许用户快速确认一个结果”的标准场景，适合成员搜索、命令查找、模板选择和最近项匹配。它比纯 `combo_box` 更强调建议列表，也比完整文本框更轻。

## 2. 为什么现有控件不够用
- `combo_box` 更强调固定候选里的当前值选择，不强调建议语义。
- `text_box` 更偏自由输入，不提供标准的建议结果列表表达。
- `menu_flyout` 与 `command_bar` 更偏命令容器，而不是输入字段。
- 仓库里当前 `input/auto_suggest_box` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`AutoSuggest Box`
- 主区：一个保留真实建议展开与提交链路的主 `auto_suggest_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Recent`
- 右侧 preview：`read only`，固定显示 `Recent`

目录：
- `example/HelloCustomWidgets/input/auto_suggest_box/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认折叠态
   `Alicia Gomez`
2. people suggestions expanded
   默认建议列表展开
3. `Allen Park` highlighted
   键盘向下导航后的展开态
4. `Deploy Worker` committed
   切到命令集后 `Enter + End + Space` 提交结果

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Recent`
2. `read only`
   `Recent`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主控件：`196 x 34`
- 底部 preview 行：`216 x 28`
- 单个 preview：`104 x 28`
- 页面结构：标题 -> 主 `auto_suggest_box` -> 底部 `compact / read only`
- 风格约束：使用浅色 page panel、白色字段面和轻边框；展开列表只保留低噪音高亮和单层白底；焦点强调保留轻量边框；`compact` 只缩减尺寸和可见项数量；`read only` 保留当前值显示但不接管交互

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Alicia Gomez` | `Recent` | `Recent` |
| 快照 2 | people suggestions expanded | 保持不变 | 保持不变 |
| 快照 3 | `Allen Park` highlighted | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Deploy Worker` committed | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_auto_suggest_box.c` 当前覆盖 `7` 条用例：

1. suggestions、current index 与 current text 基线。
   覆盖 `set_suggestions()`、`set_current_index()` 与空 suggestions 的回退。
2. 样式 helper 与 params。
   覆盖 `apply_standard_style()`、`apply_compact_style()`、`apply_read_only_style()`、`set_font()`、`set_max_visible_items()`、icons / arrow icons / gap，以及 `apply_params()` / `init_with_params()`。
3. wrapper setters 清理交互状态。
   覆盖 `set_suggestions()`、`set_current_index()` 与 `apply_compact_style()` 对 `pressed / expanded` 的清理。
4. 触摸展开、选择与下拉高度收敛。
   覆盖主区点击展开、选择项提交，以及靠近屏幕底部时的可见项裁剪。
5. 键盘导航与提交。
   覆盖 `Down / Home / End / Enter / Space / Escape` 的展开、导航、提交与收起闭环。
6. `disabled / empty` guard。
   覆盖 `!enable` 与空 suggestions 下的输入拒绝。
7. static preview 吞掉输入且保持状态不变。
   固定校验 `items / item_count / current_index / current_text / collapsed_height / item_height / max_visible_items / colors / icons / font / region_screen` 不变，并要求 `pressed_index / pressed_is_header / is_pressed` 被清理且 `on_selected` 不触发。

说明：
- 主区真实交互继续保留建议展开、键盘导航和提交；录制路径主要走 `Down / Enter / End / Space` 的键盘闭环与显式命令快照切换。
- wrapper setters、样式 helper、`disabled / empty` guard 和 static preview 都统一要求先清理残留 `pressed / expanded`，再处理后续状态。
- 底部 `compact / read only` preview 统一通过 `hcw_auto_suggest_box_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不改 `suggestions / current_index / current_text / region_screen / palette`，也不触发 `on_selected`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件键盘闭环，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Alicia Gomez`，同时恢复底部 `compact / read only` 固定状态并抓取首帧，等待 `AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT`。
2. 发送一次 `Down`，展开 people suggestions，等待 `AUTO_SUGGEST_BOX_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT`。
4. 再发送一次 `Down`，把高亮移动到 `Allen Park`，等待 `AUTO_SUGGEST_BOX_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT`。
6. 切换到命令 suggestions，并发送 `Enter + End + Space`，提交 `Deploy Worker`，等待 `AUTO_SUGGEST_BOX_RECORD_WAIT`。
7. 抓取第四组主区快照，等待 `AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT`。
8. 保持 `Deploy Worker` 提交结果不变并导出最终稳定帧，等待 `AUTO_SUGGEST_BOX_RECORD_FINAL_WAIT`。

说明：
- 录制阶段只有主区状态会变化，底部 `compact / read only` preview 在整条 reference 轨道中保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证默认态、展开态、高亮导航态、命令提交态与最终稳定帧的布局口径一致。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview；录制入口和键盘动作先通过 `focus_primary_box()` 收敛焦点，再进入显式布局后的稳定抓帧路径。
- README 这里按当前 `test.c` 如实保留 `AUTO_SUGGEST_BOX_RECORD_WAIT / AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT / AUTO_SUGGEST_BOX_RECORD_FINAL_WAIT` 三档等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/auto_suggest_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_auto_suggest_box
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `默认 / 展开 / 高亮导航 / 命令提交` 四组 reference 状态必须能从截图中稳定区分。
- 主控件展开、导航、提交以及 wrapper setters / 样式 helper 的状态清理链路收口后不能残留 `pressed / expanded` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_auto_suggest_box` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_auto_suggest_box/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(22, 124) - (457, 283)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 284` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `combo_box`：这里强调建议输入与候选结果，不是固定当前值字段。
- 相比 `text_box`：这里保留建议列表语义，不强调自由文本编辑能力。
- 相比 `menu_flyout / command_bar`：这里是输入字段，不是命令容器。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Alicia Gomez`
  - people suggestions expanded
  - `Allen Park` highlighted
  - `Deploy Worker` committed
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 建议展开
  - 键盘 `Down / Home / End / Enter / Space / Escape`
  - 命令提交
  - 静态 preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide 与状态文案
  - preview 标签点击桥接
  - 录制阶段 `compact` 快照切换与 preview 收尾动作
  - 非必要的分组标题、异步刷新和 showcase 化页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `auto_suggest_box` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_auto_suggest_box/default`
  - 共捕获 `10` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/auto_suggest_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_auto_suggest_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1571 colors=100`
- 截图复核结论：
  - 主区覆盖默认 `Alicia Gomez`、展开态、高亮 `Allen Park` 与 `Deploy Worker` 提交结果四组 reference 状态
  - 最终稳定帧保持 `Deploy Worker`
  - 主区 RGB 差分边界收敛到 `(22, 124) - (457, 283)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 284` 裁切后全程保持单哈希静态
