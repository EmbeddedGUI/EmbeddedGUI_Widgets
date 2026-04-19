# PipsPager 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI PipsPager`
- 补充对照控件：`flip_view`、`tab_strip`
- 对应组件名：`PipsPager`
- 当前保留形态：`Onboarding`、`Right`、`End`、`Gallery`、`compact`、`read only`
- 当前保留交互：主区继续保留 `previous / next / pip` 命中、same-target release、pip same-index release、`Tab / Right / Home / End / + / -` 键盘入口，以及 `title / helper / font / meta_font / palette / page_metrics / current_index / current_part / compact / read_only` setter 统一清理 `pressed_part / pressed_index / is_pressed`；`compact / read_only / !enable` guard 会先清理 `pressed` 再拒绝后续 `touch / key` 输入；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 `on_changed`
- 当前移除内容：compact preview 第二轨道、preview 点击清主控件 focus 的桥接动作、与离散分页本体无关的额外交互录制，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内已有 `pips_pager` 基础实现，本轮只收口 `reference` 页面说明、README 与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`pips_pager` 用来表达离散页码切换、前后翻页和当前位置反馈。它适合 onboarding、轻量轮播和短流程向导这类“页数有限，但位置必须一眼可见”的场景。

## 2. 为什么现有控件不够
- `tab_strip` 更偏 section 切换，不强调页码位置。
- `flip_view` 以内容卡片翻页为主，不以 pips rail 本体为中心。
- `scroll_bar` 是连续范围值，不适合离散页码。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `PipsPager` 语义的 reference 示例。

## 3. 当前页面结构
- 标题：`Pips Pager`
- 主区：一个主 `pips_pager`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Compact / 3 of 6`
- 右侧 preview：`read only`，固定显示 `Read only / 4 of 7`
- 页面结构统一收口为：标题 -> 主 `pips_pager` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/pips_pager/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Onboarding`
   默认主状态，显示 `2 of 7`
2. `Right`
   发送 `Right`，验证向后翻页
3. `End`
   发送 `End`，验证尾页边界
4. `Gallery`
   程序化切换主 snapshot，验证长页数和短窗口 rail
5. `Onboarding`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Compact / 3 of 6`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Read only / 4 of 7`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 198`
- 主控件尺寸：`196 x 92`
- 底部对照行尺寸：`216 x 58`
- 单个 preview 尺寸：`104 x 58`
- 页面结构：标题 -> 主 `pips_pager` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色主卡、低噪音边框，以及清晰但克制的 previous / next、当前页 pill、inactive dots 与灰蓝只读 palette

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Onboarding` | 是 | 否 | 否 |
| `Right` | 是 | 否 | 否 |
| `End` | 是 | 否 | 否 |
| `Gallery` | 是 | 否 | 否 |
| `Compact / 3 of 6` | 否 | 是 | 否 |
| `Read only / 4 of 7` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| previous / next same-target release | 是 | 否 | 否 |
| pip same-index release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_pips_pager.c` 当前覆盖 `9` 条用例：

1. `set_page_metrics()` 覆盖 total/current/visible 的钳制，以及 pip region 可解析性。
2. `set_title / helper / font / meta_font / palette / current_index / current_part / page_metrics / compact / read_only` setter 全部覆盖 `pressed_part / pressed_index / is_pressed` 清理。
3. 键盘导航覆盖 `Tab / Right / Home / End / + / -`，验证 current part 轮换与 current index 更新。
4. `previous / next` 触摸交互继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
5. pip 触摸交互继续遵守 same-index release：只有回到原始 pip 再 `UP` 才提交页码切换。
6. `ACTION_CANCEL` 只清理 `pressed`，不会误改 `current_index / current_part`，也不会误触发 `on_changed`。
7. `compact_mode` guard 会清理残留 `pressed`，并忽略后续 `touch / key` 输入；恢复后重新允许交互。
8. `read_only / !enable` guard 会清理残留 `pressed`，并忽略后续 `touch / key` 输入；恢复后重新允许交互。
9. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_index / current_part / compact_mode` 不变，且不会触发 `on_changed`。

补充说明：
- 主区 previous / next 属于非拖拽点击目标，统一遵守 same-target release。
- 主区 pip 还需要额外满足 `pressed_index` 与释放目标一致，不能移出后误提交。
- 底部 `compact / read only` preview 统一通过 `egui_view_pips_pager_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Onboarding` 和底部两个静态 preview，请求首帧并等待 `PIPS_PAGER_RECORD_FRAME_WAIT`。
2. 发送 `Right`，等待 `PIPS_PAGER_RECORD_WAIT`。
3. 请求第二帧并等待 `PIPS_PAGER_RECORD_FRAME_WAIT`。
4. 发送 `End`，等待 `PIPS_PAGER_RECORD_WAIT`。
5. 请求第三帧并等待 `PIPS_PAGER_RECORD_FRAME_WAIT`。
6. 程序化切到 `Gallery`，等待 `PIPS_PAGER_RECORD_WAIT`。
7. 请求第四帧并等待 `PIPS_PAGER_RECORD_FRAME_WAIT`。
8. 回到默认 `Onboarding`，同步底部 preview 固定状态并等待 `PIPS_PAGER_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `PIPS_PAGER_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`，保证初始化和录制首尾使用同一条布局路径。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Onboarding / Right / End / Gallery` 四组状态，以及最终回到默认 `Onboarding` 的稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pips_pager
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pips_pager
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- previous / next、当前页 pill 和 inactive dots 都必须可辨识，但不能回到高噪音 showcase 风格。
- 主区域需要出现 `Onboarding -> Right -> End -> Gallery -> Onboarding` 的真实变化。
- `previous / next` 必须继续满足 same-target release，pip 必须继续满足 same-index release，不能误提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_index / current_part`。
- `compact / read_only / !enable / static preview` 都不能误触发 `on_changed`，并且要先清理残留 `pressed`。
- 最终稳定帧必须显式回到默认 `Onboarding`，底部 `compact / read only` preview 全程保持静态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_pips_pager/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Onboarding`、`Right`、`End` 和 `Gallery`
  - 主区 RGB 差分边界为 `(54, 127) - (299, 206)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 206` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Onboarding`

## 12. 与现有控件的边界
- 相比 `tab_strip`：这里强调页码位置和前后翻页，不是 section 切换。
- 相比 `flip_view`：这里以 pips rail 本体为中心，不是内容卡片翻页容器。
- 相比 `scroll_bar`：这里是离散页码，不是连续范围值。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留 preview 桥接与额外录制职责。

## 13. 本轮保留与删减
- 保留的主区状态：`Onboarding`、`Right`、`End`、`Gallery`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`previous / next / pip` 命中、same-target release、pip same-index release、`Tab / Right / Home / End / + / -` 键盘导航、`title / helper / font / meta_font / palette / page_metrics / current_index / current_part / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：compact preview 第二轨道、preview 点击清主控件 focus 的桥接动作、与离散分页本体无关的额外交互录制、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc`
  - 本轮沿用已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `pips_pager` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_pips_pager/default`
  - 共捕获 `11` 帧
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用最近一次 navigation 分类回归结果：`13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pips_pager`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pips_pager`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.151 colors=90`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Onboarding`、`Right`、`End` 和 `Gallery`
  - 主区 RGB 差分边界为 `(54, 127) - (299, 206)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 206` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Onboarding`，底部 `compact / read only` preview 全程保持静态
