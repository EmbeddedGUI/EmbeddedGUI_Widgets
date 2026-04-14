# menu_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`MenuBar`
- 本次保留状态：`standard`、`active menu`、`dropdown panel`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态文案、section label、可点击 preview 卡、重阴影、强描边、桌面系统级多级菜单叙事
- EGUI 适配说明：继续复用仓库内 `menu_bar` 基础实现，本轮只收口 `reference` 页面结构、示例命令内容和绘制强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`menu_bar` 用来表达“常驻顶层命令分类 + 当前菜单下拉面板”的标准桌面菜单栏语义，适合 `File / Edit / View / Tools` 这类结构稳定、分组明确的命令入口。

## 2. 为什么现有控件不够用
- `menu_flyout` 是局部弹出菜单，不承担常驻顶层菜单栏语义。
- `command_bar` 更接近工具栏，不强调顶层分类菜单与下拉层级。
- `nav_panel` 负责页面导航，不是命令菜单。
- `tab_strip` 负责页面切换，不是命令分组入口。

因此这里继续保留 `menu_bar`，但示例页回到统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主控件：展示标准 `MenuBar`，保留当前菜单高亮、下拉面板、危险项、子菜单入口和只读外的真实交互。
- `compact` 预览：保留相同菜单语义，但压缩为更小尺寸，用于验证小尺寸 reference 收口。
- `read only` 预览：保留菜单摘要和锁定态，只作为静态对照，不再承担点击或焦点循环职责。
- 底部两个 preview 统一走静态 preview API，吞掉 touch / key 输入，不再承担切换或清焦收尾职责。
- 页面只保留标题、主 `menu_bar` 和底部 `compact / read only` 双预览，不再保留 guide、状态栏、额外说明 chrome。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 108`
- 底部对照行尺寸：`216 x 74`
- `compact` 预览：`104 x 74`
- `read only` 预览：`104 x 74`
- 页面结构：标题 -> 主 `menu_bar` -> `compact / read only`
- 样式约束：
  - 保持浅底、白色菜单卡、轻边框、低噪音阴影的 Fluent 方向。
  - 顶层当前菜单只保留轻量填充和细 underline，不做厚重按钮化处理。
  - 下拉面板继续保留层级关系，但边框、separator、focus ring 和行内状态都要克制。
  - 底部两个 preview 固定为静态 reference 对照，不再参与页面交互闭环。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `menu_bar_primary` | `egui_view_menu_bar_t` | `196 x 108` | `File` 菜单展开 | 主 `MenuBar` |
| `menu_bar_compact` | `egui_view_menu_bar_t` | `104 x 74` | compact | 底部紧凑静态对照 |
| `menu_bar_read_only` | `egui_view_menu_bar_t` | `104 x 74` | read only | 底部只读静态对照 |
| `primary_snapshots` | `egui_view_menu_bar_snapshot_t[4]` | - | `File / Edit / View / Tools` | 主控件状态轨道 |
| `compact_snapshots` | `egui_view_menu_bar_snapshot_t[1]` | - | `Open / Compact` | 紧凑预览固定对照轨道 |
| `read_only_snapshots` | `egui_view_menu_bar_snapshot_t[1]` | - | `Pinned` 摘要 | 只读预览固定数据 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `File` | 默认菜单面板 | 保留 submenu 入口和 separator |
| 主控件 | `Edit` | 键盘与焦点迁移 | 验证当前项切换和选中行 |
| 主控件 | `View` | 紧凑/过滤类状态项 | 验证 `On` meta 与当前行高亮 |
| 主控件 | `Tools` | 危险命令 | 验证 danger 行仍保持低噪音 |
| `compact` | `Open / Compact` | 紧凑对照 | 固定 compact 对照，不参与状态切换 |
| `read only` | `Pinned` | 只读摘要 | 固定锁定态，对外禁用触摸和焦点 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 输出默认 `File` 截图。
3. 程序化切到主控件 `Edit`。
4. 输出 `Edit` 截图。
5. 程序化切到主控件 `View`。
6. 输出 `View` 截图。
7. 程序化切到主控件 `Tools`。
8. 输出 `Tools` 截图。
9. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担切换、桥接或收尾职责。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/menu_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_menu_bar
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见。
- 顶层菜单 underline、面板锚点、separator、当前行和危险项都必须可辨识，但不能变成高噪音装饰。
- `compact / read only` preview 必须统一吞掉 touch / key，不再参与焦点循环或菜单切换。
- `compact / read only` preview 在所有 runtime 帧里都必须保持静态一致。
- 切换到 `read only`、`disabled` 或 setter 更新时都要立即清空残留 `pressed_menu / pressed_item / is_pressed`。
- 触摸释放语义必须继续满足“按下与抬起命中同一目标才提交”。
- runtime 需要重点复核主区域 `File -> Edit -> View -> Tools -> File` 的变化，以及底部 `compact / read only` preview 的静态一致性。

## 9. 已知限制与后续方向
- 当前仍用固定 `snapshot` 驱动菜单与面板，不实现真实多级子菜单状态机。
- 命令激活只联动 `snapshot / current item` 变化，不承接真实业务回调。
- 文本宽度仍使用近似规则，不引入复杂测量与弹性布局求解。

## 10. 与现有控件的边界
- 相比 `menu_flyout`：这里是常驻顶层菜单栏，不是局部弹出菜单。
- 相比 `command_bar`：这里强调命令分类和下拉面板，不是工具栏按钮集合。
- 相比 `nav_panel`：这里表达命令结构，不承担页面导航。
- 相比 `tab_strip`：这里不是页面切换，而是命令入口组织。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`MenuBar`
- 本次保留核心状态：
  - `standard`
  - `active menu`
  - `dropdown panel`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰
- 删掉页面级 guide、状态说明和 section label。
- 删掉可点击 preview 卡、preview 焦点循环和 preview 清焦收尾职责。
- 删掉重阴影、厚描边、重型 focus ring 和强按钮化顶层菜单。
- 删掉桌面系统级菜单接管、复杂 nested submenu 动画和原生快捷键分发。

## 14. EGUI 适配时的简化点与约束
- 用 `snapshot` 数组驱动顶层菜单和下拉内容，优先保证 reference 稳定。
- 底部 `compact / read only` 固定放在同一行，只承担静态对照职责。
- `read only` 直接使用 `read_only_mode`，避免页面语义和控件实现脱节。
- 当前先作为 `HelloCustomWidgets` reference 示例维护，后续是否下沉框架层再单独评估。
