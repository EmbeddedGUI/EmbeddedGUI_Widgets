# NavigationView 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI NavigationView`
- 补充对照控件：`menu_bar`、`tab_strip`
- 对应组件名：`NavigationView`
- 当前保留形态：`Overview`、`Library`、`People`、`compact`、`read only`
- 当前保留交互：主区继续保留 rail item 选择、same-target release、`Up / Down / Home / End` 键盘导航，以及 `items / header / footer / footer_badge / font / meta_font / palette / current_index / compact / read_only` setter 统一清理 `pressed_index / is_pressed`；`read_only / !enable` guard 会先清理 `pressed` 再拒绝后续 `touch / key` 输入；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 selection listener
- 当前移除内容：页面级 `guide`、状态文案、旧双列 preview 包裹壳、preview 交互职责、preview 清焦桥接、过重 selected row、过强 indicator、过亮 footer / badge chrome，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `nav_panel` 基础实现，本轮只收口 `reference` 页面说明、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`nav_panel` 用来承载页内常驻导航，而不是弹出菜单或一次性的命令入口。它适合设置页、管理页和工具页左侧的一级切换，让用户能持续看到“当前在哪个分区”。

## 2. 为什么现有控件不够
- `menu_bar` 更偏命令入口，不适合作为常驻导航面板。
- `breadcrumb_bar` 表达的是路径层级，不负责平级导航切换。
- `tab_strip` 更像横向页签条，不适合纵向常驻导航。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `NavigationView` 语义的轻量 rail。

## 3. 当前页面结构
- 标题：`Nav Panel`
- 主区：一个主 `nav_panel`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Home / Files / Rules`
- 右侧 preview：`read only`，固定显示 `Feed / Teams / Audit`
- 页面结构统一收口为：标题 -> 主 `nav_panel` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/nav_panel/`

## 4. 主区 reference 快照
主区录制轨道只保留三组主区状态与最终稳定帧：

1. `Overview`
   默认主状态，显示 `Workspace` header、`Overview / Library / People` rail 和 `Settings` footer
2. `Library`
   程序化切到第二项，验证 selected row、indicator 和 footer 保持同步
3. `People`
   程序化切到第三项，验证尾项选择状态
4. `Overview`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Home / Files / Rules`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Feed / Teams / Audit`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`198 x 108`
- 底部对照行尺寸：`152 x 74`
- 单个 preview 尺寸：`58 x 74`
- 页面结构：标题 -> 主 `nav_panel` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 rail 面板、低噪音边框、轻量 selected row、克制的 selection indicator，以及弱化的灰蓝只读 palette

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Overview` | 是 | 否 | 否 |
| `Library` | 是 | 否 | 否 |
| `People` | 是 | 否 | 否 |
| `Home / Files / Rules` | 否 | 是 | 否 |
| `Feed / Teams / Audit` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| item same-target release | 是 | 否 | 否 |
| keyboard navigation | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_nav_panel.c` 当前覆盖 `8` 条用例：

1. `set_items()` 与 `set_current_index()` 覆盖 item 数量钳制、visible item 上限、空数据回落，以及 `pressed_index / is_pressed` 清理。
2. `set_header_text / footer_text / footer_badge / font / meta_font / palette / compact / read_only` setter 全部覆盖 `pressed` 清理，并校验 helper 行为。
3. metrics 与 hit-testing 覆盖标准态 / compact 态下的 header、footer 和 item region 计算。
4. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交；`ACTION_CANCEL` 只清理 `pressed`。
5. 键盘导航覆盖 `Down / Up / Home / End`，验证 current index 与 selection listener 更新。
6. `compact_mode` 切换会清理残留 `pressed`，同时保留既有 selection 语义；compact 态下仍允许主区正常切换。
7. `read_only / !enable` guard 会清理残留 `pressed` 并忽略后续 `touch / key` 输入；恢复后继续允许正常导航。
8. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_index / compact_mode` 不变，且不会触发 selection listener。

补充说明：
- 主区 rail item 属于非拖拽点击目标，统一遵守 same-target release。
- 底部 `compact / read only` preview 统一通过 `egui_view_nav_panel_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- preview 输入只清理残留 `pressed`，不改 `current_index`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Overview` 和底部两个静态 preview，请求首帧并等待 `NAV_PANEL_RECORD_FRAME_WAIT`。
2. 切到 `Library`，等待 `NAV_PANEL_RECORD_WAIT`。
3. 请求第二帧并等待 `NAV_PANEL_RECORD_FRAME_WAIT`。
4. 切到 `People`，等待 `NAV_PANEL_RECORD_WAIT`。
5. 请求第三帧并等待 `NAV_PANEL_RECORD_FRAME_WAIT`。
6. 回到默认 `Overview`，同步底部 preview 固定状态并请求最终稳定帧，继续等待 `NAV_PANEL_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_index()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Overview / Library / People` 三组状态，以及最终回到默认 `Overview` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/nav_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_nav_panel
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- header、selected row、indicator、badge 和 footer 需要可辨识，但整体不能回到高噪音 showcase 风格。
- 主区域需要出现 `Overview -> Library -> People -> Overview` 的真实变化。
- rail item 必须继续满足 same-target release，不能误提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_index`。
- `read_only / !enable / static preview` 都不能误触发 selection listener，并且要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `Overview`，底部 `compact / read only` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_nav_panel/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Overview`、`Library` 和 `People`
  - 主区 RGB 差分边界为 `(52, 122) - (428, 232)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 232` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Overview`

## 12. 与现有控件的边界
- 相比 `menu_bar`：这里是页内常驻导航，不是命令入口。
- 相比 `breadcrumb_bar`：这里表达分区切换，不表达路径层级。
- 相比 `tab_strip`：这里是纵向 rail，不是横向页签。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和 preview 交互桥接逻辑。

## 13. 本轮保留与删减
- 保留的主区状态：`Overview`、`Library`、`People`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：rail item 选择、same-target release、`Up / Down / Home / End` 键盘导航、`items / header / footer / footer_badge / font / meta_font / palette / current_index / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、状态文案、旧双列 preview 包裹壳、preview 交互职责、preview 清焦桥接、过重 selected row、过强 indicator、过亮 footer / badge chrome、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `nav_panel` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_nav_panel/default`
  - 共捕获 `8` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/nav_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_nav_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=116`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Overview`、`Library` 和 `People`
  - 主区 RGB 差分边界为 `(52, 122) - (428, 232)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 232` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Overview`，底部 `compact / read only` preview 全程保持静态
