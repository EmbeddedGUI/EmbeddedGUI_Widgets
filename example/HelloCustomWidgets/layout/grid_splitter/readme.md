# grid_splitter 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI GridSplitter`
- 对应组件语义：`GridSplitter`
- 本次保留语义：`Canvas split`、`Audit split`、`Inspector split`、`compact`、`read only`、`keyboard resize`、`drag resize`
- 本次删除内容：旧 preview focus bridge、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_grid_splitter` reference 实现，本轮只收口参考页结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid_splitter` 用来表达“左右两个 pane 共享一条可拖拽分隔柄”的标准布局语义。它适合资源管理器、属性检查器、工作台和设置页这类需要在同一屏内动态调节两栏宽度的场景。

## 2. 为什么现有控件不够用
- `split_view` 更偏向“导航 pane 展开/收起”，不等同于持续调整列宽的 `GridSplitter`。
- `data_list_panel` 和 `master_detail` 更偏向选择与阅读，不提供独立的 resize handle。
- SDK 基础布局虽然能做双栏，但没有一个可直接审阅的 `GridSplitter` reference 控件。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `grid_splitter`，以及底部两个静态 preview。
- 主控件展示三组 snapshot：
  - `Canvas split`
  - `Audit split`
  - `Inspector split`
- 左下角是 `compact` 静态 preview，只负责对照紧凑布局密度。
- 右下角是 `read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_grid_splitter_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改变 `current_snapshot / split_ratio / compact_mode / read_only_mode`
  - 不触发 ratio listener

目标目录：`example/HelloCustomWidgets/layout/grid_splitter/`

## 4. 视觉与布局规格
- 根布局：`224 x 236`
- 主控件：`196 x 118`
- 底部对照行：`216 x 74`
- `compact` preview：`104 x 74`
- `read only` preview：`104 x 74`
- 页面结构：标题 -> 主 `grid_splitter` -> `compact / read only`
- 样式约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量色带层级。
  - handle 始终作为唯一 resize affordance，不能被其它装饰干扰。
  - 底部 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或页面桥接职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `splitter_primary` | `egui_view_grid_splitter_t` | `196 x 118` | `Canvas split` | 主 `GridSplitter` |
| `splitter_compact` | `egui_view_grid_splitter_t` | `104 x 74` | `Compact split` | 紧凑静态 preview |
| `splitter_read_only` | `egui_view_grid_splitter_t` | `104 x 74` | `Read only split` | 只读静态 preview |
| `primary_snapshots` | `egui_view_grid_splitter_snapshot_t[3]` | - | `Canvas / Audit / Inspector` | 主状态轨道 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Canvas split` | 默认状态，右侧 pane 强调 |
| 主控件 | `Audit split` | 第二组 snapshot，左侧 pane 更宽 |
| 主控件 | `Inspector split` | 第三组 snapshot，右侧 pane 更宽 |
| 主控件 | `keyboard resize` | `Left / Right / Home / End / Tab / Enter / Space` 闭环 |
| 主控件 | `drag resize` | 真实拖拽 handle 调整比例 |
| `compact` | `Compact split` | 固定静态对照，只验证紧凑布局 |
| `read only` | `Read only split` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实 handle 拖拽闭环：
  - `ACTION_DOWN` 命中 handle 后进入拖拽
  - `ACTION_MOVE` 连续更新分栏比例
  - `ACTION_UP / CANCEL` 清理残留 `pressed`
- 主控件保留 `dispatch_key_event()` 路径：
  - `Left / Right`：按固定步长调整比例
  - `Home / End`：跳到最小/最大比例
  - `Tab`：切到下一组 snapshot，并恢复该 snapshot 默认比例
  - `Enter / Space`：回到当前 snapshot 默认比例
- `set_snapshots()`、`set_current_snapshot()`、`set_split_ratio()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`。
- `read_only` 与 `!enable` 期间：
  - `touch / dispatch_key_event` 都不能改动 `current_snapshot / split_ratio`
  - `compact_mode / read_only_mode` 必须保持符合预期
  - 不触发 ratio listener
  - 恢复后 `Right / Enter / Space` 等行为要重新可用
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / split_ratio / compact_mode / read_only_mode` 不变
  - 不触发 ratio listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Canvas split`
2. 对主控件发送一次 `Right`，输出键盘步进后的主区状态
3. 切到 `Audit split`，输出第二组 snapshot
4. 对主控件执行一次真实拖拽，输出拖拽后的比例变化
5. 切到 `Inspector split`，输出第三组 snapshot
6. 恢复主控件默认状态并输出最终稳定帧

录制只导出主控件状态变化。底部两个 preview 在整条 reference 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview dismiss / preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_splitter PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_splitter --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_splitter
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_splitter
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑白屏、裁切或重叠。
- 主区三组 snapshot、键盘步进态和拖拽态要清晰可辨，底部 preview 全程保持静态。
- `dispatch_key_event` 路径下的 `Right / Tab / Enter / Space`、`read only`、`!enable`、`static preview keeps state` 要全部通过单测。
- `snapshot / ratio / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid_splitter/default`
- 复核目标：
  - 主区存在多组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到底部 preview

## 12. 与现有控件的边界
- 相比 `split_view`：这里强调连续调整列宽，而不是 pane 展开/收起。
- 相比 `master_detail`：这里的核心交互是 handle drag，不是列表选中。
- 相比基础线性布局：这里提供可直接审阅的 `GridSplitter` 标准语义和静态 preview 对照。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Canvas split`
  - `Audit split`
  - `Inspector split`
  - `compact`
  - `read only`
  - `keyboard resize`
  - `drag resize`
- 删减的旧桥接与轨道：
  - preview 点击清主控件 focus
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-17）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid_splitter PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `grid_splitter` suite `13 / 13`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_splitter --track reference --timeout 10 --keep-screenshots`
  - `13` 帧输出到 `runtime_check_output/HelloCustomWidgets_layout_grid_splitter/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_splitter`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_splitter`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=303`
- 截图复核结论：
  - 主区变化边界保持在 `(60, 106) - (435, 250)`
  - 主区共 `6` 组唯一状态，覆盖默认态、键盘步进态、第二/第三组 snapshot 与拖拽比例变化轨道
  - 按 `y >= 251` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
