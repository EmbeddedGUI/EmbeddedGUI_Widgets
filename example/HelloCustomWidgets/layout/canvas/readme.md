# Canvas 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`Canvas`
- 本次保留语义：绝对定位、固定叠放、轻量 overlay、紧凑静态 preview、固定 snapshot 对照
- 本次不做内容：自由绘图、拖拽编辑、缩放平移、复杂命中层级、动态 z-order 管理
- EGUI 适配说明：在 custom 层复用 SDK `egui_view_group`，只补轻量 style helper、子项原点设置、静态 preview API 与 reference 演示页面

## 1. 为什么需要这个控件
`Canvas` 用来表达“子项按明确坐标固定放置”的布局关系，适合批注板、固定锚点提示、小型 overlay 和轻量地图式信息面板。

## 2. 为什么现有控件不够用
- `stack_panel`、`grid`、`wrap_panel` 都围绕顺序或规则流式布局，不适合精确绝对定位。
- `relative_panel` 负责关系约束，不适合直接表达“放在这个坐标点”。
- 直接使用底层 `group` 无法形成清晰的 reference 语义、静态 preview 和 Fluent / WPF 风格对照。

## 3. 目标场景与示例概览
- 主面板展示一个 `Canvas`，录制轨道依次切换：
  - `Pinned notes`
  - `Status overlay`
  - `Compact board`
- 底部两个静态 preview：
  - `Pinned`
  - `Compact`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 240`
- 主面板：`196 x 120`
- 主 `Canvas`：`176 x 64`
- 底部容器：`216 x 76`
- 单个 preview 面板：`104 x 76`

## 5. 控件清单与状态矩阵
| 区域 | 语义 | 说明 |
| --- | --- | --- |
| 主控件 | Standard canvas | 默认绝对定位主态 |
| 主控件 | Overlay board | 固定叠放对照 |
| 主控件 | Compact board | 紧凑坐标板对照 |
| Pinned preview | Static preview | 吞掉 `touch / key` |
| Compact preview | Static preview | 吞掉 `touch / key` |

## 6. 录制动作设计
1. 初始显示 `Pinned notes`
2. 抓取第一帧
3. 切换到 `Status overlay`
4. 抓取第二帧
5. 切换到 `Compact board`
6. 抓取第三帧
7. 恢复默认状态并收尾

## 7. 编译 / runtime / 截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/canvas PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/canvas --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/canvas
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_canvas
```

验收重点：
- 主 `Canvas` 与两个 preview 都必须完整可见。
- 三组 snapshot 的卡片坐标切换不能出现裁切、错位或重叠。
- 静态 preview 必须吞掉新增的 `touch / key` 输入。

## 8. 参考设计体系与母本
- Fluent 2 轻量 surface 与 overlay 表达
- WPF UI / WPF `Canvas`

## 9. 保留与删减
- 保留：绝对定位、轻量 overlay、固定坐标板、低噪音对照
- 删减：自由绘图、手势编辑、复杂层级控制、滚动缩放

## 10. EGUI 简化点与限制
- 当前仅做 reference wrapper，不下沉 SDK。
- 子项坐标通过 helper 写入，不做编辑器式交互。
- 不支持动态 z-order 管理和拖拽重排。
