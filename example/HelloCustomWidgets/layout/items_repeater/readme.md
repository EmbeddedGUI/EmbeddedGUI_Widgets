# items_repeater 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ItemsRepeater`
- 对应组件名：`ItemsRepeater`
- 本次保留语义：`wrap / stack / strip preset`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_items_repeater`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`items_repeater` 用来表达“同一批数据按统一 item template 重复排布，但布局本身保持轻量、可嵌入、可按场景切换”的语义，适合最近项摘要、标签集合、轻量资源卡片和多状态条目带这类需要重复呈现同构内容、但又不希望直接上完整 `ListView` 或页面级列表控件的场景。

## 2. 为什么现有控件不够用

- `data_list_panel` 更偏带容器语义的 `ListView` 页面，不是轻量的 repeater 宿主。
- `wrap_panel`、`uniform_grid`、`virtualizing_wrap_panel` 更偏布局容器，不负责统一 item template、当前项和提交语义。
- `virtualizing_stack_panel` 解决的是纵向窗口化列表，不覆盖非虚拟化 repeater 的模板化重复布局。
- 当前仓库需要一个能完整走通 reference、单测、runtime 和 web 发布链路的 `ItemsRepeater` 页面。

## 3. 目标场景与示例概览

- 主控件保留真实 `ItemsRepeater` 语义，展示 `Wrap repeater`、`Stack repeater`、`Strip repeater` 三组主 snapshot。
- 主录制轨道只导出主控件的 layout preset 切换，不再让底部 preview 承担叙事职责。
- 底部左侧是 `compact` 静态 preview，只用于对照窄尺寸下的重复项密度。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化层级。
- 页面只保留标题、一个主 `items_repeater` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_items_repeater_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改写 `current_snapshot / current_item / layout_mode`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/items_repeater/`

## 4. 视觉与布局规格

- 根布局：`224 x 244`
- 主控件：`196 x 124`
- 底部对照行：`216 x 78`
- `compact` preview：`104 x 78`
- `read only` preview：`104 x 78`
- 页面结构：标题 -> 主 `items_repeater` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 `item / meta / tone` 层级。
  - 主区突出同一模板在 `wrap / stack / strip` 三种 preset 下的重复布局变化，不再叠加旧 preview 交互桥接。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `wrap_primary` | `egui_view_items_repeater_t` | `196 x 124` | `Wrap repeater` | 主 `ItemsRepeater` |
| `wrap_compact` | `egui_view_items_repeater_t` | `104 x 78` | `Compact wrap` | 紧凑静态对照 |
| `wrap_read_only` | `egui_view_items_repeater_t` | `104 x 78` | `Read only stack` | 只读静态对照 |
| `primary_snapshots` | `egui_view_items_repeater_snapshot_t[3]` | - | `Wrap / Stack / Strip` | 主控件语义轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Wrap repeater` | 默认状态，展示模板在 wrap preset 下的重复排布 |
| 主控件 | `Stack repeater` | 第二组 snapshot，展示同一模板切到纵向 lane |
| 主控件 | `Strip repeater` | 第三组 snapshot，展示紧凑 strip 复用同一模板 |
| `compact` | `Compact wrap` | 固定静态对照，验证窄尺寸下的重复项密度 |
| `read only` | `Read only stack` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口

- 主控件保留真实 `ItemsRepeater` 键盘与触摸语义：
  - `Left / Right / Up / Down`：按当前布局顺序在相邻项之间移动
  - `Home / End`：跳到首尾项
  - `Tab`：在当前 snapshot 内前进，必要时切到下一组 snapshot
  - `Enter / Space`：激活当前项并触发 listener
- 触摸交互保持 same-target release：只在同一项 `DOWN -> UP` 时提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_item()`、`set_layout_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计

`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Wrap repeater`。
2. 切到 `Stack repeater`，输出第二组主状态。
3. 切到 `Strip repeater`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主区切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、运行时、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/items_repeater PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_repeater --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/items_repeater
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_items_repeater
```

验收重点：
- 主控件必须能直接看出同一模板在 `wrap / stack / strip` 三种 preset 下的布局变化。
- `same-target release / keyboard navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向

- 当前只收口单容器 `ItemsRepeater` reference，不实现真实数据绑定管线和通用模板系统。
- 继续使用 snapshot 描述 item 集合与 layout preset，不下沉到 SDK 通用布局层。
- 若后续复用价值稳定，再评估是否抽象出与 `wrap_panel`、`uniform_grid` 或 `virtualizing_stack_panel` 共享的 metrics / layout helper。

## 11. 与现有控件的边界

- 相比 `wrap_panel`：这里强调模板化重复项与当前项语义，不只是流式布局。
- 相比 `uniform_grid`：这里允许不同宽度权重的 item 模板复用，不只强调规则网格。
- 相比 `data_list_panel`：这里是轻量 repeater，而不是页面级列表容器。

## 12. 本次保留的核心状态与删减项

- 保留的核心状态：
  - `wrap preset`
  - `stack preset`
  - `strip preset`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作

## 13. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/items_repeater PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `items_repeater` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/items_repeater --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_items_repeater/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/items_repeater`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_items_repeater`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.186 colors=322`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8]`、`[2,3]`、`[4,5]`
  - 主区变化边界收敛到 `(56, 96) - (419, 260)`
  - 按 `y >= 261` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Wrap repeater / Stack repeater / Strip repeater` 三组 reference 状态，最终稳定帧已显式回到默认快照
