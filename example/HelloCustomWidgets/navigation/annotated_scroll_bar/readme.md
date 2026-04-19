# AnnotatedScrollBar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AnnotatedScrollBar`
- 补充对照控件：`scroll_bar`、`pips_pager`
- 对应组件名：`AnnotatedScrollBar`
- 当前保留形态：`Gallery rail`、`Down`、`+`、`End`、`Release rail`、`compact`、`read only`
- 当前保留交互：主区继续保留 `decrease / rail / increase / marker` 命中、`Tab / Up / Down / Home / End / - / + / Enter / Space / Escape` 键盘入口、marker jump、rail drag，以及 `set_markers / content_metrics / step_size / offset / current_part / compact / read_only` setter 统一清理 `pressed`；`compact / read_only / !enable` guard 会先清理 `pressed` 再拒绝后续输入；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 `on_changed`
- 当前移除内容：页面级 `guide`、状态栏、外部 preview 标签、第二条 `compact` 预览轨道、preview 点击桥接、额外页面 chrome、过重的 bubble / connector / rail 装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `annotated_scroll_bar` 基础实现，本轮只收口 `reference` 页面说明、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`annotated_scroll_bar` 用来表达长内容中的语义分段导航。它适合年表、版本里程碑、审计记录、归档列表这类“需要按 section 快速跳转，但又不是分页切换”的界面。

## 2. 为什么现有控件不够
- `scroll_bar` 只表达比例和当前位置，不表达 marker 与注释语义。
- `pips_pager` 是离散页切换，不适合连续长列表定位。
- `breadcrumb_bar` 负责路径层级，不负责纵向 rail 导航。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `AnnotatedScrollBar` 语义的 custom widget。

## 3. 当前页面结构
- 标题：`Annotated Scroll Bar`
- 主区：一个主 `annotated_scroll_bar`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Mix / Edit / Focus / Wrap`
- 右侧 preview：`read only`，固定显示 `Plan / Check / Ship / Archive`
- 页面结构统一收口为：标题 -> 主 `annotated_scroll_bar` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/annotated_scroll_bar/`

## 4. 主区 reference 快照
主区录制轨道只保留五组主区状态与最终稳定帧：

1. `Gallery rail`
   默认主状态，显示 `Travel imports / Remote shoots / Archive cleanup`
2. `Down`
   发送 `Down` 小步进，验证 active marker 和 rail 位置联动
3. `+`
   发送 `+` 大步进，验证大步长跳转
4. `End`
   跳到末尾，验证尾部定位与终点状态
5. `Release rail`
   程序化切到下一组 snapshot，验证 `summary / bubble / marker / rail` 同步更新
