# data_grid 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`DataGrid`
- 本次保留状态：`selected row`、`compact`、`read only`、`accent`、`success`、`warning`
- 本次删除效果：页面级 `guide`、故事化业务壳、排序箭头、列拖拽、列宽调整、Acrylic、重阴影、showcase 外层
- EGUI 适配说明：在 custom 层实现轻量 `DataGrid` reference，收口 `header / rows / cells / row selection`，统一 same-target release、键盘激活和静态 preview 输入抑制，不修改 `sdk/EmbeddedGUI`
- 补充对照实现：`ModernWpf`

## 1. 为什么需要这个控件？
`data_grid` 用来表达“列头 + 多行数据 + 单行选择”的标准表格语义。它适合发布检查、审阅列表、任务排期和轻量运营面板这类需要在有限空间里并排比较多个字段的场景。

## 2. 为什么现有控件不够用？
- `data_list_panel` 更偏 `ListView`，核心是行摘要，不强调列头和二维表格关系。
- `settings_panel`、`settings_expander` 面向设置项排布，不适合表达表格列。
- `card_panel`、`card_action`、`card_control` 都是卡片摘要语义，不是多列数据对照。
- SDK `table` 只提供基础单元格绘制，没有当前仓库要求的 Fluent reference 页面、row selection、static preview 和验收闭环。

因此这里单独保留 `data_grid`，把 `DataGrid` 的 reference 语义收口到统一的 custom widget。

## 3. 目标场景与示例概览
- 主控件展示标准 `DataGrid`，覆盖 `Rollout board`、`Audit board`、`Release board` 三组关键 snapshot。
- 底部左侧展示 `compact` 静态预览，验证小尺寸下列头和选中行仍然稳定可辨认。
- 底部右侧展示 `read only` 静态预览，验证只读弱化后的表格层级与输入抑制。
- 页面结构统一收口为：标题 -> 主 `data_grid` -> `compact / read only` 双 preview。
- 两个 preview 都通过 `egui_view_data_grid_override_static_preview_api()` 固定为静态 reference，只吞掉输入并协助主控件清焦点。

目标目录：`example/HelloCustomWidgets/layout/data_grid/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 248`
- 主控件尺寸：`196 x 126`
- 底部对照行尺寸：`216 x 80`
- `compact` preview：`104 x 80`
- `read only` preview：`104 x 80`
- 页面结构：标题 + 主表格 + 底部双 preview
- 风格约束：
  - 保持浅底、低噪音边框、轻量表头和细分隔线。
  - 选中态通过左侧 selection strip、row fill 和轻微 tone 差异表达，不依赖重阴影。
  - `ACTION_MOVE` 只负责 same-target release 的按压回显，移出命中行后不能提交。
  - `set_current_snapshot / set_current_row / set_font / set_meta_font / set_palette / set_compact_mode / set_read_only_mode` 后都不能残留旧的 `pressed` 渲染。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 248` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Data Grid` | 页面标题 |
| `grid_primary` | `egui_view_data_grid_t` | `196 x 126` | `Rollout board` | 主表格 |
| `grid_compact` | `egui_view_data_grid_t` | `104 x 80` | `Compact grid` | `compact` 静态对照 |
| `grid_read_only` | `egui_view_data_grid_t` | `104 x 80` | `Read only grid` | `read only` 静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 关键状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Rollout board` | 默认状态，验证标准 DataGrid 结构 |
| 主控件 | `Audit board` | 验证切换 snapshot 后的选中行回落 |
| 主控件 | `Release board` | 验证 warning 语义下的 row selection |
| `compact` | `Compact grid` | 默认紧凑预览 |
| `compact` | `Compact audit` | runtime 第二组关键帧 |
| `read only` | `Read only grid + read only` | 固定只读预览 |

## 7. 交互与状态语义
- 主控件的交互粒度是“行”，不是单个 cell。
- `activate_current_row()` 统一触发行激活回调；触摸释放和键盘 `Enter / Space` 都走这一路径。
- `ACTION_MOVE` 会实时更新按压回显：
  - 移出原命中行时取消 `pressed`
  - 回到原命中行后恢复 `pressed`
  - 只有 `UP` 仍停留在原命中行上才提交行激活
- `ACTION_CANCEL` 只清理 `pressed`，不修改 `current_snapshot / current_row`
- 键盘支持 `Up / Down / Home / End / Tab` 切换当前行，`Enter / Space` 激活当前行
- `read_only_mode`、`!enable` 和空数据状态都会先清理残留 `pressed`，再拒绝新的 `touch / key`
- static preview 的 `touch / key` 只消费事件并清理残留 `pressed`，不会改 `current_row`，也不会触发 listener

## 8. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件和 `compact` preview 到默认状态
2. 请求初始帧
3. 切到 `Audit board`
4. 请求第二帧
5. 切到 `Release board`
6. 请求第三帧
7. 切换 `compact` 到 `Compact audit`
8. 请求 `compact` 第二帧
9. 主动给主控件请求 `focus`
10. 点击 `compact` preview 的第二行，只执行 preview 的静态收尾逻辑
11. 请求 preview 点击后的收尾帧，确认没有残留 `pressed / focus`

## 9. 编译、Touch、Runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_grid PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_grid --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_grid
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_grid
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切
- 表头、各列文本、选中行和分隔线必须稳定可辨认
- 主控件必须通过 same-target release / cancel 回归：移出行后不提交，回到原行释放才提交
- setter、guard 和 preview 都必须遵守“先清理残留 `pressed` 再处理后续状态”的语义
- static preview 的 `touch / key` 不能改动 `current_snapshot / current_row`，也不能触发 listener

## 11. 已知限制与后续方向
- 当前仍是固定尺寸 reference 示例，不覆盖真实滚动、大量数据和虚拟化
- 当前不做列排序、列宽调整、冻结列或编辑态
- 当前 cell 内容仍由静态 snapshot 驱动，不接真实数据模型

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里保留显式列头和多列 cell，不是单列列表摘要
- 相比 `settings_panel`：这里承载的是表格行，不是设置行组合
- 相比 SDK `table`：这里补的是 Fluent `DataGrid` 的 reference 语义、交互和验收闭环
