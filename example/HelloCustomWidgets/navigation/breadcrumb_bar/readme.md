# breadcrumb_bar 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`Breadcrumb`
- 本次保留状态：`standard`、`compact`、`read only`、`current item`
- 删减效果：页面级 guide / 状态文案 / section label、溢出菜单、Acrylic、阴影与复杂图标
- EGUI 适配说明：保留轻量路径条、当前项弱强调与紧凑折叠策略，用固定尺寸容器优先保证 `480 x 480` 下的可读性

## 1. 为什么需要这个控件

`breadcrumb_bar` 用来表达页面内的层级路径，是设置页、文档页、管理后台和资源浏览页里最常见的导航辅助控件之一。它比 `tab_strip` 更偏层级表达，也比旧 showcase 风格路径卡更适合作为 reference 主线的标准导航条。

## 2. 为什么现有控件不够用

- `tab_strip` 强调平级 section 切换，不表达层级路径
- `menu` 和 `button_matrix` 适合入口选择，不适合持续显示当前位置
- 旧的 showcase 路径类控件容器感和装饰感过重，不适合 Fluent / WPF UI 主线
- 当前主线需要一版更克制的标准 breadcrumb

## 3. 目标场景与示例概览

- 主卡展示标准 breadcrumb，用于页面顶部路径导航
- 左下 `Compact` 预览展示窄宽度下的折叠策略：`首项 / ... / 当前项`
- 右下 `Read only` 预览展示只读弱化路径条，验证锁定语义
- 示例页结构收敛为标题、主 `breadcrumb_bar` 和 compact / read-only 双预览，不再保留页面级 guide、状态文案和 section label

目标目录：

- `example/HelloCustomWidgets/navigation/breadcrumb_bar/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 140`
- 主路径条：`198 x 48`
- 底部双预览容器：`216 x 36`
- `Compact` / `Read only` 预览：`104 x 36`
- 视觉规则：
  - 使用浅灰 page panel + 白底轻边框路径条
  - 非当前项只保留文字与 chevron，不叠加重装饰
  - 当前项使用轻量蓝色 pill 高亮，但整体仍保持克制
  - `Compact` 通过折叠中间层级收紧，不再依赖额外标签解释
  - 页面只保留控件本体与双预览，不再出现说明性 chrome

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 140` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Breadcrumb Bar` | 页面标题 |
| `bar_primary` | `egui_view_breadcrumb_bar_t` | `198 x 48` | `Docs path` | 标准路径条 |
| `bar_compact` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Compact bills` | 紧凑折叠预览 |
| `bar_locked` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Read only` | 只读弱化预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主路径条 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Home / Docs / Nav / Details` | `Home / ... / Bills` | `Home / ... / Audit` |
| current item | `Docs / Forms / Review` 三组上下文轮换 | `Bills / Access` 两组折叠上下文轮换 | 固定为 `Audit` |
| compact shell | 不适用 | 有 | 有 |
| read only | 无 | 无 | 有 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 首帧固定主路径条为 `Docs path`、compact 预览为 `Compact bills`
2. 切到主路径条 `Forms path`
3. 切到主路径条 `Review path`
4. 切到 compact 预览 `Compact access`
5. 每次切换后请求精确截图，确保 runtime 直接抓到稳定状态

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
```

验收重点：

- 主路径条和底部双预览都必须完整可见，不能再出现大片空白页板
- 当前项 pill 高亮要明确，但整体不能回到厚重 showcase 视觉
- 分隔符、文字截断和折叠省略号要稳定，不显得拥挤
- `Compact` 在窄宽度下仍需清楚体现层级折叠
- `Read only` 弱化后仍要能清晰看出当前路径

## 9. 已知限制与后续方向

- 当前版本采用近似字符宽度估算，未接入真实字体测量
- 折叠策略仍是静态规则，没有实现真实的 `More` 溢出菜单
- 当前主页面只验证路径条本体，不联动页面跳转

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `tab_strip`：这里表达层级路径，不承担平级切换
- 相比 `menu_bar`：这里用于当前位置说明，不是命令分组入口
- 相比 `nav_panel`：这里不是常驻导航面板，而是页面顶部路径线索
- 相比旧 showcase 路径卡：这里是低噪音 reference 版本，不保留重容器装饰

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Breadcrumb`
- 本次保留状态：
  - `standard`
  - `current item`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、section label 与额外说明控件
- 不做真实的 `More` 溢出菜单
- 不引入额外图标资源，只保留文字与 chevron
- 不做阴影、Acrylic、背景模糊和系统级特效

## 14. EGUI 适配时的简化点与约束

- 使用固定尺寸和静态路径样本，优先保证 `480 x 480` 下的审阅效率
- 用近似字符宽度和省略号做轻量级截断，不引入复杂测量逻辑
- compact 与 read-only 直接复用同一控件语义，不再依赖外围标签解释
- 先完成 reference 版路径条核心语义，再决定是否上升到框架层
