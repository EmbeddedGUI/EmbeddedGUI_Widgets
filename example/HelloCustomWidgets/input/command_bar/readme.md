# command_bar 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`CommandBar / Toolbar`
- 本次保留状态：`standard`、`compact`、`disabled`
- 删除效果：页面级 `guide`、状态回显、section divider、外部 preview 标签、标签点击切换、场景化 demo 壳、复杂溢出菜单动画、完整 hover / pressed / focus ring 体系
- EGUI 适配说明：保留常驻命令栏的主命令、scope pill、overflow 入口和 `compact / disabled` 对照；在 `480 x 480` 画布中优先保证低噪音、稳定留白和命令层级可读

## 1. 为什么需要这个控件？
`command_bar` 用来表达页内常驻工具栏，而不是弹出菜单或整页导航。它适合编辑、审核、布局、发布等需要“一组高频命令长期停留在页面顶部”的场景，强调主命令、当前 scope 和 overflow 入口的分层。

## 2. 为什么现有控件不够用
- `menu_flyout` 是局部弹出菜单，不是常驻命令栏
- `nav_panel`、`breadcrumb_bar`、`tab_strip` 属于导航结构，不承担主操作语义
- `split_button`、`drop_down_button` 只表达单个命令入口，不表达整条命令栏
- 旧的 showcase / HMI 风格页面噪音偏高，不适合作为 Fluent 命令栏 reference

因此这里继续保留 `command_bar`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `command_bar`，覆盖 `Edit / Review / Layout / Publish` 四组 snapshot
- 左下 `compact` 预览展示紧凑 icon-first rail
- 右下 `disabled` 预览展示禁用态命令栏
- 主控件保留真实命令焦点切换与 `Left / Right / Tab / Home / End` 键盘闭环
- 页面只保留标题、主 `command_bar` 和底部 `compact / disabled` 双预览，不再保留旧版 `guide`、状态回显、分隔线和外部标签

目录：
- `example/HelloCustomWidgets/input/command_bar/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 192`
- 页面结构：标题 -> 主 `command_bar` -> `compact / disabled` 双预览
- 主控件：`196 x 88`
- 底部双预览容器：`216 x 64`
- `compact` 预览：`104 x 64`
- `disabled` 预览：`104 x 64`
- 视觉规则：
  - 使用浅灰白 `page panel` 和低噪音白色工具栏卡片，避免 HMI / 工业风装饰
  - 主控件保留 `eyebrow + title + scope + command rail + footer` 五段层级
  - `compact` 预览收敛为静态 icon-first 摘要，不再承担演示切换职责
  - `disabled` 预览通过禁用态调色板与输入屏蔽表达不可用，而不是加重装饰

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 192` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Command Bar` | 页面标题 |
| `bar_primary` | `egui_view_command_bar_t` | `196 x 88` | `Edit` | 标准主控件 |
| `bar_compact` | `egui_view_command_bar_t` | `104 x 64` | `Quick` | 紧凑静态预览 |
| `bar_disabled` | `egui_view_command_bar_t` | `104 x 64` | `Locked` | 禁用静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Disabled |
| --- | --- | --- | --- |
| 默认 `Edit` | 是 | 否 | 否 |
| 切换到 `Review` | 是 | 否 | 否 |
| 切换到 `Layout` | 是 | 否 | 否 |
| 切换到 `Publish` | 是 | 否 | 否 |
| 切换到 `Compact review` | 否 | 是 | 否 |
| 键盘焦点切换 | 是 | 否 | 否 |
| 禁用锁定 | 否 | 否 | 是 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主控件和 `compact` 预览状态
2. 抓取首帧 `Edit` reference
3. 通过 `Right` 键把主控件焦点切到下一条命令
4. 抓取第二帧键盘切换结果
5. 切到 `Review` snapshot 并继续通过键盘切换到下一条命令
6. 抓取第三帧焦点变化结果
7. 切到 `Layout` snapshot 并通过 `End` 跳到末尾命令
8. 抓取第四帧键盘收口结果
9. 程序化把主控件切到 `Publish`，同时把 `compact` 预览切到第二组对照
10. 抓取最终收尾截图并保留 `disabled` 对照

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部双预览必须完整可见，不能被裁切
- `scope pill`、命令按钮、overflow 入口之间留白必须稳定，不能贴边
- 主控件键盘切换必须清晰可辨，不再依赖外部 `guide` 或状态文本
- 页面中不再出现旧版 `guide`、状态回显、分隔线和外部 `preview` 标签
- `compact` 与 `disabled` 必须保持 Fluent / WPF UI 风格的低噪音浅色 reference

## 9. 已知限制与后续方向
- 当前是固定尺寸 reference 实现，未覆盖真实响应式隐藏与测量
- 当前 overflow 只保留入口语义，没有真实弹出菜单
- 当前 glyph 使用双字母占位，不接入真实图标资源
- 若后续需要下沉到框架层，再评估与 `menu_flyout`、`button`、`toolbar` 抽象的复用边界

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `menu_flyout`：这里是常驻 rail，不是弹出式命令面板
- 相比 `nav_panel` / `breadcrumb_bar` / `tab_strip`：这里是操作命令，不是导航结构
- 相比 `split_button` / `drop_down_button`：这里承载命令组，而不是单个按钮入口
- 相比旧 showcase / HMI 页面：这里强调低噪音 Fluent reference，而非场景化大装饰

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`CommandBar / Toolbar`
- 本次保留状态：
  - `standard`
  - `compact`
  - `disabled`
  - `edit`
  - `review`
  - `layout`
  - `publish`

## 13. 相比参考原型删除了哪些效果或装饰
- 不做页面级 `guide`、状态回显、分隔线和外部 `preview` 标签
- 不做真实 overflow 弹层、响应式隐藏算法和多级菜单转场
- 不做桌面级 hover / pressed / focus ring 动画与复杂阴影
- 不做真实图标资源、快捷键标签和额外徽标

## 14. EGUI 适配时的简化点与约束
- 使用固定 snapshot 驱动 reference，优先保证 `480 x 480` 下的稳定审阅
- 主控件保留 `eyebrow + title + scope + command rail + footer` 五段结构
- `compact` 预览通过 touch/key override 固定为静态对照
- `disabled` 预览通过 `disabled_mode + compact_mode` 固定为静态不可交互对照
