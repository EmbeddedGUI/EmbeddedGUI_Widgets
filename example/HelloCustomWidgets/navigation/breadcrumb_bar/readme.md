# breadcrumb_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI BreadcrumbBar`
- 补充对照控件：`tree_view`、`tab_strip`
- 对应组件名：`BreadcrumbBar`
- 本次保留状态：`standard`、`current item`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态说明、旧双列 preview 包裹壳、preview 点击切换职责、过重 current item 和 separator 强调
- EGUI 适配说明：继续复用仓库内 `breadcrumb_bar` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和绘制强度，不修改 `sdk/EmbeddedGUI`；`snapshot / compact / read only / disabled` 切换共享同一套 `pressed` 清理语义，并把 `same-target release`、`static preview` 与 preview 点击后的 focus/渲染收尾纳入统一验收

## 1. 为什么需要这个控件

`breadcrumb_bar` 用来表达页面层级路径和当前位置，适合设置页、文档树、资源浏览和管理后台的顶部路径导航。相比标签页，它更强调“我现在处在路径的哪一层”，而不是“我在切哪个 section”。

## 2. 为什么现有控件不够用

- `tab_strip` 适合平级 section 切换，不负责层级路径表达。
- `menu_bar` 更偏命令入口，不适合持续显示当前位置。
- `tree_view` 强调层级浏览本体，不承担顶部路径摘要职责。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI 的轻量 `BreadcrumbBar`。

## 3. 目标场景与示例概览

- 主控件展示标准 `BreadcrumbBar`，覆盖 `Docs path`、`Forms path`、`Review path` 三组主 snapshot。
- 底部左侧展示 `compact` 静态对照，验证窄宽度下的路径折叠。
- 底部右侧展示 `read only` 静态对照，验证冻结交互后的弱化路径样式。
- 页面结构统一收口为：标题 -> 主 `breadcrumb_bar` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和点击轮换逻辑已删除，底部两个 preview 直接挂在同一行容器下。

目标目录：`example/HelloCustomWidgets/navigation/breadcrumb_bar/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 140`
- 主控件尺寸：`198 x 48`
- 底部对照行尺寸：`216 x 36`
- `compact` 预览：`104 x 36`
- `read only` 预览：`104 x 36`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白色路径条和低噪音浅边框。
  - 当前项保留轻量蓝色 pill，但降低填充、边框和底线强调。
  - separator 使用更弱的 chevron，对齐 Fluent 风格的轻分隔。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `bar_primary` | `egui_view_breadcrumb_bar_t` | `198 x 48` | `Docs path` | 主 `BreadcrumbBar` |
| `bar_compact` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Compact bills` | 底部 compact 静态对照 |
| `bar_read_only` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Read only` | 底部 read only 静态对照 |
| `primary_snapshots` | `egui_view_breadcrumb_bar_snapshot_t[3]` | - | `Docs / Forms / Review` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_breadcrumb_bar_snapshot_t[2]` | - | `Compact bills`、`Compact access` | compact 预览程序化切换 |
| `read_only_snapshots` | `egui_view_breadcrumb_bar_snapshot_t[1]` | - | `Static preview` | read only 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Docs path` | 默认主 snapshot，验证标准层级路径 |
| 主控件 | `Forms path` | 程序化切换第二组路径 |
| 主控件 | `Review path` | 程序化切换第三组路径 |
| `compact` | `Compact bills` | 默认 compact 对照 |
| `compact` | `Compact access` | 第二组 compact 折叠路径 |
| `read only` | `Static preview` | 固定只读轨道，禁 touch、禁 focus、禁键盘 |
| 交互语义 | `same-target release` | `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到 A 后 `UP(A)` 才提交 |
| 交互语义 | `static preview` | preview 吞掉 `touch / key`，只负责清残留 `pressed`，不改 snapshot、不触发 click |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 请求默认截图。
3. 程序化切换主控件到 `Forms path`。
4. 请求第二张截图。
5. 程序化切换主控件到 `Review path`。
6. 请求第三张截图。
7. 程序化切换 `compact` 到 `Compact access`，并重新请求主控件 focus。
8. 请求第四张截图。
9. 点击 `compact` preview，只做主控件 focus 收尾，不允许误改 snapshot。
10. 请求 preview 点击后的收尾帧。
11. 再请求最终稳定帧，确认没有残留 `pressed` 或整屏污染。

## 8. 编译、touch、runtime、单测与文档检查

```bash
make clean APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc
make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部 `compact / read only` 预览都必须完整可见，不能重新长出旧 preview 壳层。
- 当前项 pill、separator 和底部 underline 需要可辨识，但整体不能回到高噪音 showcase 风格。
- `compact` 在小尺寸下仍要看得出 `Home / ... / Current` 的层级关系。
- `read only` 只能做静态展示，不能响应 touch、focus 或键盘；切换到 `read only` 时还要清空 pressed 状态。
- `same-target release` 必须满足 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，`DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交。
- `static preview` 必须吞掉 `touch / key`，只做主控件 focus 收尾，不能误改 snapshot 或触发 click。
- `snapshot / compact / read only / disabled` 切换后不能残留 `pressed` 高亮或下压位移渲染。
- `read only / disabled` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染。
- runtime 关键帧至少要复核默认态、`Forms path`、`Review path`、`Compact access`、preview 点击收尾帧和最终稳定帧，确认没有黑白屏、裁切、整屏污染或残留 `pressed`。
- `HelloUnitTest` 里已有的 snapshot clamp、palette setter、touch / key click listener、`same-target release / cancel` 和 helper 语义不能回归。

## 9. 已知限制与后续方向

- 当前仍使用固定 snapshot 数据，不接真实页面导航。
- 当前不实现真实的 overflow 菜单，只保留省略号折叠表达。
- 当前优先验证 reference 语义、布局和绘制收口，不联动页面跳转。

## 10. 与现有控件的边界

- 相比 `tab_strip`：这里表达层级路径，不是平级 section 切换。
- 相比 `menu_bar`：这里用于当前位置说明，不是命令入口。
- 相比 `tree_view`：这里是顶部路径摘要，不是层级浏览主体。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和点击桥接逻辑。

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI BreadcrumbBar`

## 12. 对应组件名与本次保留的核心状态

- 对应组件名：`BreadcrumbBar`
- 本次保留核心状态：
  - `standard`
  - `current item`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰

- 删除页面级 `guide`、状态说明、preview 标签和旧的双列 preview 包裹结构。
- 删除 preview 参与点击轮换和焦点职责的页面桥接逻辑。
- 删除过重 current item、separator 和底部强调线。
- 删除与 reference 无关的说明性外壳和场景叙事。

## 14. EGUI 适配时的简化点与约束

- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- `read only` 直接使用 `read_only_mode`，避免页面语义和控件实现脱节。
- 通过程序化切换 snapshot 保证 runtime 能稳定抓到路径变化。
- preview 使用控件自己的 `static preview API`，不再依赖页面外部 `consume_preview_touch` 兜底。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
