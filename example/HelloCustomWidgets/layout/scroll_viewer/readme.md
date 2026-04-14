# scroll_viewer 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`、`WPF UI`
- 对应组件名：`ScrollViewer`
- 本次保留语义：`surface / viewport / scrollbar`、`vertical / horizontal offset`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、`compact` 第二条 preview 轨道、录制里的 preview dismiss 收尾
- EGUI 适配说明：继续复用仓库内 `scroll_viewer` 基础实现，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`scroll_viewer` 用来表达“内容区域本身可滚动，而不是只摆一个滚动条”的标准语义。它适合长设置页、诊断摘要、富文本说明、卡片列表等内容超过单屏高度、但又不想绑定到特定数据容器的场景。

## 2. 为什么现有控件不够用
- `scroll_bar` 只覆盖滚动条本身，不承担 viewport、内容裁切与滚动 surface 的完整语义。
- `data_list_panel` 更偏向列表数据容器，不适合表达任意内容块的通用滚动区域。
- `split_view`、`card_panel` 这类控件能承载内容，但不直接提供 Fluent / WinUI `ScrollViewer` 的 reference 验证闭环。

## 3. 目标场景与示例概览
- 主控件：保留真实 `ScrollViewer` 语义，展示 `Release pane`、`Diagnostics lane`、`Backlog feed` 三组 `viewport / offset / scrollbar` 状态。
- `compact` 预览：压缩为小尺寸滚动容器，只作为静态 reference 对照。
- `read only` 预览：保留冻结态滚动容器和弱化 palette，只作为静态 reference 对照。
- 页面只保留标题、主 `scroll_viewer` 和底部 `compact / read only` 双 preview，不再保留 preview 焦点桥接与额外收尾动作。
- 底部两个 preview 统一通过 `egui_view_scroll_viewer_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只负责清理残留 `pressed / drag / track`
  - 不修改 `current_snapshot / vertical_offset / horizontal_offset / scrollbar_visibility`
  - 不触发 `on_view_changed`

目标目录：`example/HelloCustomWidgets/layout/scroll_viewer/`

## 4. 视觉与布局规格
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部对照行：`216 x 88`
- `compact` 预览：`104 x 88`
- `read only` 预览：`104 x 88`
- 页面结构：标题 -> 主 `scroll_viewer` -> `compact / read only`
- 样式约束：
  - 继续保持浅色 Fluent 卡片容器、低噪音边框和可读的 viewport 层级。
  - 保留顶部标题/摘要、中央滚动 surface、右侧 scrollbar 与底部横向指示条。
  - 主控件继续显示 snapshot 名称、offset 和 extent 等低噪音指标。
  - 底部两个 preview 固定为静态 reference 对照，不再承担点击桥接、焦点收尾或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `scroll_viewer_primary` | `egui_view_scroll_viewer_t` | `196 x 160` | `Release pane` | 主 `ScrollViewer` |
| `scroll_viewer_compact` | `egui_view_scroll_viewer_t` | `104 x 88` | `Compact pane` | 紧凑静态对照 |
| `scroll_viewer_read_only` | `egui_view_scroll_viewer_t` | `104 x 88` | `Read only pane` | 只读静态对照 |
| `primary_snapshots` | `egui_view_scroll_viewer_snapshot_t[3]` | - | `Release / Diagnostics / Backlog` | 主控件录制轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Release pane` | 默认状态，展示基础 viewport 与滚动指标 |
| 主控件 | `Diagnostics lane` | 展示第二组内容与滚动偏移 |
| 主控件 | `Backlog feed` | 展示紧凑内容堆叠与水平偏移 |
| `compact` | `Compact pane` | 固定静态对照，验证紧凑滚动容器 |
| `read only` | `Read only pane` | 固定静态对照，验证只读渲染与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件保留真实 `ScrollViewer` 键盘与触摸语义：
  - `Up / Down`：按行滚动
  - `Home / End`：跳到顶部 / 底部
  - `Left / Right`：调整水平 offset
  - `+ / -`：按页滚动
  - `Tab`：在 surface 与 thumb 之间移动
  - `Enter / Space`：thumb 聚焦时向下翻页
- 触摸语义继续区分两类：
  - 滚动轨道点击遵循 same-target release
  - thumb 拖拽按连续交互更新 vertical offset
- 脱靶移动时只允许保留提交目标，不允许继续显示错误的 pressed 视觉。
- `set_snapshots()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`set_scrollbar_visibility()` 都必须先清理残留 `pressed / drag / track`。
- 底部 `compact / read only` preview 固定为静态 reference 对照，不再承担焦点桥接或额外页面交互职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Release pane`。
2. 切到 `Diagnostics lane`，输出第二组主状态。
3. 切到 `Backlog feed`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担点击桥接、焦点 dismiss 或收尾职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/scroll_viewer PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_viewer --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_viewer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_viewer
```

验收重点：
- 主控件三组 snapshot 必须能直接看出内容区与 offset 变化。
- scrollbar / thumb / track 在脱靶移动后不能残留错误的 pressed 视觉。
- `same-target release / read only / !enable / static preview` 必须全部通过单测。
- 两个 preview 必须完整可见，不能裁切、黑屏或白屏，并且在所有 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向
- 当前只收口单容器 `ScrollViewer` reference，不接入真实虚拟化、大文档分块、嵌套惯性滚动或系统级滚轮桥接。
- 仍用 snapshot 数组描述内容块、viewport 和 offset，不下沉为 SDK 级通用滚动容器。
- 当前不下沉到 `src/widget/`，先维持在 `HelloCustomWidgets` 的 reference 维护范围内。

## 11. 与现有控件的边界
- 相比 `scroll_bar`：这里强调完整 viewport 与内容 surface，而不只是滚动条本体。
- 相比 `data_list_panel`：这里承载任意内容块，不绑定列表数据容器。
- 相比 `card_control`：这里强调滚动与 viewport，而不是卡片选择动作。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `surface / viewport / scrollbar`
  - `vertical / horizontal offset`
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸轨道点击与 thumb 拖拽
  - 键盘 `Tab / Up / Down / Left / Right / Home / End / + / - / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - `compact` 第二条 preview 轨道
  - 录制里的 preview dismiss 收尾动作
