# calendar_view 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 CalendarView`
- 开源母本：`WPF UI`
- 对应组件名：`CalendarView`
- 本轮保留语义：`standard / range preview / browse month / compact / read only`
- 本轮移除内容：页面级 guide、状态文案、`Standard` 标签、preview 标签点击桥接、旧录制轨道里的 `compact` 快照切换
- EGUI 适配说明：继续复用仓库内 `calendar_view` 基础实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`calendar_view` 用来表达“在常驻月历面板里浏览月份并选择日期区间”的标准语义，适合排期、冻结窗口、值班表、预订区间和交付窗口等场景。

## 2. 为什么现有控件不够用

- `date_picker` 更偏字段输入加弹出面板，不是常驻月历浏览。
- `time_picker` 只覆盖时间，不覆盖日期网格和月份切换。
- 普通文本字段不适合低噪音日期区间输入。
- 仓库里当前 `calendar_view` 页面虽然已经 reference 化，但录制轨道、静态 preview 单测和 README 仍保留旧口径，没有真正收口到当前 static preview 工作流。

## 3. 目标场景与页面结构

- 页面结构统一为：标题 -> 主 `calendar_view` -> 底部 `compact / read only` 双静态 preview。
- 主区只负责展示真实 `CalendarView` 的区间预览、月份浏览和快照切换。
- 底部 `compact` preview 固定显示 `May 05-08` 的紧凑对照，不再承担录制轨道切换职责。
- 底部 `read only` preview 固定显示 `Jul 18-22` 的只读对照，不再承担任何交互职责。
- 两个 preview 都通过 `egui_view_calendar_view_override_static_preview_api()` 收口：
  - 吞掉新增 `touch / dispatch_key_event()`
  - 收到输入时立即清理残留 `pressed_part / pressed_day / is_pressed`
  - 不改 `selection / display month / current_part / region_screen / palette / font`

目标目录：`example/HelloCustomWidgets/input/calendar_view/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 232`
- 主控件：`196 x 144`
- 底部对照行：`216 x 50`
- `compact` preview：`104 x 50`
- `read only` preview：`104 x 50`

视觉约束：

- 使用浅色 page panel、白色 surface 和轻边框，不回退到 showcase 式重装饰。
- 主控件保留 `CalendarView` 的标题、weekday header 和 6x7 日期网格层次。
- 区间继续通过起止日、高亮桥接和 today marker 表达，不额外叠加页面级说明卡片。
- 底部 `compact / read only` preview 必须是低噪音静态对照，不再承担切换或清焦职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 232` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Calendar View` | 页面标题 |
| `calendar_primary` | `egui_view_calendar_view_t` | `196 x 144` | `Mar 2026 / 09-13` | 主 `CalendarView` |
| `calendar_compact` | `egui_view_calendar_view_t` | `104 x 50` | `May 05-08` | 紧凑静态 preview |
| `calendar_read_only` | `egui_view_calendar_view_t` | `104 x 50` | `Jul 18-22` | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Booking window` / `Mar 2026 / 09-13` | 默认区间 |
| 主控件 | `Mar 2026 / 09-15` | `Enter + Right + Right` 后的区间预览态 |
| 主控件 | `Apr 2026 / 09-15` | 提交区间后 `+` 浏览下一月 |
| 主控件 | `Release freeze` / `Nov 2026 / 03-07` | 第二组主快照 |
| `compact` preview | `May 05-08` | 全程静态对照 |
| `read only` preview | `Jul 18-22` | 全程静态对照 |

## 7. 交互语义与单测口径

- 主控件继续保留真实 `touch` 双击提交区间、`Tab` 部件切换、方向键浏览、`Enter / Space` 区间编辑、`+ / -` 月份切换与 `Escape` 收口。
- `set_range()`、`set_display_month()`、`set_palette()`、`set_current_part()`、`set_compact_mode()` 与 `set_read_only_mode()` 必须在切换时清理残留 `pressed`。
- 静态 preview 用例统一收口为 “consumes input and keeps state”。
- preview 键盘入口统一走 `dispatch_key_event()`，不再直接调用旧的 `on_key_event()`。
- 静态 preview 用例必须验证：
  - `selection year / month / start / end` 不变
  - `display year / month / current_part / compact_mode / read_only_mode` 不变
  - `region_screen / palette / font / meta_font` 不变
  - `pressed_part / pressed_day / is_pressed` 被清理

## 8. 录制动作设计

`egui_port_get_recording_action()` 的 reference 轨道顺序如下：

1. 恢复主控件默认 `Booking window`，同时恢复底部 `compact / read only` 静态 preview，并输出首帧。
2. 发送 `Enter + Right + Right`，把主区区间预览扩展到 `09-15`。
3. 输出区间预览截图。
4. 发送 `Enter + Plus`，提交区间并浏览到 `Apr 2026`。
5. 输出月份浏览截图。
6. 切换到第二组主快照 `Release freeze`。
7. 输出第二组主快照截图。
8. 保持 `Release freeze` 不变，作为尾帧稳定等待。
9. 输出最终稳定帧。

录制只允许主区发生状态变化。底部 `compact / read only` preview 在整条 reference 轨道里必须保持静态一致。

## 9. 编译、单测、运行时与文档检查

```bash
make all APP=HelloCustomWidgets APP_SUB=input/calendar_view PORT=pc

# 修改 HelloUnitTest 后优先在 X:\ 短路径下 clean + rebuild
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/calendar_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/calendar_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_calendar_view
```

## 10. 验收重点

- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `默认 / 区间预览 / 月份浏览 / Release freeze` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持单一静态对照。
- 静态 preview 收到输入后，不能改 `selection / display month / current_part / region_screen / palette`。
- README、demo 录制轨道、单测断言与验收命令链必须保持一致。

## 11. runtime 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_input_calendar_view/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮罩主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 已知限制

- 当前仍是单月网格，不做 year / decade 层级切换。
- 当前区间仍限定在单月内，不做跨月连续选择。
- 暂不接入 locale 文案切换，只保留英文月份缩写和单字母 weekday。

## 13. 与现有控件的边界

- 相比 `date_picker`：这里是常驻月历视图，不是字段加弹出面板。
- 相比 `time_picker`：这里处理日期网格和月份浏览，不处理时分选择。
- 相比普通文本字段：这里不做自由日期文本输入。

## 14. EGUI 适配时的简化点与约束

- 直接复用 `calendar_view` 基础实现，避免在示例层重复搭状态桥。
- 用统一浅色 palette 收口到 `Fluent 2 / WPF UI` reference 方向。
- 主控件保留最小必要的真实触摸和键盘闭环，preview 不再承担切换或清焦职责。
- 先完成 reference 级 `CalendarView` 收口，再决定是否继续补跨月范围、年份跳转或更多选择模式。
