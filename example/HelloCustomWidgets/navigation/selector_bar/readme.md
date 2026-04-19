# selector_bar 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI SelectorBar`
- 补充对照控件：`tab_strip`、`segmented_control`、`nav_panel`
- 对应组件名：`SelectorBar`
- 当前保留形态：主区 `Recent`、`Search`、`Saved` 三组 reference 状态，以及底部 `compact / icon only` 静态 preview
- 当前保留交互：主区 item 命中、`same-target release`、`Right / End / Left / Home / Tab / Enter` 键盘入口、`INDEX_NONE` 焦点回落，以及 setter / static preview 的残留 `pressed` 清理闭环
- 当前移除内容：页面级 guide、旧 preview 标签桥接、额外内容面板壳层、过重 active fill、强装饰 tab shell 与场景化叙事说明
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_selector_bar`，统一收口 icon+text item、icon only item、焦点回落、same-target release 和静态 preview；本轮只把 README 收口到当前 finalize 模板，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`selector_bar` 用来承载少量同级页面或内容区域之间的快速切换。它比 `nav_panel` 更轻，也比完整的 `tab_view` 更适合放在页面正文上方，表达“这里有几个并列分区，可以直接切换”的语义。

## 2. 为什么现有控件不够用

- `segmented_control` 更偏输入选择器，视觉上是分段按钮，不适合承载页面级导航语义。
- `tab_strip` 主要覆盖纯文本 section bar，本次仍缺少 `SelectorBar` 的 icon+text item 语义。
- `nav_panel` 属于常驻导航容器，信息层级更重，不适合正文内的轻量切换。
- `tab_view` 自带内容页签壳层，超出了 `SelectorBar` 只负责“选择入口”的范围。

## 3. 当前页面结构

- 标题：`SelectorBar`
- 主区：一个带 heading、主 `selector_bar` 和说明 note 的轻量 surface panel
- 底部：两个真正静态的 preview panel
- 左侧 preview：`compact`，固定显示 `Home`
- 右侧 preview：`icon only`，固定显示 `Search`
- 页面结构统一收口为：标题 -> 主 panel -> 底部 `compact / icon only`

目录：`example/HelloCustomWidgets/navigation/selector_bar/`

## 4. 主区 reference 快照

主区录制轨道只保留三组主区状态与最终稳定帧：

1. `Recent`
   默认主状态，heading 为 `Recent activity`
2. `Search`
   程序化切到第 2 项，heading 为 `Quick search`
3. `Saved`
   程序化切到第 3 项，heading 为 `Pinned collection`
4. `Recent`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   选中 `Home`
   静态紧凑对照，固定 `compact_mode`
2. `icon only`
   选中 `Search`
   静态图标对照，固定 `compact_mode + icon only`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根容器尺寸：`224 x 206`
- 主面板尺寸：`196 x 104`
- 主控件尺寸：`180 x 54`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 面板尺寸：`104 x 72`
- 单个 preview 控件尺寸：`84 x 42`
- 页面风格：浅灰 `page panel`、白色主 panel、低噪音边框和轻量 active fill + underline；`compact` 与 `icon only` 继续复用同一套控件语义，只改变内容密度和展示方式

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Icon only preview |
| --- | --- | --- | --- |
| `Recent` | 是 | 否 | 否 |
| `Search` | 是 | 否 | 否 |
| `Saved` | 是 | 否 | 否 |
| `Home` | 否 | 是 | 否 |
| `Search` icon only | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `icon only` item 布局 | 否 | 否 | 是 |
| item same-target release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_selector_bar.c` 当前覆盖 `4` 条用例：

1. `set_items()` 会钳制到 `EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS`，`set_current_index / set_font / set_icon_font / set_compact_mode / set_palette` 都会清理残留 `pressed_index / is_pressed`，重复设置当前项不会重复通知
2. 焦点进入且当前索引为 `INDEX_NONE` 时，会自动回落到第 1 项；键盘导航覆盖 `Right / End / Left / Home / Tab / Enter`
3. 主区触摸继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到 `A` 后 `UP(A)` 才提交；`ACTION_CANCEL` 只清理 `pressed`
4. 静态 preview 用例验证 “consumes input and preserves selection”：吞掉 `touch / key`，只清 pressed，不改变 `current_index`，也不触发 `on_selection_changed`

补充说明：

- 主区只处理轻量 section/page 入口切换，不承担完整 tab/page 壳层
- `compact` preview 复用文本 + 图标，但压缩内容密度
- `icon only` preview 复用同一控件实现，只去掉文本输入

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原主区默认 `Recent` 与底部两个静态 preview，请求首帧并等待 `SELECTOR_BAR_RECORD_FRAME_WAIT = 170`
2. 程序化切到 `Search`，等待 `SELECTOR_BAR_RECORD_WAIT = 90`
3. 请求第二帧并继续等待 `170`
4. 程序化切到 `Saved`，等待 `90`
5. 请求第三帧并继续等待 `170`
6. 回到默认 `Recent`，同时重新应用底部两个静态 preview，等待 `90`
7. 请求最终稳定帧，并继续等待 `SELECTOR_BAR_RECORD_FINAL_WAIT = 520`

说明：

- 全部截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制
- `apply_primary_snapshot()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`
- 底部 `compact / icon only` preview 在整条轨道中不承担切换、桥接或收尾职责

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/selector_bar PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/selector_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/selector_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_selector_bar
```

## 10. 验收重点

- 主控件与底部两个 preview 必须完整可见，不能黑屏、白屏或裁切
- 主控件选中态必须稳定可辨识，但不能回到高噪音 tab shell
- `compact` 与 `icon only` 必须在同一套控件语义下成立，而不是额外拼装页面壳
- item 触摸必须继续满足 same-target release；只有回到原 item 才提交
- 最终稳定帧必须显式回到默认 `Recent`，底部 preview 全程保持静态一致

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_selector_bar/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Recent`、`Search` 与 `Saved`
  - 主区 RGB 差分边界为 `(84, 116) - (397, 229)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 按 `y >= 229` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Recent`

## 12. 与现有控件的边界

- 相比 `segmented_control`：这里表达导航，不表达输入选项切换
- 相比 `tab_strip`：这里补齐 icon+text item 语义，而不承载 tab shell
- 相比 `nav_panel`：这里是正文内的轻量 section/page 选择条，不是常驻导航容器
- 相比 `tab_view`：这里不绑定内容页签壳层，也不处理 close/add 等重交互

## 13. 本轮保留与删减

- 保留的主区状态：`Recent`、`Search`、`Saved`
- 保留的底部对照：`compact`、`icon only`
- 保留的交互与实现约束：icon+text item、icon only item、`INDEX_NONE` 焦点回落、same-target release、`Right / End / Left / Home / Tab / Enter` 键盘切换、setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 guide、旧 preview 标签桥接、额外内容面板壳层、过重 active fill、强装饰 tab shell 与场景化叙事说明

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/selector_bar PORT=pc`
  - 本轮沿用 `2026-04-19` 已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test` 与 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `selector_bar` suite `4 / 4`
- 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/selector_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_selector_bar/default`
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用 `2026-04-19` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_selector_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1571 colors=119`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `3` 组唯一状态，对应 `Recent`、`Search` 与 `Saved`
  - 主区 RGB 差分边界为 `(84, 116) - (397, 229)`
  - 按 `y >= 229` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Recent`，底部 `compact / icon only` preview 全程保持静态
