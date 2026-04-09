# skeleton 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`Skeleton`
- 本次保留状态：`wave`、`compact`、`read only`
- 本次删除效果：页面级 `guide`、外部 preview 标签、旧双列预览壳层、过强 placeholder 对比、过亮 wave / pulse 高光
- EGUI 适配说明：沿用仓库内 `skeleton` 基础实现，本轮只收口 `reference` 页面结构、静态对照预览和视觉强度，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`skeleton` 用于在真实内容尚未到达时先表达页面结构、信息密度和重点区域，比单纯的 `spinner` 更能说明“内容将会如何排布”，适合文章页、列表页、设置页和轻量卡片占位。

## 2. 为什么现有控件不够用？
- `spinner` 只能表达“正在加载”，不能表达页面骨架结构。
- `progress_bar` 更适合数值进度，不适合内容占位。
- 旧版 `skeleton_loader` 更偏 showcase 风格，视觉更重，也不再符合当前 `reference` 主线。
- 当前 `reference` 主线仍需要一版贴近 Fluent / WPF UI 的浅色 `Skeleton` 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `wave` skeleton，通过录制动作覆盖 `Article / Feed / Settings` 三组页面骨架。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下的紧凑占位布局与轻量 `pulse` 强调。
- 底部右侧展示 `read only` 静态对照，验证禁用动画和弱化 chrome 后的只读占位。
- 页面结构统一收口为：标题 -> 主 `skeleton` -> `compact / read only`。
- 旧的 preview 列容器、外部标签和页面桥接逻辑全部移除。

目标目录：`example/HelloCustomWidgets/feedback/skeleton/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 124`
- 底部对照行尺寸：`216 x 60`
- `compact` 预览：`104 x 60`
- `read only` 预览：`104 x 60`
- 页面结构：标题 + 主控件 + 底部双预览
- 样式约束：
  - 使用浅灰 page panel、白底 skeleton card 和低噪音浅边框。
  - 主控件保留轻量 `wave shimmer`，`compact` 保留更柔和的 `pulse` 强调，`read only` 关闭动画。
  - emphasis block 只做轻量灰蓝提亮，不回到高饱和 showcase 风格。
  - 底部两个 preview 都禁用 touch 和 focus，只做静态 reference 对照。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Skeleton` | 页面标题 |
| `skeleton_primary` | `egui_view_skeleton_t` | `196 x 124` | `Article` | 标准主控件 |
| `skeleton_compact` | `egui_view_skeleton_t` | `104 x 60` | `Compact row` | `compact` 静态对照 |
| `skeleton_read_only` | `egui_view_skeleton_t` | `104 x 60` | `Read only` | `read only` 静态对照 |
| `primary_snapshots` | `egui_view_skeleton_snapshot_t[3]` | - | `Article / Feed / Settings` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_skeleton_snapshot_t[2]` | - | `Compact row / Compact tile` | `compact` 程序化切换 |
| `read_only_snapshots` | `egui_view_skeleton_snapshot_t[1]` | - | `Read only` | `read only` 固定对照数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Article` | 默认 `wave` |
| 主控件 | `Feed` | 第二组主控件骨架 |
| 主控件 | `Settings` | 第三组主控件骨架 |
| `compact` | `Compact row` | 默认紧凑对照 |
| `compact` | `Compact tile` | 第二组紧凑对照 |
| `read only` | `Read only` | 固定只读对照，禁用动画与外部交互 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认状态。
2. 请求默认截图。
3. 程序化切换主控件到 `Feed`。
4. 请求第二张截图。
5. 程序化切换主控件到 `Settings`。
6. 请求第三张截图。
7. 程序化切换 `compact` 到第二组 snapshot。
8. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见。
- `wave`、`compact`、`read only` 三种语义要能从截图直接分辨。
- 主控件的 shimmer 必须保持轻量，不能回到高噪音 showcase 风格。
- `read only` 只做静态展示，不能响应 touch、focus 或页面桥接。
- 单测已有的 snapshot、palette、timer、touch 和 key click 语义不能回归。

## 9. 已知限制与后续方向
- 当前版本仍使用固定 snapshot 数据，不接真实业务骨架配置。
- 当前 `wave` 和 `pulse` 都是轻量近似动画，不做更复杂的渐变带。
- 当前优先验证 `reference` 语义、布局稳定性和视觉收口，不扩展成长列表骨架系统。

## 10. 与现有控件的边界
- 相比 `spinner`：这里表达内容骨架，不只是等待状态。
- 相比 `progress_bar`：这里表达结构占位，不承担数值进度反馈。
- 相比旧版 `skeleton_loader`：这里更浅、更轻、更接近 Fluent / WPF UI `reference`。
- 相比 `card_panel`：这里不承载真实内容卡片，只表达加载前占位。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`Skeleton`
- 本次保留核心状态：
  - `wave`
  - `compact`
  - `read only`

## 13. 相比参考原型删除的效果或装饰
- 删除页面级 `guide`、preview 标签和旧双列预览壳层。
- 删除 preview 参与点击切换、页面桥接和焦点承接的职责。
- 删除过强的 accent placeholder、过亮 wave band 和过重 read-only chrome。
- 删除与 `reference` 无关的说明性外壳和场景化叙事。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot` 数据保证录制稳定。
- `compact / read only` 直接复用同一控件模式，减少额外页面壳层。
- 通过程序化切换 snapshot 保证 runtime 稳定抓取状态变化。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续是否下沉框架层再单独评估。
