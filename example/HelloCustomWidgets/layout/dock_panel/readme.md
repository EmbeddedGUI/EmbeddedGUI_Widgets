# dock_panel 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF DockPanel`
- 对应组件语义：`DockPanel`
- 本次保留语义：`Inspector shell / Reading pane / Compact tools / Rail / Footer / last child fill`
- 本次删除内容：旧录制末尾多余的恢复抓帧、旧单测 `on_key_event` 注入路径、与当前 static preview 工作流不一致的旧 README 结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/dock_panel`，公开 C API 保持 `hcw_dock_panel_*`，本轮只收口 `reference` 页面结构、录制轨道、单测入口和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`dock_panel` 用来表达“子项按顺序停靠在顶部、底部、左侧、右侧，剩余区域自动留给填充内容”的标准布局关系。它适合 inspector shell、阅读页框架、工具条 + 内容区、边栏 + 工作区这类 Fluent / WPF 常见容器场景。

## 2. 为什么现有控件不够用
- `stack_panel` 只负责顺序堆叠，不表达停靠边和剩余区域填充。
- `canvas` 依赖绝对坐标，不能表达“先停靠边缘，再让中间自然填满”的容器语义。
- `grid` 和 `uniform_grid` 依赖显式网格切分，不适合这种按顺序占边的布局模型。
- `split_view` 更偏导航抽屉和侧栏切换，不是通用的停靠布局容器。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `dock_panel` -> 底部 `Rail / Footer` 双静态 preview。
- 主控件负责导出三组主区状态：
  - `Inspector shell`
  - `Reading pane`
  - `Compact tools`
- 底部左侧是 `Rail` 静态 preview，固定展示顶部 + 左侧 + 填充区的标准停靠关系。
- 底部右侧是 `Footer` 静态 preview，固定展示填充区 + 底栏的安静 shell 对照。
- 两个 preview 统一通过 `hcw_dock_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `last_child_fill / content_inset`
  - 不触发布局外的额外交互

目标目录：`example/HelloCustomWidgets/layout/dock_panel/`

## 4. 视觉与布局规格
- 根布局：`224 x 236`
- 主面板：`196 x 118`
- 主 `dock_panel`：`176 x 64`
- 底部对照行：`216 x 76`
- 单个 preview 面板：`104 x 76`
- `Rail` preview 的内部停靠区：`84 x 32`
- `Footer` preview 的内部停靠区：`84 x 32`
- 风格约束：
  - 保持浅色 Fluent surface、轻量边界和低噪音色块。
  - 主区保留停靠方向、最后子项填充和紧凑模式差异，不叠加额外交互 chrome。
  - 底部 preview 全程静态，不再承担场景切换或点击收尾职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_dock_panel` | `hcw_dock_panel_t` | `176 x 64` | `Inspector shell` | 主 `DockPanel` |
| `rail_preview_panel` | `hcw_dock_panel_t` | `84 x 32` | `Rail` | 标准静态 preview |
| `footer_preview_panel` | `hcw_dock_panel_t` | `84 x 32` | `Footer` | 紧凑静态 preview |
| `primary_snapshots` | `dock_snapshot_t[3]` | - | `Inspector / Reading / Compact` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Inspector shell` | 顶栏 + 左右栏 + 填充区 |
| 主控件 | `Reading pane` | 顶栏 + 左栏 + 底栏 + 填充区 |
| 主控件 | `Compact tools` | 紧凑 inset 的窄外壳 |
| `Rail` preview | `Rail` | 固定静态对照，验证顶部 + 左栏停靠 |
| `Footer` preview | `Footer` | 固定静态对照，验证底栏 + 填充区 |

## 7. 交互语义与单测要求
- `dock_panel` 本体不承担选择、焦点切换或键盘导航语义，核心验收点是布局结果与静态 preview 防输入行为。
- `apply_standard_style()`、`apply_compact_style()`、`set_last_child_fill()`、`set_child_dock()` 之后都不能残留旧的 `pressed` 高亮。
- `layout_childs()` 必须覆盖：
  - `top + left + right + fill`
  - `top + bottom + fill`
  - `last child fill = 0`
  - `compact inset`
- static preview 用例必须验证：
  - 输入被消费
  - `last_child_fill / content_inset` 保持不变
  - 子项布局区域保持不变
  - `on_click_listener` 不触发
  - `pressed / is_pressed` 被清理
- 预览态键盘入口统一走 `dispatch_key_event()`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部双 preview，输出默认 `Inspector shell`
2. 切到 `Reading pane`
3. 切到 `Compact tools`
4. 恢复默认主状态并输出最终稳定帧

录制只导出主区状态变化。底部 `Rail / Footer` preview 在整条 reference 轨道里保持静态一致，不再包含额外 preview 状态切换，也不再保留旧的中间恢复抓帧。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `DOCK_PANEL_RECORD_FINAL_WAIT`，中间状态切换仍保留 `DOCK_PANEL_RECORD_WAIT / DOCK_PANEL_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/dock_panel PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/dock_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/dock_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_dock_panel
```

## 10. 验收重点
- 主控件三组主状态必须能直接看出停靠边变化和填充区变化。
- `last child fill` 的开关差异必须清晰可辨，不能出现重叠、裁切或错误填充。
- 单测里的 style helper、布局路径和 static preview `dispatch_key_event()` 入口都必须通过。
- 两个 preview 必须完整可见，不能黑白屏、裁切或重叠，并且在所有 runtime 帧里保持静态一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_dock_panel/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `stack_panel`：这里强调停靠顺序和剩余区域填充，不是简单堆叠。
- 相比 `canvas`：这里不依赖手写绝对坐标，而是表达布局语义。
- 相比 `grid`：这里不需要显式行列定义，只依赖顺序停靠。
- 相比 `split_view`：这里不承担导航抽屉行为，只保留容器布局本体。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `inspector shell`
  - `reading pane`
  - `compact tools`
  - `rail`
  - `footer`
  - `last child fill`
- 保留的交互：
  - static preview consumes input
  - `dispatch_key_event()` 预览守卫
- 删减的旧桥接与旧口径：
  - 旧录制末尾多余的恢复抓帧
  - 旧单测 `on_key_event` 注入路径
  - 与当前 static preview 工作流不一致的旧 README 结构

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/dock_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `dock_panel` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/dock_panel --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_dock_panel/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/dock_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_dock_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=116`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(64, 94) - (416, 216)`
  - 按 `y >= 216` 裁切底部 preview 区域后保持单一哈希，确认 `Rail / Footer` preview 全程静态
  - 结论：主区覆盖默认 `Inspector shell`、`Reading pane` 与 `Compact tools` 三组 reference 状态，最终稳定帧已显式回到默认快照
