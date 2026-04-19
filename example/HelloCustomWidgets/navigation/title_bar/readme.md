# TitleBar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TitleBar`
- 补充对照控件：`nav_panel`、`menu_bar`、`selector_bar`
- 对应组件名：`TitleBar`
- 当前保留形态：`Atlas`、`Files`、`Release`、`Read back`、`compact`、`read only`
- 当前保留交互：主区保留 `back / pane toggle / primary action / secondary action` 四类 part 命中、same-target release、`Left / Right / Home / End / Tab` 键盘导航、`Enter / Space` 激活，以及 `current_snapshot / current_part / font / meta_font / icon_font / palette / compact / read_only` setter 统一清理 `pressed`；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 action listener
- 当前移除内容：系统 caption button、窗口拖拽职责、原生窗口宿主耦合、额外内容页壳层、第二条 `compact` 预览轨道、可点击 preview 卡、preview 清主控件 focus 的桥接逻辑、重描边和旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `egui_view_title_bar` 实现，本轮只收口 `reference` 页面说明、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`title_bar` 用来表达页面或工作区顶部的导航与状态入口。它不是系统窗口栏的完整复刻，而是一个贴近 Fluent / WPF UI `TitleBar` 语义的轻量 reference 容器，负责承载返回、pane toggle、标题、副标题和右侧动作入口。

## 2. 为什么现有控件不够
- `menu_bar` 更偏命令入口，不负责页面标题与返回语义。
- `nav_panel` 是常驻导航 rail，层级更重，不适合页面顶部的单条标题栏。
- `selector_bar` 只负责分区切换，缺少标题、副标题和左右 header 的组合表达。
- 当前 reference 主线仍需要一版明确对齐 `Fluent / WPF UI TitleBar` 的轻量顶栏控件。

## 3. 当前页面结构
- 标题：`TitleBar`
- 主区：一个主 `title_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Atlas`
- 右侧 preview：`read only`，固定显示 `Preview`
- 页面结构统一收口为：标题 -> 主 `title_bar` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/title_bar/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Atlas`
   默认主状态，显示 `Workspace / Atlas / Weekly review`
2. `Files`
   点击 `back` 后切到 `Navigation / Files / Pane toggled`
3. `Release`
   点击 `pane toggle` 后切到 `Project / Release / Go-live check`
4. `Read back`
   点击主动作后切到 `Audit / Read back / Frozen shell`
5. `Atlas`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Atlas`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Preview`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 214`
- 主控件尺寸：`196 x 96`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 尺寸：`104 x 72`
- 页面结构：标题 -> 主 `title_bar` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色标题栏卡片、低噪音阴影、左侧导航 affordance、轻量 leading icon badge，以及克制的右侧 action pill 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Atlas` | 是 | 是 | 否 |
| `Files` | 是 | 否 | 否 |
| `Release` | 是 | 否 | 否 |
| `Read back` | 是 | 否 | 否 |
| `Preview` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `same-target release` | 是 | 否 | 否 |
| `keyboard navigation` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_title_bar.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 与 `set_current_part()` 覆盖 snapshot 数量钳制、空数据重置，以及 `current_snapshot / current_part` 默认值回落。
2. `current_snapshot / current_part / font / meta_font / icon_font / palette / compact / read_only` setter 统一覆盖 `pressed_part / is_pressed` 清理。
3. metrics 与 hit-testing 覆盖 `back / pane toggle / primary action / secondary action` 的布局区域可解析性，以及 `compact_mode` 下 header / subtitle 隐藏。
4. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 action；`ACTION_CANCEL` 只清理 `pressed`。
5. 键盘 `Right / Tab / Home / End` 覆盖 part 导航，`Enter` 覆盖当前 part 激活。
6. focus 变化用例覆盖 `current_part == NONE` 时自动回落到第一个可交互 part。
7. `read_only_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入，恢复后继续允许 part 导航。
8. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_snapshot / current_part / compact_mode` 不变，且不会触发 action listener。

补充说明：
- 主区 same-target release 继续遵守非拖拽控件口径：只有回到初始命中 part 再 `UP` 才能提交。
- 底部 `compact / read only` preview 统一通过 `egui_view_title_bar_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- 预览输入只清理残留 `pressed`，不改 `current_snapshot / current_part`，也不触发 action listener。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Atlas` 和底部两个静态 preview，请求首帧并等待 `TITLE_BAR_RECORD_FRAME_WAIT`。
2. 切到 `Files`，等待 `TITLE_BAR_RECORD_WAIT`。
3. 请求第二帧并等待 `TITLE_BAR_RECORD_FRAME_WAIT`。
4. 切到 `Release`，等待 `TITLE_BAR_RECORD_WAIT`。
5. 请求第三帧并等待 `TITLE_BAR_RECORD_FRAME_WAIT`。
6. 切到 `Read back`，等待 `TITLE_BAR_RECORD_WAIT`。
7. 请求第四帧并等待 `TITLE_BAR_RECORD_FRAME_WAIT`。
8. 回到默认 `Atlas`，同步底部 preview 固定状态并等待 `TITLE_BAR_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `TITLE_BAR_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Atlas / Files / Release / Read back` 四组 snapshot，以及最终回到默认 `Atlas` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/title_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/title_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/title_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_title_bar
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 左侧返回 / pane toggle、leading icon、标题栈和右侧动作 pill 必须都能被辨识。
- 主区域需要出现 `Atlas -> Files -> Release -> Read back -> Atlas` 的真实变化。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能误触发 action，只有回到 `A` 才能提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_part` 或误触发 action listener。
- `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态，也不触发 action listener。
- 最终稳定帧必须显式回到默认 `Atlas`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_title_bar/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Atlas`、`Files`、`Release` 和 `Read back`
  - 主区 RGB 差分边界为 `(57, 114) - (425, 197)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 198` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Atlas`

## 12. 与现有控件的边界
- 相比 `menu_bar`：这里是标题与导航 affordance 的组合，不是菜单命令条。
- 相比 `nav_panel`：这里是页面顶部 title shell，不是左侧常驻 rail。
- 相比 `selector_bar`：这里保留标题、副标题和 header 状态，不只做 section 选择。
- 相比原生窗口标题栏：这里不处理系统按钮和窗口管理职责。

## 13. 本轮保留与删减
- 保留的主区状态：`Atlas`、`Files`、`Release`、`Read back`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`back / pane toggle / primary action / secondary action`、keyboard navigation、same-target release、`current_snapshot / current_part / font / meta_font / icon_font / palette / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：系统 caption button、窗口拖拽职责、原生窗口宿主耦合、额外内容页壳层、第二条 `compact` 预览轨道、可点击 preview 卡、preview 清主控件 focus 的桥接逻辑、重描边和旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/title_bar PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `title_bar` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/title_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_title_bar/default`
  - 共捕获 `11` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/title_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_title_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1632 colors=164`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Atlas`、`Files`、`Release` 和 `Read back`
  - 主区 RGB 差分边界为 `(57, 114) - (425, 197)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 198` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Atlas`，底部 `compact / read only` preview 全程保持静态
