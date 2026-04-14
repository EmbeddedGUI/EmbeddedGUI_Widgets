# list 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React List`
- 对应组件语义：`List`
- 本次保留语义：`Inbox / Review / Archive / compact / read only / selection focus`
- 本次删除内容：旧录制末尾额外选中第 3 行的收尾态、旧单测 `on_key_event` 注入路径、与当前 static preview 工作流不一致的旧 README 结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/list`，公开 C API 保持 `egui_view_reference_list_*`，本轮只收口 `reference` 页面结构、录制轨道、单测入口和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`list` 用来表达“同一组轻量条目按单列顺序排列，并保留当前项焦点与选择语义”的标准列表结构。它适合消息队列、审核清单、归档入口、资源面板侧栏和小规模任务摘要这类需要快速浏览与切换当前项的场景。

## 2. 为什么现有控件不够用
- `data_list_panel` 更偏页面级摘要列表，不适合轻量单列选择组件。
- `items_repeater` 只承担重复布局宿主，不负责当前项和列表输入闭环。
- `settings_panel`、`master_detail` 更偏设置页和主从布局，不适合承载小型列表本体。
- SDK 自带 `List` 更偏基础能力，本仓库仍需要一个 Fluent 2 reference 风格的自定义列表页面、单测和 web 验收入口。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `list` -> 底部 `compact / read only` 双静态 preview。
- 主控件负责导出三组主区状态：
  - `Inbox`
  - `Review`
  - `Archive`
- 底部左侧是 `compact` 静态 preview，固定展示压缩行高与弱化 meta。
- 底部右侧是 `read only` 静态 preview，固定展示只读弱化状态。
- 两个 preview 统一通过 `egui_view_reference_list_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `current_index / compact_mode / read_only_mode`
  - 不触发 selection listener

目标目录：`example/HelloCustomWidgets/layout/list/`

## 4. 视觉与布局规格
- 根布局：`224 x 224`
- 主控件：`196 x 112`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 页面结构：标题 + 主区 + 底部双 preview
- 风格约束：
  - 保持浅色 Fluent 容器、低噪音分隔线和轻量 tone 差异。
  - 主区保留 `title / meta / badge` 三层信息，不叠加额外 snapshot header/footer。
  - 底部 preview 全程静态，不再承担场景切换或额外交互职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_list` | `egui_view_reference_list_t` | `196 x 112` | `Inbox` | 主 `List` |
| `compact_list` | `egui_view_reference_list_t` | `104 x 72` | `Compact` | 紧凑静态 preview |
| `read_only_list` | `egui_view_reference_list_t` | `104 x 72` | `Read only` | 只读静态 preview |
| `primary_snapshots` | `list_demo_state_t[3]` | - | `Inbox / Review / Archive` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Inbox` | 默认状态，accent 选中项 |
| 主控件 | `Review` | warning 选中项 |
| 主控件 | `Archive` | warning / neutral 混合条目 |
| `compact` preview | `Compact` | 固定静态对照，验证紧凑行高 |
| `read only` preview | `Read only` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实列表输入闭环：
  - `Up / Left`：上一项
  - `Down / Right`：下一项
  - `Home / End`：首项 / 末项
  - `Tab`：循环到下一项
  - `Enter / Space`：只消费输入，不重复通知当前项
- 触摸遵循 same-target release：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_items()`、`set_current_index()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 之后都不能残留旧的 `pressed` 高亮。
- 主控件键盘入口统一走 `dispatch_key_event()`。
- `read only`、`!enable` 和空数据状态下：
  - 会先清理残留 `pressed`
  - 后续 `touch / dispatch_key_event()` 不会改变 `current_index / compact_mode / read_only_mode`
  - 不触发 selection listener
- static preview 用例必须验证：
  - 输入被消费
  - `current_index / compact_mode / read_only_mode` 保持不变
  - listener 不触发
  - `pressed_index / is_pressed` 被清理

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部双 preview，输出默认 `Inbox`
2. 切到 `Review`
3. 切到 `Archive`
4. 恢复默认主状态并输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致，不再包含额外 preview 状态切换，也不再保留旧的收尾尾帧。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/list PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/list --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/list
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_list
```

## 10. 验收重点
- 主控件三组主状态必须能直接看出当前项和 tone 变化。
- `same-target release / dispatch_key_event / read only / !enable / empty items / static preview` 全部通过单测。
- 两个 preview 必须完整可见，不能黑白屏、裁切或重叠，并且在所有 runtime 帧里保持静态一致。
- setter、guard 和 static preview 都必须统一遵守“先清理残留 `pressed` 再处理后续状态”的语义。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_list/default`
- 复核目标：
  - 主区存在 3 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `data_list_panel`：这里强调轻量单列列表，不是页面级摘要面板。
- 相比 `items_repeater`：这里保留当前项和输入语义，不只是模板重复器。
- 相比 `settings_panel`：这里不承担设置行布局，只保留列表本体。
- 相比 SDK 原生 `List`：这里是当前仓库维护的 Fluent 2 reference 页面与验收闭环。
