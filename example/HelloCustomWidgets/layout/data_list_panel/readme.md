# `data_list_panel` 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`ListView` / `ItemsView`
- 当前保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`
- 删除效果：页面级 guide、状态回显、preview 外部标签、Acrylic、复杂 hover、虚拟滚动和桌面级列头行为

## 1. 为什么需要这个控件
`data_list_panel` 用来承载“卡片内的数据列表”语义，适合任务队列、资产审阅、归档清单和同步记录这类需要在一张低噪音面板里快速浏览多条结构化条目的场景。

## 2. 为什么现有控件不够用
- `list` 只有基础列表能力，不强调 Fluent 风格的数据行层级和焦点项表达。
- `table` 更偏网格数据，不适合当前这种单卡片、低密度、强调视觉留白的列表面板。
- `settings_panel` 强调设置项行和尾部控件，不适合表达记录列表。
- `master_detail` 强调主从联动，不是当前这种单面板内的摘要式数据浏览。

## 3. 目标场景与示例概览
- 页面只保留标题、一个主 `data_list_panel`、底部两个静态 preview。
- 主控件负责真实交互，覆盖 `sync queue`、`asset review`、`archive sweep` 三组快照。
- 左下 `compact` preview 只做紧凑布局对照。
- 右下 `read only` preview 只做只读弱化态对照。

目录：
- `example/HelloCustomWidgets/layout/data_list_panel/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主控件：`196 x 116`
- 底部双 preview 容器：`216 x 80`
- `compact` preview：`104 x 80`
- `read only` preview：`104 x 80`

视觉规则：
- 使用浅灰 page panel + 白底列表卡片，保持 `Fluent 2 / WPF UI` 的浅色、低噪音层级。
- 主卡保留 `eyebrow / title / summary / rows / footer` 五层结构。
- 数据行只保留 `glyph + title + value` 的最小表达，不再挂载外部说明标签。
- `compact` 和 `read only` 直接通过控件自身模式表达，不依赖外部文案提示。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 236` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Data List` | 页面标题 |
| `panel_primary` | `egui_view_data_list_panel_t` | `196 x 116` | `sync queue` | 主交互控件 |
| `panel_compact` | `egui_view_data_list_panel_t` | `104 x 80` | `compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_data_list_panel_t` | `104 x 80` | `read only` | 只读静态 preview |

## 6. 状态覆盖矩阵
| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `sync queue + accent row` | `compact recent` | `read only locked` |
| 切换 1 | `sync queue + success row` | 保持 | 保持 |
| 切换 2 | `asset review + warning row` | 保持 | 保持 |
| 切换 3 | `archive sweep + warning row` | 保持 | 保持 |
| preview 切换 | 保持 | `compact recent -> compact review` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 弱化但内容保留 |

## 7. 交互与 preview 约束
- 主控件保留真实 `touch / key` 交互，支持条目切换与快照切换后的焦点项同步。
- `compact` 和 `read only` preview 统一通过 `egui_view_data_list_panel_override_static_preview_api()` 挂成静态 reference。
- 静态 preview 会吞掉 `touch / key`，并清掉残留 `pressed`，但不会修改 `current_snapshot` 和 `current_index`。
- preview 点击时只允许清理 `panel_primary` 的 focus，不承担额外交互职责。
- `set_snapshots / set_current_snapshot / set_current_index / set_font / set_meta_font / set_compact_mode / set_read_only_mode / set_palette` 共用同一套 `pressed` 清理语义。

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主状态与 `compact` 状态，并请求 `panel_primary` focus。
2. 请求首帧截图。
3. 程序化切换主控件到 `sync queue` 的最后一行。
4. 请求第二帧截图。
5. 程序化切换主控件到 `asset review` 的 `warning` 行。
6. 请求第三帧截图。
7. 程序化切换主控件到 `archive sweep`。
8. 请求第四帧截图。
9. 程序化切换 `compact` preview 到第二组快照。
10. 请求第五帧截图。
11. 再次请求 `panel_primary` focus。
12. 点击 `compact` preview，仅验证主控件焦点收尾。
13. 请求最终收尾帧并保留等待。

## 9. 编译、单测与 runtime 验收
```bash
make clean APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc
make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots
```

验收重点：
- 主控件和底部双 preview 必须完整可见，不黑屏、不白屏、不裁切。
- 行内 `glyph / title / value / footer` 间距稳定，保持低噪音视觉节奏。
- preview 不再承担真实交互，只作为 reference 对照。
- 点击 preview 后只清主控件 focus，最终收尾帧仍保持完整布局。
- `selection / snapshot / compact / read only / view disabled` 切换后不能残留错误的 `pressed` 状态。

## 10. 与现有控件的边界
- 相比 `list`：这里强调标准化数据行和当前焦点项语义。
- 相比 `table`：这里不是网格表格，而是卡片内的摘要式数据清单。
- 相比 `settings_panel`：这里不承载设置项尾部控件，重点是列表记录。
- 相比 `master_detail`：这里不拆分详情面板，信息表达收敛在单卡内部。

## 11. 已知限制
- 当前是固定尺寸的 `reference` 实现，不覆盖长列表、滚动容器和虚拟化。
- 当前 `value` 仍是短文本 / 数值表达，不支持复杂列头、排序和过滤。
- 当前数据仍由静态 `snapshot + item` 数组驱动，不接入真实数据源。
- 如果后续需要沉入框架层，再单独评估数据源接口、滚动容器和虚拟化能力。
