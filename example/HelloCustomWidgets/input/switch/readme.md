# switch 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI ToggleSwitch`
- 对应组件名：`ToggleSwitch`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`、`checked`
- 删除效果：页面级说明文案、额外标签、section divider、showcase 式夸张阴影与场景化叙事
- EGUI 适配说明：复用 SDK `switch` 的轨道 / thumb 绘制、same-target release 与 `perform_click` 语义，只在 custom 层补齐 Fluent 风格配色、`Space / Enter` 键盘闭环、setter `pressed` 清理、轻量 focus ring 与静态 preview 输入吞掉

## 1. 为什么需要这个控件？
`switch` 用于表达“状态立即生效的开 / 关切换”。它适合通知、同步、自动化和权限类设置项，比 `toggle_button` 更接近设置页里的标准拨杆控件。

## 2. 为什么现有控件不够用？
- `toggle_button` 更像命令按钮，强调按钮语义，不是设置页拨杆。
- `check_box` 更适合表单字段和清单选择，不强调轨道式开关反馈。
- SDK 已有基础 `switch`，但当前仓库还缺少一版对齐 `Fluent 2 / WPF UI` 的 `ToggleSwitch` reference 页面与单测闭环。

因此这里补上一版 `input/switch` reference，把标准 `ToggleSwitch` 接入 `HelloCustomWidgets` 主线。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `switch`，保留真实触摸切换与 `Space / Enter` 键盘切换。
- 左下 `compact` preview 展示小尺寸静态对照。
- 右下 `read only` preview 展示只读静态对照。
- 页面只保留标题、主控件和底部双 preview，不再保留说明文字和额外 chrome。

目录：
- `example/HelloCustomWidgets/input/switch/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 132`
- 页面结构：标题 -> 主 `switch` -> `compact / read only` 双 preview
- 主控件尺寸：`108 x 44`
- 底部 preview 行：`160 x 32`
- 单个 preview：`76 x 32`

视觉约束：
- 使用浅色 page panel 和低噪音背景。
- 主控件保留轻量 focus ring，checked 态使用 Fluent 主色轨道。
- `compact` 仅压缩尺寸与配色密度，不改变语义。
- `read only` 保留结果展示，但不承载真实输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 132` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toggle Switch` | 页面标题 |
| `primary_switch` | `egui_view_switch_t` | `108 x 44` | checked | 主控件 |
| `compact_switch` | `egui_view_switch_t` | `76 x 32` | checked | 紧凑静态预览 |
| `read_only_switch` | `egui_view_switch_t` | `76 x 32` | checked | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| checked / unchecked | 是 | 是 | 是 |
| 键盘 `Space / Enter` | 是 | 否 | 否 |
| same-target release | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focus ring | 是 | 否 | 否 |

## 7. 交互语义
- `Space / Enter`：按下进入 `pressed`，抬起时切换 `checked`。
- `DOWN(A) -> MOVE(B) -> UP(B)`：不提交切换。
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)`：只在回到同一目标后提交。
- 主控件继续复用 SDK `switch` 的默认触摸释放语义，不改 `sdk/EmbeddedGUI`。
- 底部 `compact / read only` preview 通过 `hcw_switch_override_static_preview_api()` 统一吞掉 `touch / key`，输入到达时立即清理残留 `pressed`。

## 8. 本轮收口内容
- 新增 `egui_view_switch.h/.c`，作为 SDK `switch` 的 Fluent reference 包装层。
- 在包装层补齐：
  - `standard / compact / read only` 样式 helper
  - `set_checked()`、`set_state_icons()`、`set_icon_font()` 的 `pressed` 清理
  - `Space / Enter` 键盘闭环
  - 静态 preview 输入吞掉
  - 主控件轻量 focus ring
- 新增 `test.c` reference 页面，只保留主控件与底部双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖样式 helper、setter 清理、same-target release、键盘切换、disabled 与静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 还原主控件、`compact` 和 `read only` 默认快照，并给主控件 request focus。
2. 截默认态。
3. 发送 `Space` 切到 unchecked。
4. 截第一组切换结果。
5. 程序化切到第二组图标快照，并把 `compact` 同步切到 unchecked 对照。
6. 截第二组快照。
7. 发送 `Enter` 把主控件切回 unchecked with icon 对照。
8. 截收尾帧。

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/switch PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/switch --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/switch
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_switch
```

验收重点：
- 主 `switch` 与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- `Space / Enter` 切换必须稳定，focus ring 可辨认。
- 触摸释放只允许同一目标提交，不允许漂移提交。
- `compact / read only` preview 必须吞掉输入并保持静态 reference。

## 11. 已知限制
- 当前只覆盖标准二态 `ToggleSwitch`，不扩展带说明文案的设置行组合。
- 不做 hover 动画、系统主题联动和更复杂的容器级设置表单。
- 不做单独文本标签；示例页只验证 switch 本体。

## 12. 与现有控件的差异边界
- 相比 `toggle_button`：这里是设置页开关，而不是命令按钮。
- 相比 `check_box`：这里强调轨道与 thumb 反馈，而不是复选框字段。
- 相比 `slider`：这里是二态切换，不提供连续值调节。

## 13. 对应组件名，以及本次保留的核心状态
- 对应组件名：`ToggleSwitch`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `focused`
  - `checked`

## 14. EGUI 适配时的简化点与约束
- 继续复用 SDK `switch` 的基础绘制和触摸释放逻辑，避免重写底层状态机。
- 在 custom 层统一收口样式、键盘与静态 preview 语义，不改 `sdk/EmbeddedGUI`。
- 页面保持最小 reference 结构，先完成控件级闭环，再考虑未来是否需要带说明文字的设置行包装。
