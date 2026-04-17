# Skeleton 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI Skeleton`
- 对应组件名：`Skeleton`
- 当前保留状态：`article`、`feed`、`settings`、`compact`、`read only`
- 当前移除内容：preview 动画、preview snapshot 切换、preview 清主控件 focus 的桥接逻辑、与 reference 页面无关的额外交互录制
- EGUI 适配说明：继续使用仓库内 `skeleton` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`skeleton` 用于在真实内容尚未到达时先表达布局结构、信息密度和重点区域，比单纯的 `spinner` 更适合文章、列表和设置页的加载占位。仓库当前已经收口到 `Fluent 2 / WPF UI` 主线，因此仍需要一版浅色、低噪音、结构清晰的 `Skeleton` reference 页面。

## 2. 为什么现有控件不够用
- `spinner` 只能表达“正在加载”，不能表达内容将如何排布。
- `progress_bar` 适合数值进度，不适合页面级骨架占位。
- 旧版 `skeleton` preview 仍承担动画、snapshot 切换和焦点桥接职责，不符合当前 static preview workflow。
- 当前仓库仍需要一版贴近 Fluent / WPF UI 的标准 `Skeleton` 示例，用于与 `spinner`、`progress_bar` 形成明确边界。

## 3. 当前页面结构
- 标题：`Skeleton`
- 主区：一个主 `skeleton`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `skeleton` -> `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/skeleton/`

## 4. 主区 reference 轨道
主控件录制轨道只保留三组主区 snapshot 和最终稳定帧：

1. `Article`
   `Loading article`，默认文章骨架
2. `Feed`
   `Loading feed`，列表流骨架
3. `Settings`
   `Loading settings`，设置页骨架
4. `Article`
   回到默认文章骨架，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Compact row`
   紧凑静态 preview，无 footer、无动画
2. `read only`
   `Read only`
   只读弱化配色、无 footer、无动画

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 224`
- 主 `skeleton` 尺寸：`196 x 124`
- 底部容器尺寸：`216 x 60`
- 单个 preview 尺寸：`104 x 60`
- 页面结构：标题 -> 主 `skeleton` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色 skeleton surface、低噪音浅边框、柔和蓝色 emphasis 和清晰但克制的标题 / footer 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Article` snapshot | 是 | 否 | 否 |
| `Feed` snapshot | 是 | 否 | 否 |
| `Settings` snapshot | 是 | 否 | 否 |
| `animation_mode = WAVE` | 是 | 否 | 否 |
| `animation_mode = NONE` | 否 | 是 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `show_footer` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `skeleton` 仍是 display-only 的加载占位控件，但保留真实 `wave` 动画。
- 主区 snapshot 切换只由录制轨道程序化触发，不再绑定 preview 输入。
- `compact / read only` preview 统一通过 `egui_view_skeleton_override_static_preview_api()` 吞掉 `touch / key`，不再承担 snapshot 切换、焦点桥接或收尾动作。
- `compact / read only` preview 统一使用 `set_animation_mode(..., NONE)` 停止动画。
- `read only` preview 继续通过 `set_read_only_mode(..., 1)` 保持弱化语义，并确保 timer 不启动。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主控件默认 `Article` 和底部 preview 固定状态
2. 请求首帧
3. 切到 `Feed`
4. 请求第二帧
5. 切到 `Settings`
6. 请求第三帧
7. 回到默认 `Article` 并请求最终稳定帧

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自 `Article / Feed / Settings` 三组骨架和最终回到默认 `Article` 的稳定帧。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `SKELETON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和回到默认态的稳定收尾都统一走显式布局路径。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_skeleton.c` 当前覆盖八部分：

1. `set_snapshots()` 钳制与状态复位
2. `current_snapshot` guard、`emphasis_block`、字体、footer、palette、模式 setter 的 `pressed` 清理
3. attach / detach、tick 和内部 helper 覆盖
4. 触摸 same-target release 与 cancel 行为
5. 键盘 `Enter` 激活 click listener
6. `compact_mode` 切换后清理 `pressed` 并保留点击行为
7. `read_only_mode / !enable` guard
8. 静态 preview 不变性断言

其中静态 preview 用例通过 `skeleton_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / snapshots / font / on_click_listener / api / palette / snapshot_count / current_snapshot / emphasis_block / show_footer / compact_mode / read_only_mode / animation_mode / anim_phase / timer_started / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/skeleton
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_skeleton
```

## 11. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc`
- `HelloUnitTest`：`PASS`，在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `skeleton` suite `8 / 8`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_skeleton/default`
- feedback 分类 compile/runtime 回归：`PASS`
  compile `10 / 10`，runtime `10 / 10`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_skeleton`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1708 colors=67`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_skeleton/default`

- 总帧数：`9`
- 主区 RGB 差分边界：`(54, 109) - (229, 277)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`9`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `skeleton` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区出现 `9` 组唯一状态，对应 `Article / Feed / Settings` 三组骨架布局与其间的 `wave shimmer` 推进，最终稳定帧回到默认 `Article` 布局。
- 按 `y >= 278` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 13. 已知限制
- 当前版本继续使用固定 snapshot 数据，不接真实业务状态流。
- 当前 `wave` 是轻量 reference 动画，不追求更复杂的渐变带和骨架系统能力。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不扩展为通用列表骨架框架。

## 14. 与现有控件的边界
- 相比 `spinner`：这里表达内容结构占位，不只是等待态。
- 相比 `progress_bar`：这里不表达数值进度，而是页面布局骨架。
- 相比 `activity_ring`：这里表达内容加载占位，不表达任务进度。

## 15. EGUI 适配说明
- 主区继续使用 custom `skeleton` 绘制，不下沉到 SDK。
- 主区 `wave` 通过控件内部 timer 驱动，`read_only` 或 `animation_mode = NONE` 时停止 timer。
- preview 统一复用静态 preview API，吞掉输入并保持完整状态不变。
- README、demo、单测和 runtime 验收口径已经对齐到当前 reference workflow。
