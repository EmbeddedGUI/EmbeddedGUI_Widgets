# tab_strip 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`TabView / TabStrip`
- 本次保留状态：`standard`、`compact`、`read only`、`current tab`
- 删减效果：页面级 guide / 状态文案 / section label、图标页签、关闭按钮、拖拽重排、Acrylic 和复杂 hover 动效
- EGUI 适配说明：保留变宽 tab、轻量 underline 与弱填充选中态，用固定尺寸容器优先保证 `480 x 480` 下的页内导航可读性

## 1. 为什么需要这个控件

`tab_strip` 用来承载同一页面上下文里的平级 section 切换，例如 `Overview / Usage / Access` 这样的页内导航。它比 `breadcrumb_bar` 更偏平级导航，也比 `tab_view` 更轻，不需要额外内容面板时更适合直接放在正文上方。

## 2. 为什么现有控件不够用

- `tab_bar` 采用均分宽度，更像基础分页控件，不够接近 Fluent 的轻量 variable-width tab strip
- `tab_view` 带有 header + content shell，语义比页内标签条更重
- `breadcrumb_bar` 表达层级路径，不适合平级 section 切换
- `menu_bar` 和 `nav_panel` 属于命令或常驻导航，不是正文内的横向页签条

## 3. 目标场景与示例概览

- 主区域展示标准 `tab_strip`，对应 `Overview / Usage / Access` 三个 section
- 左下 `Compact` 预览展示窄宽度下的轻量双标签条
- 右下 `Read only` 预览展示只读弱化版标签条
- 示例页结构收敛为标题、主 `tab_strip` 和 compact / read-only 双预览，不再保留页面级 guide、状态文案和 section label

目标目录：

- `example/HelloCustomWidgets/navigation/tab_strip/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 136`
- 主标签条：`198 x 44`
- 底部双预览容器：`216 x 36`
- `Compact` / `Read only` 预览：`104 x 36`
- 视觉规则：
  - 使用浅灰 page panel + 白底轻边框标签容器
  - 页签按文本内容宽度自然排布，不回退到均分分栏
  - 当前项只保留弱填充、轻量文字强调和 underline 指示器
  - `Read only` 通过同一 palette 的统一弱化表达锁定，不额外添加说明标签
  - 页面只保留控件本体与双预览，不再堆叠说明性 chrome

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 136` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Tab Strip` | 页面标题 |
| `bar_primary` | `egui_view_tab_strip_t` | `198 x 44` | `Overview` | 标准标签条 |
| `bar_compact` | `egui_view_tab_strip_t` | `104 x 36` | `Home` | 紧凑标签条 |
| `bar_locked` | `egui_view_tab_strip_t` | `104 x 36` | `Audit` | 只读弱化标签条 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主标签条 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Overview` | `Home` | `Audit` |
| current tab | `Overview / Usage / Access` 三项切换 | `Home / Logs` 切换 | 固定为 `Audit` |
| compact shell | 不适用 | 有 | 有 |
| read only | 无 | 无 | 有 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 首帧固定主标签条为 `Overview`、compact 预览为 `Home`
2. 切到主标签条 `Usage`
3. 切到主标签条 `Access`
4. 切到 compact 预览 `Logs`
5. 每次切换后请求精确截图，确保 runtime 直接抓到稳定状态

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_strip PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_strip --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
```

验收重点：

- 主标签条和底部双预览都必须完整可见，不能再出现大片空白页板
- variable-width tab 必须保持自然留白，不能退化成均分按钮
- 当前项 underline 要明显但克制，不出现过厚或高饱和强调
- `Compact` 在窄宽度下仍要可读
- `Read only` 弱化后仍能明确看出当前页签

## 9. 已知限制与后续方向

- 当前版本只覆盖纯文本页签，不做图标、关闭按钮和拖拽重排
- 文本宽度仍采用轻量估算，没有接入真实字体测量
- 当前主页面只验证页签条本体，不联动内容面板

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `tab_bar`：这里按内容宽度排布，不走等宽分栏
- 相比 `tab_view`：这里不承载内容面板，只保留页内标签条语义
- 相比 `breadcrumb_bar`：这里表达平级切换，不表达层级路径
- 相比 `menu_bar`：这里是页面 section 导航，不是命令分组入口

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`TabView / TabStrip`
- 本次保留状态：
  - `standard`
  - `current tab`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、section label 与额外说明控件
- 不做图标页签、关闭按钮和拖拽排序
- 不做复杂 hover / pressed 过渡、Acrylic 和系统级特效
- 不做与 `tab_view` 类似的内容面板壳层

## 14. EGUI 适配时的简化点与约束

- 使用固定尺寸和轻量 palette，优先保证 `480 x 480` 下的审阅效率
- 通过近似字符宽度与省略号控制 tab 宽度，不引入复杂布局系统
- read-only 通过控件内弱化逻辑完成，不再依赖外部标签解释
- 先完成 reference 版页内导航条，再决定是否上升到框架公共控件
