# wrap_panel 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`WrapPanel`
- 本次保留语义：`flow layout`、`current item`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_wrap_panel`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`wrap_panel` 用来表达“项目按流式顺序排布，遇到边界后自动换行”的标准布局语义。它适合标签集合、过滤项组、轻量操作卡片和状态 chips 这类需要保持顺序但又不能固定成等宽矩阵的场景。

## 2. 为什么现有控件不够用

- `uniform_grid` 强调固定列数和等尺寸单元，不适合长短不一的条目。
- `data_list_panel` 偏纵向列表，不表达流式换行。
- `settings_panel`、`settings_card` 偏设置页信息流，不适合做轻量标签面板。
- 当前仓库需要一个能完整走通 reference、单测、runtime 和 web 发布链路的 `WrapPanel` 页面。

## 3. 目标场景与示例概览

- 主控件保留真实 `WrapPanel` 语义，展示 `Operations queue`、`Release review`、`Density preview` 三组换行快照。
- 底部左侧是 `compact` 静态 preview，只用于对照窄尺寸下的换行密度。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化层级。
- 页面只保留标题、一个主 `wrap_panel` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_wrap_panel_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改写 `current_snapshot / current_item`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/wrap_panel/`

## 4. 视觉与布局规格

- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部对照行：`216 x 78`
- `compact` preview：`104 x 78`
- `read only` preview：`104 x 78`
- 页面结构：标题 -> 主 `wrap_panel` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和清晰的 `badge / title / meta` 层级。
  - 条目允许不同宽度，但行高统一，换行后保持稳定间距。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `wrap_primary` | `egui_view_wrap_panel_t` | `196 x 124` | `Operations queue` | 主 `WrapPanel` |
| `wrap_compact` | `egui_view_wrap_panel_t` | `104 x 78` | `Compact wrap` | 紧凑静态对照 |
| `wrap_read_only` | `egui_view_wrap_panel_t` | `104 x 78` | `Read only wrap` | 只读静态对照 |
| `primary_snapshots` | `egui_view_wrap_panel_snapshot_t[3]` | - | `Operations / Release / Density` | 主控件录制轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Operations queue` | 默认状态，展示多宽度条目的基础流式排布 |
| 主控件 | `Release review` | 展示第二组条目与不同宽度组合 |
| 主控件 | `Density preview` | 展示更密的换行排列 |
| `compact` | `Compact wrap` | 固定静态对照，验证窄尺寸排布 |
| `read only` | `Read only wrap` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口

- 主控件保留真实 `WrapPanel` 键盘与触摸语义：
  - `Left / Right`：在邻项间移动
  - `Up / Down`：按视觉行关系跳到最近项
  - `Home / End`：跳到首尾项
  - `Tab`：在当前 snapshot 内循环，必要时切到下一组 snapshot
  - `Enter / Space`：激活当前项并触发 listener
- 触摸交互保持 same-target release：只在同一项 `DOWN -> UP` 时提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Operations queue`。
2. 切到 `Release review`，输出第二组主状态。
3. 切到 `Density preview`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。

## 9. 编译、运行时、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/wrap_panel PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/wrap_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/wrap_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_wrap_panel
```

验收重点：
- 主控件三组 snapshot 必须能直接看出条目换行和当前项变化。
- `same-target release / key navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向

- 当前只收口单容器 `WrapPanel` reference，不实现真实子控件容器。
- 继续使用 snapshot 描述条目与换行布局，不下沉到 SDK 通用布局层。
- 若后续复用价值稳定，再评估是否抽象为更通用的 flow layout 基础控件。

## 11. 与现有控件的边界
- 相比 `uniform_grid`：这里强调不同宽度条目与自动换行，不是等宽矩阵。
- 相比 `data_list_panel`：这里强调流式排布，而不是单轴列表。
- 相比 `settings_panel`：这里负责轻量标签和条目流，不负责设置页信息结构。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `flow layout`
  - `current item`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作
