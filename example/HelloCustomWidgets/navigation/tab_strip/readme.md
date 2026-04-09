# tab_strip 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TabView`
- 补充对照控件：`tab_view`、`breadcrumb_bar`
- 对应组件名：`TabStrip`
- 本次保留状态：`standard`、`current tab`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态文案、旧双列 preview 包裹壳、preview 交互职责、过重 active tab 与 underline 强调
- EGUI 适配说明：继续复用仓库内 `tab_strip` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和绘制强度，不修改 `sdk/EmbeddedGUI`；`tabs / current index / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件

`tab_strip` 用来承载同一页面上下文里的平级 section 切换，例如 `Overview / Usage / Access` 这样的页内导航。它比 `breadcrumb_bar` 更偏平级导航，也比 `tab_view` 更轻，不需要额外内容面板时更适合直接放在正文上方。

## 2. 为什么现有控件不够用

- `tab_view` 带有 header + content shell，语义比页内标签条更重。
- `breadcrumb_bar` 表达层级路径，不适合平级 section 切换。
- `menu_bar` 和 `nav_panel` 属于命令或常驻导航，不是正文内的横向页签条。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI 的轻量 `TabStrip`。

## 3. 目标场景与示例概览

- 主控件展示标准 `tab_strip`，覆盖 `Overview`、`Usage`、`Access` 三个 section。
- 底部左侧展示 `compact` 静态对照，验证窄宽度下的轻量双标签条。
- 底部右侧展示 `read only` 静态对照，验证冻结交互后的弱化标签条。
- 页面结构统一收口为：标题 -> 主 `tab_strip` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和点击桥接逻辑已删除，底部两个 preview 直接挂在同一行容器下。

目标目录：`example/HelloCustomWidgets/navigation/tab_strip/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 136`
- 主控件尺寸：`198 x 44`
- 底部对照行尺寸：`216 x 36`
- `compact` 预览：`104 x 36`
- `read only` 预览：`104 x 36`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白色标签条容器和低噪音浅边框。
  - 页签按文本内容宽度自然排布，不回退到等宽分栏。
  - 当前项保留轻量填充和 underline，但降低 active fill、文字染色和双层底线强度。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `bar_primary` | `egui_view_tab_strip_t` | `198 x 44` | `Overview` | 主 `TabStrip` |
| `bar_compact` | `egui_view_tab_strip_t` | `104 x 36` | `Home` | 底部 compact 静态对照 |
| `bar_read_only` | `egui_view_tab_strip_t` | `104 x 36` | `Audit` | 底部 read only 静态对照 |
| `primary_tabs` | `const char *[3]` | - | `Overview / Usage / Access` | 主控件录制轨道 |
| `compact_tabs` | `const char *[2]` | - | `Home / Logs` | compact 预览程序化切换 |
| `read_only_tabs` | `const char *[2]` | - | `Usage / Audit` | read only 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview` | 默认主 snapshot，验证标准 tab strip |
| 主控件 | `Usage` | 程序化切换第二项 |
| 主控件 | `Access` | 程序化切换第三项 |
| `compact` | `Home` | 默认 compact 对照 |
| `compact` | `Logs` | 第二组 compact 对照 |
| `read only` | `Audit` | 固定只读轨道，禁 touch、禁 focus、禁键盘 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `Usage`。
4. 请求第二张截图。
5. 程序化切换主控件到 `Access`。
6. 请求第三张截图。
7. 程序化切换 `compact` 到 `Logs`。
8. 请求最终截图。

## 8. 编译、touch、runtime、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部 `compact / read only` 预览都必须完整可见，不能重新长出旧 preview 壳层。
- variable-width tab 必须保持自然留白，不能退化成均分按钮。
- 当前项 fill、divider 和 underline 需要可辨识，但整体不能回到高噪音 showcase 风格。
- `compact` 在窄宽度下仍要保持标签可读。
- `read only` 只能做静态展示，不能响应 touch、focus 或键盘；切换到 `read only` 时还要清空 pressed 状态。
- `tabs / current index / compact / read only / view disabled` 切换后不能残留 tab 的 `pressed` 高亮或下压位移渲染。
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时先清理残留 `pressed` 状态。

## 9. 已知限制与后续方向

- 当前版本只覆盖纯文本页签，不做图标、关闭按钮和拖拽重排。
- 文本宽度仍采用轻量估算，没有接入真实字体测量。
- 当前主页面只验证页签条本体，不联动内容面板。
- 当前键盘交互只覆盖 `Left / Right / Home / End` 的主线导航。

## 10. 与现有控件的边界

- 相比 `tab_view`：这里不承载内容面板，只保留页内标签条语义。
- 相比 `breadcrumb_bar`：这里表达平级切换，不表达层级路径。
- 相比 `menu_bar`：这里是页面 section 导航，不是命令分组入口。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和预览交互桥接逻辑。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TabView`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`TabStrip`
- 本次保留核心状态：
  - `standard`
  - `current tab`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰

- 删除页面级 `guide`、状态文案、preview 标签和旧的双列 preview 包裹结构。
- 删除 preview 参与交互和页面桥接的职责。
- 删除过重 active fill、过亮文字染色和过强双层 underline。
- 删除与 reference 无关的说明性外壳和场景叙事。

## 14. EGUI 适配时的简化点与约束

- 使用固定 tab 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- `read only` 直接使用 `read_only_mode`，避免页面语义和控件实现脱节。
- 通过程序化切换 current index 保证 runtime 能稳定抓到页签变化。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
