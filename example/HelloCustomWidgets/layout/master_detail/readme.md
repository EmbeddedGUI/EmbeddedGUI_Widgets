# MasterDetail 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF UI / MasterDetail`
- 补充参考：`ModernWpf`
- 对应组件：`MasterDetail`
- 当前保留形态：`Files`、`Review`、`Archive`、`Files compact`、`Members read only`
- 当前保留交互：主区保留真实列表选中与键盘导航；底部 `compact / read only` preview 保持静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 点击桥接，以及旧录制轨道里的额外 preview 切换与收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/master_detail` 现有实现；本轮只收口 README、reference 录制说明、静态 preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`master_detail` 用于在同一块区域里同时承载“左侧条目选择”和“右侧详情阅读”。它适合文件列表、审核队列、成员清单、归档浏览这类需要先定位条目，再在当前页立即查看摘要的场景。

## 2. 为什么现有控件不够用
- `nav_panel` 解决的是导航入口，不强调当前条目与 detail pane 的同步关系。
- `settings_panel` 强调设置行与 value cell，不是 master-detail 双栏结构。
- `list` 与 `data_grid` 只负责列出条目，缺少同屏 detail 面板语义。
- `card_control` 更偏摘要卡，不承接左列驱动右侧详情的联动。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `master_detail` -> 底部 `compact / read only` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Files`
  - `Review`
  - `Archive`
- 录制最终稳定帧显式回到默认 `Files`。
- 底部左侧是 `compact` 静态 preview，固定展示 `Files compact`。
- 底部右侧是 `read only` 静态 preview，固定展示 `Members read only`。
- 两个 preview 统一通过 `egui_view_master_detail_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改变 `current_index / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed`

目标目录：
- `example/HelloCustomWidgets/layout/master_detail/`

## 4. 主区 reference 快照
主区录制轨道只保留 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Files`
2. 快照 2
   `Review`
3. 快照 3
   `Archive`
4. 最终稳定帧
   回到默认 `Files`

底部 preview 在整条轨道中固定为：
1. `Files compact`
2. `Members read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 208`
- 主控件：`196 x 96`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `master_detail` -> 底部 `compact / read only`
- 风格约束：使用浅灰 page panel 与白底低噪音 master-detail 容器；主区保留左侧 master list、中间分隔与右侧 detail pane 的标准层级；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或状态切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Files` | `Files compact` | `Members read only` |
| 快照 2 | `Review` | 保持不变 | 保持不变 |
| 快照 3 | `Archive` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Files` | 保持不变 | 保持不变 |
| 主区真实选中与键盘导航 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_master_detail.c` 当前覆盖 `9` 条用例：

1. `set_items()` 的 clamp、`current_index` 回退、listener guard 与空数据回退。
2. `set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都要先清理 `pressed`。
3. metrics、hit testing 与 compact 布局下的 master/detail 区域口径。
4. 主区真实 `touch` 选中、`cancel` 取消与 `read_only / !enable` 守卫。
5. `compact_mode` 切换后清掉旧 `pressed`，但不破坏选中行为。
6. 键盘 `Left / Right / Up / Down / Home / End / Tab` 导航与 `read_only / !enable` 守卫。
7. `read_only_mode` 切换会清理残留 `pressed`，并屏蔽后续 `touch / key`。
8. view disabled 会清理残留 `pressed`，并屏蔽后续 `touch / key`。
9. static preview 吞掉 `touch / key`，且保持 `current_index / item_count / compact_mode / read_only_mode / row_regions` 不变，不触发 `on_selection_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Files`，同时重放底部 `compact / read only` preview 固定状态。
2. 抓取默认主区首帧。
3. 切到 `Review` 并抓帧。
4. 切到 `Archive` 并抓帧。
5. 恢复主区默认 `Files`，同时重放底部 preview 固定状态。
6. 等待 `MASTER_DETAIL_RECORD_FINAL_WAIT` 后抓取最终稳定帧。

说明：
- 主区继续保留真实列表选中与键盘导航语义，供手动复核与单测覆盖。
- runtime 录制阶段不再真实发送底部 preview 输入，也不再保留额外的 preview 切换或收尾动作。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照与最终稳定帧的布局口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板，主区首轮切换与最终稳定抓帧统一走 `MASTER_DETAIL_RECORD_FINAL_WAIT`，中间状态切换仍保留 `MASTER_DETAIL_RECORD_WAIT / MASTER_DETAIL_RECORD_FRAME_WAIT`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc

# 在 X:\ 短路径下执行
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
- 主区与底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Files`、`Review`、`Archive` `3` 组可识别状态，且最终稳定帧必须回到默认态。
- 主区继续保留真实列表选中与键盘导航语义。
- `selection / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮或位移。
- 底部 `compact / read only` preview 必须在整条 runtime 轨道里保持静态一致。
- static preview 输入后不能改动 `current_index`、布局区域或 listener 状态。
- WASM demo 必须能以 `HelloCustomWidgets_layout_master_detail` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_master_detail/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区差分边界：`(50, 116) - (430, 250)`
  - 以 `y >= 250` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Files`

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里强调“当前选中项 + detail pane”的同步关系，不承接导航容器职责。
- 相比 `settings_panel`：这里没有 value cell、switch、chevron 语义。
- 相比 `list / data_grid`：这里保留条目驱动详情的双栏联动，而不是单纯列表展示。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Files`
  - `Review`
  - `Archive`
- 保留的底部对照：
  - `Files compact`
  - `Members read only`
- 保留的交互：
  - 主区条目选中
  - 键盘 `Left / Right / Up / Down / Home / End / Tab`
- 删减的旧桥接与旧轨道：
  - 页面级 guide 与状态说明
  - preview 点击桥接
  - 旧录制轨道里的额外 preview 切换与收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/master_detail PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `master_detail` suite `9 / 9`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/master_detail --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_master_detail/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/master_detail`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_master_detail`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1586 colors=194`
- 截图复核结论：
  - 主区覆盖 `Files / Review / Archive` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Files`
  - 主区差分边界收敛到 `(50, 116) - (430, 250)`
  - 底部 `compact / read only` preview 全程保持静态一致
