# Toolbar 自定义控件设计说明

## 参考来源
- 官方语义参考：`Fluent UI React / Toolbar`
- 参考设计体系：`Fluent 2`
- 对应组件：`Toolbar`
- 当前保留形态：嵌入式工具栏、icon+label 工具项、切换态工具项、`compact` 与 `read only` 静态 preview
- 当前保留交互：same-target release、键盘 `Left / Right / Home / End / Enter / Space`、单选切换组、静态 preview 输入吞掉
- 当前移除内容：多行 ribbon、命令溢出菜单、富文本编辑器专用格式面板、场景化工具说明
- EGUI 适配说明：目录使用 `input/toolbar`，只在 `HelloCustomWidgets` custom 层实现轻量 reference view，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`toolbar` 用来表达嵌入在内容区、编辑区或检查区里的低噪音工具集合。它比普通按钮更强调一组相关工具的顺序、当前工具、切换态和键盘 roving focus。

## 2. 为什么现有控件不够用
- `command_bar` 更偏页面级命令条和 overflow 语义，不适合嵌入式小工具组。
- `segmented_control` 更偏模式选择，缺少 icon+label 工具按钮与单个工具 action 的表达。
- `toggle_button` 只覆盖单个按钮，不覆盖工具组导航和 checked item 管理。

## 3. 当前页面结构
- 标题：`Toolbar`
- 主区：一个 `Edit / Find / Sync / Done` 四项工具栏
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 icon-only 工具组
- 右侧 preview：`read only`，固定显示锁定的工具组

目录：
- `example/HelloCustomWidgets/input/toolbar/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. `Edit`
2. `Find`
3. `Sync`
4. `Done`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`236 x 144`
- 主控件：`212 x 48`
- 底部 preview 行：`216 x 42`
- 单个 preview：`104 x 42`
- 风格约束：白色 toolbar surface、轻边框、浅蓝 checked fill、短 label 与 Material Symbols icon；compact preview 只显示图标，read only preview 使用弱化颜色。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 | `Edit` checked | 固定 `Edit` checked | 固定 `Open` checked |
| 触摸切换 | `Find` checked | 保持不变 | 保持不变 |
| 键盘切换 | `Sync` checked | 保持不变 | 保持不变 |
| 最终回落 | `Edit` checked | 保持不变 | 保持不变 |
| 静态 preview 吞输入 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_toolbar.c` 覆盖：

1. 文本拟合 helper 的边界行为。
2. `set_items / set_current_index / set_item_checked / set_item_disabled / set_fonts / set_palette / compact / read only` 清理 pressed。
3. `activate_item()` 的单选切换和 listener 回调。
4. `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后才提交。
5. 键盘 `Right / End / Enter / Space` 导航与提交。
6. `read only / !enable` guard 清理残留 pressed 并拒绝提交。
7. `compact / read only` 静态 preview 吞掉 touch / key 且状态不变。

## 8. 录制动作设计
`egui_port_get_recording_action()` 只驱动主区，底部 preview 保持静态：

1. 恢复默认 `Edit`，抓首帧。
2. 触摸 `Find`。
3. 抓取 `Find` 状态。
4. 键盘 `Right + Space` 激活 `Sync`。
5. 抓取 `Sync` 状态。
6. 键盘 `Right + Enter` 激活 `Done`。
7. 抓取 `Done` 状态。
8. 回到默认 `Edit` 并导出最终稳定帧。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/toolbar PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe toolbar

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/sync_widget_catalog.py --check
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toolbar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/toolbar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_toolbar
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Edit / Find / Sync / Done` 四组 reference 状态必须稳定可辨认。
- 工具项同目标释放必须严格成立。
- 键盘导航不能越过 disabled guard 后留下 pressed 污染。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 与现有控件的边界
- 相比 `command_bar`：`toolbar` 是嵌入式工具组，不承担页面级命令面板或 overflow。
- 相比 `segmented_control`：`toolbar` 保留工具 icon、action listener 与 roving focus。
- 相比 `toggle_button`：`toolbar` 管理多个相关工具项和单选 checked 状态。

## 12. 本轮保留与删减
保留：
- icon+label 工具项
- 单选 checked 状态
- same-target release
- 键盘 roving focus
- `compact / read only` 静态 preview

删减：
- 多行 ribbon
- overflow flyout
- 复杂编辑器格式命令
- 页面级说明 chrome

## 13. 当前验收结果
本轮新增后按 workflow 验收，结果记录在 `.claude/workflow/widget_progress_tracker.md`。
