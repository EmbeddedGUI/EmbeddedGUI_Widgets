# ThumbRate 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`ThumbRate`
- 当前保留形态：`none`、`liked`、`disliked`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release`、再次点击当前项回到 `none`、`Left / Right / Home / End / Tab` 焦点移动、`Enter / Space` 提交与 `Escape` 清空；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：hover 动画、visited 态、额外文案区块、系统图标依赖、场景化 showcase 装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：仓库内没有现成的 thumb 图标资源，本控件继续在 custom 层自绘抽象化 `thumb up / thumb down` 图形，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`ThumbRate` 适合表达“有帮助 / 没帮助”这类二选一反馈，同时保留未投票的中立态。它比星级评分更轻量，也比普通按钮更明确地表达“反馈”而非“执行命令”。

## 2. 为什么现有控件不够用？
- `button` / `hyperlink_button` 更偏向动作触发，不带三态反馈语义。
- `toggle_button` 只有单个开关面，无法自然表达 `like / dislike` 两个互斥选项。
- `rating_control` 面向离散等级，不适合“正负反馈”这种双分支场景。

## 3. 当前页面结构
- 标题：`ThumbRate`
- 主区：一个带标题与说明文案的 `ThumbRate` 反馈面板
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `liked`
- 右侧 preview：`read only`，固定显示 `disliked`

目录：
- `example/HelloCustomWidgets/input/thumb_rate/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，最终稳定帧回到默认快照；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   标题：`Release note helpful?`
   状态：`none`
   说明：`No vote recorded yet.`
2. 快照 2
   标题：`Release note helpful?`
   状态：`liked`
   说明：`Marked as helpful.`
3. 快照 3
   标题：`Setup article helpful?`
   状态：`disliked`
   说明：`Setup guidance needs work.`
4. 快照 4
   标题：`Setup article helpful?`
   状态：`none`
   说明：`Feedback is still pending.`
5. 最终稳定帧
   标题：`Release note helpful?`
   状态：`none`
   说明：`No vote recorded yet.`

底部 preview 在整条轨道中始终固定：

1. `compact`
   状态：`liked`
2. `read only`
   状态：`disliked`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 176`
- 主面板：`196 x 96`
- 主控件：`172 x 52`
- 底部容器：`200 x 36`
- 单个 preview：`96 x 36`
- 页面结构：标题 -> 主反馈面板 -> 底部 `compact / read only`
- 风格约束：保持 Fluent 风格的浅灰页面、白色表面和轻阴影；`liked` 使用冷色强调，`disliked` 使用警示红色强调；`compact` 隐去标签，只保留两段式 thumb 反馈；`read only` 保留状态可见性，但降低对比度并吞掉输入

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 `none` | 是 | 否 | 否 |
| `liked` 已选择 | 是 | 是 | 否 |
| `disliked` 已选择 | 是 | 否 | 是 |
| 清空后的 `none` | 是 | 否 | 否 |
| 最终稳定帧回到默认快照 | 是 | 否 | 否 |
| `same-target release` | 是 | 否 | 否 |
| `Left / Right / Home / End / Tab` 切换焦点项 | 是 | 否 | 否 |
| `Enter / Space` 提交当前焦点项 | 是 | 否 | 否 |
| `Escape` 清空当前选择 | 是 | 否 | 否 |
| 静态 preview 吞 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_thumb_rate.c` 当前覆盖 `8` 条用例：

1. 样式 helper 更新标志位并清理 `pressed`。
   覆盖 `apply_compact_style()` 与 `apply_read_only_style()` 对 `compact_mode`、`read_only_mode`、palette 与 `pressed_part` 的处理。
2. setter 清理 `pressed` 并做归一化。
   覆盖 `set_labels()`、`set_palette()`、`set_state()` 与 `set_current_part()`，要求非法状态回落到 `none`，非法 part 不改现值。
3. 触摸点击提交，并支持再次点击当前项回到 `none`。
   覆盖 `none -> liked -> none` 的 `same-part toggle off` 闭环。
