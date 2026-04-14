# annotated_scroll_bar 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AnnotatedScrollBar`
- 补充对照控件：`scroll_bar`、`pips_pager`
- 对应组件名：`AnnotatedScrollBar`
- 本次保留状态：`standard`、`keyboard step`、`marker jump`、`rail drag`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态栏、外部 preview 标签、第二条 `compact` 预览轨道、preview 点击桥接、过重 bubble 装饰和过强 rail chrome
- EGUI 适配说明：继续复用仓库内 `annotated_scroll_bar` 基础实现，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`annotated_scroll_bar` 用来表达长内容中的语义分段导航。它适合年表、版本里程碑、审计记录、归档列表这类“需要按 section 快速跳转，但又不是分页切换”的界面。

## 2. 为什么现有控件不够用
- `scroll_bar` 只表达比例和当前位置，不表达 marker 与注释语义。
- `pips_pager` 是离散页切换，不适合连续长列表定位。
- `breadcrumb_bar` 负责路径层级，不负责纵向 rail 导航。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `AnnotatedScrollBar` 语义的 custom widget。

## 3. 目标场景与示例概览
- 主控件：保留真实 `AnnotatedScrollBar` 语义，展示 `Gallery rail`、`Release rail`、`Incident rail` 三组主 snapshot。
- `compact` 预览：压缩为小尺寸摘要 + rail，只作为静态 reference 对照。
- `read only` 预览：保留冻结态 rail 与 marker 结构，只作为静态 reference 对照。
- 页面只保留标题、主 `annotated_scroll_bar` 和底部 `compact / read only` 双 preview，不再保留 guide、状态栏、外部 preview 标签和额外页面 chrome。
- 底部两个 preview 统一通过 `egui_view_annotated_scroll_bar_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只负责清理残留 `pressed`
  - 不修改 `offset / current_part / active_marker / compact_mode / read_only_mode`
  - 不触发 `on_changed`

目标目录：`example/HelloCustomWidgets/navigation/annotated_scroll_bar/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 260`
- 主控件尺寸：`196 x 150`
- 底部对照行尺寸：`216 x 68`
- `compact` 预览尺寸：`104 x 68`
- `read only` 预览尺寸：`104 x 68`
- 页面结构：标题 -> 主 `annotated_scroll_bar` -> `compact / read only`
- 样式约束：
  - 使用浅灰 page panel、白色主卡和低噪音浅边框。
  - 主控件继续保留 title、helper、summary、bubble、marker label 和 rail 语义，但统一压轻边框、填充和 focus。
  - 底部两个 preview 直接依赖控件模式区分，不再依赖外部标签、包裹卡片或切换桥接。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `annotated_scroll_bar_primary` | `egui_view_annotated_scroll_bar_t` | `196 x 150` | `Gallery rail` | 主 `AnnotatedScrollBar` |
| `annotated_scroll_bar_compact` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Compact / Focus` | 紧凑静态对照 |
| `annotated_scroll_bar_read_only` | `egui_view_annotated_scroll_bar_t` | `104 x 68` | `Read only / Ship` | 只读静态对照 |
| `primary_snapshots` | `annotated_scroll_bar_snapshot_t[3]` | - | `Gallery / Release / Incident` | 主控件录制轨道 |
| `compact_snapshot` | `annotated_scroll_bar_snapshot_t` | - | `Compact / Focus` | `compact` 固定对照数据 |
| `read_only_snapshot` | `annotated_scroll_bar_snapshot_t` | - | `Static preview` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Gallery rail` | 默认主 snapshot，保留完整 title / helper / bubble / marker label |
| 主控件 | `Down` | 验证小步进和 active marker 更新 |
| 主控件 | `+` | 验证大步进跳转 |
| 主控件 | `End` | 验证尾部定位和 rail 终点表现 |
| 主控件 | `Release rail` | 程序化切到下一组主 snapshot，验证 summary / bubble / rail 同步 |
| `compact` | `Compact / Focus` | 固定静态对照，验证紧凑 rail 收口 |
| `read only` | `Static preview` | 固定静态对照，验证只读渲染与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件继续保留真实 `touch / key` 交互，并遵守 non-dragging 控件的 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_markers()`、`set_content_metrics()`、`set_step_size()`、`set_offset()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()`、`!enable` 都必须先清理残留 `pressed`。
- `compact`、`read_only` 和 `!enable` 不仅要忽略后续输入，还要在收到新输入时先清理残留 `pressed`。
- 底部 `compact / read only` preview 固定为静态 reference 对照，不再清主控件 focus，也不再承担轨道切换和额外页面交互职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Gallery rail`。
2. 对主控件发送 `Down`，输出小步进后的主状态。
3. 对主控件发送 `+`，输出大步进后的主状态。
4. 对主控件发送 `End`，输出尾部定位后的主状态。
5. 程序化切到下一组主 snapshot，输出 `Release rail`。
6. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担点击桥接、轨道切换或收尾职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/annotated_scroll_bar PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/annotated_scroll_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_annotated_scroll_bar
```

验收重点：
- 不能黑屏、白屏、主控件缺失或 preview 裁切。
- summary、bubble、marker label、indicator、顶部标题和底部 `compact / read only` preview 必须完整可见。
- active marker、rail indicator 和按钮状态必须可辨识，但不能回退到旧的高噪音重装饰风格。
- `same-target release / cancel / compact / read_only / !enable / static preview` 必须全部通过单测。
- 底部 `compact / read only` preview 在所有 runtime 帧里都必须保持静态一致。
- runtime 需要重点复核主区域 `Gallery rail -> Down -> + -> End -> Release rail -> Gallery rail` 的变化，以及底部 preview 的静态一致性。

## 10. 已知限制与后续方向
- 当前仍使用固定 snapshot 数据，不接真实滚动容器。
- 当前不实现桌面端 hover tooltip 和复杂 reveal 动画。
- marker label 仍采用轻量避让规则，不做复杂碰撞排版。
- 当前优先验证 reference 语义、布局和交互闭环，不联动外部内容区。

## 11. 与现有控件的边界
- 相比 `scroll_bar`：这里强调 marker 与注释，不强调 viewport 比例。
- 相比 `pips_pager`：这里是连续长列表定位，不是离散分页。
- 相比 `breadcrumb_bar`：这里是纵向 rail 导航，不是路径层级。
- 相比旧版 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部叙事壳层。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `standard`
  - `keyboard step`
  - `marker jump`
  - `rail drag`
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸 `decrease / rail / increase / marker`
  - 键盘 `Tab / Up / Down / Home / End / - / + / Enter / Space / Escape`
- 删除的装饰或桥接：
  - 页面级 `guide`、状态栏、外部 preview 标签和旧的双列 preview 包裹结构
  - preview 点击桥接和第二条 `compact` 预览轨道
  - 过重的 bubble、connector、focus ring 和 rail 装饰强度
  - 与 reference 无关的外部说明壳层和场景化叙事

## 13. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + marker` 数据保证录制稳定。
- 用控件内部 summary 与 bubble 承载当前 section 和 offset，不依赖外部状态栏。
- 底部 `compact / read only` 放在同一行，但只承担静态 reference 对照职责。
- preview 的输入职责收口到控件自己的 static preview API，页面层不再追加 touch / key 桥接逻辑。
