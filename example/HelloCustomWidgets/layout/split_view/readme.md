# split_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / SplitView`
- 补充参考：`ModernWpf`
- 对应组件语义：`SplitView`
- 本轮保留语义：`Overview open / Overview compact / Review / Archive / compact / read only`
- 本轮移除内容：页面级 guide、状态文案、preview 点击桥接、旧录制轨道里的 compact preview 切换与 preview click 收尾动作
- EGUI 适配说明：继续复用仓库内 `split_view` 控件实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`split_view` 用来表达“可折叠导航 pane + detail 面板”的双栏结构。它适合设置中心、资料库、审核工作区这类需要左侧导航和右侧内容同屏存在的场景。

## 2. 为什么现有控件不够用
- `nav_panel` 只覆盖导航 rail，不承载 detail pane。
- `data_list_panel` 更偏单卡片列表，不等同于可折叠 pane 结构。
- `master_detail` 更偏主从阅读，不是 `SplitView` 的导航抽屉语义。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `split_view` -> 底部 `compact / read only` 双静态 preview。
- 主区保留四组录制状态：
  - `Overview open`
  - `Overview compact`
  - `Review`
  - `Archive`
- 底部左侧是 `compact` 静态 preview，固定展示紧凑态和收起 pane。
- 底部右侧是 `read only` 静态 preview，固定展示只读态。
- 两个 preview 都通过 `egui_view_split_view_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / dispatch_key_event()`
  - 只清理残留 `pressed`
  - 不改 `current_index / pane_expanded / compact_mode / read_only_mode`
  - 不触发 `on_selection_changed / on_pane_state_changed`

目标目录：`example/HelloCustomWidgets/layout/split_view/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主控件：`196 x 104`
- 底部对照行：`216 x 74`
- `compact` preview：`104 x 74`
- `read only` preview：`104 x 74`
- 视觉约束：
  - 保持浅底、白色 pane 卡片、低噪音边框和轻量 detail 面板的 Fluent 层级。
  - `warning / neutral` 只通过 tone 微差表达，不回退到 showcase 式高噪音效果。
  - 主控件继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
  - `item / pane / compact / read only / !enable` 切换后都不能残留旧的 `pressed` 高亮。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Split View` | 页面标题 |
| `panel_primary` | `egui_view_split_view_t` | `196 x 104` | `Overview open` | 主区标准 `SplitView` |
| `panel_compact` | `egui_view_split_view_t` | `104 x 74` | `Compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_split_view_t` | `104 x 74` | `Read only` | 只读静态 preview |
| `primary_states` | `split_view_state_t[4]` | - | `Overview open / Overview compact / Review / Archive` | 主区录制轨道 |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview open` | 默认 reference 状态 |
| 主控件 | `Overview compact` | 验证 pane 收起后的主区轨道 |
| 主控件 | `Review` | 验证 warning tone 和 detail 布局 |
| 主控件 | `Archive` | 验证 neutral tone 和归档态 |
| `compact` preview | `Compact` | 固定静态对照，不随录制轨道变化 |
| `read only` preview | `Read only` | 固定静态对照，不随录制轨道变化 |

## 7. 交互语义与单测要求
- 主控件继续保留真实 row 选择、pane toggle 和键盘导航闭环。
- 单测覆盖：
  - `set_items / set_current_index / set_pane_expanded / set_font / set_meta_font / set_palette / set_compact_mode / set_read_only_mode` 的 pressed 清理语义
  - same-target release / cancel 回归
  - `Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘切换
  - 静态 preview 用例改为 “consumes input and keeps state”
- preview 键盘入口统一走 `dispatch_key_event()`，不再使用旧的 `on_key_event()` 直连路径。
- 静态 preview 用例必须验证：
  - 输入前后的 `current_index / item_count / pane_expanded / compact_mode / read_only_mode` 保持不变
  - `content / pane / toggle / title / detail / rows` 区域保持不变
  - `show_secondary_body / show_meta / draw_detail` 保持不变
  - `pressed_index / pressed_toggle / is_pressed` 被清理
  - `on_selection_changed / on_pane_state_changed` 不触发

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 恢复主控件默认 `Overview open`，同时恢复底部 `compact / read only` preview，并直接输出首帧
2. 切到主区 `Overview compact`
3. 输出第二组主区帧
4. 切到主区 `Review`
5. 输出第三组主区帧
6. 切到主区 `Archive`
7. 输出第四组主区帧
8. 恢复主区默认 `Overview open`
9. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/split_view PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/split_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/split_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_split_view
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Overview open / Overview compact / Review / Archive` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全程 runtime 帧里保持静态一致。
- 静态 preview 输入后不能改变 `current_index`、`pane_expanded`、布局区域或 listener 状态。
- README、demo 录制轨道、单测入口与验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_split_view/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里包含 detail pane，不只是导航 rail。
- 相比 `data_list_panel`：这里强调可折叠 pane，而不只是列表选择。
- 相比 `master_detail`：这里更接近 `SplitView` 的导航抽屉语义。
