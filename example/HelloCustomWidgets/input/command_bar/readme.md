# command_bar 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`CommandBar / Toolbar`
- 本次保留状态：`standard`、`compact`、`disabled`
- 删除效果：页面级 `guide`、状态回显、分隔线、外部 preview 标签、标签点击切换、场景化装饰、真实 overflow 菜单动画、完整 hover 动画体系
- EGUI 适配说明：保留主命令、scope pill、overflow 入口和底部双 preview，对 `480 x 480` 画布优先保证低噪音、稳定留白和清晰层级

## 1. 为什么需要这个控件
`command_bar` 用来承载页面内长期驻留的一组高频命令，强调主命令、当前 scope 和 overflow 入口的分层表达。它适合编辑、审核、布局和发布等场景，不等同于一次性弹出菜单，也不等同于页面导航。

## 2. 为什么现有控件不够用
- `menu_flyout` 解决的是弹出式菜单，不是常驻命令栏。
- `nav_panel`、`breadcrumb_bar`、`tab_strip` 解决的是导航，不承担主操作语义。
- `split_button`、`drop_down_button` 只表达单个入口，不表达一整组页面命令。
- 旧的 showcase / HMI 风格页面噪音过高，不能继续作为 Fluent reference。

因此仓库继续保留 `command_bar`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `command_bar`，覆盖 `Edit / Review / Layout / Publish` 四组 snapshot。
- 左下 `compact` preview 展示紧凑 icon-first rail。
- 右下 `disabled` preview 展示禁用态命令栏。
- 底部两个 preview 统一通过 `egui_view_command_bar_override_static_preview_api()` 固定为静态 reference，不再承担交互职责。
- 点击 preview 只允许清理主控件 `bar_primary` 的 focus，不允许改变 `current_snapshot`、`current_index` 或抢占焦点。
- 主控件保留真实键盘闭环：`Left / Right / Tab / Home / End`。

目录：
- `example/HelloCustomWidgets/input/command_bar/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 192`
- 页面结构：标题 -> 主 `command_bar` -> `compact / disabled` 双 preview
- 主控件：`196 x 88`
- 底部双 preview 容器：`216 x 64`
- `compact` preview：`104 x 64`
- `disabled` preview：`104 x 64`
- 视觉规则：
  - 使用浅灰白 page panel 和低噪音浅色命令栏卡片，避免 HMI / 工业风装饰。
  - 主控件保留 `eyebrow + title + scope + command rail + footer` 五段层级。
  - `compact` preview 只做静态紧凑对照，不再承担切换演示。
  - `disabled` preview 通过禁用态配色表达不可用，而不是增加额外装饰。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 192` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Command Bar` | 页面标题 |
| `bar_primary` | `egui_view_command_bar_t` | `196 x 88` | `Edit` | 标准主控件 |
| `bar_compact` | `egui_view_command_bar_t` | `104 x 64` | `Quick` | 紧凑静态 preview |
| `bar_disabled` | `egui_view_command_bar_t` | `104 x 64` | `Locked` | 禁用静态 preview |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Disabled |
| --- | --- | --- | --- |
| 默认 `Edit` | 是 | 否 | 否 |
| 切换到 `Review` | 是 | 否 | 否 |
| 切换到 `Layout` | 是 | 否 | 否 |
| 切换到 `Publish` | 是 | 否 | 否 |
| 切换到 `Compact review` | 否 | 是 | 否 |
| 键盘焦点切换 | 是 | 否 | 否 |
| 静态 preview 吞输入 | 否 | 是 | 是 |
| 禁用锁定 | 否 | 否 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 初始化主控件和 `compact` preview，并给主控件重新 `request focus`。
2. 截取首帧 `Edit` reference。
3. 通过 `Right` 键在主控件里切换焦点。
4. 截取第二帧键盘切换结果。
5. 切到 `Review` snapshot，并继续通过键盘切换当前命令。
6. 截取第三帧状态变化结果。
7. 切到 `Layout` snapshot，并通过 `End` 跳到末尾命令。
8. 截取第四帧键盘收口结果。
9. 程序化切到 `Publish`，同时把 `compact` preview 切到第二组静态对照。
10. 截取主状态稳定帧。
11. 主控件重新 `request focus`，点击 `compact` preview，只验证焦点收尾。
12. 截取最终收尾帧，确认 preview 点击后主控件失焦、preview 仍保持静态 reference。

## 8. 编译、单测、语义检查与 runtime 验收
```bash
make clean APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc
make all APP=HelloCustomWidgets APP_SUB=input/command_bar PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/command_bar --track reference --timeout 10 --keep-screenshots
```

验收重点：
- 主控件和底部双 preview 必须完整可见，不能黑屏、白屏、裁切或重排异常。
- `scope pill`、命令项和 overflow 入口之间的留白必须稳定。
- 主控件键盘切换必须清晰可辨，不再依赖外部 guide 或状态文本。
- `compact` 与 `disabled` preview 必须通过 `egui_view_command_bar_override_static_preview_api()` 固定为静态 reference。
- static preview 必须吞掉 `touch / key`，并清理残留 `pressed`，但不能改变 `current_snapshot` 或 `current_index`。
- 点击 preview 后只允许清理 `bar_primary` 的 focus，preview 自身不能抢焦点。
- runtime 最后一帧必须是“主控件重新聚焦 -> 点击 compact preview -> 收尾截图”之后的稳定结果。
- `same-target release` 语义必须继续满足：只有 `DOWN` 和 `UP` 命中同一命令项才允许提交。

## 9. 已知限制与后续方向
- 当前是固定尺寸 reference 实现，未覆盖真实响应式隐藏与测量。
- overflow 只保留入口语义，没有真实弹出菜单。
- glyph 仍使用双字母占位，不接入真实图标资源。
- 如需下沉到框架层，再评估与 `menu_flyout`、`button`、`toolbar` 抽象的复用边界。

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `menu_flyout`：这里是常驻 rail，不是弹出式命令面板。
- 相比 `nav_panel` / `breadcrumb_bar` / `tab_strip`：这里表达操作命令，不表达导航结构。
- 相比 `split_button` / `drop_down_button`：这里承载的是命令组，而不是单个按钮入口。
- 相比旧 showcase / HMI 页面：这里只保留低噪音 Fluent reference，不再保留场景化装饰。

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
- 不再保留页面级 `guide`、状态回显、分隔线和外部 `preview` 标签。
- 不做真实 overflow 弹层、响应式隐藏算法和多级菜单跳转。
- 不做完整桌面级 hover / pressed / focus ring 动画体系。
- 不接入真实图标资源、快捷键标签和额外徽标。

## 14. EGUI 适配时的简化点与约束
- 使用固定 snapshot 驱动 reference，优先保证 `480 x 480` 画布下的稳定审阅。
- 主控件保留 `eyebrow + title + scope + command rail + footer` 五段结构。
- `compact` 和 `disabled` preview 必须通过 `egui_view_command_bar_override_static_preview_api()` 固定为静态对照。
- preview 点击只负责清理主控件 focus，不承担状态切换、快照切换或新的交互职责。
- setter、模式切换、disabled guard、`touch cancel`、static preview touch/key 都必须共享同一套 `pressed` 清理语义。