4. `same-target release` 只有回到原目标才提交。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及 `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交。
5. 键盘导航与提交。
   覆盖 `Right` 切到 `Dislike`、`Space` 提交、`Home` 回到 `Like`、`Enter` 再次提交的焦点与状态闭环。
6. `Escape` 清空当前状态。
   覆盖从 `disliked` 回到 `none`，并保留当前 part 语义。
7. disabled / read only 清理 `pressed` 且不提交。
   覆盖只读与禁用分支下的 `touch / key` 输入不会改写状态。
8. static preview 吞输入且保持状态不变。
   覆盖 preview 侧 `touch` 与 `Enter` 输入只清理 `is_pressed / pressed_part`，不改写 `state`。

说明：
- 主区继续保留真实 `ThumbRate` 交互语义：触摸遵循 `same-target release`，再次点击当前已选项会回到 `none`。
- 键盘 `Left / Right / Home / End / Tab` 只移动当前焦点项，不直接改值；`Enter / Space` 提交当前焦点项；`Escape` 清空当前选择。
- 底部 `compact / read only` preview 统一通过 `egui_view_thumb_rate_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主控件默认快照、同步底部双静态 preview、聚焦主控件并请求首帧截图，等待 `THUMB_RATE_RECORD_FRAME_WAIT`。
2. 真实触摸点击 `Like`，等待 `THUMB_RATE_RECORD_WAIT`。
3. 请求 `liked` 帧，等待 `THUMB_RATE_RECORD_FRAME_WAIT`。
4. 切到第二组文案，并通过真实 `Right + Space` 键盘闭环提交 `Dislike`，等待 `THUMB_RATE_RECORD_WAIT`。
5. 请求 `disliked` 帧，等待 `THUMB_RATE_RECORD_FRAME_WAIT`。
6. 再次按 `Space` 清空当前 `Dislike`，等待 `THUMB_RATE_RECORD_WAIT`。
7. 请求清空后的 `none` 帧，等待 `THUMB_RATE_RECORD_FRAME_WAIT`。
8. 显式恢复默认快照并恢复主控件 focus，等待 `THUMB_RATE_RECORD_FINAL_WAIT`。
9. 请求最终稳定帧，并继续等待 `THUMB_RATE_RECORD_FINAL_WAIT`。

说明：
- 录制阶段保留主区真实 `touch / key` 轨道，不再通过 preview 触发清焦或额外页面桥接。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：保留 `THUMB_RATE_RECORD_WAIT`、`THUMB_RATE_RECORD_FRAME_WAIT`、`THUMB_RATE_RECORD_FINAL_WAIT`、`apply_primary_default_state()`、`focus_primary_rate()` 与显式布局重放路径。
- `set_state / set_current_part / key dispatch / touch dispatch / snapshot request` 都先走显式布局路径，保证四组主区状态与最终稳定帧口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/thumb_rate PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/thumb_rate --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/thumb_rate
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_thumb_rate
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不黑屏、不白屏、不裁切。
- `none / liked / disliked / none` 四组 reference 状态必须能从截图中稳定区分。
- 触摸跨目标移动时不得错误提交。
- 最终稳定帧必须显式回到默认快照。
- `compact` 与 `read only` preview 必须保持静态 reference，不响应真实输入。
- WASM demo 必须能以 `HelloCustomWidgets_input_thumb_rate` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_thumb_rate/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界收敛到 `(44, 139) - (380, 245)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 246` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `button` / `hyperlink_button`：这里表达的是双分支反馈值，不是命令触发。
- 相比 `toggle_button`：这里保留 `like / dislike / none` 三态闭环，而不是单一开关。
- 相比 `rating_control`：这里是轻量正负反馈，不是多档等级评分。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `none`
  - `liked`
  - `disliked`
  - 清空后的 `none`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - `same-target release`
  - 再次点击当前项回到 `none`
  - `Left / Right / Home / End / Tab` 焦点移动
  - `Enter / Space` 提交
  - `Escape` 清空
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - hover 动画和 visited 态
  - 额外文案区块与场景化 showcase 装饰
  - 系统 thumb 图标资源依赖
  - 让 preview 承担清焦或状态桥接的旧链路
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/thumb_rate PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `thumb_rate` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/thumb_rate --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_thumb_rate/default`
  - 共捕获 `11` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/thumb_rate`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_thumb_rate`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1342 colors=179`
- 截图复核结论：
  - 主区覆盖默认 `none`、`liked`、`disliked` 与清空后的 `none` 四组 reference 状态
  - 最终稳定帧已显式回到默认快照
  - 主区 RGB 差分边界收敛到 `(44, 139) - (380, 245)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 246` 裁切后全程保持单哈希静态
