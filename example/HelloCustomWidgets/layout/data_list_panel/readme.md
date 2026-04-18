# data_list_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / ListView`
- 补充参考：`ModernWpf / ItemsView`
- 对应组件语义：`ListView`
- 本轮保留语义：`Sync queue / Asset review / Archive sweep / compact / read only`
- 本轮移除内容：页面级 guide、状态文案、preview 点击桥接、旧录制轨道里的第二条 compact preview 切换与收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/data_list_panel` 控件实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`data_list_panel` 用于在单张低噪音卡片里快速浏览结构化数据行。它适合任务队列、资源审核、归档检查、同步记录这类需要在同一块面板里查看多条摘要数据的场景。

## 2. 为什么现有控件不够用
- `list` 更偏基础列表，不强调数据面板式的 row 层级和摘要结构。
- `data_grid` 更偏网格表格，不适合当前这种单卡片低密度列表。
- `settings_panel` 强调设置项和值单元，不适合表达记录型数据行。
- `master_detail` 强调左右联动，这里只需要单面板列表浏览。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `data_list_panel` -> 底部 `compact / read only` 双静态 preview。
- 主区保留三组录制状态：
  - `Sync queue`
  - `Asset review`
  - `Archive sweep`
- 底部左侧是 `compact` 静态 preview，固定展示紧凑版数据列表。
- 底部右侧是 `read only` 静态 preview，固定展示只读版数据列表。
- 两个 preview 都通过 `egui_view_data_list_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / dispatch_key_event()`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_index / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed`

目标目录：`example/HelloCustomWidgets/layout/data_list_panel/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 236`
- 主控件：`196 x 116`
- 底部对照行：`216 x 80`
- `compact` preview：`104 x 80`
- `read only` preview：`104 x 80`
- 视觉约束：
  - 使用浅灰 page panel 和白底数据卡片，保持 Fluent 风格低噪音层级。
  - 主区保留 `eyebrow / title / summary / rows / footer` 五层结构。
  - 底部两个 preview 只做静态参考，不再承担焦点收尾或状态切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 236` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Data List` | 页面标题 |
| `panel_primary` | `egui_view_data_list_panel_t` | `196 x 116` | `Sync queue` | 主区标准数据面板 |
| `panel_compact` | `egui_view_data_list_panel_t` | `104 x 80` | `Compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_data_list_panel_t` | `104 x 80` | `Read only` | 只读静态 preview |
| `primary_states` | `data_list_panel_state_t[3]` | - | `Sync queue / Asset review / Archive sweep` | 主区录制轨道 |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Sync queue` | 默认 reference 状态，展示标准数据行层级 |
| 主控件 | `Asset review` | 切到资源审核快照与 warning row |
| 主控件 | `Archive sweep` | 切到归档巡检快照 |
| `compact` preview | `Compact` | 固定静态对照，不随录制轨道变化 |
| `read only` preview | `Read only` | 固定静态对照，不随录制轨道变化 |

## 7. 交互语义与单测要求
- 主控件继续保留真实的 `touch` 选中与键盘导航。
- 单测覆盖：
  - `set_snapshots / set_current_snapshot / set_current_index / set_compact_mode / set_read_only_mode` 的 pressed 清理语义
  - `touch` 选中、`read only`、`!enable` 守卫
  - `Left / Right / Up / Down / Home / End / Tab` 键盘切换
  - 静态 preview 用例改为 “consumes input and keeps state”
- preview 键盘入口统一走 `dispatch_key_event()`，不再回退到旧的 `on_key_event()` 直连路径。
- 静态 preview 用例必须验证：
  - 输入前后的 `current_snapshot / current_index / compact_mode / read_only_mode / snapshot_count` 保持不变
  - `content_region / eyebrow_region / title_region / summary_region / footer_region / row_regions` 保持不变
  - `pressed_index / is_pressed` 被清理
  - `on_selection_changed` 不触发

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 恢复主控件默认 `Sync queue`，同时恢复底部 `compact / read only` preview，并直接输出首帧
2. 切到主区 `Asset review`
3. 输出第二组主区帧
4. 切到主区 `Archive sweep`
5. 输出第三组主区帧
6. 恢复主区默认 `Sync queue`
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `DATA_LIST_PANEL_RECORD_FINAL_WAIT`，中间状态切换仍保留 `DATA_LIST_PANEL_RECORD_WAIT / DATA_LIST_PANEL_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_list_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_list_panel
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Sync queue / Asset review / Archive sweep` 三组可识别状态。
- 底部 `compact / read only` preview 必须在全程 runtime 帧里保持静态一致。
- 静态 preview 输入后不能改变 `current_snapshot`、`current_index`、布局区域或 listener 状态。
- README、demo 录制轨道、单测入口与验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_data_list_panel/default`
- 复核目标：
  - 主区裁剪后只出现 `3` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `list`：这里强调标准化数据行层级，而不只是基础列表。
- 相比 `data_grid`：这里保留卡片式摘要数据面板，而不是网格表格。
- 相比 `settings_panel`：这里不承担设置项 value cell 语义。
- 相比 `master_detail`：这里不拆出 detail pane，而是把信息收敛在单卡内部。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `sync queue`
  - `asset review`
  - `archive sweep`
  - `compact`
  - `read only`
- 保留的交互：
  - touch selection
  - 键盘 `Left / Right / Up / Down / Home / End / Tab`
- 删减的旧桥接与旧口径：
  - 页面级 guide 和状态文案
  - preview 点击桥接
  - 旧录制轨道里的第二条 `compact` preview 切换与收尾动作

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/data_list_panel PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `data_list_panel` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/data_list_panel --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_data_list_panel/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/data_list_panel`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_data_list_panel`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=203`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(50, 95) - (430, 259)`
  - 按 `y >= 259` 裁切底部 preview 区域后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Sync queue`、`Asset review` 与 `Archive sweep` 三组 reference 状态，最终稳定帧已显式回到默认快照
