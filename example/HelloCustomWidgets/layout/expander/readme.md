# Expander 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI Expander`
- 补充对照实现：`ModernWpf`
- 对应组件：`Expander`
- 当前保留形态：`Workspace policy expanded`、`Sync rules expanded`、`Sync rules collapsed`、`Release notes expanded`、`compact`、`read only`
- 当前保留交互：主区保留 header 选择、same-target release 与 `Up / Down / Home / End / Enter / Space` 键盘闭环；底部 preview 保留静态 reference 对照
- 当前移除内容：旧 preview focus bridge、录制里的 `preview click` 收尾，以及会改写 preview 状态的第二条 preview 轨道
- EGUI 适配说明：目录和 demo 继续使用 `layout/expander`，底层仍复用仓库内现有 `egui_view_expander` 实现；本轮只收口 reference 页面结构、录制轨道、README 口径和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`expander` 用来表达“标题行可展开正文，再次触发可收起”的标准 disclosure 结构，适合设置说明、同步规则、审阅备注和按需展开的辅助内容。

仓库里已有 `settings_panel`、`tree_view`、`master_detail`、`split_view` 和 `card_control`，但仍缺一个能稳定承接 `Expander` 单层 disclosure 语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `settings_panel` 更偏向设置项列表和 trailing controls，不负责正文展开。
- `tree_view` 更偏向层级导航，不是单层 disclosure。
- `master_detail`、`split_view` 是双栏联动布局，不适合页内轻量展开说明。
- `card_control` 只负责固定摘要卡片，不包含展开 / 收起语义。

## 3. 当前页面结构
- 标题：`Expander`
- 主区：1 个保留真实 disclosure 语义的 `expander`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Mode expanded + compact`
- 右侧 preview：`read only`，固定显示 `Audit expanded + read only`

目录：
- `example/HelloCustomWidgets/layout/expander/`

## 4. 主区 reference 快照
主区录制轨道只保留 4 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Workspace policy expanded`
2. 快照 2
   `Sync rules expanded`
3. 快照 3
   `Sync rules collapsed`
4. 快照 4
   `Release notes expanded`
5. 最终稳定帧
   回到默认 `Workspace policy expanded`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Mode expanded + compact`
2. `read only`
   `Audit expanded + read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 226`
- 主控件：`196 x 110`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- 页面结构：标题 -> 主 `expander` -> 底部 `compact / read only`
- 风格约束：浅色 Fluent 容器、低噪音边框和轻量 tone 差异；`success / warning / neutral` 只在局部 tone 上变化，不回退到 showcase 风格；底部两个 preview 固定为静态 reference 对照，不再承担额外交互职责

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Workspace policy expanded` | `Mode expanded + compact` | `Audit expanded + read only` |
| 快照 2 | `Sync rules expanded` | 保持不变 | 保持不变 |
| 快照 3 | `Sync rules collapsed` | 保持不变 | 保持不变 |
| 快照 4 | `Release notes expanded` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Workspace policy expanded` | 保持不变 | 保持不变 |
| `expanded / collapsed`、键盘导航与 same-target release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Sync rules expanded`
4. 抓取第二组主区快照
5. 切到 `Sync rules collapsed`
6. 抓取第三组主区快照
7. 切到 `Release notes expanded`
8. 抓取第四组主区快照
9. 恢复默认主区和底部 preview 固定状态
10. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 header 触摸、same-target release 和 `Up / Down / Home / End / Enter / Space` 键盘语义，供运行时手动复核。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留 `preview click` 收尾。
- 底部 preview 统一通过 `egui_view_expander_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，主区首轮切换与最终稳定抓帧使用 `EXPANDER_RECORD_FINAL_WAIT`，中间状态切换保留 `EXPANDER_RECORD_WAIT / EXPANDER_RECORD_FRAME_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_expander.c` 当前覆盖 `10` 条用例：

1. `set_items / set_current_index / set_expanded_index` 的 clamp、listener guard 与状态清理。
2. 字体、配色、compact/read only setter 与内部 helper 的状态清理和回退。
3. metrics、命中测试和条目边界行为。
4. header 触摸 same-target release 与 cancel 语义。
5. compact 模式切换后的 pressed 清理与 toggle 保持。
6. 键盘导航、`Enter / Space` 展开收起闭环。
7. `read only` guard 清理残留 `pressed` 且屏蔽输入。
8. `!enable` guard 清理残留 `pressed` 且屏蔽输入。
9. static preview 吞掉 `touch / key` 且保持 `current_index / expanded_index / compact_mode / read_only_mode` 不变。
10. 空数据状态忽略输入且不触发 `selection / expanded` listener。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/expander
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_expander
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Workspace policy expanded`、`Sync rules expanded`、`Sync rules collapsed`、`Release notes expanded` 4 组可识别状态，最终稳定帧必须回到默认态。
- `Sync rules collapsed` 不能残留旧正文高度、错位文本或 `pressed` 污染。
- 主区真实交互仍需保留 same-target release、`read only / !enable / empty items` guard 和键盘展开收起语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- WASM demo 必须能以 `HelloCustomWidgets_layout_expander` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_expander/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界收敛到 `(46, 99) - (434, 235)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 235` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里强调单层 disclosure，不是设置行 trailing controls。
- 相比 `tree_view`：这里没有层级树结构，只保留单层展开。
- 相比 `master_detail / split_view`：这里不是双栏布局，而是页内纵向展开。
- 相比 `card_control`：这里自带展开 / 收起状态，不是固定摘要卡片。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `workspace policy`
  - `sync rules expanded`
  - `sync rules collapsed`
  - `release notes`
  - `compact`
  - `read only`
  - `expanded / collapsed`
- 本次保留交互：
  - same-target release
  - 键盘 `Up / Down / Home / End / Enter / Space`
- 删减的装饰或桥接：
  - preview focus bridge
  - 会改写 preview 状态的第二条 preview 轨道
  - 录制里的 `preview click` 收尾

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `expander` suite `10 / 10`
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
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/expander --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_expander/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/expander`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_expander`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1723 colors=208`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界为 `(46, 99) - (434, 235)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 235` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Workspace policy expanded`、`Sync rules expanded`、`Sync rules collapsed` 与 `Release notes expanded` 4 组 reference 快照，最终稳定帧显式回到默认 `Workspace policy expanded`，底部 `compact / read only` preview 全程静态
