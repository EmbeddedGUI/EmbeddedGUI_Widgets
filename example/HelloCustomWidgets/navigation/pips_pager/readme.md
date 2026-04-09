# pips_pager 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI PipsPager`
- 补充对照控件：`tab_strip`、`flip_view`
- 对应组件名：`PipsPager`
- 本次保留状态：`standard`、`current page`、`previous / next`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、状态文案、外部 preview 标签、旧双列 preview 包裹壳、过亮按钮和过强当前页强调
- EGUI 适配说明：继续复用仓库内 `pips_pager` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和绘制强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`pips_pager` 用来表达离散页码切换、短轨道分页预览和 onboarding 式分步导航。它适合步骤页、幻灯分页、简短内容翻页这类“页数有限、当前位置必须一眼可见”的界面。

## 2. 为什么现有控件不够用

- `tab_strip` 更偏 section 标签切换，不强调页码位置信息。
- `flip_view` 强调内容卡片翻页，不以 pips rail 为主语义。
- `scroll_bar` 是连续范围值，不适合离散分页。
- 当前 reference 主线仍需要一版贴近 Fluent / WPF UI `PipsPager` 的 custom widget。

## 3. 目标场景与示例概览

- 主控件展示标准 `PipsPager`，覆盖 `Onboarding`、`Gallery`、`Report deck` 三组主 snapshot。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下的短轨道分页。
- 底部右侧展示 `read only` 静态对照，保留当前页可见性但冻结交互。
- 页面结构统一收口为：标题 -> 主 `pips_pager` -> `compact / read only`。
- 旧的 preview 列容器和外部标签已移除，底部两张 preview 直接挂在同一行容器下。

目标目录：`example/HelloCustomWidgets/navigation/pips_pager/`

## 4. 视觉与布局规格

- 根容器尺寸：`224 x 198`
- 主控件尺寸：`196 x 92`
- 底部对照行尺寸：`216 x 58`
- `compact` 预览：`104 x 58`
- `read only` 预览：`104 x 58`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白色主卡、低噪音浅边框。
  - 保留 title、helper、previous/next 和 pips rail 的清晰层级，但压轻按钮填充、inactive dots 和 focus。
  - `current page` 继续可辨识，但不再用高饱和高亮 pill。
  - `read only` 使用更弱的灰蓝 palette，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `pager_primary` | `egui_view_pips_pager_t` | `196 x 92` | `Onboarding / 2 of 7` | 主 `PipsPager` |
| `pager_compact` | `egui_view_pips_pager_t` | `104 x 58` | `Compact / 3 of 6` | 底部 compact 静态对照 |
| `pager_read_only` | `egui_view_pips_pager_t` | `104 x 58` | `Read only / 4 of 7` | 底部 read only 静态对照 |
| `primary_snapshots` | `pager_snapshot_t[3]` | - | `Onboarding / Gallery / Report deck` | 主控件录制轨道 |
| `compact_snapshots` | `pager_snapshot_t[2]` | - | `Compact / 3 of 6`、`Compact / 6 of 8` | compact 预览程序化切换 |
| `read_only_snapshot` | `pager_snapshot_t` | - | `Static preview` | read only 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Onboarding` | 默认主 snapshot，验证标准分页层级 |
| 主控件 | `Right` | 验证当前页前进一步 |
| 主控件 | `End` | 验证尾页边界 |
| 主控件 | `Gallery` | 程序化切换主 snapshot，验证较长页数和短窗口 |
| `compact` | `Compact / 3 of 6` | 默认 compact 对照 |
| `compact` | `Compact / 6 of 8` | 第二条 compact 轨道，只做程序化切换 |
| `read only` | `Static preview` | 固定只读轨道，禁 touch、禁 focus、禁键盘 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 请求默认截图。
3. 发送 `Right`，验证当前页向后一步。
4. 请求第二张截图。
5. 发送 `End`，验证尾页状态。
6. 请求第三张截图。
7. 程序化切换到下一组主 snapshot。
8. 请求第四张截图。
9. 程序化切换 `compact` 到第二组 snapshot。
10. 请求最终截图。

## 8. 编译、touch、runtime、单测与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部 `compact / read only` 预览都必须完整可见，不能重新长出外部壳层。
- previous / next、当前页 pill 和 inactive dots 需要清晰可辨，但整体不能回到高噪音 showcase 风格。
- `compact` 在小尺寸下仍要看得出当前页位置。
- `read only` 只能做静态展示，不能响应 touch、focus 或键盘。
- unit test 中已有的键盘导航、`+ / -`、visible pip 选择和只读/紧凑忽略输入语义不能回归。

## 9. 已知限制与后续方向

- 当前仍使用固定 snapshot 数据，不接真实业务内容。
- 当前不实现复杂分页动画或外部内容联动。
- 当前优先验证 reference 语义、布局和交互闭环，不联动页面内容区。

## 10. 与现有控件的边界

- 相比 `tab_strip`：这里强调离散页码反馈，不是 section 标签。
- 相比 `flip_view`：这里以 pips rail 为中心，不以内容卡片为中心。
- 相比 `scroll_bar`：这里是离散分页，不是连续范围值。
- 相比旧版 showcase 页面：这里回到统一的 Fluent reference 结构，不保留外部说明壳层。

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

- 删除页面级 `guide`、状态文案、preview 标签和旧的双列 preview 包裹结构。
- 删除让 preview 参与交互的点击与焦点职责。
- 删除过亮按钮、过强当前页 pill 和过重 focus ring。
- 删除与 reference 无关的外部说明壳层和场景化叙事。

## 14. EGUI 适配时的简化点与约束

- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 snapshot 保证 runtime 能稳定抓到分页状态。
- 当前先作为 `HelloCustomWidgets` 的 reference widget 维护，后续是否下沉框架层再单独评估。
