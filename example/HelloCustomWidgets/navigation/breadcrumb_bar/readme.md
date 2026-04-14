# breadcrumb_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI BreadcrumbBar`
- 补充对照控件：`tree_view`、`tab_strip`
- 对应组件名：`BreadcrumbBar`
- 当前保留状态：`standard`、`current item`、`compact`、`read only`
- 当前删除内容：compact preview 第二轨道、preview 点击清 focus 的桥接动作，以及与路径摘要语义无关的额外交互录制
- EGUI 适配说明：继续复用仓库内 `breadcrumb_bar` 基础实现，本轮只收口 `reference` 页面结构、静态 preview 语义与录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`breadcrumb_bar` 用来表达页面层级路径和当前位置，适合设置页、文档树、资源浏览和管理后台顶部路径导航。相比标签页，它更强调“我现在处在路径的哪一层”，而不是“我在切哪个 section”。

## 2. 为什么现有控件不够用
- `tab_strip` 适合平级 section 切换，不负责层级路径表达。
- `menu_bar` 更偏命令入口，不适合持续显示当前位置。
- `tree_view` 强调层级浏览本体，不承担顶部路径摘要职责。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `BreadcrumbBar` 语义的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `BreadcrumbBar`，录制轨道覆盖 `Docs path`、`Forms path`、`Review path` 三组主状态。
- 底部左侧展示 `compact` 静态 preview，验证窄宽度下的路径折叠。
- 底部右侧展示 `read only` 静态 preview，验证冻结交互后的弱化路径样式。
- 页面结构固定为：标题 -> 主 `breadcrumb_bar` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_breadcrumb_bar_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/navigation/breadcrumb_bar/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 140`
- 主控件尺寸：`198 x 48`
- 底部对照行尺寸：`216 x 36`
- `compact` preview：`104 x 36`
- `read only` preview：`104 x 36`

视觉约束：
- 使用浅灰 `page panel`、白色路径条和低噪音浅边框。
- 当前项保留轻量蓝色 pill，但降低填充、边框和底线强调。
- separator 使用更弱的 chevron，对齐 Fluent 风格的轻分隔。
- `read only` 除了弱化 palette，还要作为真正静态 preview，不再承接录制桥接逻辑。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 140` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Breadcrumb Bar` | 页面标题 |
| `bar_primary` | `egui_view_breadcrumb_bar_t` | `198 x 48` | `Docs path` | 主 `BreadcrumbBar` |
| `bar_compact` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Compact bills / static` | 紧凑静态对照 |
| `bar_read_only` | `egui_view_breadcrumb_bar_t` | `104 x 36` | `Read only / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Docs path` | 默认主 snapshot，验证标准层级路径 |
| 主控件 | `Forms path` | 程序化切换第二组路径 |
| 主控件 | `Review path` | 程序化切换第三组路径 |
| `compact` | `Compact bills` | 紧凑静态 preview |
| `read only` | `Read only` | 只读静态 preview，禁用交互并弱化 palette |

## 7. 交互语义要求
- 主控件继续保留真实 touch / key 闭环，并遵守 same-target release。
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 必须先清理残留 `pressed` 再刷新。
- `set_snapshots(NULL, 0)` 必须把 `snapshot_count` 收口到 `0`，并同步清理 `pressed`。
- `read only / disabled` 的 `touch / key guard` 必须在拒绝输入前清理残留 `pressed`。
- 静态 preview 必须吞掉 `touch / key`，不能修改 `current_snapshot`，也不能触发 click。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/navigation/breadcrumb_bar/test.c`
- 调整底部 `compact` preview 为单一静态轨道，删除 compact preview 第二组状态
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件三组 snapshot 切换与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定 snapshot”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `Docs path`，并同步两个静态 preview 状态。
2. 请求默认截图。
3. 切到 `Forms path`。
4. 请求第二张截图。
5. 切到 `Review path`。
6. 请求第三张截图。
7. 恢复默认 `Docs path` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、交互、runtime 与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_bar PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_breadcrumb_bar
```

验收重点：
- 主控件三张关键截图必须能清晰区分 `Docs path`、`Forms path`、`Review path`。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或轨道切换产生额外变化。
- 页面不能出现黑白屏、裁切、pressed 残留或底部 preview 脏态。
- preview 不响应触摸或键盘输入，也不改变 `current_snapshot`。

## 11. 已知限制
- 当前仍使用固定 snapshot 数据，不接真实页面导航。
- 当前不实现真实的 overflow 菜单，只保留省略号折叠表达。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动页面跳转。
