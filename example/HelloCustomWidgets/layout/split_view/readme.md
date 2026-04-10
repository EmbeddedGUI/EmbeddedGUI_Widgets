# split_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`SplitView`
- 本次保留状态：`standard`、`compact`、`read only`、`pane open`、`pane compact`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、状态文案、`standard / section / preview` 标签、过重阴影和 showcase 外壳
- EGUI 适配说明：继续复用仓库内 `split_view` 基础实现，本轮只收口 `reference` 页面结构、交互语义和 runtime 收尾，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`split_view` 用来表达“侧栏导航 + 内容面板”的双栏结构，同时保留 pane 展开/收起和当前选择项的统一交互语义。它适合设置中心、资料库、审核工作区这类需要左侧 rail 和右侧 detail 同屏存在的场景。

## 2. 为什么现有控件不够用？
- `nav_panel` 只负责导航 rail，不承载 detail 面板。
- `data_list_panel` 更偏列表选择，不等同于 `SplitView` 的可收起 pane 结构。
- `master_detail` 更偏主从阅读，不等同于可折叠导航 pane。

因此这里继续保留 `split_view`，但示例页只保留符合 Fluent / WPF UI 主线的 `reference` 结构。

## 3. 目标场景与示例概览
- 主控件展示标准 `SplitView`：左侧 pane 保留 toggle、row selection 和 tone 区分，右侧 detail 保留标题、摘要和 footer。
- 底部左侧展示 `compact` 静态对照，用于验证小尺寸和收起 pane 下的渲染收口。
- 底部右侧展示 `read only` 静态对照，用于验证只读态弱化后的结构稳定性。
- 页面结构统一收口为：标题 -> 主 `split_view` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_split_view_override_static_preview_api()` 收口：
  - preview 吞掉 `touch / key` 输入；
  - preview 只负责清理残留 `pressed`；
  - preview 不会修改 `current_index / pane_expanded`，也不会触发 listener；
  - preview 点击只保留一个最小收尾逻辑：清主控件 focus。

目标目录：`example/HelloCustomWidgets/layout/split_view/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 104`
- 底部对照行尺寸：`216 x 74`
- `compact` preview：`104 x 74`
- `read only` preview：`104 x 74`
- 页面结构：标题 + 主控件 + 底部双 preview
- 样式约束：
  - 保持浅底、白色 pane 卡片、低噪音边框和轻量 detail 面板的 Fluent 分层。
  - `warning` 和 `neutral` 只通过 tone 微差表达，不回退到高噪音 showcase 视觉。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
  - `item / pane / compact / read only / !enable` 切换后都不能残留旧的 `pressed` 高亮。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Split View` | 页面标题 |
| `panel_primary` | `egui_view_split_view_t` | `196 x 104` | `Overview + pane open` | 主 `SplitView` |
| `panel_compact` | `egui_view_split_view_t` | `104 x 74` | `Overview + pane compact` | `compact` 静态对照 |
| `panel_read_only` | `egui_view_split_view_t` | `104 x 74` | `Members + read only` | `read only` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview + pane open` | 初始默认态 |
| 主控件 | `Overview + pane compact` | 验证 pane 收起 |
| 主控件 | `Review + warning` | 验证 warning tone |
| 主控件 | `Archive + neutral` | 作为最终稳定态 |
| `compact` | `Overview + pane compact` | 初始紧凑态 |
| `compact` | `Review + pane open` | runtime 第二组关键帧 |
| `read only` | `Members + read only` | 固定静态只读态 |

## 7. 交互与状态语义
- 主控件保留真实 row 选择、pane toggle 和键盘导航闭环。
- `set_items()`、`set_current_index()`、`set_pane_expanded()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都会先清理残留 `pressed`。
- `ACTION_MOVE` 会实时更新视觉下压状态：
  - 移出原目标时取消 `pressed` 渲染；
  - 回到原目标后恢复 `pressed` 渲染；
  - 只有释放时仍在原目标上才提交。
- `read_only_mode`、`!enable` 和空数据状态都会先清理残留 `pressed`，再拒绝新的 `touch / key` 输入。
- preview 不参与任何 selection 或 pane 切换，只承担 reference 对照与主控件 focus 收尾。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认状态。
2. 请求初始帧。
3. 切到 `Overview + pane compact`。
4. 请求第二帧。
5. 切到 `Review + warning`。
6. 请求第三帧。
7. 切到 `Archive + neutral`。
8. 请求第四帧。
9. 切换 `compact` 到 `Review + pane open`，并主动给主控件请求 focus。
10. 请求 `compact` 第二帧。
11. 点击 `compact` preview，只执行 preview 的静态收尾逻辑。
12. 请求 preview 点击后的收尾帧。
13. 再请求最终稳定帧，确认没有残留 `pressed / focus`、黑白屏、裁切或整屏污染。

## 9. 编译、touch、runtime、单测与文档检查
```bash
make clean APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc
make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- pane、detail、正文和 footer 的层级必须稳定，不能因为交互切换出现整屏污染。
- 主控件必须通过 same-target release / cancel 回归：移出命中区后不提交，回到原目标后释放才提交。
- `set_items / set_current_index / set_pane_expanded / set_font / set_meta_font / set_palette / set_compact_mode / set_read_only_mode` 后都不能残留 `pressed`。
- static preview 的 `touch / key` 都不能改动 `current_index / pane_expanded`，也不能触发 listener。
- runtime 需要重点复核初始帧、pane compact 帧、warning 帧、preview 点击后的收尾帧和最终稳定帧。

## 11. 已知限制与后续方向
- 当前仍是固定尺寸 `reference` 示例，不覆盖更长列表和更复杂的多级 pane。
- 当前不做真实导航容器嵌套、hover 和桌面系统级焦点辅助文案。
- detail 内容仍是静态快照，不接入真实数据源。

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里包含 detail 面板，不只是导航 rail。
- 相比 `data_list_panel`：这里强调 pane 展开/收起，而不只是列表选择。
- 相比 `master_detail`：这里更接近 `SplitView` 的可折叠导航语义。

## 13. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 14. 对应组件名与本次保留的核心状态
- 对应组件名：`SplitView`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `pane open`
  - `pane compact`
  - `warning`
  - `neutral`

## 15. 相比参考原型删除的效果与 EGUI 适配约束
- 删除页面级 `guide`、状态说明、preview 标签和 showcase 包装。
- 删除 preview 参与页面切换和额外交互桥接的职责，只保留静态对照与主控件 focus 收尾。
- 删除过重阴影、复杂容器装饰和多级联动叙事。
- 通过统一的 `sv_clear_pressed_state()` 和 static preview API，把 setter、guard、preview 的状态清理收口到同一套语义。
