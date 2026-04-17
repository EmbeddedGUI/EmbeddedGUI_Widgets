# virtualizing_stack_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`VirtualizingStackPanel`
- 本次保留语义：`windowed stack`、`window anchor`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_virtualizing_stack_panel`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`virtualizing_stack_panel` 用来表达“项目按纵向栈式顺序排列，但视口内只绘制当前窗口内容”的布局语义，适合设置列表、搜索结果、消息摘要或属性面板这类数据量较大、阅读方向稳定、又不希望一次性把全部条目都铺满页面的场景。

## 2. 为什么现有控件不够用

- `data_list_panel` 更偏带语义的 `ListView` 页面，不是可嵌入的基础虚拟栈容器。
- `wrap_panel` 与 `virtualizing_wrap_panel` 强调多列换行密度，不适合单轴纵向窗口化列表。
- `uniform_grid` 强调等尺寸网格，不适合主次文本明显的纵向条目。
- 当前仓库需要一个能完整走通 reference、单测、runtime 和 web 发布链路的 `VirtualizingStackPanel` 页面。

## 3. 目标场景与示例概览

- 主控件保留真实 `VirtualizingStackPanel` 语义，展示 `Operations queue` 与 `Release review` 两组主 snapshot。
- 主录制轨道额外保留一次 `Ctrl+Down` 的窗口跳转，用于直观看到 `window anchor` 与可视窗口变化。
- 底部左侧是 `compact` 静态 preview，只用于对照窄尺寸下的行密度。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化层级。
- 页面只保留标题、一个主 `virtualizing_stack_panel` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_virtualizing_stack_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改写 `current_snapshot / current_item / window_anchor`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/virtualizing_stack_panel/`

## 4. 视觉与布局规格

- 根布局：`224 x 232`
- 主控件：`196 x 108`
- 底部对照行：`216 x 82`
- `compact` preview：`104 x 82`
- `read only` preview：`104 x 82`
- 页面结构：标题 -> 主 `virtualizing_stack_panel` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 `row / meta / thumb` 层级。
  - 主区突出窗口化 stack 与 anchor 变化，不再叠加旧 preview 交互桥接。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `stack_primary` | `egui_view_virtualizing_stack_panel_t` | `196 x 108` | `Operations queue` | 主 `VirtualizingStackPanel` |
| `stack_compact` | `egui_view_virtualizing_stack_panel_t` | `104 x 82` | `Compact window` | 紧凑静态对照 |
| `stack_read_only` | `egui_view_virtualizing_stack_panel_t` | `104 x 82` | `Read only window` | 只读静态对照 |
| `primary_snapshots` | `egui_view_virtualizing_stack_panel_snapshot_t[2]` | - | `Operations / Release` | 主控件语义轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Operations queue` | 默认状态，展示窗口化 stack 与当前行 |
| 主控件 | `Ctrl+Down page jump` | 保留一次真实窗口跳转，验证 `window_anchor` 变化 |
| 主控件 | `Release review` | 第二组 snapshot，展示另一组窗口内容与默认 anchor |
| `compact` | `Compact window` | 固定静态对照，验证窄尺寸窗口化布局 |
| `read only` | `Read only window` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口

- 主控件保留真实 `VirtualizingStackPanel` 键盘与触摸语义：
  - `Up / Down`：在上下相邻项之间移动
  - `Left / Right`：复用单列列表的前后移动语义
  - `Ctrl+Up / Ctrl+Down`：按窗口大小做 page jump
  - `Home / End`：跳到首尾项并更新 `window_anchor`
  - `Tab`：在当前 snapshot 内循环，必要时切到下一组 snapshot
  - `Enter / Space`：激活当前项并触发 listener
- 触摸交互保持 same-target release：只在同一项 `DOWN -> UP` 时提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_window_anchor()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计

`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Operations queue`。
2. 对主控件发送一次 `Ctrl+Down`，输出窗口跳转后的主状态。
3. 切到 `Release review`，输出第二组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区 page jump / snapshot 切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、运行时、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/virtualizing_stack_panel PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_virtualizing_stack_panel
```

验收重点：
- 主控件必须能直接看出窗口化 stack、`window_anchor` 跳转和 snapshot 切换。
- `same-target release / page jump / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向

- 当前只收口单容器 `VirtualizingStackPanel` reference，不实现真实数据源虚拟化容器。
- 继续使用 snapshot 描述总项数、当前项与窗口锚点，不下沉到 SDK 通用布局层。
- 若后续复用价值稳定，再评估是否抽象出与 `data_list_panel` 或 `virtualizing_wrap_panel` 共享的 window helper。

## 11. 与现有控件的边界

- 相比 `data_list_panel`：这里强调虚拟窗口和 anchor，不承担更强的列表页面语义。
- 相比 `virtualizing_wrap_panel`：这里强调单列 stack，不处理多列换行。
- 相比 `scroll_viewer`：这里负责条目选择与激活，不只是滚动容器。

## 12. 本次保留的核心状态与删减项

- 保留的核心状态：
  - `windowed stack`
  - `window anchor`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Up / Down / Left / Right / Ctrl+Up / Ctrl+Down / Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作

## 13. 当前验收结果（2026-04-17）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/virtualizing_stack_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `virtualizing_stack_panel` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel --track reference --timeout 10 --keep-screenshots`
  - `8` 帧输出到 `runtime_check_output/HelloCustomWidgets_layout_virtualizing_stack_panel/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/virtualizing_stack_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_virtualizing_stack_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1769 colors=301`
- 截图复核结论：
  - 主区变化边界收敛到 `(48, 97) - (432, 251)`
  - 主区共 `3` 组唯一状态，对应 `Operations queue / Ctrl+Down page jump / Release review`
  - 按 `y >= 252` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
