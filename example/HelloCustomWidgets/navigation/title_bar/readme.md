# title_bar 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TitleBar`
- 补充对照控件：`nav_panel`、`menu_bar`、`selector_bar`
- 对应组件名：`TitleBar`
- 本次保留状态：`standard`、`back / pane toggle affordance`、`compact`、`read only`
- 本次删除效果：窗口级拖拽职责、系统 caption button、原生窗口宿主耦合、额外内容页壳层
- EGUI 适配说明：在 custom 层新增轻量 `egui_view_title_bar`，统一收口 `icon / title / subtitle / leading header / trailing header / primary action / secondary action`，补齐 same-target release、键盘切换与静态 preview 输入抑制，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`title_bar` 用来表达页面或工作区顶部的导航与状态入口。它不是系统窗口栏的完整复刻，而是一个贴近 Fluent / WPF UI `TitleBar` 语义的轻量 reference 容器，负责承载返回、pane toggle、标题、副标题和右侧动作入口。

## 2. 为什么现有控件不够用

- `menu_bar` 更偏命令入口，不负责页面标题与返回语义。
- `nav_panel` 是常驻导航 rail，层级更重，不适合页面顶部的单条标题栏。
- `selector_bar` 只负责分区切换，缺少标题、副标题和左右 header 的组合表达。
- 当前 reference 主线还缺少一版明确对齐 `Fluent / WPF UI TitleBar` 的轻量顶栏控件。

## 3. 目标场景与示例概览

- 主控件展示标准 `title_bar`，覆盖 `back`、`pane toggle`、leading icon、标题、副标题和右侧动作。
- 底部左侧保留 `compact` static preview，对照更高密度的标题栏收口方式。
- 底部右侧保留 `read only` static preview，对照冻结交互后的弱化状态。
- 页面结构统一为：标题 -> 主 `title_bar` -> `compact / read only` 双 preview。

目标目录：`example/HelloCustomWidgets/navigation/title_bar/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 214`
- 主控件尺寸：`196 x 96`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 尺寸：`104 x 72`
- 风格约束：
  - 使用浅灰 page panel、白色标题栏卡片和低噪音阴影。
  - 主控件保留左侧 affordance、leading icon badge、标题栈和右侧动作 pill。
  - `compact` 模式收掉 header / subtitle，只保留最核心的导航与动作骨架。
  - `read only` 仅做静态 reference 对照，不承担实际交互职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `title_bar_primary` | `egui_view_title_bar_t` | `196 x 96` | `Atlas` | 主 `TitleBar` |
| `title_bar_compact` | `egui_view_title_bar_t` | `104 x 72` | `Atlas` | 底部 compact static preview |
| `title_bar_read_only` | `egui_view_title_bar_t` | `104 x 72` | `Preview` | 底部 read only static preview |
| `primary_snapshots` | `egui_view_title_bar_snapshot_t[4]` | - | `Atlas / Files / Release / Read back` | 主控件状态轨道 |
| `compact_snapshots` | `egui_view_title_bar_snapshot_t[1]` | - | `Atlas` | compact 固定对照 |
| `read_only_snapshots` | `egui_view_title_bar_snapshot_t[1]` | - | `Preview` | read only 固定对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Atlas` | 默认状态，展示完整 title bar |
| 主控件 | `Files` | 点击 `back` 后的第二状态 |
| 主控件 | `Release` | 点击 `pane toggle` 后的第三状态 |
| 主控件 | `Read back` | 点击主动作后的第四状态 |
| `compact` | `Atlas` | 固定 compact 对照，不参与状态切换 |
| `read only` | `Preview` | 固定只读对照，不参与状态切换 |

- 主控件保留真实 `touch / key` 导航闭环：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不触发动作
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才触发动作
- `set_snapshots()`、`set_current_snapshot()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都会先清理残留 `pressed_part / is_pressed`
- `back / pane toggle / primary action / secondary action` 四类 affordance 共用同一套 current-part 键盘导航顺序
- 底部 preview 通过 `egui_view_title_bar_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清残留 pressed，不改变 `snapshot` 和 `current_part`
  - 不触发 action listener

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件和两个 preview 到默认状态。
2. 输出默认截图。
3. 程序化切换主控件到 `Files`、`Release`、`Read back` 三组状态，并分别输出截图。
4. 恢复主控件默认状态，输出最终稳定帧。

主控件本身仍保留真实 `touch / key` 交互；录制轨道只负责稳定导出主区域状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担收尾点击或 snapshot 切换职责。

## 8. 编译、单测、touch、runtime 与 web 验收路径

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/title_bar PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

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

验收重点：

- 主控件与底部两个 preview 必须完整可见，不能裁切。
- 左侧返回 / pane toggle、leading icon、标题栈和右侧动作必须都能被辨识。
- 主区域需要出现 `Atlas -> Files -> Release -> Read back -> Atlas` 的真实变化。
- 底部 `compact / read only` preview 在整条 runtime 轨道中必须保持静态一致。
- same-target release 必须通过，不能在 `ACTION_MOVE` 改写 `pressed_part`。

## 9. 已知限制与后续方向

- 当前不承担系统窗口拖拽、caption button 与平台原生宿主接入。
- 当前只覆盖固定 snapshot 数据，不引入自定义标题模板。
- 当前动作 pill 采用轻量文本宽度估算，不接入真实文本测量。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，是否下沉框架层后续再评估。

## 10. 与现有控件的边界

- 相比 `menu_bar`：这里是标题与导航 affordance 的组合，不是菜单命令条。
- 相比 `nav_panel`：这里是页面顶部 title shell，不是左侧常驻 rail。
- 相比 `selector_bar`：这里保留标题、副标题和 header 状态，不只做 section 选择。
- 相比原生窗口标题栏：这里不处理系统按钮和窗口管理职责。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TitleBar`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`TitleBar`
- 本次保留核心状态：
  - `standard`
  - `back / pane toggle affordance`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰

- 删除系统 caption button、窗口拖拽区和平台窗口管理职责。
- 删除额外内容页壳层和场景化说明卡片。
- 删除高噪音描边、重阴影和过亮的动作强调。
- 删除 preview 承担真实切换职责与清主控件焦点的旧桥接逻辑。

## 14. EGUI 适配时的简化点与约束

- 使用 snapshot 轨道保证录制和 web 演示稳定。
- 四类交互 part 统一收口到 `current_part / pressed_part`，简化触摸和键盘闭环。
- `compact` 与 `read only` 直接复用同一控件实现，不新增额外页面外壳。
- static preview 只负责清 pressed，不改变内部 `snapshot / current_part`。
