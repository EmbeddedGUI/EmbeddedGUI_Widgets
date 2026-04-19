# BreadcrumbBar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI BreadcrumbBar`
- 补充对照控件：`tree_view`、`tab_strip`
- 对应组件名：`BreadcrumbBar`
- 当前保留形态：`Docs path`、`Forms path`、`Review path`、`compact`、`read only`
- 当前保留交互：主区保留 same-target release、`Enter` 键盘点击、`set_snapshots / current_snapshot / font / palette / compact / read_only` setter 统一清理 `pressed`，以及 `compact_mode` 下继续保留点击行为；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 click listener
- 当前移除内容：compact preview 第二轨道、preview 点击清 focus 的桥接动作、与路径摘要语义无关的额外交互录制，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `breadcrumb_bar` 基础实现，本轮只收口 `reference` 页面结构、静态 preview、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`breadcrumb_bar` 用来表达页面层级路径和当前位置，适合设置页、文档树、资源浏览和管理后台顶部路径导航。相比标签页，它更强调“我现在处在路径的哪一层”，而不是“我在切哪个 section”。

## 2. 为什么现有控件不够
- `tab_strip` 适合平级 section 切换，不负责层级路径表达。
- `menu_bar` 更偏命令入口，不适合持续显示当前位置。
- `tree_view` 强调层级浏览本体，不承担顶部路径摘要职责。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `BreadcrumbBar` 语义的 reference 示例。

## 3. 当前页面结构
- 标题：`Breadcrumb Bar`
- 主区：一个主 `breadcrumb_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Compact bills`
- 右侧 preview：`read only`，固定显示 `Read only`
- 页面结构统一收口为：标题 -> 主 `breadcrumb_bar` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/breadcrumb_bar/`

## 4. 主区 reference 快照
主区录制轨道只保留三组主区状态与最终稳定帧：

1. `Docs path`
   默认主状态，显示 `Home / Docs / Nav / Details`
2. `Forms path`
   程序化切到第二组路径，显示 `Home / Demos / Forms / Value`
3. `Review path`
   程序化切到第三组路径，显示 `Home / Files / Grid / Review`
4. `Docs path`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Compact bills`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Read only`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 140`
- 主控件尺寸：`198 x 48`
- 底部对照行尺寸：`216 x 36`
- 单个 preview 尺寸：`104 x 36`
- 页面结构：标题 -> 主 `breadcrumb_bar` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色路径条、低噪音浅边框、轻量蓝色当前项 pill，以及更弱的 chevron separator

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Docs path` | 是 | 否 | 否 |
| `Forms path` | 是 | 否 | 否 |
| `Review path` | 是 | 否 | 否 |
| `Compact bills` | 否 | 是 | 否 |
| `Read only` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `same-target release` | 是 | 否 | 否 |
| `keyboard click` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_breadcrumb_bar.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 覆盖 snapshot 数量钳制、空数据重置，以及 `current_snapshot / pressed` 清理。
2. `current_snapshot / font / palette / compact / read_only` setter 覆盖越界钳制、默认字体回落、调色板写入、模式钳制，以及 `pressed` 清理。
3. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 click；`ACTION_CANCEL` 只清理 `pressed`。
4. 键盘 `Enter` 继续驱动 click listener。
5. `compact_mode` 切换后会清理残留 `pressed`，但保留点击行为。
6. `read_only_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入，恢复后继续允许 click。
7. 静态 preview 用例验证“consumes input and keeps snapshot”，固定校验 `current_snapshot / compact_mode` 不变，且不会触发 click listener。
8. 内部 helper 用例覆盖 entries、overflow、省略标签、测量函数与 disabled 混色逻辑。

补充说明：
- 主区 same-target release 继续遵守非拖拽控件口径：只有回到初始命中区域再 `UP` 才能提交。
- 底部 `compact / read only` preview 统一通过 `egui_view_breadcrumb_bar_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- 预览输入只清理残留 `pressed`，不改 `current_snapshot`，也不触发 click listener。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Docs path` 和底部两个静态 preview，请求首帧并等待 `BREADCRUMB_RECORD_FRAME_WAIT`。
2. 切到 `Forms path`，等待 `BREADCRUMB_RECORD_WAIT`。
3. 请求第二帧并等待 `BREADCRUMB_RECORD_FRAME_WAIT`。
4. 切到 `Review path`，等待 `BREADCRUMB_RECORD_WAIT`。
5. 请求第三帧并等待 `BREADCRUMB_RECORD_FRAME_WAIT`。
6. 回到默认 `Docs path`，同步底部 preview 固定状态并等待 `BREADCRUMB_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `BREADCRUMB_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Docs path / Forms path / Review path` 三组状态，以及最终回到默认 `Docs path` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_breadcrumb_bar
```

## 10. 验收重点
- 主控件三张关键截图必须能清晰区分 `Docs path`、`Forms path`、`Review path`。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或轨道切换产生额外变化。
- 页面不能出现黑屏、白屏、裁切、`pressed` 残留或底部 preview 脏态。
- preview 不响应触摸或键盘输入，也不改变 `current_snapshot`。
- 最终稳定帧必须显式回到默认 `Docs path`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_breadcrumb_bar/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Docs path`、`Forms path` 和 `Review path`
  - 主区 RGB 差分边界为 `(156, 188) - (324, 229)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 229` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Docs path`

## 12. 与现有控件的边界
- 相比 `tab_strip`：这里表达层级路径，不表达平级 section 切换。
- 相比 `tree_view`：这里提供顶部路径摘要，不承担层级浏览本体。
- 相比 `menu_bar`：这里是当前位置表达，不是命令入口。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留 preview 桥接与外部说明壳层。

## 13. 本轮保留与删减
- 保留的主区状态：`Docs path`、`Forms path`、`Review path`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：same-target release、`Enter` 键盘点击、`set_snapshots / current_snapshot / font / palette / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：compact preview 第二轨道、preview 点击清 focus 的桥接动作、与路径摘要语义无关的额外交互录制、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `breadcrumb_bar` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_breadcrumb_bar/default`
  - 共捕获 `9` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_breadcrumb_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1068 colors=112`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Docs path`、`Forms path` 和 `Review path`
  - 主区 RGB 差分边界为 `(156, 188) - (324, 229)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 229` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Docs path`，底部 `compact / read only` preview 全程保持静态
