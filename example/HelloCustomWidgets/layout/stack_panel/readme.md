# StackPanel 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF / StackPanel`
- 对应组件：`StackPanel`
- 当前保留形态：`Review flow`、`Inline tools`、`Compact notes`、`Horizontal`、`Compact`
- 当前保留交互：主区保留真实顺序堆叠与方向切换语义；底部 `Horizontal / Compact` preview 保持静态 reference 对照
- 当前移除内容：旧录制末尾“恢复后立即抓帧”的模板化收尾、旧单测 `on_key_event` 直调入口、与当前 static preview 工作流不一致的旧 README 口径
- EGUI 适配说明：目录和 demo 继续使用 `layout/stack_panel`，底层仍复用 SDK `egui_view_linearlayout`；本轮只收口 README、reference 录制说明、单测口径与静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`stack_panel` 用来表达“子项按单一方向顺序堆叠”的基础布局语义，适合审核流程、工具条、摘要列表和轻量信息面板。它是 Fluent / WPF 里最常见的基础容器之一，也适合作为多个高层布局控件的 reference 语义底座。

## 2. 为什么现有控件不够用
- `grid` 更强调显式列布局，不适合纯顺序堆叠。
- `dock_panel` 负责停靠边和剩余区域填充，不是顺序布局容器。
- `wrap_panel` 会自动换行，不适合固定阅读顺序的单轴内容流。
- 直接裸用 `linearlayout` 不能完整承载当前 reference 页面、static preview 和验收闭环。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `stack_panel` -> 底部 `Horizontal / Compact` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Review flow`
  - `Inline tools`
  - `Compact notes`
- 录制最终稳定帧显式回到默认 `Review flow`。
- 底部左侧是 `Horizontal` 静态 preview，只负责对照横向操作条堆叠语义。
- 底部右侧是 `Compact` 静态 preview，只负责对照紧凑纵向堆叠语义。
- 两个 preview 统一通过 `hcw_stack_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `orientation / align_type / auto_size`
  - 不触发布局外的额外交互

目标目录：
- `example/HelloCustomWidgets/layout/stack_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Review flow`
2. 快照 2
   `Inline tools`
3. 快照 3
   `Compact notes`
4. 最终稳定帧
   回到默认 `Review flow`

底部 preview 在整条轨道中固定为：
1. `Horizontal`
2. `Compact`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 226`
- 主面板：`196 x 116`
- 主 `stack_panel`：`176 x 64`
- 底部 preview 行：`216 x 74`
- 单个 preview 面板：`104 x 74`
- `Horizontal` preview 的内部堆叠区：`84 x 34`
- `Compact` preview 的内部堆叠区：`84 x 32`
- 风格约束：保持浅色 Fluent surface、低噪音色块和轻量文字层级；主区只保留堆叠方向、密度和 item 顺序差异，不叠加额外交互 chrome；底部 preview 全程静态，不再承担场景切换或收尾刷新职责。

## 6. 状态矩阵
| 状态 | 主控件 | Horizontal preview | Compact preview |
| --- | --- | --- | --- |
| 默认显示 | `Review flow` | `Horizontal` | `Compact` |
| 快照 2 | `Inline tools` | 保持不变 | 保持不变 |
| 快照 3 | `Compact notes` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Review flow` | 保持不变 | 保持不变 |
| 主区布局切换 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_stack_panel.c` 当前覆盖 `4` 条用例：

1. `apply_standard_style()`、`apply_horizontal_style()`、`apply_compact_style()`、`set_orientation()`、`set_align_type()` 的 `pressed` 清理与布局属性更新。
2. `layout_childs()` 覆盖标准纵向、横向与紧凑纵向三种布局结果，验证子项落位。
3. `layout_childs()` 在重新布局时会清理残留 `pressed` 状态。
4. static preview 吞掉 `touch / dispatch_key_event()` 输入，并保持 `is_orientation_horizontal / align_type / is_auto_width / is_auto_height / cell regions` 不变，同时不触发 `on_click_listener`。

说明：
- `stack_panel` 本体不承担选择或提交语义，核心验收点是布局结果与静态 preview 防输入行为。
- 预览态键盘入口统一走 `dispatch_key_event()`，不再直接调用 `on_key_event()`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Review flow`，同时重放底部 `Horizontal / Compact` preview 固定状态并抓取首帧。
2. 切到 `Inline tools`，等待 `STACK_PANEL_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Compact notes`，等待 `STACK_PANEL_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Review flow`，同时重放底部 preview 固定状态，等待 `STACK_PANEL_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `STACK_PANEL_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Horizontal / Compact` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板；README 这里按当前实现如实保留首轮切换使用 `STACK_PANEL_RECORD_FINAL_WAIT`、后续主区回落仍使用 `STACK_PANEL_RECORD_WAIT` 的等待口径。

## 9. 验收命令
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Review flow`、`Inline tools`、`Compact notes` `3` 组可识别状态，最终稳定帧必须回到默认态。
- `vertical -> horizontal -> compact vertical` 切换过程中不能出现重叠、裁切、整块缺失或旧布局残留。
- 单测里的 style helper、布局路径、`dispatch_key_event()` 入口和 static preview 状态保持断言必须全部通过。
- 底部 `Horizontal / Compact` preview 必须在所有 runtime 帧里保持静态一致。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_stack_panel` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_stack_panel/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(64, 101) - (366, 222)`
  - 以 `y >= 223` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Review flow`

## 12. 与现有控件的边界
- 相比 `grid`：这里强调单轴顺序堆叠，不表达显式列数。
- 相比 `dock_panel`：这里不处理边缘停靠和剩余区域填充。
- 相比 `wrap_panel`：这里不做流式换行，顺序和方向显式可控。
- 相比 `virtualizing_stack_panel`：这里不做虚拟化或滚动容器能力，只保留基础布局语义。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Review flow`
  - `Inline tools`
  - `Compact notes`
- 保留的底部对照：
  - `Horizontal`
  - `Compact`
- 保留的核心语义：
  - ordered stacking
  - standard vertical / horizontal / compact vertical
- 删减的旧桥接与旧口径：
  - 录制末尾“恢复后立即抓帧”的旧式收尾
  - 单测里直接走 `on_key_event()` 的旧入口
  - 与当前 static preview 工作流不一致的 README 结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/stack_panel PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `stack_panel` suite `4 / 4`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_stack_panel/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/stack_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_stack_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1723 colors=121`
- 截图复核结论：
  - 主区覆盖 `Review flow / Inline tools / Compact notes` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Review flow`
  - 主区差分边界收敛到 `(64, 101) - (366, 222)`
  - 以 `y >= 223` 裁切后，底部 `Horizontal / Compact` preview 全程保持单哈希静态