6. `Gallery rail`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Mix / Edit / Focus / Wrap`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Plan / Check / Ship / Archive`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 260`
- 主控件尺寸：`196 x 150`
- 底部对照行尺寸：`216 x 68`
- 单个 preview 尺寸：`104 x 68`
- 页面结构：标题 -> 主 `annotated_scroll_bar` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色主卡、低噪音边框、轻量 summary / bubble / marker label，以及克制的 rail 高亮与灰蓝只读 palette

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Gallery rail` | 是 | 否 | 否 |
| `Down` | 是 | 否 | 否 |
| `+` | 是 | 否 | 否 |
| `End` | 是 | 否 | 否 |
| `Release rail` | 是 | 否 | 否 |
| `Mix / Edit / Focus / Wrap` | 否 | 是 | 否 |
| `Plan / Check / Ship / Archive` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `marker jump / rail drag` | 是 | 否 | 否 |
| 非拖拽部件 same-target release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_annotated_scroll_bar.c` 当前覆盖 `13` 条用例：

1. `set_content_metrics()`、`set_offset()` 与 region 查询覆盖内容长度、viewport、offset 的钳制，以及 marker region 可解析性。
2. `set_markers / content_metrics / step_size / offset / current_part` 覆盖数量钳制、当前 part 回落，以及 `pressed_part / pressed_marker / rail_dragging / is_pressed` 的统一清理。
3. 键盘导航用例覆盖 `Tab / Down / + / Home / End`，验证当前 part 轮换与 offset 跳转。
4. 触摸点击用例覆盖 `increase` 与 `marker` 两条命中路径，验证 offset 与 active marker 更新。
5. 拖拽用例覆盖 `rail drag` 到末尾，以及 `marker -> rail` 拖拽切换。
6. `ACTION_CANCEL` 只清理 `pressed`，不会残留 `pressed_part / pressed_marker / rail_dragging`。
7. `compact_mode` guard 会清理残留 `pressed`，并忽略后续 `touch / key` 输入；恢复后重新允许交互。
8. `read_only_mode` guard 会清理残留 `pressed`，并忽略后续 `touch / key` 输入；恢复后重新允许交互。
9. `!enable` guard 会清理残留 `pressed`，并忽略后续 `touch / key` 输入；恢复后重新允许交互。
10. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `offset / current_part / active_marker / compact_mode` 不变，且不会触发 `on_changed`。

补充说明：
- 主区里的 `decrease / increase / marker` 继续遵守 non-dragging 控件口径：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `rail` 保留连续拖拽语义，属于 navigation 触摸语义检查里的 allowlist 例外之一。
- 底部 `compact / read only` preview 统一通过 `egui_view_annotated_scroll_bar_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Gallery rail` 和底部两个静态 preview，请求首帧并等待 `ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT`。
2. 发送 `Down`，等待 `ANNOTATED_SCROLL_BAR_RECORD_WAIT`。
3. 请求第二帧并等待 `ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT`。
4. 发送 `+`，等待 `ANNOTATED_SCROLL_BAR_RECORD_WAIT`。
5. 请求第三帧并等待 `ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT`。
6. 发送 `End`，等待 `ANNOTATED_SCROLL_BAR_RECORD_WAIT`。
7. 请求第四帧并等待 `ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT`。
8. 程序化切到 `Release rail`，等待 `ANNOTATED_SCROLL_BAR_RECORD_WAIT`。
9. 请求第五帧并等待 `ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT`。
10. 回到默认 `Gallery rail`，同步底部 preview 固定状态并等待 `ANNOTATED_SCROLL_BAR_RECORD_WAIT`。
11. 请求最终稳定帧，并继续等待 `ANNOTATED_SCROLL_BAR_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Gallery rail / Down / + / End / Release rail` 五组状态，以及最终回到默认 `Gallery rail` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_annotated_scroll_bar
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- title、helper、summary、bubble、marker label、rail、indicator 和底部两个 preview 都必须可辨识。
- 主区域需要出现 `Gallery rail -> Down -> + -> End -> Release rail -> Gallery rail` 的真实变化。
- `decrease / increase / marker` 路径必须继续满足 same-target release，不能误提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `offset / current_part / active_marker`。
- `compact / read_only / !enable / static preview` 都不能误触发 `on_changed`，并且要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `Gallery rail`，底部 `compact / read only` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_annotated_scroll_bar/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 主区共出现 `5` 组唯一状态，对应 `Gallery rail`、`Down`、`+`、`End` 和 `Release rail`
  - 主区 RGB 差分边界为 `(54, 81) - (427, 289)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 289` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Gallery rail`

## 12. 与现有控件的边界
- 相比 `scroll_bar`：这里强调 marker 与注释，不强调 viewport 比例。
- 相比 `pips_pager`：这里是连续长列表定位，不是离散分页。
- 相比 `breadcrumb_bar`：这里是纵向 rail 导航，不是路径层级。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部叙事壳层和 preview 桥接逻辑。

## 13. 本轮保留与删减
- 保留的主区状态：`Gallery rail`、`Down`、`+`、`End`、`Release rail`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`decrease / rail / increase / marker` 命中、marker jump、rail drag、非拖拽部件 same-target release、`set_markers / content_metrics / step_size / offset / current_part / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、状态栏、外部 preview 标签、第二条 `compact` 预览轨道、preview 点击桥接、额外页面 chrome、过重的 bubble / connector / rail 装饰、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `annotated_scroll_bar` suite `13 / 13`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_annotated_scroll_bar/default`
  - 共捕获 `13` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_annotated_scroll_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1982 colors=172`
- 截图复核结论：
  - 共捕获 `13` 帧
  - 主区共出现 `5` 组唯一状态，对应 `Gallery rail`、`Down`、`+`、`End` 和 `Release rail`
  - 主区 RGB 差分边界为 `(54, 81) - (427, 289)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 289` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Gallery rail`，底部 `compact / read only` preview 全程保持静态
