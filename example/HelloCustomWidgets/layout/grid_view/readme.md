# GridView 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI GridView`
- 对应组件：`GridView`
- 当前保留形态：`assets gallery`、`template board`、`team board`、`compact`、`read only`
- 当前保留交互：主区保留 tile 集合导航、same-target release、`Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘闭环与静态 preview 对照
- 当前移除内容：页面级 `guide`、preview 清主区焦点桥接、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续使用当前目录下的 `hcw_grid_view` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下完成 reference 页面、README、录制轨道和单测收口

## 1. 为什么需要这个控件
`grid_view` 用来表达“同构内容按多列磁贴方式浏览，并保留当前项焦点、选择和激活语义”的标准集合视图，适合素材墙、模板库、人员卡片墙、设备概览和资源挑选这类需要快速扫视多项内容的场景。

仓库里已有 `items_repeater`、`uniform_grid`、`data_list_panel` 和 `wrap_panel`，但仍缺少一个能稳定承接 `GridView` 集合导航与激活语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `items_repeater` 更偏模板复用基础件，本身不承担 `GridView` 的当前项与激活语义。
- `uniform_grid` 更偏固定网格容器，不负责集合项焦点和输入闭环。
- `wrap_panel`、`virtualizing_wrap_panel` 更偏布局容器，不负责 snapshot 与当前项切换。
- `data_list_panel` 更偏单列摘要列表，不适合表达多列磁贴集合浏览。

## 3. 当前页面结构
- 标题：`Grid View`
- 主区：1 个保留真实集合导航语义的 `grid_view`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact grid`
- 右侧 preview：`read only`，固定显示 `Read only grid`

目录：
- `example/HelloCustomWidgets/layout/grid_view/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Assets gallery`
2. 快照 2
   `Template board`
3. 快照 3
   `Team board`
4. 最终稳定帧
   回到默认 `Assets gallery`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact grid`
2. `read only`
   `Read only grid`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 288`
- 主控件：`196 x 148`
- 底部 preview 行：`216 x 86`
- 单个 preview：`104 x 86`
- 页面结构：标题 -> 主 `grid_view` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音描边、轻量 title 与 helper 文案层级，以及稳定的 tile 密度和当前项高亮，不回退到旧 demo 的 guide 与场景包装

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Assets gallery` | `Compact grid` | `Read only grid` |
| 快照 2 | `Template board` | 保持不变 | 保持不变 |
| 快照 3 | `Team board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Assets gallery` | 保持不变 | 保持不变 |
| same-target release / 键盘集合导航 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Template board`
4. 抓取第二组主区快照
5. 切到 `Team board`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `current_snapshot / current_item / layout_mode`、same-target release 和键盘集合导航，供运行时手动交互。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `hcw_grid_view_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_grid_view.inc` 当前覆盖 `7` 条用例，分为两部分：

1. 主控件交互与状态守卫
   覆盖 `set_snapshots()` clamp/default、setter 清理 pressed、区域激活 listener、same-target touch release、`Home / End / Tab / Enter` 键盘导航，以及 `read only / !enable` guard。
2. 静态 preview 输入抑制
   通过单独 preview widget 固定校验 `touch / key / navigation` 输入被吞掉后，`current_snapshot / current_item / layout_mode / compact_mode / read_only_mode` 保持不变，且 `action_count == 0`、`pressed_item` 与 `is_pressed` 会被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc

# 在 X:\ 短路径下执行
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

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Assets gallery`、`Template board`、`Team board` 3 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留集合导航、same-target release 与激活 listener 语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `current_snapshot / current_item / layout_mode / compact_mode / read_only_mode / region_screen / palette`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(56, 62) - (403, 264)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `items_repeater`：这里保留集合导航和激活语义，不只是模板重复器。
- 相比 `uniform_grid`：这里保留不等宽磁贴节奏和 snapshot 切换，不是固定网格容器。
- 相比 `data_list_panel`：这里强调多列磁贴浏览，不是单列摘要列表。
- 相比 `wrap_panel / virtualizing_wrap_panel`：这里是 reference 级集合控件，不是纯布局器。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Assets gallery`
  - `Template board`
  - `Team board`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 `guide`
  - preview 清主区焦点桥接
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/grid_view PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_grid_view/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/grid_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_grid_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2196 colors=316`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(56, 62) - (403, 264)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Assets gallery`、`Template board` 与 `Team board` 3 组 reference 快照，最终稳定帧已回到默认 `Assets gallery`，底部 `compact / read only` preview 全程静态
