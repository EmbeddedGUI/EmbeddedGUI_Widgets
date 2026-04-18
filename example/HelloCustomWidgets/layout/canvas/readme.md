# canvas 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Canvas`
- 对应组件语义：`Canvas`
- 本次保留语义：`Pinned notes`、`Status overlay`、`Compact board`、`Pinned`、`Compact`、`absolute positioning`
- 本次删除内容：旧录制末尾“恢复后立即抓帧”的模板化收尾、旧单测 `on_key_event` 直调入口、与当前 static preview 工作流不一致的旧 README 口径
- EGUI 适配说明：目录和 demo 继续使用 `layout/canvas`，底层仍复用 SDK `egui_view_group`；本轮只收口 `reference` 页面结构、录制轨道、单测入口和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`canvas` 用来表达“子项按明确坐标固定放置”的基础布局语义，适合批注板、固定锚点提示、小型 overlay 和轻量地图式信息面板。它负责承载 Fluent / WPF 中那类不依赖顺序流式布局、而是依赖绝对位置表达的信息层。

## 2. 为什么现有控件不够用
- `stack_panel`、`grid`、`wrap_panel` 都围绕顺序或规则流式布局，不适合精确绝对定位。
- `relative_panel` 更强调关系约束，不适合直接表达“放在这个坐标点”。
- 直接裸用 `group` 不能完整承载当前 reference 页面、static preview 和验收闭环。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `canvas` -> 底部 `Pinned / Compact` 双静态 preview。
- 主控件负责展示三组主区状态：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
- 底部左侧是 `Pinned` 静态 preview，固定展示两个锚点卡片的坐标关系。
- 底部右侧是 `Compact` 静态 preview，固定展示紧凑坐标板。
- 两个 preview 统一通过 `hcw_canvas_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改子项坐标或区域
  - 不触发布局外的额外交互

目标目录：`example/HelloCustomWidgets/layout/canvas/`

## 4. 视觉与布局规格
- 根布局：`224 x 240`
- 主面板：`196 x 120`
- 主 `canvas`：`176 x 64`
- 底部对照行：`216 x 76`
- 单个 preview 面板：`104 x 76`
- preview `canvas`：`84 x 30`
- 风格约束：
  - 保持浅色 Fluent surface、低噪音色块和轻量说明文案。
  - 主区只保留坐标、密度和 overlay 排布差异，不叠加额外交互 chrome。
  - 底部 preview 全程静态，不再承担场景切换或收尾刷新职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_canvas` | `egui_view_group_t` | `176 x 64` | `Pinned notes` | 主 `Canvas` |
| `pinned_preview_canvas` | `egui_view_group_t` | `84 x 30` | `Pinned` | 标准静态 preview |
| `compact_preview_canvas` | `egui_view_group_t` | `84 x 30` | `Compact` | 紧凑静态 preview |
| `primary_snapshots` | `canvas_snapshot_t[3]` | - | `Pinned / Status / Compact` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Pinned notes` | 默认绝对定位批注板 |
| 主控件 | `Status overlay` | 固定 overlay 坐标对照 |
| 主控件 | `Compact board` | 紧凑坐标板 |
| `Pinned` preview | `Pinned` | 固定静态对照，验证锚点坐标 |
| `Compact` preview | `Compact` | 固定静态对照，验证紧凑定位 |

## 7. 交互语义与单测要求
- `canvas` 本体不承担选择或提交语义，核心验收点是布局结果与静态 preview 防输入行为。
- `apply_standard_style()`、`apply_compact_style()`、`set_child_origin()` 之后都不能残留旧的 `pressed` 高亮。
- `layout_childs()` 必须覆盖：
  - 标准绝对定位
  - 紧凑定位
- static preview 用例必须验证：
  - `touch / dispatch_key_event()` 输入会被消耗
  - group `pressed / is_pressed` 被清理
  - 子项布局区域保持不变
  - `on_click_listener` 不触发
- 预览态键盘入口统一走 `dispatch_key_event()`，不再直接调用 `on_key_event()`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部双 preview，直接输出默认 `Pinned notes`
2. 切到 `Status overlay`
3. 输出第二组主区状态
4. 切到 `Compact board`
5. 输出第三组主区状态
6. 恢复默认主状态
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `Pinned / Compact` preview 在整条 reference 轨道里保持静态一致，不再包含额外 preview 刷新或“恢复后立刻抓帧”的旧式收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `CANVAS_RECORD_FINAL_WAIT`，中间状态切换仍保留 `CANVAS_RECORD_WAIT / CANVAS_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
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
- 主控件三组主状态必须能直接看出坐标变化或密度变化。
- 坐标切换过程中不能出现裁切、错位、重叠或旧位置残留。
- 单测里的 style helper、布局路径、`dispatch_key_event()` 入口和 static preview 状态保持断言必须全部通过。
- 两个 preview 必须完整可见，不黑白屏、不抖动，并且在所有 runtime 帧里保持静态一致。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_canvas/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `stack_panel`：这里强调显式坐标，不是单轴顺序堆叠。
- 相比 `grid`：这里不表达显式行列关系。
- 相比 `relative_panel`：这里不依赖相对约束，位置由明确坐标决定。
- 相比 `wrap_panel`：这里不做流式排布或自动换行。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
  - `Pinned`
  - `Compact`
  - `absolute positioning`
- 删减的旧桥接与旧口径：
  - 录制末尾“恢复后立即抓帧”的旧式收尾
  - 单测里直接走 `on_key_event()` 的旧入口
  - 与当前 static preview 工作流不一致的 README 结构

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `canvas` suite `4 / 4`
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
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_canvas/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/canvas`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_canvas`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.183 colors=119`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(64, 91) - (415, 212)`
  - 按 `y >= 213` 裁切底部 preview 后保持单一哈希，确认 `Pinned / Compact` preview 全程静态
  - 结论：主区覆盖默认 `Pinned notes`、`Status overlay` 与 `Compact board` 三组 reference 状态，最终稳定帧已显式回到默认快照
