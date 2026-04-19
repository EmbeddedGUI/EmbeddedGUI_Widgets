# TabStrip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TabView`
- 补充对照控件：`tab_view`、`breadcrumb_bar`
- 对应组件名：`TabStrip`
- 当前保留形态：`Overview`、`Usage`、`Access`、`compact`、`read only`
- 当前保留交互：主区保留 same-target release、`Left / Right / Home / End` 键盘切换、`set_tabs / current_index / font / palette / compact / read_only` setter 统一清理 `pressed`，以及 `compact_mode` 下继续允许标签切换；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 `on_tab_changed`
- 当前移除内容：页面级 `guide`、旧 preview 标签、双列 preview 包裹、让 preview 继续承担切换职责的桥接逻辑、过重 active tab 与 underline 强调，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `tab_strip` 基础实现，本轮只收口 `reference` 页面结构、静态 preview、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`tab_strip` 用来承载同一页面上下文里的平级 section 切换，例如 `Overview / Usage / Access` 这样的页内导航。它比 `breadcrumb_bar` 更偏平级导航，也比 `tab_view` 更轻，不需要额外内容面板时更适合直接放在正文上方。

## 2. 为什么现有控件不够
- `tab_view` 带有 header + content shell，语义比页内标签条更重。
- `breadcrumb_bar` 表达层级路径，不适合平级 section 切换。
- `menu_bar` 和 `nav_panel` 属于命令或常驻导航，不是正文内的横向标签条。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI 的轻量 `TabStrip`。

## 3. 当前页面结构
- 标题：`Tab Strip`
- 主区：一个主 `tab_strip`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Home`
- 右侧 preview：`read only`，固定显示 `Audit`
- 页面结构统一收口为：标题 -> 主 `tab_strip` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/tab_strip/`

## 4. 主区 reference 快照
主区录制轨道只保留三组主区状态与最终稳定帧：

1. `Overview`
   默认主状态，显示标准 `TabStrip`
2. `Usage`
   程序化切到第二项，验证中间态
3. `Access`
   程序化切到第三项，验证尾态
4. `Overview`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Home`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Audit`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 136`
- 主控件尺寸：`198 x 44`
- 底部对照行尺寸：`216 x 36`
- 单个 preview 尺寸：`104 x 36`
- 页面结构：标题 -> 主 `tab_strip` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色页签容器、低噪音边框、variable-width tab 自然留白，以及克制的 active fill / underline 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Overview` | 是 | 否 | 否 |
| `Usage` | 是 | 否 | 否 |
| `Access` | 是 | 否 | 否 |
| `Home` | 否 | 是 | 否 |
| `Audit` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `same-target release` | 是 | 否 | 否 |
| `keyboard navigation` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_tab_strip.c` 当前覆盖 `7` 条用例：

1. `set_tabs()` 覆盖 tab 数量钳制、空数据重置，以及 `current_index / pressed` 清理。
2. `current_index / font / palette / compact / read_only` setter 覆盖 listener 行为、默认字体回落、调色板写入、模式钳制、文本省略与布局 helper，以及 `pressed` 清理。
3. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发切换；`ACTION_CANCEL` 只清理 `pressed`。
4. 键盘 `Left / Right / Home / End` 继续驱动当前 tab 切换。
5. `compact_mode` 切换后会清理残留 `pressed`，但保留标签切换能力。
6. `read_only_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入，恢复后继续允许切换。
7. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_index / compact_mode` 不变，且不会触发 `on_tab_changed`。

补充说明：
- 主区 same-target release 继续遵守非拖拽控件口径：只有回到初始命中项再 `UP` 才能提交。
- 底部 `compact / read only` preview 统一通过 `egui_view_tab_strip_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- 预览输入只清理残留 `pressed`，不改 `current_index`，也不触发 `on_tab_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Overview` 和底部两个静态 preview，请求首帧并等待 `TAB_STRIP_RECORD_FRAME_WAIT`。
2. 切到 `Usage`，等待 `TAB_STRIP_RECORD_WAIT`。
3. 请求第二帧并等待 `TAB_STRIP_RECORD_FRAME_WAIT`。
4. 切到 `Access`，等待 `TAB_STRIP_RECORD_WAIT`。
5. 请求第三帧并等待 `TAB_STRIP_RECORD_FRAME_WAIT`。
6. 回到默认 `Overview`，同步底部 preview 固定状态并等待 `TAB_STRIP_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `TAB_STRIP_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_index()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Overview / Usage / Access` 三组状态，以及最终回到默认 `Overview` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tab_strip
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tab_strip
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能重新长出旧 preview 壳层。
- variable-width tab 必须保持自然留白，不能退化成均分按钮。
- 当前项 fill、divider 和 underline 需要可辨识，但整体不能回到高噪音 showcase 风格。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能误提交，只有回到 `A` 才能提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_index` 或误触发监听器。
- `read_only_mode / !enable / static preview` 不仅要忽略后续输入，还要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `Overview`，底部 `compact / read only` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_tab_strip/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Overview`、`Usage` 和 `Access`
  - 主区 RGB 差分边界为 `(156, 202) - (322, 224)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 224` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Overview`

## 12. 与现有控件的边界
- 相比 `tab_view`：这里不承载内容面板，只保留页内标签条语义。
- 相比 `breadcrumb_bar`：这里表达平级切换，不表达层级路径。
- 相比 `menu_bar`：这里是页面 section 导航，不是命令分组入口。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和 preview 桥接逻辑。

## 13. 本轮保留与删减
- 保留的主区状态：`Overview`、`Usage`、`Access`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：same-target release、`Left / Right / Home / End` 键盘切换、`set_tabs / current_index / font / palette / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、旧 preview 标签、双列 preview 包裹、让 preview 继续承担切换职责的桥接逻辑、过重 active tab 与 underline 强调、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `tab_strip` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_tab_strip/default`
  - 共捕获 `9` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tab_strip`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tab_strip`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1038 colors=95`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Overview`、`Usage` 和 `Access`
  - 主区 RGB 差分边界为 `(156, 202) - (322, 224)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 224` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Overview`，底部 `compact / read only` preview 全程保持静态
