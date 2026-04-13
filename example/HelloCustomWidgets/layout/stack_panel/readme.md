# StackPanel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`StackPanel`
- 本次保留语义：顺序堆叠、纵向主态、横向排布静态 preview、紧凑静态 preview、固定 snapshot 对照
- 本次不做内容：子项虚拟化、自动换行、复杂测量、共享尺寸组、拖拽重排
- EGUI 适配说明：在 custom 层复用 SDK `egui_view_linearlayout`，只补轻量 style helper、静态 preview API 与 reference 演示页面

## 1. 为什么需要这个控件
`StackPanel` 是最基础的顺序布局语义之一，用来表达“内容按固定顺序连续堆叠”的 Fluent / WPF 布局关系，适合设置摘要、审核步骤、操作列表和轻量信息面板。

## 2. 为什么现有控件不够用
- `grid` 强调列对齐，不适合纯顺序堆叠。
- `wrap_panel` 负责自动换行，不适合固定阅读顺序的纵向信息流。
- 直接使用底层 `linearlayout` 不足以作为 reference 组件展示 Fluent / WPF 的布局语义和静态 preview 对照。

## 3. 目标场景与示例概览
- 主面板展示一个 `StackPanel`，录制轨道依次切换：
  - `Review flow`
  - `Inline tools`
  - `Compact notes`
- 底部两个静态 preview：
  - `Horizontal`
  - `Compact`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 226`
- 主面板：`196 x 116`
- 主 `StackPanel`：`176 x 64`
- 底部容器：`216 x 74`
- 单个 preview 面板：`104 x 74`

## 5. 控件清单与状态矩阵
| 区域 | 语义 | 说明 |
| --- | --- | --- |
| 主控件 | Vertical stack | 默认纵向堆叠 |
| 主控件 | Horizontal strip | 横向排布对照 |
| 主控件 | Compact stack | 紧凑堆叠对照 |
| Horizontal preview | Static preview | 吞掉 `touch / key` |
| Compact preview | Static preview | 吞掉 `touch / key` |

## 6. 录制动作设计
1. 初始显示 `Review flow`
2. 抓取第一帧
3. 切换到 `Inline tools`
4. 抓取第二帧
5. 切换到 `Compact notes`
6. 抓取第三帧
7. 恢复默认状态并收尾

## 7. 编译 / runtime / 截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/stack_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/stack_panel --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/stack_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_stack_panel
```

验收重点：
- 主 `StackPanel` 与两个 preview 都必须完整可见。
- 三组 snapshot 的排布切换不能出现裁切、错位或重叠。
- 静态 preview 必须吞掉新增的 `touch / key` 输入。

## 8. 参考设计体系与母本
- Fluent 2 基础布局语义
- WPF UI `StackPanel`

## 9. 保留与删减
- 保留：顺序堆叠、纵向与横向基础布局语义、低噪音 surface 对照
- 删减：复杂滚动、虚拟化、动态测量、交互式拖拽

## 10. EGUI 简化点与限制
- 当前仅做 reference wrapper，不下沉 SDK。
- 不实现自动换行与虚拟化能力。
- style helper 只服务 reference 语义，不扩展为完整通用布局系统。
