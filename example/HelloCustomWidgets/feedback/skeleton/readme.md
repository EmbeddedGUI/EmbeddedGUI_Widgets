# Skeleton 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI Skeleton`
- 对应组件名：`Skeleton`
- 当前保留形态：`Article`、`Feed`、`Settings`、`compact`、`read only`
- 当前保留交互：主区保留 `wave` 动画、`Article / Feed / Settings` 三组 snapshot 切换，以及 `set_snapshots / current_snapshot / emphasis_block / font / footer / palette / compact / animation / read_only` setter 统一清理 `pressed`；attach / detach / tick 继续覆盖 timer 生命周期；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变
- 当前移除内容：preview 动画、preview snapshot 切换、preview 清主控件 focus 的桥接逻辑、与 reference 页面无关的额外交互录制，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续使用仓库内 `skeleton` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测口径，不修改 `sdk/EmbeddedGUI`

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
- 左侧 preview：`compact`，固定显示 `Compact row`
- 右侧 preview：`read only`，固定显示 `Read only`
- 页面结构统一收口为：标题 -> 主 `skeleton` -> `compact / read only`

目录：
- `example/HelloCustomWidgets/feedback/skeleton/`

## 4. 主区 reference 快照
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

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_skeleton.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 钳制 snapshot 数量上限，并在重置数据后清理 `current_snapshot / pressed`。
2. `current_snapshot / emphasis_block / font / show_footer / palette / compact / animation / read_only` setter 覆盖 guard、默认字体回落、timer 状态切换与 `pressed` 清理。
3. attach / detach、tick 和内部 helper 覆盖 `clamp_snapshot_count`、`clamp_block_count`、`pulse_mix`、`disabled mix`、动画 phase 回绕和 timer 生命周期。
4. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 click；`ACTION_CANCEL` 只清理 `pressed`。
5. 键盘 `Enter` 继续触发 click listener。
6. `compact_mode` 切换会清理残留 `pressed`，但保留点击语义。
7. `read_only_mode / !enable` guard 会清理 `pressed`、停用 timer，并忽略后续 `touch / key` 输入。
8. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `region_screen / background / snapshots / font / on_click_listener / api / palette / snapshot_count / current_snapshot / emphasis_block / show_footer / compact_mode / read_only_mode / animation_mode / anim_phase / timer_started / alpha / enable / is_focused / is_pressed / padding`。

补充说明：
- 预览测试使用 `skeleton_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 做静态快照对比。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。
- `compact / read only` preview 统一通过 `egui_view_skeleton_override_static_preview_api()` 吞掉输入，不再承担 snapshot 切换、焦点桥接或收尾动作。
- `compact / read only` preview 统一使用 `set_animation_mode(..., NONE)` 停止动画，`read only` preview 额外通过 `set_read_only_mode(..., 1)` 保持 timer 不启动。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主控件默认 `Article` 和底部 preview 固定状态，请求首帧并等待 `SKELETON_RECORD_FRAME_WAIT`。
2. 切到 `Feed`，等待 `SKELETON_RECORD_WAIT`。
3. 请求第二帧并等待 `SKELETON_RECORD_FRAME_WAIT`。
4. 切到 `Settings`，等待 `SKELETON_RECORD_WAIT`。
5. 请求第三帧并等待 `SKELETON_RECORD_FRAME_WAIT`。
6. 回到默认 `Article`，等待 `SKELETON_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `SKELETON_RECORD_FINAL_WAIT`。

说明：
- 录制期间统一通过 `request_page_snapshot()` 触发布局、刷新和截图请求。
- 底部 `compact / read only` preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自 `Article / Feed / Settings` 三组骨架和最终回到默认 `Article` 的稳定帧。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：新增 `SKELETON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，默认态恢复与稳定收尾都统一走显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

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

## 10. 验收重点
- 主区与底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Article / Feed / Settings` 三组骨架布局必须能从截图中稳定区分。
- 主区 `wave shimmer` 要继续存在，但底部 preview 必须保持静态无动画。
- 最终稳定帧必须显式回到默认 `Article`，不能停在 `Feed` 或 `Settings`。
- `current_snapshot / emphasis_block / font / footer / palette / compact / animation / read_only` setter 清理 `pressed`、以及 attach / detach timer 生命周期不能回退。
- `read_only_mode / !enable` guard 与 static preview 对照需要和单测口径一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_feedback_skeleton/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区共出现 `9` 组唯一状态，对应 `Article / Feed / Settings` 三组骨架与其间 `wave shimmer` 推进
  - 主区 RGB 差分边界为 `(54, 109) - (229, 277)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 278` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Article`

## 12. 与现有控件的边界
- 相比 `spinner`：这里表达内容结构占位，不只是等待态。
- 相比 `progress_bar`：这里不表达数值进度，而是页面布局骨架。
- 相比 `activity_ring`：这里表达内容加载占位，不表达任务进度。

## 13. 本轮保留与删减
- 保留的主区状态：`Article`、`Feed`、`Settings`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：主区 `wave` 动画、attach / detach timer 生命周期、`same-target release` 点击语义、`read_only / !enable` guard、static preview 对照
- 删减的旧桥接与旧装饰：preview 动画、preview snapshot 切换、preview 清主控件 focus 的桥接逻辑、与 reference 页面无关的额外交互录制、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `skeleton` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=10 custom_skipped_allowlist=0`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_feedback_skeleton/default`
  - 共捕获 `9` 帧
- feedback 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category feedback --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64`
  - feedback `10 / 10` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/skeleton`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_skeleton`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=67`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区共出现 `9` 组唯一状态，对应 `Article / Feed / Settings` 三组骨架与其间 `wave shimmer` 推进
  - 主区 RGB 差分边界为 `(54, 109) - (229, 277)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 278` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Article`，底部 `compact / read only` preview 全程保持静态
