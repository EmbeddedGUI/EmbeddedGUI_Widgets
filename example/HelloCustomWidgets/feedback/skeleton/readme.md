# skeleton 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`Skeleton`
- 本次保留状态：`wave`、`compact`、`read only`
- 删减效果：页面级 guide / section label / preview label、额外强调色标签、Acrylic、复杂渐变、真实内容切换动画、长列表无限骨架
- EGUI 适配说明：使用固定骨架块模板、轻量 wave / pulse 动画和程序化 snapshot 切换，在 `480 x 480` 下优先保证结构可读、层级清晰和低噪音表达

## 1. 为什么需要这个控件

`skeleton` 用来在真实内容尚未到达时，先把页面结构、重点区域和内容密度表达出来。相比单纯的旋转 loading，它更能说明“内容将会长什么样”，适合文章、列表、设置页和卡片面板这类通用页面。

## 2. 为什么现有控件不够用

- `spinner` 只能表达“正在加载”，不能表达页面骨架结构
- `progress_bar` 更适合线性进度，不适合内容占位
- 旧 `skeleton_loader` 是深色 showcase 骨架卡，视觉更重，也更偏演示页
- 当前主线仍需要一版接近 `Fluent 2 / WPF UI` 的浅色、低噪音 skeleton reference

因此这里不继续修补旧结构，而是把 `skeleton` 收敛为统一的 reference 页面。

## 3. 目标场景与示例概览

- 主卡展示标准 `wave` skeleton，程序化轮换 `Article / Feed / Settings` 三类页面骨架
- 左下预览展示 `compact` 形态，保留更紧凑的骨架密度和局部 pulse 强调
- 右下预览展示 `read only` 形态，关闭动画并弱化对比，验证静态占位层级
- 示例页只保留标题、主骨架卡和底部 `compact / read only` 双预览，不再保留外部说明 chrome

目录：

- `example/HelloCustomWidgets/feedback/skeleton/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 224`
- 页面结构：标题 -> 主骨架卡 -> `compact / read only` 双预览
- 主骨架卡：`196 x 124`
- 底部双预览容器：`216 x 60`
- `compact` 预览：`104 x 60`
- `read only` 预览：`104 x 60`
- 视觉规则：
  - 使用浅灰白 page panel + 白底轻边框容器
  - 骨架块采用统一浅灰填充，不做重阴影和高饱和描边
  - 主卡保留轻量 wave shimmer，`compact` 保留 pulse 强调，`read only` 关闭动画
  - accent 仅用于轻度提亮强调块，不再保留旧版青绿色标签语法

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Skeleton` | 页面标题 |
| `skeleton_primary` | `egui_view_skeleton_t` | `196 x 124` | `Article` | 主 `wave` 骨架 |
| `skeleton_compact` | `egui_view_skeleton_t` | `104 x 60` | `Compact row` | 紧凑 pulse 预览 |
| `skeleton_locked` | `egui_view_skeleton_t` | `104 x 60` | `Read only` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主骨架 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Article` | `Compact row` | `Read only` |
| 切换 1 | `Feed` | 保持 | 保持 |
| 切换 2 | `Settings` | 保持 | 保持 |
| 紧凑切换 | 保持 | `Compact row -> Compact tile` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 保持无动画、弱对比、只做结构提示 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主快照与 `compact` 快照
2. 请求第一页截图
3. 程序化切换主卡到 `Feed`
4. 请求第二页截图
5. 程序化切换主卡到 `Settings`
6. 请求第三页截图
7. 程序化切换 `compact` 到第二组快照
8. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 标题、主骨架和底部双预览都必须完整可见
- `wave`、`compact`、`read only` 三种语义要能从截图直接分辨
- 主卡里的 shimmer 必须保持轻量，不能回到高噪音 showcase 风格
- 页面中不再出现 guide、section divider、`Pulse` / `Static` 外部标签
- 预览区域不再承担点击切换职责，只作为对照展示

## 9. 已知限制与下一轮迭代计划

- 当前 shimmer 仍是简化版条带扫光，不是完整渐变波浪
- 骨架模板仍使用固定 snapshot，未做运行时自由拼装
- 当前只覆盖小型页面骨架，不包含更长列表和复杂表单
- 先完成 reference 版收敛，再决定是否沉入框架层

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `skeleton_loader`：本控件更浅、更轻、更标准，不再走深色 showcase 卡路线
- 相比 `spinner`：本控件表达内容结构，不只是等待状态
- 相比 `progress_bar`：本控件表达页面骨架，不承担数值进度反馈
- 相比 `card_panel` 候选方向：本控件只表达“加载前占位”，不承载真实内容卡片

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Skeleton`
- 本次保留状态：
  - `wave`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、section label、preview label 和额外状态说明条
- 不做真实内容淡入切换
- 不做复杂渐变和高频 shimmer 动画
- 不做长列表无限滚动骨架
- 不做阴影、Acrylic、背景模糊和系统级转场

## 14. EGUI 适配时的简化点与约束

- 使用固定骨架块模板，优先保证 `480 x 480` 下的审阅效率
- wave / pulse 动画使用轻量定时刷新，避免引入重型动画系统
- 通过统一的浅灰白 palette 维持 `Fluent 2 / WPF UI` 低噪音参考页风格
- 先完成示例级 reference 版本，再决定是否上升到框架公共控件
