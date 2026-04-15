# master_detail 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / MasterDetail`
- 补充参考：`ModernWpf`
- 对应组件语义：`MasterDetail`
- 本轮保留语义：`Files / Review / Archive / compact / read only`
- 本轮移除内容：页面级 guide、状态说明文案、preview 点击桥接、旧录制轨道里的额外 preview 切换与收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/master_detail` 控件实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`master_detail` 用于在同一块区域里同时承载“左侧条目选择”和“右侧详情阅读”。它适合文件列表、审核队列、成员清单、归档浏览这类需要先定位条目，再在当前页立即查看摘要的场景。

## 2. 为什么现有控件不够用
- `nav_panel` 解决的是导航入口，不强调当前条目的 detail pane。
- `settings_panel` 强调设置项与 value cell，不是 master-detail 双栏结构。
- `list` 和 `data_grid` 只负责列出条目，缺少同屏 detail 面板语义。
- `card_control` 更偏单卡摘要，不承担左列驱动右侧详情的联动。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `master_detail` -> 底部 `compact / read only` 双静态 preview。
- 主区保留三组录制状态：
  - `Files`
  - `Review`
  - `Archive`
- 底部左侧是 `compact` 静态 preview，固定展示紧凑版 master-detail。
- 底部右侧是 `read only` 静态 preview，固定展示只读版 master-detail。
- 两个 preview 都通过 `egui_view_master_detail_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / dispatch_key_event()`
  - 只清理残留 `pressed`
  - 不改 `current_index / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed`

目标目录：`example/HelloCustomWidgets/layout/master_detail/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 208`
- 主控件：`196 x 96`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 视觉约束：
  - 使用浅灰 page panel 和白底低噪音 master-detail 容器。
  - 主区保留左侧 master list、中央分隔与右侧 detail pane 的标准层级。
  - 底部两个 preview 只做静态参考，不再承担焦点收尾或状态切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 208` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Master Detail` | 页面标题 |
| `panel_primary` | `egui_view_master_detail_t` | `196 x 96` | `Files` | 主区标准 master-detail |
| `panel_compact` | `egui_view_master_detail_t` | `104 x 72` | `Files compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_master_detail_t` | `104 x 72` | `Members read only` | 只读静态 preview |
| `primary_snapshots` | `uint8_t[3]` | - | `Files / Review / Archive` | 主区录制轨道 |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Files` | 默认 reference 状态，展示标准双栏结构 |
| 主控件 | `Review` | 切到 warning 语义详情 |
| 主控件 | `Archive` | 切到 neutral 语义详情 |
| `compact` preview | `Files compact` | 固定静态对照，不随录制轨道变化 |
| `read only` preview | `Members read only` | 固定静态对照，不随录制轨道变化 |

## 7. 交互语义与单测要求
- 主控件继续保留真实的 `touch` 选中与键盘导航。
- 单测覆盖：
  - `set_items / set_current_index / set_compact_mode / set_read_only_mode` 的 pressed 清理语义
  - `touch` 选中、`read only`、`!enable` 守卫
  - `Left / Right / Up / Down / Home / End / Tab` 键盘切换
  - 静态 preview 用例改为 “consumes input and keeps state”
- preview 键盘入口统一走 `dispatch_key_event()`，不再回退到旧的 `on_key_event()` 直连路径。
- 静态 preview 用例必须验证：
  - 输入前后的 `current_index / compact_mode / read_only_mode / item_count` 保持不变
  - `master_region / detail_region / row_regions` 保持不变
  - `pressed_index / is_pressed` 被清理
  - `on_selection_changed` 不触发

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 恢复主控件默认 `Files`，同时恢复底部 `compact / read only` preview，并直接输出首帧
2. 切到主区 `Review`
3. 输出第二组主区帧
4. 切到主区 `Archive`
5. 输出第三组主区帧
6. 恢复主区默认 `Files`
7. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/master_detail --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/master_detail
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_master_detail
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Files / Review / Archive` 三组可识别状态。
- 底部 `compact / read only` preview 必须在全程 runtime 帧里保持静态一致。
- 静态 preview 输入后不能改变 `current_index`、布局区域或 listener 状态。
- README、demo 录制轨道、单测入口与验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_master_detail/default`
- 复核目标：
  - 主区裁剪后只出现 `3` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里强调“当前选中项 + detail pane”的同步关系，不承担导航容器职责。
- 相比 `settings_panel`：这里没有 value cell、switch、chevron 语义。
- 相比 `list` / `data_grid`：这里保留条目驱动详情的双栏联动，而不是单纯列表展示。
