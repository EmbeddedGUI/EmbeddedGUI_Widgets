# pips_pager 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI PipsPager`
- 补充对照控件：`flip_view`、`tab_strip`
- 对应组件名：`PipsPager`
- 本次保留状态：`standard`、`current page`、`previous / next`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、旧 preview 标签和双列包裹、让 preview 继续承担切换职责的桥接逻辑、过强的 pressed / focus 装饰
- EGUI 适配说明：继续复用仓库内 `pips_pager` 基础实现，本轮重点收口 `reference` 页面结构、static preview 语义、same-target release 触控规则和 pressed 清理，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`pips_pager` 用来表达离散页码切换、向前 / 向后翻页和当前页位置反馈。它适合 onboarding、轻量轮播、短流程向导这类“页面数量有限，但当前位置必须一眼可见”的场景。

## 2. 为什么现有控件不够用
- `tab_strip` 更偏向 section 切换，不强调页码位置。
- `flip_view` 强调内容卡片翻页，不以 pips rail 本体为中心。
- `scroll_bar` 是连续范围值，不适合离散页码。
- 当前 reference 主线仍然需要一版贴近 Fluent / WPF UI 语义的 `PipsPager`。

## 3. 目标场景与示例概览
- 主控件展示标准 `PipsPager`，覆盖 `Onboarding`、`Gallery`、`Report deck` 三组 snapshot。
- 底部左侧保留 `compact` static preview，对照小尺寸 rail 的渲染结果。
- 底部右侧保留 `read only` static preview，对照冻结交互后的弱化状态。
- 页面结构统一收口为：标题 -> 主 `pips_pager` -> `compact / read only`。
- preview 只负责静态对照和最小收尾：点击 preview 只清主控件 focus，不再承担页面切换职责。

目标目录：`example/HelloCustomWidgets/navigation/pips_pager/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 198`
- 主控件尺寸：`196 x 92`
- 底部对照行尺寸：`216 x 58`
- `compact` preview：`104 x 58`
- `read only` preview：`104 x 58`
- 页面结构：标题 + 主控件 + 底部双 preview
- 样式约束：
  - 使用浅灰 page panel、白色主卡片和低噪音浅边框。
  - previous / next 按钮、当前页 pill、inactive dots 需要可辨识，但整体不能回到高噪音 showcase 风格。
  - `compact` 保留短 rail 语义，不再依赖外部说明标签。
  - `read only` 使用弱化 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `pager_primary` | `egui_view_pips_pager_t` | `196 x 92` | `Onboarding / 2 of 7` | 主 `PipsPager` |
| `pager_compact` | `egui_view_pips_pager_t` | `104 x 58` | `Compact / 3 of 6` | 底部 compact static preview |
| `pager_read_only` | `egui_view_pips_pager_t` | `104 x 58` | `Read only / 4 of 7` | 底部 read only static preview |
| `primary_snapshots` | `pager_snapshot_t[3]` | - | `Onboarding / Gallery / Report deck` | 主控件录制轨道 |
| `compact_snapshots` | `pager_snapshot_t[2]` | - | `Compact / 3 of 6`、`Compact / 6 of 8` | compact 对照轨道 |
| `read_only_snapshot` | `pager_snapshot_t` | - | `Read only / 4 of 7` | 固定只读对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Onboarding` | 默认 snapshot，验证标准分页层级 |
| 主控件 | `Right` | 键盘推进一页 |
| 主控件 | `End` | 验证尾页边界 |
| 主控件 | `Gallery` | 程序化切换主 snapshot，验证长页数和短窗口 |
| `compact` | `Compact / 3 of 6` | 默认 compact 对照 |
| `compact` | `Compact / 6 of 8` | 第二组 compact 对照 |
| `read only` | `Static preview` | 固定只读轨道，禁 touch、禁 key、禁 focus 交互 |

- 主控件继续保留真实 touch / key 闭环，并补齐 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交。
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交。
  - `PIP` 还要求 `pressed_index` 与释放目标一致。
- `set_font()`、`set_meta_font()`、`set_title()`、`set_helper()`、`set_palette()`、`set_page_metrics()`、`set_current_index()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()` 都会先清理残留 `pressed_part / pressed_index / is_pressed`。
- `compact_mode`、`read_only_mode`、`!enable` 收到新的 touch / key 输入时，会先清掉残留 pressed，再拒绝提交。
- 底部 preview 统一通过 `egui_view_pips_pager_override_static_preview_api()` 收口：
  - 吞掉 touch / key 输入。
  - 只清残留 pressed，不改 `current_index / current_part`。
  - 不触发 `on_changed`。
  - demo 侧只额外挂一个最小的 preview 点击收尾：清主控件 focus。

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 输出默认截图。
3. 对主控件发送 `Right`，验证向后翻页。
4. 输出第二张截图。
5. 对主控件发送 `End`，验证尾页状态。
6. 输出第三张截图。
7. 程序化切换主控件到下一组 snapshot。
8. 输出第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot，并重新请求主控件 focus。
10. 输出 `compact` 切换后的截图。
11. 点击 `compact` preview，验证 static preview 只做焦点收尾。
12. 输出 preview 点击后的收尾帧。
13. 再输出最终稳定帧，确认没有残留 pressed 或整屏污染。

## 8. 编译、单测、touch、runtime 与文档检查
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots

python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能出现黑白屏、裁切或旧 preview 壳层回流。
- previous / next、当前页 pill、inactive dots 在主控件和 compact 对照里都要可辨识。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能误提交，只有回到 `A` 才能提交。
- `ACTION_CANCEL` 只能清理 pressed，不能误改页码或误通知监听器。
- `compact / read_only_mode / !enable / static preview` 不仅要忽略后续输入，还要先清掉残留 pressed。
- runtime 需要重点复核这些帧：
  - 默认帧
  - `Right` 后的主控件翻页帧
  - `End` 后的尾页帧
  - 主 snapshot 切换帧
  - `compact` 切换帧
  - preview 点击后的收尾帧
  - 最终稳定帧

## 9. 已知限制与后续方向
- 当前继续使用固定 snapshot 数据，不接真实业务内容。
- 当前不实现复杂分页动画或外部内容联动。
- 当前以 reference 语义、状态闭环和渲染稳定性为主，不扩展额外场景化包装。

## 10. 与现有控件的边界
- 相比 `tab_strip`：这里强调离散页码反馈，不是 section 标签。
- 相比 `flip_view`：这里以 pips rail 为中心，不以内容卡片为中心。
- 相比 `scroll_bar`：这里是离散分页，不是连续范围值。
- 相比旧 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层和 preview 桥接逻辑。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI PipsPager`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`PipsPager`
- 本次保留核心状态：
  - `standard`
  - `current page`
  - `previous / next`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、旧 preview 标签和双列 preview 包裹结构。
- 删除让 preview 继续承担切换、桥接和额外说明的职责。
- 删除过强的按钮填充、pressed 装饰和高饱和当前页强调。
- 删除与 reference 语义无关的场景化叙事壳层。

## 14. EGUI 适配时的简化点与约束
- 使用固定 snapshot 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过统一 `clear_pressed_state()` 与 static preview API，把 setter、guard、preview 的状态清理收口到同一套语义。
- 通过程序化切换 snapshot 再补一个 preview 点击收尾帧，保证 runtime 能稳定抓到交互后的渲染结果。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
