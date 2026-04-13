# DockPanel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`DockPanel`
- 本次保留语义：顶部栏、底部栏、左/右侧栏、剩余区域填充、紧凑静态 preview、固定 snapshot 对照
- 本次不做内容：动态拖拽换边、复杂最小尺寸协商、任意层叠、分离式 resize handle
- EGUI 适配说明：在 custom 层复用 SDK `egui_view_group`，只补轻量 style helper、停靠边设置、最后子项填充与静态 preview API

## 1. 为什么需要这个控件
`DockPanel` 用来表达“子项依次停靠在边缘，剩余区域留给内容主体”的布局关系，适合 inspector shell、阅读页外框、工具栏 + 内容面板等典型 Fluent / WPF 外壳结构。

## 2. 为什么现有控件不够用
- `stack_panel` 强调顺序堆叠，不表达边缘停靠和剩余区域填充。
- `canvas` 强调绝对定位，不表达“先占边缘，再把中间留给内容”的容器语义。
- `split_view` 更接近导航 pane，不是通用的停靠布局容器。

## 3. 目标场景与示例概览
- 主面板展示一个 `DockPanel`，录制轨道依次切换：
  - `Inspector shell`
  - `Reading pane`
  - `Compact tools`
- 底部两个静态 preview：
  - `Rail`
  - `Footer`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主面板：`196 x 118`
- 主 `DockPanel`：`176 x 64`
- 底部容器：`216 x 76`
- 单个 preview 面板：`104 x 76`

## 5. 控件清单与状态矩阵
| 区域 | 语义 | 说明 |
| --- | --- | --- |
| 主控件 | Inspector shell | 顶部 + 左右边栏 + 填充区 |
| 主控件 | Reading pane | 顶部 + 左栏 + 底栏 + 填充区 |
| 主控件 | Compact tools | 紧凑停靠外壳 |
| Rail preview | Static preview | 吞掉 `touch / key` |
| Footer preview | Static preview | 吞掉 `touch / key` |

## 6. 录制动作设计
1. 初始显示 `Reading pane`
2. 抓取第一帧
3. 切换到 `Inspector shell`
4. 抓取第二帧
5. 切换到 `Compact tools`
6. 抓取第三帧
7. 恢复默认状态并收尾

## 7. 编译 / runtime / 截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/dock_panel PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/dock_panel --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/dock_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_dock_panel
```

验收重点：
- 主 `DockPanel` 与两个 preview 都必须完整可见。
- 三组 snapshot 切换时，停靠边与填充区不能重叠或裁切。
- 静态 preview 必须吞掉新增的 `touch / key` 输入。

## 8. 参考设计体系与母本
- Fluent 2 低噪音 surface / shell 组织
- WPF UI / WPF `DockPanel`

## 9. 保留与删减
- 保留：边缘停靠、剩余区域填充、紧凑 shell、低噪音对照
- 删减：拖拽换边、复杂尺寸协商、层叠编辑、带交互的 resize

## 10. EGUI 简化点与限制
- 当前仅做 reference wrapper，不下沉 SDK。
- 停靠顺序由子项添加顺序决定。
- 只覆盖基础 `last child fill` 语义，不扩展为复杂布局系统。
