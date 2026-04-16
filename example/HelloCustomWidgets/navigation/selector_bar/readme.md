# selector_bar 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI SelectorBar`
- 补充对照控件：`tab_strip`、`segmented_control`、`nav_panel`
- 对应组件名：`SelectorBar`
- 本次保留状态：`standard`、`selected item`、`compact`、`icon only`
- 本次删除效果：页面级 guide、旧 preview 标签桥接、额外的内容面板壳层、过重的 active fill 和强调性 tab shell
- EGUI 适配说明：在 custom 层实现轻量 `egui_view_selector_bar`，统一收口 icon+text item、same-target release、键盘切换和静态 preview，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`selector_bar` 用来承载少量同级页面或内容区域之间的快速切换。它比 `nav_panel` 更轻，也比完整的 `tab_view` 更适合放在页面正文上方，表达“这里有几个并列分区，可以直接切换”的语义。

## 2. 为什么现有控件不够用

- `segmented_control` 更偏输入选择器，视觉上是分段按钮，不适合承载页面级导航语义。
- `tab_strip` 主要覆盖纯文本 section bar，本次仍缺少 `SelectorBar` 的 icon+text item 语义。
- `nav_panel` 属于常驻导航容器，信息层级更重，不适合正文内的轻量切换。
- `tab_view` 自带内容页签壳层，超出了 `SelectorBar` 只负责“选择入口”的范围。

## 3. 目标场景与示例概览

- 主控件展示标准 `selector_bar`，使用 `Recent / Search / Saved` 三个 item，保留 icon+text 组合。
- 底部左侧保留 `compact` static preview，对照较窄宽度下的轻量布局。
- 底部右侧保留 `icon only` static preview，对照只用图标表达入口时的收口方式。
- 页面结构统一为：标题 -> 主 `selector_bar` -> `compact / icon only` 双 preview。

目标目录：`example/HelloCustomWidgets/navigation/selector_bar/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 206`
- 主面板尺寸：`196 x 104`
- 主控件尺寸：`180 x 54`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 面板尺寸：`104 x 72`
- 单个 preview 控件尺寸：`84 x 42`
- 风格约束：
  - 使用浅灰 page panel、白色主面板和低噪音边框。
  - 主控件保留轻量 active fill + underline，不回退到重型 tab shell。
  - `compact` 与 `icon only` 仍沿用同一控件语义，只改变内容密度和展示方式。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_bar` | `egui_view_selector_bar_t` | `180 x 54` | `Recent` | 主 `SelectorBar` |
| `compact_bar` | `egui_view_selector_bar_t` | `84 x 42` | `Home` | 底部 compact static preview |
| `icon_only_bar` | `egui_view_selector_bar_t` | `84 x 42` | `Search` | 底部 icon only static preview |
| `primary_items` | `const char *[3]` | - | `Recent / Search / Saved` | 主控件文本轨道 |
| `primary_icons` | `const char *[3]` | - | `Schedule / Search / Favorite` | 主控件图标轨道 |
| `compact_items` | `const char *[2]` | - | `Home / Find` | compact 对照 |
| `icon_only_icons` | `const char *[3]` | - | `Home / Search / Settings` | icon only 对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Recent` | 默认状态，验证标准 SelectorBar 视觉 |
| 主控件 | `Search` | 程序化切换第二项 |
| 主控件 | `Saved` | 程序化切换第三项 |
| `compact` | `Home` | 固定静态 compact 对照 |
| `icon only` | `Search` | 固定静态 icon only 对照 |

- 主控件保留真实 `touch / key` 切换闭环：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_items()`、`set_current_index()`、`set_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()` 都会先清理残留 `pressed_index / is_pressed`
- 焦点进入且当前无选中项时，自动把焦点落到第一项
- 底部 preview 通过 `egui_view_selector_bar_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清 pressed，不改变 `current_index`
  - 不触发 `on_selection_changed`

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件与两个 preview 到默认状态
2. 输出默认截图
3. 切换主控件到 `Search`
4. 输出第二张截图
5. 切换主控件到 `Saved`
6. 输出第三张截图
7. 恢复主控件默认 `Recent`，同时重新应用底部两个静态 preview
8. 输出最终稳定帧

## 8. 编译、单测、touch、runtime 与 web 验收路径

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/selector_bar PORT=pc

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

验收重点：

- 主控件与底部两个 preview 必须完整可见，不能裁切。
- 主控件选中态必须稳定可辨识，但不能回到高噪音 tab shell。
- `compact` 与 `icon only` 必须在同一套控件语义下成立，而不是额外拼装页面壳。
- same-target release 语义必须通过：只有回到原 item 才提交。
- 静态 preview 必须在整条录制轨道中保持固定状态，同时吞掉输入并清掉残留 pressed。

## 9. 已知限制与后续方向

- 当前只覆盖少量固定 item，不引入数据源、滚动和溢出折叠。
- 当前 item 宽度仍是轻量估算，不接入真实文本测量。
- 当前只做 icon+text / icon only 两类内容，不引入自定义模板。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，是否下沉 SDK 层后续再评估。

## 10. 与现有控件的边界

- 相比 `segmented_control`：这里表达导航，不表达输入选项切换。
- 相比 `tab_strip`：这里补齐 icon+text item 语义，而不承载 tab shell。
- 相比 `nav_panel`：这里是正文内的轻量 section/page 选择条，不是常驻导航容器。
- 相比 `tab_view`：这里不绑定内容页签壳层，也不处理 close/add 等重交互。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI SelectorBar`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`SelectorBar`
- 本次保留核心状态：
  - `standard`
  - `selected item`
  - `compact`
  - `icon only`

## 13. 相比参考原型删除的效果或装饰

- 删除 guide、状态解释条和旧 preview 桥接逻辑
- 删除完整 tab/page 壳层，避免与 `tab_view` 语义重叠
- 删除过重 active fill、过亮文本和强装饰性底线
- 删除与 reference 无关的场景化叙事说明

## 14. EGUI 适配时的简化点与约束

- 使用固定 item 数据保证 runtime 录制稳定。
- 通过统一 `clear_pressed_state()` 把 setter、guard 和 static preview 的状态清理收口到一处。
- `compact` 与 `icon only` 直接复用同一控件实现，不增加额外页面壳层。
- 焦点进入时仅在“无当前选中项”场景下自动回落到第一项，避免与已有选中态冲突。
