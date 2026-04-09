# tab_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`TabView`
- 本次保留状态：`standard`、`close current tab`、`restore closed tabs`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态栏、section label、额外 workspace/helper 说明条、可点击 preview 卡、重阴影、强按钮化 tab shell
- EGUI 适配说明：继续复用仓库内 `tab_view` 基础实现，本轮只收口 `reference` 页面结构、示例文案和绘制强度，不修改 `sdk/EmbeddedGUI`；`snapshot / current tab / current part / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件
`tab_view` 用来表达“页签头和内容面板是一体的工作区容器”，适合文档页、后台工作区、设置页中的多面板切换场景。

## 2. 为什么现有控件不够用
- `tab_strip` 只表达页签头，不承载内容面板。
- `flip_view` 是顺序浏览，不是并列工作区页签。
- `nav_panel` 负责导航，不表达标签式工作区。
- `menu_bar` 和 `command_bar` 负责命令组织，不负责页签内容容器。

因此这里继续保留 `tab_view`，但示例页需要回到统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主控件：展示标准 `TabView`，保留 tab header、content body、关闭当前页签和恢复隐藏页签入口。
- `compact` 预览：保留相同语义，但压缩 body chrome 和文本密度，用于验证小尺寸 reference 收口。
- `read only` 预览：保留冻结态和内容摘要，只作为静态对照，不再承担点击职责。
- 页面只保留标题、主 `tab_view` 和底部 `compact / read only` 双预览，不再保留 guide、状态栏和额外说明条。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`198 x 112`
- 底部对照行尺寸：`216 x 72`
- `compact` 预览：`104 x 72`
- `read only` 预览：`104 x 72`
- 页面结构：标题 -> 主 `tab_view` -> `compact / read only`
- 样式约束：
  - 维持浅底、轻边框、低噪音 tab shell 与 body card。
  - active tab 只保留轻量 fill、细 underline 和 close 入口，不做厚重按钮化。
  - body 面板保留 badge / title / footer 的层级，但需要压轻描边和胶囊强调。
  - 底部两个 preview 固定为静态 reference 对照，不再参与交互闭环。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `tab_view_primary` | `egui_view_tab_view_t` | `198 x 112` | `Docs workspace / Home` | 主 `TabView` |
| `tab_view_compact` | `egui_view_tab_view_t` | `104 x 72` | compact | 底部紧凑静态对照 |
| `tab_view_read_only` | `egui_view_tab_view_t` | `104 x 72` | read only | 底部只读静态对照 |
| `primary_snapshots` | `egui_view_tab_view_snapshot_t[2]` | - | `Docs / Ops` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_tab_view_snapshot_t[2]` | - | `Compact docs / Compact ops` | 紧凑预览程序化切换轨道 |
| `read_only_snapshots` | `egui_view_tab_view_snapshot_t[1]` | - | `Read only` | 只读预览固定数据 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `Docs workspace` | 默认工作区 | 保留 active tab、body、footer |
| 主控件 | `Docs / Publish` | 切换当前页签 | 验证 body 和 footer 同步变化 |
| 主控件 | `Docs / close current` | 关闭页签 | 验证 header 可见数量收缩 |
| 主控件 | `Docs / restore` | 恢复页签 | 验证 `+` 恢复已隐藏页签 |
| 主控件 | `Ops workspace` | 第二条工作区轨道 | 验证 snapshot 切换后内容壳仍稳定 |
| `compact` | `Compact docs` | 紧凑对照 | 验证小尺寸 shell 与 body 收口 |
| `compact` | `Compact ops` | 第二条预览轨道 | 只做程序化切换，不参与交互 |
| `read only` | `Read only` | 冻结态摘要 | 固定只读，对外禁用触摸和焦点 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认 snapshot，并输出首帧。
2. 切到主控件 `Publish` 页签。
3. 关闭当前页签，记录可见 tab 数量变化。
4. 通过 `+` 恢复已关闭页签。
5. 切到 `Ops workspace` snapshot。
6. 程序化切换 `compact` 到第二条预览轨道。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- header、body、footer 和底部 `compact / read only` 预览都必须完整可见。
- active tab、close、add、body card 和 footer pill 必须可辨识，但不能回到旧的重按钮化风格。
- `compact / read only` 不再响应触摸，也不承担页面状态切换职责。
- `snapshot / current tab / current part / compact / read only / view disabled` 切换后不能残留 tab、close 或 add 的 `pressed` 高亮与下压渲染。
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时先清理残留 `pressed` 状态。
- `close current tab` 与 `restore closed tabs` 的行为不能回归。

## 9. 已知限制与后续方向
- 当前不做拖拽重排、tear-out、多行 header command 区和窗口级集成。
- 关闭页签继续使用本地 `closed_mask`，只在当前 snapshot 内生效。
- 文本宽度仍使用轻量估算，不引入复杂字体测量和弹性布局。

## 10. 与现有控件的边界
- 相比 `tab_strip`：这里把 content body 视为控件语义的一部分。
- 相比 `flip_view`：这里是并列页签，不是前后翻页。
- 相比 `nav_panel`：这里表达工作区页签，不是导航结构。
- 相比 `menu_bar`：这里是内容容器，不是命令入口。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`TabView`
- 本次保留核心状态：
  - `standard`
  - `close current tab`
  - `restore closed tabs`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰
- 删掉页面级 guide、状态栏、section label 与额外 workspace/helper 说明条。
- 删掉可点击 preview 卡与 preview 轨道的外部交互职责。
- 删掉拖拽重排、tear-out、窗口级集成和复杂 hover 动画。
- 删掉重阴影、厚描边和高噪音 tab shell/body 装饰。

## 14. EGUI 适配时的简化点与约束
- 使用 `snapshot + closed_mask` 驱动工作区状态，优先保证 reference 稳定。
- 底部 `compact / read only` 固定放在同一行，只承担静态对照职责。
- `read only` 继续复用 `read_only_mode`，但页面语义统一表述为 `read only`。
- 当前先作为 `HelloCustomWidgets` reference 示例维护，后续是否下沉框架层再单独评估。
