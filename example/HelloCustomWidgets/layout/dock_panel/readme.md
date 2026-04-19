# DockPanel 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF DockPanel`
- 对应组件：`DockPanel`
- 当前保留形态：`Inspector shell`、`Reading pane`、`Compact tools`、`Rail`、`Footer`
- 当前保留交互：主区保留 top/left/right/bottom/fill 停靠布局与 `last child fill` 切换语义，底部 preview 保留静态 reference 对照与输入吞掉语义
- 当前移除内容：录制末尾多余的恢复抓帧、单测里直接调用 `on_key_event()` 的旧入口、旧版 finalize README 章节结构
- EGUI 适配说明：目录与 demo 继续使用 `layout/dock_panel`，公开 C API 保持 `hcw_dock_panel_*`；本轮只收口 README、reference 录制说明和验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`dock_panel` 用来表达“子项按顺序停靠在顶部、底部、左侧、右侧，剩余区域自动留给填充内容”的标准布局语义，适合 inspector shell、阅读页框架、工具栏加内容区、边栏加工作区这类 Fluent / WPF 常见容器场景。

仓库里已有 `stack_panel`、`canvas`、`grid` 和 `split_view`，但仍缺少一个能稳定承接 `DockPanel` 停靠布局语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `stack_panel` 只负责顺序堆叠，不表达停靠边与剩余区域填充。
- `canvas` 依赖绝对坐标，不适合表达“先占边缘，再让中间自然填满”的容器语义。
- `grid`、`uniform_grid` 依赖显式网格切分，不适合这种按顺序停靠的布局模式。
- `split_view` 更偏导航抽屉和侧栏切换，不是通用的停靠布局容器。

## 3. 当前页面结构
- 标题：`DockPanel`
- 主区：1 个承接停靠布局语义的 `dock_panel`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Rail`，固定显示 top + left + fill 的标准停靠关系
- 右侧 preview：`Footer`，固定显示 fill + bottom 的安静 shell 对照

目录：
- `example/HelloCustomWidgets/layout/dock_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Inspector shell`
2. 快照 2
   `Reading pane`
3. 快照 3
   `Compact tools`
4. 最终稳定帧
   回到默认 `Inspector shell`

底部 preview 在整条轨道中始终固定：

1. `Rail`
   `Static preview.`
2. `Footer`
   `Quiet shell.`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主面板：`196 x 118`
- 主 `dock_panel`：`176 x 64`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- preview `dock_panel`：`84 x 32`
- 页面结构：标题 -> 主 `dock_panel` -> 底部 `Rail / Footer`
- 风格约束：浅色 page panel、低噪音停靠卡片、轻量 heading 与 note 文案层级，以及稳定的停靠方向与填充区差异，不回退到旧 demo 的额外收尾与交互包装

## 6. 状态矩阵
| 状态 | 主控件 | Rail preview | Footer preview |
| --- | --- | --- | --- |
| 默认显示 | `Inspector shell` | `Rail` | `Footer` |
| 快照 2 | `Reading pane` | 保持不变 | 保持不变 |
| 快照 3 | `Compact tools` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Inspector shell` | 保持不变 | 保持不变 |
| `set_child_dock()` + `last child fill` 停靠布局 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Reading pane`
4. 抓取第二组主区快照
5. 切到 `Compact tools`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `set_child_dock()`、`layout_childs()`、`last child fill` 和标准/紧凑风格切换语义，供运行时手动复核。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `hcw_dock_panel_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_dock_panel.c` 当前覆盖 `4` 条用例，分为两部分：

1. 主控件布局与状态守卫
   覆盖 `apply_standard_style()`、`apply_compact_style()`、`set_last_child_fill()`、`set_child_dock()` 清理 `pressed`，以及 `top + left + right + fill`、`top + bottom + fill` 两组停靠布局结果。
2. 静态 preview 输入抑制
   通过独立 preview `dock_panel` 固定校验 `touch / dispatch_key_event()` 输入被吞掉后，`last_child_fill`、`content_inset`、子项 `region`、`is_pressed` 和 `on_click_listener` 结果保持不变。

同时要求：
- `g_click_count == 0`
- preview panel `is_pressed == false`
- preview `last_child_fill == 1`
- preview `content_inset == 4`
- 所有 preview 子项 `region` 保持不变

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/dock_panel PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Inspector shell`、`Reading pane`、`Compact tools` 3 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实布局仍需保留停靠方向变化和 `last child fill` 差异。
- 底部 `Rail / Footer` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `last_child_fill`、`content_inset`、子项 `region`、点击计数或 `pressed` 状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_dock_panel/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(64, 94) - (415, 215)`
  - 遮罩主区边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 216` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `stack_panel`：这里强调停靠顺序和剩余区域填充，不是简单堆叠。
- 相比 `canvas`：这里不依赖绝对坐标，而是表达停靠布局语义。
- 相比 `grid`：这里不需要显式行列定义，只依赖顺序停靠。
- 相比 `split_view`：这里不承担导航抽屉行为，只保留容器布局本体。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Inspector shell`
  - `Reading pane`
  - `Compact tools`
  - `Rail`
  - `Footer`
- 本次保留交互：
  - `last child fill`
  - static preview consumes input
- 删减的装饰或桥接：
  - 录制末尾多余的恢复抓帧
  - 单测里直接调用 `on_key_event()` 的旧入口
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/dock_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `dock_panel` suite `4 / 4`
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
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/dock_panel --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_dock_panel/default`
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
  - 主区 RGB 差分边界为 `(64, 94) - (415, 215)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 216` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Inspector shell`、`Reading pane` 与 `Compact tools` 3 组 reference 快照，最终稳定帧显式回到默认 `Inspector shell`，底部 `Rail / Footer` preview 全程静态
