# viewbox 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI Viewbox`
- 对应组件语义：`Viewbox`
- 本次保留语义：`Device preview`、`Cover preview`、`Inspector thumb`、`compact`、`read only`、`stretch preset`
- 本次删除内容：旧 preview focus bridge、第二条 `compact` preview 轨道、录制中的 `preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_viewbox` reference 实现，本轮只收口参考页结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`viewbox` 用来表达“同一块内容在受限视口内按统一缩放规则适配”的标准语义。它适合设备预览、封面容器、缩略卡片、审查面板这类需要保留内容整体比例、同时又要让用户看清缩放结果的场景。

## 2. 为什么现有控件不够用
- `card_control`、`card_action` 更偏卡片入口语义，不负责展示缩放规则本身。
- `uniform_grid`、`wrap_panel`、`items_repeater` 关注多项排布，不是单一内容块的缩放容器。
- 现有 SDK 基础布局能控制尺寸，但没有当前仓库需要的 `Viewbox` reference 页面、stretch preset 和静态 preview 验收闭环。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `viewbox`，以及底部两个静态 preview。
- 主控件展示三组主状态：
  - `Device preview`
  - `Cover preview`
  - `Inspector thumb`
- 左下角是 `compact` 静态 preview，只负责对照紧凑尺寸下的缩放结果。
- 右下角是 `read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_viewbox_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_preset / stretch_mode / compact_mode / read_only_mode`
  - 不触发 action listener

目标目录：`example/HelloCustomWidgets/layout/viewbox/`

## 4. 视觉与布局规格
- 根布局：`224 x 240`
- 主控件：`196 x 120`
- 底部对照行：`216 x 78`
- `compact` preview：`104 x 78`
- `read only` preview：`104 x 78`
- 页面结构：标题 -> 主 `viewbox` -> `compact / read only`
- 样式约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量强调色。
  - 主区的 preset pills 是唯一交互焦点，不让底部 preview 承担额外交互职责。
  - 录制里只导出主区状态变化，底部 preview 全程保持静态。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `viewbox_primary` | `egui_view_viewbox_t` | `196 x 120` | `Device preview` | 主 `Viewbox` |
| `viewbox_compact` | `egui_view_viewbox_t` | `104 x 78` | `Compact fit` | 紧凑静态 preview |
| `viewbox_read_only` | `egui_view_viewbox_t` | `104 x 78` | `Read only viewbox` | 只读静态 preview |
| `primary_snapshots` | `egui_view_viewbox_snapshot_t[3]` | - | `Device / Cover / Inspector` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Device preview` | 默认状态，展示标准等比缩放 |
| 主控件 | `Cover preview` | `Fill` 模式占满视口 |
| 主控件 | `Inspector thumb` | `Downscale only` 保持小内容不放大 |
| 主控件 | `stretch preset` | `Uniform / Fill / Down only` pills 切换 |
| `compact` | `Compact fit` | 固定静态对照，只验证紧凑布局 |
| `read only` | `Read only viewbox` | 固定静态对照，只验证只读弱化和输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实 preset 触摸闭环：
  - `ACTION_DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `ACTION_DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- 主控件键盘入口统一走 `dispatch_key_event()`：
  - `Left / Right`：在 preset 之间切换
  - `Home / End`：跳到首个 / 末个 preset
  - `Tab`：优先切下一个 preset，走到末尾后切到下一组 snapshot 并恢复该组默认 preset
  - `Enter / Space`：提交当前 preset 并触发 listener
- `set_snapshots()`、`set_current_snapshot()`、`set_current_preset()`、`set_stretch_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`
- `read_only` 与 `!enable` 期间：
  - `touch / dispatch_key_event` 都不能改动 `current_snapshot / current_preset / stretch_mode`
  - 不触发 action listener
  - 恢复后 `Right / Enter / Space` 要重新可用
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / current_preset / stretch_mode / compact_mode / read_only_mode` 不变
  - 不触发 action listener

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Device preview`
2. 对主控件发送一次 `Right`，输出 preset 切换后的主区状态
3. 切到 `Cover preview`，输出第二组 snapshot
4. 切到 `Inspector thumb`，输出第三组 snapshot
5. 恢复主控件默认状态并输出最终稳定帧

录制只导出主控件状态变化。底部两个 preview 在整条 reference 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview click` 收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、preset 切换、snapshot 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/viewbox PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/viewbox --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/viewbox
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_viewbox
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑白屏、裁切或重叠。
- 主区三组 snapshot 和一次 preset 切换要清晰可辨，底部 preview 全程保持静态。
- `dispatch_key_event` 路径下的 `Right / Tab / Enter / Space`、`read only`、`!enable`、`static preview keeps state` 都要通过单测。
- `snapshot / preset / stretch / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_viewbox/default`
- 复核目标：
  - 主区存在多组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到底部 preview

## 12. 与现有控件的边界
- 相比 `card_control`：这里强调缩放语义，不是卡片入口容器。
- 相比 `uniform_grid`、`wrap_panel`：这里处理单块内容适配，不负责多项排布。
- 相比基础布局容器：这里提供可直接审阅的 `Viewbox` 标准语义和静态 preview 对照。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Device preview`
  - `Cover preview`
  - `Inspector thumb`
  - `compact`
  - `read only`
  - `stretch preset`
- 保留的交互：
  - preset 触摸提交
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space`
- 删减的旧桥接与轨道：
  - preview 点击清主控件 focus
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview click` 收尾

## 14. 当前验收结果（2026-04-17）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/viewbox PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `viewbox` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/viewbox --track reference --timeout 10 --keep-screenshots`
  - `11` 帧输出到 `runtime_check_output/HelloCustomWidgets_layout_viewbox/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/viewbox`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_viewbox`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.183 colors=237`
- 截图复核结论：
  - 主区变化边界收敛到 `(57, 100) - (436, 255)`
  - 主区共 `4` 组唯一状态，覆盖默认态、一次 preset 切换态、`Cover preview` 与 `Inspector thumb`
  - 按 `y >= 256` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
