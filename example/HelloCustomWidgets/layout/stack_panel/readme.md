# stack_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF StackPanel`
- 对应组件语义：`StackPanel`
- 本次保留语义：`Review flow`、`Inline tools`、`Compact notes`、`Horizontal`、`Compact`、`ordered stacking`
- 本次删除内容：旧录制末尾“恢复后立即抓帧”的模板化收尾、旧单测 `on_key_event` 直调入口、与当前 static preview 工作流不一致的旧 README 口径
- EGUI 适配说明：目录和 demo 继续使用 `layout/stack_panel`，底层仍复用 SDK `egui_view_linearlayout`；本轮只收口 `reference` 页面结构、录制轨道、单测入口和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`stack_panel` 用来表达“子项按单一方向顺序堆叠”的基础布局语义，适合审核流程、工具条、摘要列表和轻量信息面板。它是 Fluent / WPF 里最常见的基础容器之一，适合作为多个高层布局控件的 reference 语义底座。

## 2. 为什么现有控件不够用
- `grid` 更强调显式列布局，不适合纯顺序堆叠。
- `dock_panel` 负责停靠边和剩余区域填充，不是顺序布局容器。
- `wrap_panel` 会自动换行，不适合固定阅读顺序的单轴内容流。
- 直接裸用 `linearlayout` 不能完整承载当前 reference 页面、static preview 和验收闭环。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `stack_panel` -> 底部 `Horizontal / Compact` 双静态 preview。
- 主控件负责展示三组主区状态：
  - `Review flow`
  - `Inline tools`
  - `Compact notes`
- 底部左侧是 `Horizontal` 静态 preview，固定展示横向操作条堆叠语义。
- 底部右侧是 `Compact` 静态 preview，固定展示紧凑纵向堆叠语义。
- 两个 preview 统一通过 `hcw_stack_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `orientation / align_type / auto_size`
  - 不触发布局外的额外交互

目标目录：`example/HelloCustomWidgets/layout/stack_panel/`

## 4. 视觉与布局规格
- 根布局：`224 x 226`
- 主面板：`196 x 116`
- 主 `stack_panel`：`176 x 64`
- 底部对照行：`216 x 74`
- 单个 preview 面板：`104 x 74`
- `Horizontal` preview 的内部堆叠区：`84 x 34`
- `Compact` preview 的内部堆叠区：`84 x 32`
- 风格约束：
  - 保持浅色 Fluent surface、低噪音色块和轻量文字层级。
  - 主区只保留堆叠方向、密度和 item 顺序差异，不叠加额外交互 chrome。
  - 底部 preview 全程静态，不再承担场景切换或收尾刷新职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_stack_panel` | `egui_view_linearlayout_t` | `176 x 64` | `Review flow` | 主 `StackPanel` |
| `horizontal_preview_stack` | `egui_view_linearlayout_t` | `84 x 34` | `Horizontal` | 横向静态 preview |
| `compact_preview_stack` | `egui_view_linearlayout_t` | `84 x 32` | `Compact` | 紧凑静态 preview |
| `primary_snapshots` | `stack_snapshot_t[3]` | - | `Review / Inline / Compact` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Review flow` | 默认纵向阅读流 |
| 主控件 | `Inline tools` | 横向工具条 |
| 主控件 | `Compact notes` | 紧凑纵向摘要流 |
| `Horizontal` preview | `Horizontal` | 固定静态对照，验证横向堆叠 |
| `Compact` preview | `Compact` | 固定静态对照，验证紧凑纵向堆叠 |

## 7. 交互语义与单测要求
- `stack_panel` 本体不承担选择或提交语义，核心验收点是布局结果与静态 preview 防输入行为。
- `apply_standard_style()`、`apply_horizontal_style()`、`apply_compact_style()`、`set_orientation()`、`set_align_type()` 之后都不能残留旧的 `pressed` 高亮。
- `layout_childs()` 必须覆盖：
  - 标准纵向堆叠
  - 横向排布
  - 紧凑纵向排布
- static preview 用例必须验证：
  - `touch / dispatch_key_event()` 输入会被消耗
  - `is_orientation_horizontal / align_type / is_auto_width / is_auto_height` 保持不变
  - 子项布局区域保持不变
  - `on_click_listener` 不触发
  - `pressed / is_pressed` 被清理
- 预览态键盘入口统一走 `dispatch_key_event()`，不再直接调用 `on_key_event()`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部双 preview，直接输出默认 `Review flow`
2. 切到 `Inline tools`
3. 输出第二组主区状态
4. 切到 `Compact notes`
5. 输出第三组主区状态
6. 恢复默认主状态
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `Horizontal / Compact` preview 在整条 reference 轨道里保持静态一致，不再包含额外 preview 刷新或“恢复后立刻抓帧”的旧式收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换和最终稳定抓帧使用 `STACK_PANEL_RECORD_FINAL_WAIT`，中间状态切换仍保留 `STACK_PANEL_RECORD_WAIT / STACK_PANEL_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/stack_panel PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/stack_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/stack_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_stack_panel
```

## 10. 验收重点
- 主控件三组主状态必须能直接看出方向或密度切换差异。
- `vertical -> horizontal -> compact vertical` 切换过程中不能出现重叠、裁切、整块缺失或旧布局残留。
- 单测里的 style helper、布局路径、`dispatch_key_event()` 入口和 static preview 状态保持断言必须全部通过。
- 两个 preview 必须完整可见，不黑白屏、不抖动，并且在所有 runtime 帧里保持静态一致。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_stack_panel/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `grid`：这里强调单轴顺序堆叠，不表达显式列数。
- 相比 `dock_panel`：这里不处理边缘停靠和剩余区域填充。
- 相比 `wrap_panel`：这里不做流式换行，顺序和方向显式可控。
- 相比 `virtualizing_stack_panel`：这里不做虚拟化或滚动容器能力，只保留基础布局语义。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Review flow`
  - `Inline tools`
  - `Compact notes`
  - `Horizontal`
  - `Compact`
  - `ordered stacking`
- 删减的旧桥接与旧口径：
  - 录制末尾“恢复后立即抓帧”的旧式收尾
  - 单测里直接走 `on_key_event()` 的旧入口
  - 与当前 static preview 工作流不一致的 README 结构

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/stack_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `stack_panel` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/stack_panel --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_stack_panel/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/stack_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_stack_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1723 colors=115`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(64, 101) - (366, 222)`
  - 按 `y >= 224` 裁切底部 preview 后保持单一哈希，确认 `Horizontal / Compact` preview 全程静态
  - 结论：主区覆盖默认 `Review flow`、`Inline tools` 与 `Compact notes` 三组 reference 状态，最终稳定帧已显式回到默认快照
