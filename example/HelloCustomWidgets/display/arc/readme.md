# arc 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`Arc`
- 当前保留形态：`standard`、`subtle`、`attention`
- 当前保留交互：主区保留程序化 reference snapshot 切换；底部 `subtle / attention` 统一收口为静态 preview
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、preview 输入桥接、录制阶段额外恢复帧
- EGUI 适配说明：继续复用 custom 层 `egui_view_arc`，不修改 `sdk/EmbeddedGUI`

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
- 底部：一行并排的两个静态 preview
- 左侧 preview：`subtle`，固定显示 `24%`
- 右侧 preview：`attention`，固定显示 `72%`

目录：
- `example/HelloCustomWidgets/display/arc/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照和最终稳定帧：

1. 默认态
   value：`32%`
   palette：蓝色主线
2. 快照 2
   value：`58%`
   palette：琥珀提醒色
3. 快照 3
   value：`86%`
   palette：绿色完成态
4. 最终稳定帧
   value：`32%`
   palette：蓝色主线

底部 preview 在整条轨道中始终固定：
1. `subtle`
   value：`24%`
2. `attention`
   value：`72%`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 190`
- 标题：`224 x 18`
- 主弧线：`76 x 76`
- 数值标签：`224 x 14`
- 底部 preview 行：`92 x 42`
- 单个 preview arc：`42 x 42`
- 页面结构：标题 -> 主 `arc` -> 数值 label -> 底部 `subtle / attention`
- 风格约束：浅色 page panel、低噪音轨道、清晰的主区 palette 切换，不回退到 showcase 式说明卡片

## 6. 状态矩阵
| 状态 | 主控件 | Subtle preview | Attention preview |
| --- | --- | --- | --- |
| 默认显示 | `32%` | `24%` | `72%` |
| 快照 2 | `58%` | 保持不变 | 保持不变 |
| 快照 3 | `86%` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `32%` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `58%`
4. 抓取第二组主区快照
5. 切到 `86%`
6. 抓取第三组主区快照
7. 回到默认 `32%`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径
- 页面层不再保留主区说明 note 和 preview 辅助文案
- 底部 preview 统一通过 `egui_view_arc_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担页面桥接职责

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `ARC_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_arc.c` 当前覆盖两部分：

1. 主控件几何、样式与 setter 状态守卫
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_attention_style()`、`set_value()`、`set_angles()`、`set_stroke_width()` 与 `set_palette()` 的 clamp 和 `pressed` 清理
2. 静态 preview 不变性断言
   通过 `arc_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`track_color`、`active_color`、`value`、`stroke_width`、`start_angle`、`sweep_angle`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 当前 `arc` preview 用例继续收口为 “consumes input and keeps state”
- 为兼容 `HelloUnitTest` 当前 harness，`send_touch_to_view()` 与 `dispatch_key_event_to_view()` 直接调用 `on_touch_event()` / `on_key_event()`，避免 `dispatch_*` 口径下挂起

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

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/arc PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `arc` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_arc/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_arc`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1488 colors=154`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_arc/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(188, 129) - (292, 265)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 266` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `arc` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应 `32% / 58% / 86%` 三组主区快照；最终稳定帧已显式回到默认 `32%`。
- 按 `y >= 266` 裁切底部 preview 区域后保持单哈希，确认 `subtle / attention` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖固定百分比轨道，不接业务数据流。
- 当前不实现圆心内嵌文本、hover 或复杂动画轨道。
- 底部 `subtle / attention` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `activity_ring`：这里表达确定性百分比，不做持续旋转。
- 相比 `progress_bar`：这里强调环形占位和卡片级展示，不做线性条形语义。
- 相比 SDK `arc_slider`：这里是只读展示控件，不承担拖拽输入职责。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_arc` custom view，不修改 SDK。
- 主区保留 `32%`、`58%`、`86%` 三组 reference 快照。
- 底部 preview 通过 `egui_view_arc_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制无多余页面 chrome，再评估是否需要扩展到更多 arc 变体。
