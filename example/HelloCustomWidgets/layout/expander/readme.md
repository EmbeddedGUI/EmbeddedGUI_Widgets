# expander 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI Expander`
- 补充对照实现：`ModernWpf`
- 对应组件语义：`Expander`
- 本次保留语义：`Workspace policy / Sync rules / Release notes / compact / read only / expanded / collapsed`
- 本次删除内容：旧 preview focus bridge、录制里的 `preview click` 收尾，以及会改写 preview 状态的第二条 preview 轨道
- EGUI 适配说明：继续复用仓库内 `egui_view_expander` custom 实现；本轮只收口 `reference` 页面结构、录制轨道、README 口径和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`expander` 用来表达“标题行可展开正文，再次触发可收起”的标准 disclosure 结构，适合设置说明、同步规则、审阅备注和按需展开的辅助内容。

## 2. 为什么现有控件不够用
- `settings_panel` 更偏向设置项列表和 trailing controls，不负责正文展开。
- `tree_view` 更偏向层级导航，不是单层 disclosure。
- `master_detail`、`split_view` 是双栏联动布局，不适合页内轻量展开说明。
- `card_control` 只负责固定摘要卡片，不包含展开 / 收起语义。

因此这里继续保留 `expander`，但示例页只保留符合 Fluent / WPF UI 主线的 `reference` 结构。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `expander` -> 底部 `compact / read only` 双静态 preview。
- 主控件负责导出四组主区状态：
  - `Workspace policy expanded`
  - `Sync rules expanded`
  - `Sync rules collapsed`
  - `Release notes expanded`
- 底部左侧是 `compact` 静态 preview，固定展示 `Mode expanded + compact`。
- 底部右侧是 `read only` 静态 preview，固定展示 `Audit expanded + read only`。
- 两个 preview 统一通过 `egui_view_expander_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `current_index / expanded_index / compact_mode / read_only_mode`
  - 不触发 `selection / expanded` listener

目标目录：`example/HelloCustomWidgets/layout/expander/`

## 4. 视觉与布局规格
- 根布局：`224 x 226`
- 主控件：`196 x 110`
- 底部对照行：`216 x 76`
- `compact` preview：`104 x 76`
- `read only` preview：`104 x 76`
- 页面结构：标题 -> 主区 -> 底部 `compact / read only`
- 风格约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量 tone 差异。
  - `success / warning / neutral` 只在局部 tone 上变化，不回退到 showcase 风格。
  - 主区保留真实 header 触摸、键盘导航和展开 / 收起语义；底部 preview 不再承担额外交互职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `panel_primary` | `egui_view_expander_t` | `196 x 110` | `Workspace policy expanded` | 主 `Expander` |
| `panel_compact` | `egui_view_expander_t` | `104 x 76` | `Mode expanded + compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_expander_t` | `104 x 76` | `Audit expanded + read only` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace policy expanded` | 默认主状态 |
| 主控件 | `Sync rules expanded` | success tone 展开态 |
| 主控件 | `Sync rules collapsed` | collapsed 收口态 |
| 主控件 | `Release notes expanded` | warning tone 稳定态 |
| `compact` preview | `Mode expanded + compact` | 固定静态对照 |
| `read only` preview | `Audit expanded + read only` | 固定静态对照 |

## 7. 交互语义与单测要求
- 主控件保留真实 header 触摸提交：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `ACTION_CANCEL` 只清理 `pressed`，不修改 `current_index / expanded_index`。
- 主控件键盘入口统一走 `dispatch_key_event()`：
  - `Up / Down / Home / End` 切换当前项
  - `Enter / Space` 切换当前项的展开状态
- `set_items()`、`set_current_index()`、`set_expanded_index()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 之后都不能残留旧的 `pressed` 高亮。
- `read_only`、`!enable` 和空数据状态下：
  - 会先清理残留 `pressed`
  - 后续 `touch / dispatch_key_event()` 不会改变 `current_index / expanded_index / compact_mode / read_only_mode`
  - 不触发 `selection / expanded` listener
  - 恢复后 `Down / Enter / Space` 行为必须恢复
- static preview 用例必须验证：
  - 输入被消耗
  - 状态保持不变
  - listener 不触发
  - `pressed_index / is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部双 preview，输出默认 `Workspace policy expanded`
2. 切到 `Sync rules expanded`
3. 输出第二组主区状态
4. 切到 `Sync rules collapsed`
5. 输出第三组主区状态
6. 切到 `Release notes expanded`
7. 输出第四组主区状态
8. 恢复默认主状态
9. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致，不再包含 preview 状态切换，也不再包含 `preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `EXPANDER_RECORD_FINAL_WAIT`，中间状态切换仍保留 `EXPANDER_RECORD_WAIT / EXPANDER_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc

# 在 X:\ 短路径下执行
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
- 主区四组状态必须能直接看出 header、正文区和展开 / 收起差异。
- `Sync rules collapsed` 不能残留旧正文高度、错位文本或 pressed 污染。
- `same-target release / read_only / !enable / static preview keeps state` 必须全部通过单测。
- 两个 preview 必须完整可见，不黑白屏、不抖动，并且在所有 runtime 帧里保持静态一致。
- WASM demo 必须能以 `HelloCustomWidgets_layout_expander` 正常加载，文档面板可渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_expander/default`
- 复核目标：
  - 主区存在 `4` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `settings_panel`：这里强调单层 disclosure，不是设置行 trailing controls。
- 相比 `tree_view`：这里没有层级树结构，只保留单层展开。
- 相比 `master_detail / split_view`：这里不是双栏布局，而是页内纵向展开。
- 相比 `card_control`：这里自带展开 / 收起状态，不是固定摘要卡片。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `workspace policy`
  - `sync rules expanded`
  - `sync rules collapsed`
  - `release notes`
  - `compact`
  - `read only`
  - `expanded / collapsed`
- 保留的交互：
  - same-target touch release
  - 键盘 `Up / Down / Home / End / Enter / Space`
- 删减的旧桥接与旧口径：
  - preview focus bridge
  - 会改写 preview 状态的第二条 preview 轨道
  - 录制里的 `preview click` 收尾

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/expander PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `expander` suite `10 / 10`
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
  - `11 frames captured -> runtime_check_output/HelloCustomWidgets_layout_expander/default`
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
  - 主区变化边界保持在 `(46, 99) - (433, 234)`
  - 按 `y >= 235` 裁切底部 preview 区域后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Workspace policy expanded`、`Sync rules expanded`、`Sync rules collapsed` 与 `Release notes expanded` 四组 reference 状态，最终稳定帧已显式回到默认快照
