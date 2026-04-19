# Canvas 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF Canvas`
- 对应组件：`Canvas`
- 当前保留形态：`Pinned notes`、`Status overlay`、`Compact board`、`Pinned`、`Compact`
- 当前保留交互：主区保留绝对坐标布局与标准/紧凑排布切换语义，底部 preview 保留静态 reference 对照与输入吞掉语义
- 当前移除内容：录制末尾“恢复后立即抓帧”的旧式收尾、单测里直接调用 `on_key_event()` 的旧入口、旧版 finalize README 章节结构
- EGUI 适配说明：目录与 demo 继续使用 `layout/canvas`，底层仍复用 SDK `egui_view_group`；本轮只收口 README、reference 录制说明和验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`canvas` 用来表达“子项按明确坐标固定放置”的基础布局语义，适合批注板、固定锚点提示、小型 overlay 和轻量地图式信息面板。它负责承载 Fluent / WPF 中那类不依赖顺序流式布局、而是依赖绝对位置表达的信息层。

仓库里已有 `stack_panel`、`grid`、`wrap_panel` 和 `relative_panel`，但仍缺少一个能稳定承接 `Canvas` 绝对坐标语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `stack_panel`、`wrap_panel` 都围绕顺序流式布局，不适合精确绝对定位。
- `grid` 强调显式行列关系，不适合表达“任意坐标落点”。
- `relative_panel` 更强调控件间约束关系，不适合直接表达锚点坐标。
- 直接裸用 `group` 不能完整承载当前 reference 页面、static preview 和验收闭环。

## 3. 当前页面结构
- 标题：`Canvas`
- 主区：1 个承接绝对定位语义的 `canvas`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Pinned`，固定显示两个锚点卡片
- 右侧 preview：`Compact`，固定显示紧凑坐标板

目录：
- `example/HelloCustomWidgets/layout/canvas/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Pinned notes`
2. 快照 2
   `Status overlay`
3. 快照 3
   `Compact board`
4. 最终稳定帧
   回到默认 `Pinned notes`

底部 preview 在整条轨道中始终固定：

1. `Pinned`
   `Static preview.`
2. `Compact`
   `Quiet board.`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 240`
- 主面板：`196 x 120`
- 主 `canvas`：`176 x 64`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- preview `canvas`：`84 x 30`
- 页面结构：标题 -> 主 `canvas` -> 底部 `Pinned / Compact`
- 风格约束：浅色 page panel、低噪音卡片色块、轻量 heading 与 note 文案层级，以及稳定的绝对坐标排布，不回退到旧 demo 的额外收尾与交互包装

## 6. 状态矩阵
| 状态 | 主控件 | Pinned preview | Compact preview |
| --- | --- | --- | --- |
| 默认显示 | `Pinned notes` | `Pinned` | `Compact` |
| 快照 2 | `Status overlay` | 保持不变 | 保持不变 |
| 快照 3 | `Compact board` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Pinned notes` | 保持不变 | 保持不变 |
| `set_child_origin()` + `layout_childs()` 绝对定位 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Status overlay`
4. 抓取第二组主区快照
5. 切到 `Compact board`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `set_child_origin()`、`layout_childs()` 与标准/紧凑风格切换语义，供运行时手动复核。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `hcw_canvas_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_canvas.c` 当前覆盖 `4` 条用例，分为两部分：

1. 主控件布局与状态守卫
   覆盖 `apply_standard_style()`、`apply_compact_style()`、`set_child_origin()` 清理 `pressed`，以及标准/紧凑两组绝对坐标布局结果。
2. 静态 preview 输入抑制
   通过独立 preview `canvas` 固定校验 `touch / dispatch_key_event()` 输入被吞掉后，`region` 坐标、`background`、`is_pressed` 和 `on_click_listener` 结果保持不变。

同时要求：
- `g_click_count == 0`
- preview group `is_pressed == false`
- 所有 preview 子项 `region` 保持不变

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Pinned notes`、`Status overlay`、`Compact board` 3 组可识别状态，最终稳定帧必须回到默认态。
- 主区真实布局仍需保留绝对坐标落点和标准/紧凑排布差异。
- 底部 `Pinned / Compact` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写子项 `region`、点击计数或 `pressed` 状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_canvas/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(64, 91) - (415, 212)`
  - 遮罩主区边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 213` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `stack_panel`：这里强调显式坐标，不是单轴顺序堆叠。
- 相比 `grid`：这里不表达显式行列关系。
- 相比 `relative_panel`：这里不依赖相对约束，位置由明确坐标决定。
- 相比 `wrap_panel`：这里不做流式排布或自动换行。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
  - `Pinned`
  - `Compact`
- 删减的装饰或桥接：
  - 录制末尾“恢复后立即抓帧”的旧式收尾
  - 单测里直接调用 `on_key_event()` 的旧入口
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
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
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_canvas/default`
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
  - 主区 RGB 差分边界为 `(64, 91) - (415, 212)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 213` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Pinned notes`、`Status overlay` 与 `Compact board` 3 组 reference 快照，最终稳定帧显式回到默认 `Pinned notes`，底部 `Pinned / Compact` preview 全程静态
