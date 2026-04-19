# arc 自定义控件设计说明

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充语义参考：`WinUI Arc / Progress visual`
- 对应组件：`Arc`
- 当前保留形态：主区 `32%`、`58%`、`86%` 三组 reference 状态，以及底部 `subtle / attention` 静态 preview
- 当前保留交互：主区保留程序化 reference snapshot 切换；`style / value / angles / stroke / palette` setter 统一清理 `pressed`；底部 `subtle / attention` 继续作为静态 preview，对输入只吞不改状态
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、preview 输入桥接、录制阶段额外恢复帧与 showcase 式说明卡片
- EGUI 适配说明：继续复用 custom 层 `egui_view_arc`，保持只读展示语义和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`arc` 用来表达轻量、只读、非阻塞的环形进度或占比状态。它适合放在设置页、摘要卡片、同步概览和发布检查流中，用比 `progress_bar` 更紧凑的方式承载确定性百分比。

## 2. 为什么现有控件不够用

- `activity_ring` 更偏不确定、持续运行中的进度反馈，不适合稳定百分比。
- `progress_bar` 是线性信息条，不适合卡片级环形占位。
- SDK 自带 `arc_slider` 带有输入控件语义和拖拽 thumb，不符合 Fluent / WPF UI `Arc` 的只读展示语义。
- 当前主线仍需要一个与 `Fluent 2` 语义对齐的 `Arc` reference 页面、单测和 web 验收闭环。

## 3. 当前页面结构

- 标题：`Arc`
- 主区：一个标准 `arc` 和一个数值 label
- 底部：一行并排的两个真正静态的 preview
- 左侧 preview：`subtle`，固定显示 `24%`
- 右侧 preview：`attention`，固定显示 `72%`
- 页面结构统一收口为：标题 -> 主 `arc` -> 数值 label -> 底部 `subtle / attention`

目录：`example/HelloCustomWidgets/display/arc/`

## 4. 主区 reference 快照

主区录制轨道只保留 3 组程序化快照和最终稳定帧：

1. `32%`
   默认主状态，蓝色主线 palette
2. `58%`
   第 2 组主区快照，切到琥珀提醒色
3. `86%`
   第 3 组主区快照，切到绿色完成态
4. `32%`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中始终固定：

1. `subtle`
   `24%`
2. `attention`
   `72%`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 190`
- 标题：`224 x 18`
- 主弧线：`76 x 76`
- 数值标签：`224 x 14`
- 底部 preview 行：`92 x 42`
- 单个 preview arc：`42 x 42`
- 页面结构：标题 -> 主 `arc` -> 数值 label -> 底部 `subtle / attention`
- 页面风格：浅色 page panel、低噪音轨道、清晰的主区 palette 切换，不回退到 showcase 式说明卡片

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Subtle preview | Attention preview |
| --- | --- | --- | --- |
| `32%` | 是 | 否 | 否 |
| `58%` | 是 | 否 | 否 |
| `86%` | 是 | 否 | 否 |
| 最终稳定帧回到 `32%` | 是 | 否 | 否 |
| `24%` subtle | 否 | 是 | 否 |
| `72%` attention | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_arc.c` 当前覆盖 `3` 条用例：

1. `style helpers apply expected palette and geometry`
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_attention_style()` 的几何、palette 和 `pressed` 清理
2. `setters clamp and clear pressed state`
   覆盖 `set_value()`、`set_angles()`、`set_stroke_width()` 与 `set_palette()` 的 clamp 和 `pressed` 清理
3. `static preview consumes input and keeps state`
   通过 `arc_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`track_color`、`active_color`、`value`、`stroke_width`、`start_angle`、`sweep_angle`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变

补充说明：

- 主区 `arc` 是 display-first 的只读控件，不承接 click 语义，重点在 geometry、palette 和百分比状态
- 底部 `subtle / attention` preview 统一通过 `egui_view_arc_override_static_preview_api()` 吞掉 `touch / key`
- 为兼容 `HelloUnitTest` 当前 harness，preview 输入用例继续直接走 `on_touch_event()` / `on_key_event()`

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `32%` 和底部 preview 固定状态，请求首帧并等待 `ARC_RECORD_FRAME_WAIT = 170`
2. 切到 `58%`，等待 `ARC_RECORD_WAIT = 90`
3. 请求第二帧并继续等待 `170`
4. 切到 `86%`，等待 `90`
5. 请求第三帧并继续等待 `170`
6. 回到默认 `32%`，等待 `90`
7. 请求最终稳定帧，并继续等待 `ARC_RECORD_FINAL_WAIT = 280`

说明：

- 录制阶段最终会显式恢复主区默认态，并走统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径
- 页面层不再保留主区说明 note 和 preview 辅助文案
- 底部 preview 只负责静态 reference 对照，不再承担页面桥接职责

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/arc PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/arc --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/arc
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_arc
```

## 10. 验收重点

- 主区和底部 `subtle / attention` preview 必须完整可见，不能黑屏、白屏或裁切
- 主区 `32% / 58% / 86%` 三组 palette 与数值状态必须能从截图中稳定区分
- 最终稳定帧必须显式回到默认 `32%`
- `style / value / angles / stroke / palette` setter 收口后不能残留 `pressed`
- 底部 `24% / 72%` preview 必须保持静态 reference，对输入只吞不改状态

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_arc/default`
- 已归档复核结果：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界：`(188, 129) - (292, 265)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 266` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `32%`

## 12. 与现有控件的边界

- 相比 `activity_ring`：这里表达确定性百分比，不做持续旋转
- 相比 `progress_bar`：这里强调环形占位和卡片级展示，不做线性条形语义
- 相比 SDK `arc_slider`：这里是只读展示控件，不承担拖拽输入职责

## 13. 本轮保留与删减

- 保留的主区状态：`32%`、`58%`、`86%`
- 保留的底部对照：`subtle`、`attention`
- 保留的交互与实现约束：程序化 reference snapshot 切换、`style / value / angles / stroke / palette` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：主 panel / heading / note、preview panel / heading / body、preview 输入桥接、录制阶段额外恢复帧与 showcase 式说明卡片

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/arc PORT=pc`
  - 本轮沿用 `2026-04-18` 已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `arc` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 本轮重新执行文档编码与 display 类触摸语义检查，其余结果沿用已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/arc --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_arc/default`
  - 本轮沿用 `2026-04-18` 已归档 runtime 结果
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-18` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_arc`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1488 colors=154`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界：`(188, 129) - (292, 265)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 266` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `32% / 58% / 86%` 三组 reference 快照，最终稳定帧已回到默认 `32%`，底部 `subtle / attention` preview 全程静态
