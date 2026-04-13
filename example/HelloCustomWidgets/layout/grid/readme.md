# Grid 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`Grid`
- 本次保留语义：固定列数的等宽网格布局、标准主态、`stack / dense` 静态 preview、固定 snapshot 布局对照
- 本次删除内容：行列跨越、共享尺寸组、自适应测量规则、复杂约束求解、拖拽重排
- EGUI 适配说明：复用 SDK `egui_view_gridlayout` 作为底层布局能力，custom 层只补列数 style helper、静态 preview API 与 reference 页面

## 1. 为什么需要这个控件？
`Grid` 用来表达“内容仍然属于同一信息表面，但需要按列对齐排布”的基础布局语义，适合设置总览、状态面板、摘要卡片和轻量 dashboard。当前仓库已经有 `uniform_grid`、`grid_view`、`data_grid` 等更具体的网格控件，但还缺一个更底层、只承担布局语义的 `Grid` reference。

## 2. 为什么现有控件不够用？
- `uniform_grid` 更偏等尺寸 tile 集合和单元激活，不是基础布局容器。
- `grid_view`、`data_grid` 都自带更强的数据和交互语义，层级更高。
- 纯 `linearlayout` 很难直观表达“多列对齐”的 Fluent/WPF 基础布局语义。

## 3. 目标场景与示例概览
- 主面板：一个标准 `Grid`，在录制轨道中依次展示 `two columns / dense board / review stack` 三组布局快照。
- 底部两个静态 preview：
  - `stack`
  - `dense`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 214`
- 主面板：`196 x 118`
- 主 grid：`176 x 68`
- 底部容器：`216 x 74`
- 单个 preview 面板：`104 x 74`
- preview grid：`84 x 38`

## 5. 控件清单与状态矩阵
- 主控件：标准 `Grid`，按 snapshot 切换 `1 / 2 / 3` 列布局。
- `stack` preview：单列静态对照。
- `dense` preview：三列静态对照。

| 状态 / 能力 | 主控件 | Stack preview | Dense preview |
| --- | --- | --- | --- |
| 固定列数布局 | 是 | 是 | 是 |
| 等宽列 | 是 | 是 | 是 |
| 录制轨道切换 snapshot | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 6. 录制动作设计
1. 初始显示 `Two equal columns`
2. 抓取第一帧
3. 切到 `Dense board`
4. 抓取第二帧
5. 切到 `Review stack`
6. 抓取第三帧
7. 恢复默认布局并收尾

## 7. 编译 / runtime / 截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/grid PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/grid --track reference --timeout 10 --keep-screenshots
```

验收重点：
- 主 grid 和底部两个 preview 都必须完整可见。
- 三组 snapshot 的列数切换不能出现裁切、重叠或整块缺失。
- `stack` 和 `dense` preview 应保持低噪音，只承担 reference 对照职责。

## 8. 已知限制
- 当前只支持固定列数的等宽列，不支持 `row span / column span`。
- 不实现共享尺寸组、自适应测量和复杂约束布局。
- 不下沉 SDK，只在 custom 层补 reference 语义和验收闭环。
