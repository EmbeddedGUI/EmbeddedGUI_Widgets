# GridSplitter 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI GridSplitter`
- 对应组件：`GridSplitter`
- 当前保留形态：`Canvas split`、`Audit split`、`Inspector split`、`Compact split`、`Read only split`
- 当前保留交互：主区保留 `Left / Right / Home / End / Tab / Enter / Space` 键盘调节与真实 handle 拖拽；底部 preview 保留静态 reference 对照
- 当前移除内容：旧 preview focus bridge、第二条 `compact` preview 轨道，以及录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：目录和 demo 继续使用 `layout/grid_splitter`，底层仍复用仓库内现有 `egui_view_grid_splitter` 实现；本轮只收口 reference 页面结构、录制轨道、README 口径与静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`grid_splitter` 用来表达“左右两个 pane 共享一条可拖拽分隔柄”的标准布局语义。它适合资源管理器、属性检查器、工作台和设置页这类需要在同一屏内动态调节两栏宽度的场景。

仓库里已有 `split_view`、`master_detail`、`data_list_panel` 和 `grid`，但仍缺一个能稳定承接 `GridSplitter` 连续调节列宽语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `split_view` 更偏导航 pane 展开/收起，不等同于持续调整列宽的 `GridSplitter`。
- `master_detail` 和 `data_list_panel` 更偏选择与阅读，不提供独立的 resize handle。
- `grid` 只负责静态列布局，不包含键盘和拖拽改比例语义。
- SDK 基础布局虽然能做双栏，但没有一个可直接审阅的 `GridSplitter` reference 控件。

## 3. 当前页面结构
- 标题：`Grid Splitter`
- 主区：1 个保留真实 `GridSplitter` resize 语义的 `grid_splitter`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact split`
- 右侧 preview：`read only`，固定显示 `Read only split`

目录：
- `example/HelloCustomWidgets/layout/grid_splitter/`

## 4. 主区 reference 快照
主区录制轨道保留默认态、键盘步进态、切换快照态和拖拽轨道，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Canvas split`
2. 键盘步进态
   `Canvas split` 经一次 `Right` 调整后的分栏比例
3. 快照 2
   `Audit split`
4. 拖拽轨道
   主区在 `Audit split` 下保留 `2` 组独立拖拽比例帧，用于确认 handle drag 的动态变化
5. 快照 3
   `Inspector split`
6. 最终稳定帧
   回到默认 `Canvas split`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact split`
2. `read only`
   `Read only split`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主控件：`196 x 118`
- 底部 preview 行：`216 x 74`
- 单个 preview：`104 x 74`
- 页面结构：标题 -> 主 `grid_splitter` -> 底部 `compact / read only`
- 风格约束：浅色 Fluent 容器、低噪音边框和轻量色带层级；handle 始终作为唯一 resize affordance，不能被其它装饰干扰；底部两个 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或页面桥接职责

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Canvas split` | `Compact split` | `Read only split` |
| 键盘步进态 | `Canvas split + Right` | 保持不变 | 保持不变 |
| 快照 2 | `Audit split` | 保持不变 | 保持不变 |
| 拖拽轨道 | `Audit split + drag ratio` | 保持不变 | 保持不变 |
| 快照 3 | `Inspector split` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Canvas split` | 保持不变 | 保持不变 |
| `keyboard resize` 与 `drag resize` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 对主控件发送一次 `Right`
4. 抓取键盘步进后的主区状态
5. 切到 `Audit split`
6. 抓取第二组主区快照
7. 对主控件执行一次真实拖拽
8. 抓取拖拽轨道后的主区状态
9. 切到 `Inspector split`
10. 抓取第三组主区快照
11. 恢复默认主区和底部 preview 固定状态
12. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 handle 拖拽、`Left / Right / Home / End / Tab / Enter / Space` 键盘语义，供运行时手动复核。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留第二条 `compact` preview 轨道。
- 底部 preview 统一通过 `egui_view_grid_splitter_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证默认态、键盘步进态、`Audit split`、拖拽轨道、`Inspector split` 与最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，键盘与快照切换使用 `GRID_SPLITTER_RECORD_WAIT / GRID_SPLITTER_RECORD_FRAME_WAIT`，最终稳定抓帧使用 `GRID_SPLITTER_RECORD_FINAL_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_grid_splitter.c` 当前覆盖 `6` 条用例：

1. `set_snapshots()` 的 clamp、默认比例回退和状态清理。
2. 字体、配色、`compact / read only`、`snapshot / ratio` setter 的 pressed 清理与 listener 更新。
3. handle region、真实拖拽与 ratio listener 行为。
4. `Left / Right / Home / End / Tab / Enter / Space` 键盘调节与重置闭环。
5. `read only / !enable` guard 清理残留 `pressed` 且屏蔽输入。
6. static preview 吞掉 `touch / key` 且保持 `current_snapshot / split_ratio / compact_mode / read_only_mode` 不变。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_splitter PORT=pc

# 在 X:\ 短路径下执行
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制必须清晰覆盖默认态、键盘步进态、`Audit split`、拖拽轨道与 `Inspector split`，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 handle drag、`read only / !enable` guard 和键盘比例调整语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- setter、guard 和 static preview 都必须统一遵守“先清理残留 `pressed` 再处理后续状态”的语义。
- WASM demo 必须能以 `HelloCustomWidgets_layout_grid_splitter` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid_splitter/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 全帧共出现 `6` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6] / [7] / [8,9]`
  - 主区 RGB 差分边界收敛到 `(60, 106) - (435, 250)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 251` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `split_view`：这里强调连续调整列宽，而不是 pane 展开 / 收起。
- 相比 `master_detail`：这里的核心交互是 handle drag，不是列表选中。
- 相比 `grid`：这里保留可变分栏比例和键盘调节，而不是固定列布局。
- 相比基础线性布局：这里提供可直接审阅的 `GridSplitter` 标准语义和静态 preview 对照。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Canvas split`
  - `Audit split`
  - `Inspector split`
  - `Compact split`
  - `Read only split`
  - `keyboard resize`
  - `drag resize`
- 本次保留交互：
  - handle drag
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space`
- 删减的装饰或桥接：
  - preview 点击清主控件 focus
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid_splitter PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `grid_splitter` suite `6 / 6`
  - 无关 warning：`test_split_view.c:186:13: warning: 'get_view_center' defined but not used`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_grid_splitter/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_splitter`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_splitter`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=303`
- 截图复核结论：
  - 共捕获 `13` 帧
  - 全帧共出现 `6` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6] / [7] / [8,9]`
  - 主区 RGB 差分边界为 `(60, 106) - (435, 250)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 251` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Canvas split`、键盘步进态、`Audit split`、拖拽轨道和 `Inspector split`，最终稳定帧显式回到默认 `Canvas split`，底部 `Compact split / Read only split` preview 全程静态
