# grid_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI GridView`
- 对应组件名：`GridView`
- 本次保留语义：`assets gallery / template board / team board / compact / read only / selection focus`
- 本次删除内容：`preview` 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `hcw_grid_view` 封装，本轮只收口 `reference` 页面结构、录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`grid_view` 用来表达“同构内容按多列磁贴方式浏览，并保留当前项焦点、选择和激活语义”的标准集合视图。它适合素材墙、模板库、人员卡片墙、设备概览和资源挑选这类需要快速扫视多项内容的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏单列摘要列表，不适合表达多列磁贴集合。
- `items_repeater` 是布局和模板复用基础件，本身不承担 `GridView` 的当前项与激活语义。
- `wrap_panel`、`uniform_grid`、`virtualizing_wrap_panel` 更偏容器，不负责集合项焦点、选择和输入闭环。
- 本仓库需要一个和 `items_repeater / uniform_grid / card_control / card_action / card_expander` 对齐的 `GridView` reference 页面、单测和 web 验证入口。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `grid_view`、底部两个静态 preview。
- 主控件展示三组 snapshot：
  - `Assets gallery`
  - `Template board`
  - `Team board`
- 底部左侧是 `compact` 静态 preview，只负责展示紧凑密度。
- 底部右侧是 `read only` 静态 preview，只负责展示只读弱化状态。
- 两个 preview 统一通过 `hcw_grid_view_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_item / layout_mode`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/grid_view/`

## 4. 视觉与布局规格
- 根布局：`224 x 288`
- 主控件：`196 x 148`
- 底部对照行：`216 x 86`
- `compact` preview：`104 x 86`
- `read only` preview：`104 x 86`
- 页面结构：标题 -> 主 `grid_view` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 tone 区分。
  - 主区保留标题、摘要、磁贴区、当前项高亮和轻量 footer。
  - preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `grid_view_primary` | `hcw_grid_view_t` | `196 x 148` | `Assets gallery` | 主 `GridView` |
| `grid_view_compact` | `hcw_grid_view_t` | `104 x 86` | `Compact grid` | 紧凑静态 preview |
| `grid_view_read_only` | `hcw_grid_view_t` | `104 x 86` | `Read only grid` | 只读静态 preview |
| `primary_snapshots` | `hcw_grid_view_snapshot_t[3]` | - | `Assets / Template / Team` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Assets gallery` | 默认状态，展示标准 `GridView` 外壳与焦点项 |
| 主控件 | `Template board` | 第二组 snapshot，验证同壳体下的密度变化 |
| 主控件 | `Team board` | 第三组 snapshot，验证不同磁贴元信息仍保持同一集合语义 |
| `compact` | `Compact grid` | 固定静态对照，只验证紧凑尺寸下的磁贴密度 |
| `read only` | `Read only grid` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实输入闭环：
  - `Left / Right / Up / Down / Home / End / Tab` 负责集合导航。
  - `Enter / Space` 激活当前项并触发 listener。
  - 触摸保持 same-target release：只有同一项上的 `DOWN -> UP` 才提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`。
- `read only` 和 `!enable` 期间：
  - `touch / dispatch_key_event / handle_navigation_key` 都不能改变状态
  - `current_snapshot / current_item / layout_mode` 保持不变
  - 不触发 listener
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode` 不变
  - 不触发 listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Assets gallery`
2. 切到 `Template board`，输出第二组主状态
3. 切到 `Team board`，输出第三组主状态
4. 恢复默认主状态并输出最终稳定帧

录制只导出主控件的状态变化。底部两个 preview 在整条 `reference` 轨道里保持静态一致，不再承担第二条 `compact` 轨道、preview dismiss 或清焦点职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc

# 在 X:\ 短路径下执行，修改 HelloUnitTest 后先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_view
```

验收重点：
- 主控件必须能直接看出三组 `GridView` 主状态变化。
- `same-target release / keyboard activation / read only / !enable / static preview` 全部通过单测。
- runtime 截图里底部 preview 必须在全程保持静态一致。

## 10. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
- 复核目标：
  - 主区存在 3 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到 preview 区

## 11. 与现有控件的边界
- 相比 `items_repeater`：这里保留集合导航和激活语义，不只是模板重复器。
- 相比 `uniform_grid`：这里保留不等宽磁贴节奏和 snapshot 切换，不是固定网格容器。
- 相比 `data_list_panel`：这里强调多列磁贴浏览，不是单列摘要列表。
- 相比 `wrap_panel / virtualizing_wrap_panel`：这里是 reference 级集合控件，不是纯布局器。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `assets gallery`
  - `template board`
  - `team board`
  - `compact`
  - `read only`
  - `selection focus`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删除的旧桥接与轨道：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 13. 当前验收结果（2026-04-17）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `grid_view` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid_view --track reference --timeout 10 --keep-screenshots`
  - `9` 帧输出到 `runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2196 colors=316`
- 截图复核结论：
  - 主区变化边界收敛到 `(56, 62) - (403, 264)`
  - 主区共 `3` 组唯一状态，对应 `Assets gallery / Template board / Team board`
  - 按 `y >= 265` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
