# nav_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI NavigationView`
- 补充对照控件：`menu_bar`、`tab_strip`
- 对应组件名：`NavigationView`
- 本次保留状态：`standard`、`selected`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态文案、旧双列 preview 包裹壳、preview 交互职责、过重 selected row 与 footer chrome
- EGUI 适配说明：继续复用仓库内 `nav_panel` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和绘制强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`nav_panel` 用来承载页内常驻导航，而不是弹出菜单或一次性的命令入口。它适合设置页、管理页和工具页左侧的一级切换，让用户能持续看到“当前在哪个分区”。

## 2. 为什么现有控件不够用

- `menu` 更偏弹出式命令列表，不适合作为常驻导航面板。
- `breadcrumb_bar` 表达的是路径层级，不负责平级导航切换。
- `tab_strip` 更像横向页签条，不适合纵向常驻导航。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI 的轻量 `NavigationView` rail。

## 3. 目标场景与示例概览

- 主控件展示标准 `nav_panel`，覆盖 `Overview`、`Library`、`People` 三个一级导航项。
- 底部左侧展示 `compact` 静态对照，验证窄宽度下的 rail 视图。
- 底部右侧展示 `read only` 静态对照，验证冻结交互后的弱化 rail。
- 页面结构统一收口为：标题 -> 主 `nav_panel` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和页面桥接逻辑已删除，底部两个 preview 直接挂在同一行容器下。

目标目录：`example/HelloCustomWidgets/navigation/nav_panel/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 224`
- 主控件尺寸：`198 x 108`
- 底部对照行尺寸：`152 x 74`
- `compact` 预览：`58 x 74`
- `read only` 预览：`58 x 74`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白色面板容器和低噪音浅边框。
  - 标准态保留 header、selection indicator、badge 与 footer，但降低 selected row 和 badge 的强调强度。
  - `compact` 保留 rail 语义，不再依赖外部标签解释。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `panel_primary` | `egui_view_nav_panel_t` | `198 x 108` | `Overview` | 主 `NavigationView` |
| `panel_compact` | `egui_view_nav_panel_t` | `58 x 74` | `Home` | 底部 compact 静态对照 |
| `panel_read_only` | `egui_view_nav_panel_t` | `58 x 74` | `Teams` | 底部 read only 静态对照 |
| `primary_items` | `egui_view_nav_panel_item_t[3]` | - | `Overview / Library / People` | 主控件录制轨道 |
| `compact_items` | `egui_view_nav_panel_item_t[3]` | - | `Home / Files / Rules` | compact 预览程序化切换 |
| `read_only_items` | `egui_view_nav_panel_item_t[3]` | - | `Feed / Teams / Audit` | read only 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview` | 默认主状态，验证标准导航面板 |
| 主控件 | `Library` | 程序化切换第二项 |
| 主控件 | `People` | 程序化切换第三项 |
| 主控件 | `Library` | 回切中间状态，验证 selected row 稳定 |
| `compact` | `Home` | 默认 compact 对照 |
| `compact` | `Rules` | 第二组 compact 对照 |
| `read only` | `Teams` | 固定只读轨道，禁 touch、禁 focus、禁键盘 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `Library`。
4. 请求第二张截图。
5. 程序化切换主控件到 `People`。
6. 请求第三张截图。
7. 程序化回切主控件到 `Library`。
8. 请求第四张截图。
9. 程序化切换 `compact` 到 `Rules`。
10. 请求最终截图。

## 8. 编译、touch、runtime、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/nav_panel PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/nav_panel --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部 `compact / read only` 预览都必须完整可见，不能重新长出旧 preview 壳层。
- header、selected row、indicator、badge 和 footer 需要可辨识，但整体不能回到高噪音 showcase 风格。
- `compact` 在窄宽度下仍要保持 rail 语义清晰。
- `read only` 只能做静态展示，不能响应 touch、focus 或键盘。
- `HelloUnitTest` 里已有的 selection clamp、metrics/hit testing、touch 选择和 locked/disabled 语义不能回归。

## 9. 已知限制与后续方向

- 当前版本不做展开/收起 pane、树形层级与二级导航。
- 当前不做真实图标资源，使用 badge 字母替代。
- 当前主页面只验证导航面板本体，不联动内容区。

## 10. 与现有控件的边界

- 相比 `menu_bar`：这里是页内常驻导航，不是命令入口。
- 相比 `breadcrumb_bar`：这里表达分区切换，不表达路径层级。
- 相比 `tab_strip`：这里是纵向 rail，不是横向页签。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和 preview 交互桥接逻辑。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI NavigationView`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`NavigationView`
- 本次保留核心状态：
  - `standard`
  - `selected`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰

- 删除页面级 `guide`、状态文案、preview 标签和旧的双列 preview 包裹结构。
- 删除 preview 参与交互和页面桥接的职责。
- 删除过重 selected row、过强 indicator 与过亮 footer/badge chrome。
- 删除与 reference 无关的说明性外壳和场景叙事。

## 14. EGUI 适配时的简化点与约束

- 使用固定导航项数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 current index 保证 runtime 能稳定抓到导航状态。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
