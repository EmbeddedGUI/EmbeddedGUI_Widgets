# Canvas 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Canvas`
- 对应组件：`Canvas`
- 当前保留形态：`Pinned notes`、`Status overlay`、`Compact board`、`Pinned`、`Compact`
- 当前保留交互：主区保留绝对坐标布局、标准 / 紧凑排布切换与静态 reference 录制；底部 `Pinned / Compact` preview 保持静态 reference 对照并持续吞掉输入
- 当前移除内容：录制末尾“恢复后立即抓帧”的旧式收尾、单测里直接调用 `on_key_event()` 的旧入口、旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/canvas`，底层仍复用仓库内现有 `hcw_canvas` 封装与 SDK `egui_view_group`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`canvas` 用来表达“子项按明确坐标固定放置”的基础布局语义。它不是流式布局容器，而是 Fluent / WPF 里承接锚点卡片、轻量 overlay 和批注层的 reference 控件。

## 2. 为什么现有控件不够用
- `stack_panel`、`wrap_panel` 都围绕顺序流式布局，不适合精确绝对定位。
- `grid` 强调显式行列关系，不适合表达“任意坐标落点”。
- `relative_panel` 更强调控件间约束关系，不适合直接表达锚点坐标。
- 直接裸用 `group` 不能完整承载当前 reference 页面、static preview 和验收闭环。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `canvas` -> 底部 `Pinned / Compact` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
- 录制最终稳定帧显式回到默认 `Pinned notes`。
- 底部左侧是 `Pinned` 静态 preview，固定显示两个锚点卡片与说明 `Static preview.`。
- 底部右侧是 `Compact` 静态 preview，固定显示紧凑坐标板与说明 `Quiet board.`。
- 两个 preview 统一通过 `hcw_canvas_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 清理 group 残留 `pressed`
  - 不改子项 `region`
  - 不触发 `on_click_listener`

目标目录：
- `example/HelloCustomWidgets/layout/canvas/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Pinned notes`
2. 快照 2
   `Status overlay`
3. 快照 3
   `Compact board`
4. 最终稳定帧
   回到默认 `Pinned notes`

底部 preview 在整条轨道中固定为：
1. `Pinned`
2. `Compact`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 240`
- 主面板：`196 x 120`
- 主 `canvas`：`176 x 64`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- preview `canvas`：`84 x 30`
- 页面结构：标题 -> 主 `canvas` -> 底部 `Pinned / Compact`
- 风格约束：保持浅色 page panel、低噪音卡片色块、轻量 heading 与 note 文案层级，以及稳定的绝对坐标排布；底部 preview 固定为静态 reference 对照，不再承担旧 demo 的额外收尾与交互包装。

## 6. 状态矩阵
| 状态 | 主控件 | Pinned preview | Compact preview |
| --- | --- | --- | --- |
| 默认显示 | `Pinned notes` | `Pinned` | `Compact` |
| 快照 2 | `Status overlay` | 保持不变 | 保持不变 |
| 快照 3 | `Compact board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Pinned notes` | 保持不变 | 保持不变 |
| `set_child_origin()` + `layout_childs()` 绝对定位 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_canvas.c` 当前覆盖 `4` 条用例：

1. `apply_standard_style()`、`apply_compact_style()` 与 `set_child_origin()` 的 `pressed` 清理，以及 helper API 保持 `background == NULL`。
2. `layout_childs()` 在标准 / 紧凑两组坐标下的绝对定位结果。
3. `layout_childs()` 过程中清理 group 残留 `pressed`。
4. static preview 吞掉 `touch / key` 后保持子项 `region`、`background`、`is_pressed` 与 `on_click_listener` 结果不变。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- style helper、`set_child_origin()` 与 static preview 路径都统一要求先清理残留 `pressed`，再处理后续状态。
- 当前 preview 用例继续显式校验 `g_click_count == 0`、preview group `is_pressed == false`，以及全部 preview 子项 `region` 保持不变。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Pinned notes`，同时重放底部 `Pinned / Compact` preview 固定状态并抓取首帧，等待 `CANVAS_RECORD_FRAME_WAIT`。
2. 切到 `Status overlay`，等待 `CANVAS_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照，等待 `CANVAS_RECORD_FRAME_WAIT`。
4. 切到 `Compact board`，等待 `CANVAS_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `CANVAS_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Pinned notes`，同时重放底部 preview 固定状态，等待 `CANVAS_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `CANVAS_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Pinned / Compact` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- `apply_primary_state()` 与 `apply_preview_states()` 只在 `ui_ready` 后触发布局，避免挂载前后的布局口径分叉。
- README 这里按当前 `test.c` 如实保留首轮切换使用 `CANVAS_RECORD_FINAL_WAIT`、第二轮切换与默认回落使用 `CANVAS_RECORD_WAIT`、抓帧使用 `CANVAS_RECORD_FRAME_WAIT`、最终抓帧使用 `CANVAS_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/canvas --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/canvas
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_canvas
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Pinned notes`、`Status overlay`、`Compact board` 三组可识别状态，最终稳定帧必须回到默认 `Pinned notes`。
- 主区真实布局仍需保留绝对坐标落点和标准 / 紧凑排布差异。
- 底部 `Pinned / Compact` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key`，不能改写子项 `region`、点击计数或 `pressed` 状态。
- style helper、`set_child_origin()` 与 static preview 路径都必须先清理残留 `pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_canvas` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_canvas/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(64, 91) - (415, 212)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 213` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Pinned notes`

## 12. 与现有控件的边界
- 相比 `stack_panel`：这里强调显式坐标，不是单轴顺序堆叠。
- 相比 `grid`：这里不表达显式行列关系。
- 相比 `relative_panel`：这里不依赖相对约束，位置由明确坐标决定。
- 相比 `wrap_panel`：这里不做流式排布或自动换行。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
- 保留的底部对照：
  - `Pinned`
  - `Compact`
- 保留的交互：
  - `set_child_origin()` + `layout_childs()` 绝对定位
  - 标准 / 紧凑排布切换
  - static preview 输入吞掉
- 删减的旧桥接与旧轨道：
  - 录制末尾“恢复后立即抓帧”的旧式收尾
  - 单测里直接调用 `on_key_event()` 的旧入口
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `canvas` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/canvas --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_canvas/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/canvas`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_canvas`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.183 colors=119`
- 截图复核结论：
  - 主区覆盖 `Pinned notes / Status overlay / Compact board` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Pinned notes`
  - 主区 RGB 差分边界收敛到 `(64, 91) - (415, 212)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Pinned / Compact` preview 以 `y >= 213` 裁切后全程保持单哈希静态
