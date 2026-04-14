# tab_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`TabView`
- 本次保留状态：`standard`、`close current tab`、`restore closed tabs`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态栏、section label、额外 workspace/helper 文案、preview 点击桥接、第二条 `compact` 预览轨道、重阴影和强按钮化 tab shell
- EGUI 适配说明：继续复用仓库内 `tab_view` 基础实现，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`tab_view` 用来表达“页签头和内容面板是一体化工作区”的标准多标签容器语义，适合文档页、后台工作台、设置页和运维面板这类需要在同一个内容壳内切换多个任务页签的场景。

## 2. 为什么现有控件不够用
- `tab_strip` 只表达页签头，不承载 body 内容面板。
- `flip_view` 是顺序翻页，不是并列工作区页签。
- `nav_panel` 负责导航结构，不负责标签式内容容器。
- `menu_bar` 和 `command_bar` 负责命令组织，不承担工作区内容壳。

因此这里继续保留 `tab_view`，但示例页只保留统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主控件：保留真实 `TabView` 语义，展示页签切换、关闭当前页签和恢复隐藏页签。
- `compact` 预览：保留同一控件语义，但压缩 body chrome 和文本密度，只作为静态 reference 对照。
- `read only` 预览：保留冻结态页签壳和摘要内容，只作为静态 reference 对照。
- 页面只保留标题、主 `tab_view` 和底部 `compact / read only` 双 preview，不再保留 guide、状态栏、section label 和额外说明条。
- 底部两个 preview 统一通过 `egui_view_tab_view_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只负责清理残留 `pressed`
  - 不修改 `current_snapshot / current_tab / current_part / closed_mask / compact_mode`
  - 不触发 `on_changed / on_action`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`198 x 112`
- 底部对照行尺寸：`216 x 72`
- `compact` 预览尺寸：`104 x 72`
- `read only` 预览尺寸：`104 x 72`
- 页面结构：标题 -> 主 `tab_view` -> `compact / read only`
- 样式约束：
  - 保持浅底、轻边框、低噪音 tab shell 与 body card。
  - active tab 只保留轻量 fill、细 underline 和 close 入口，不回退到厚重按钮化风格。
  - body 面板继续保留 badge、title、footer 的层级，但压轻描边和胶囊强调。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接、轨道切换或录制收尾职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `tab_view_primary` | `egui_view_tab_view_t` | `198 x 112` | `Docs workspace / Home` | 主 `TabView` |
| `tab_view_compact` | `egui_view_tab_view_t` | `104 x 72` | `Compact docs` | 紧凑静态对照 |
| `tab_view_read_only` | `egui_view_tab_view_t` | `104 x 72` | `Read only` | 只读静态对照 |
| `primary_snapshots` | `egui_view_tab_view_snapshot_t[2]` | - | `Docs workspace / Ops workspace` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_tab_view_snapshot_t[1]` | - | `Compact docs` | `compact` 固定对照数据 |
| `read_only_snapshots` | `egui_view_tab_view_snapshot_t[1]` | - | `Read only` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `Docs workspace / Home` | 默认工作区 | 保留 active tab、body、footer |
| 主控件 | `Docs workspace / Publish` | 页签切换 | 验证 body 和 footer 同步变化 |
| 主控件 | `Docs workspace / close current` | 关闭当前页签 | 验证 header 可见数量收缩 |
| 主控件 | `Docs workspace / restore` | 恢复已关闭页签 | 验证 `+` 恢复闭合页签 |
| 主控件 | `Ops workspace` | 第二工作区轨道 | 验证 snapshot 切换后内容壳保持稳定 |
| `compact` | `Compact docs` | 小尺寸对照 | 固定静态对照，验证紧凑壳体收口 |
| `read only` | `Read only` | 冻结态对照 | 固定静态对照，验证只读渲染与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件继续保留真实 `touch / key` 交互，并遵守 non-dragging 控件的 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_snapshots()`、`set_current_snapshot()`、`set_current_tab()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`!enable` 都必须先清理残留 `pressed`。
- `read_only` 和 `!enable` 不仅要忽略后续输入，还要在收到新输入时先清理残留 `pressed`。
- 底部 `compact / read only` preview 统一固定为静态 reference 对照，不再清主控件 focus，也不再承担轨道切换和额外页面交互职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Docs workspace / Home`。
2. 切到主控件 `Publish` 页签，输出页签切换后的主状态。
3. 关闭当前页签，输出 header 收缩后的主状态。
4. 恢复已关闭页签，输出恢复后的主状态。
5. 切到 `Ops workspace`，输出第二工作区轨道。
6. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担点击桥接、轨道切换或收尾职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tab_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tab_view
```

验收重点：
- 不能黑屏、白屏、主控件缺失或 preview 裁切。
- header、body、footer 和底部 `compact / read only` preview 必须完整可见。
- active tab、close、add、body card 和 footer pill 必须可辨识，但不能回退到旧的高噪音重按钮化风格。
- `same-target release / cancel / read_only / !enable / static preview` 必须全部通过单测。
- 底部 `compact / read only` preview 在所有 runtime 帧里都必须保持静态一致。
- runtime 需要重点复核主区域 `Home -> Publish -> close current -> restore -> Ops workspace -> Home` 的变化，以及底部 preview 的静态一致性。

## 10. 已知限制与后续方向
- 当前不做拖拽重排、tear-out、多行 header command 区和窗口级集成。
- 关闭页签继续使用本地 `closed_mask`，只在当前 snapshot 内生效。
- 文本宽度仍使用轻量估算，不引入复杂字体测量和弹性布局。

## 11. 与现有控件的边界
- 相比 `tab_strip`：这里把 body 内容视为控件语义的一部分。
- 相比 `flip_view`：这里是并列页签，不是顺序翻页。
- 相比 `nav_panel`：这里表达工作区页签，不是导航结构。
- 相比 `menu_bar`：这里是内容容器，不是命令入口。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `standard`
  - `close current tab`
  - `restore closed tabs`
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸 `tab / close / add`
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space / Escape`
- 删除的装饰或桥接：
  - 页面级 guide、状态栏、section label 和额外 workspace/helper 说明条
  - preview 点击桥接和第二条 `compact` 预览轨道
  - 重阴影、厚描边和过度按钮化 tab shell
  - 拖拽重排、tear-out 和复杂 hover 动画

## 13. EGUI 适配时的简化点与约束
- 使用 `snapshot + closed_mask` 驱动工作区状态，优先保证 reference 稳定。
- 底部 `compact / read only` 继续放在同一行，但只承担静态 reference 对照职责。
- `read only` 继续复用 `read_only_mode`，页面语义统一表述为 `read only`。
- preview 的输入职责收口到控件自己的 static preview API，页面层不再追加焦点清理或点击桥接逻辑。
