# Pivot 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 平台语义参考：`WinUI 3 Pivot`
- 次级补充参考：`WPF UI`、`ModernWpf`
- 对应组件：`Pivot`
- 当前保留形态：`Overview`、`Activity`、`History`、`Compact`、`Read only`
- 当前保留交互：主区保留 `same-target release`、`Left / Right / Up / Down / Home / End / Tab / Enter / Space` 键盘语义与真实 section 切换；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：旧 `compact_panel / read_only_panel` 包装、标题说明文案、preview 轮换、录制里的 preview 切换桥接、手势翻页动画、场景化页面 chrome，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `navigation/pivot`，底层仍复用仓库内现有 `hcw_pivot` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`pivot` 用来表达“顶部 header 负责切换分区，主体区域一次只展示一个内容页”的导航语义，适合总览、活动、历史这类平级 section 的轻量切换。它比 `tab_view` 更轻，不承担页签管理；又比单纯的选择条更完整，因为它需要把当前 body 内容一起收口。

## 2. 为什么现有控件不够用
- `tab_strip` 只覆盖 header 切换，不承载单页 body 区域。
- `selector_bar` 更偏向选择入口，缺少 `header + body` 一体化结构。
- `flip_view` 强调顺序翻页，不表达顶部 header 导航语义。
- `tab_view` 语义更重，偏桌面页签容器，不适合轻量 section 切换。

## 3. 当前页面结构
- 标题：`Pivot`
- 主区：一个可真实交互的 `primary_pivot`
- 底部：两个并排的静态 preview
- 左侧 preview：`compact`，固定显示 `Home`
- 右侧 preview：`read only`，固定显示 `Audit`

目标目录：
- `example/HelloCustomWidgets/navigation/pivot/`

## 4. 主区 reference 快照
主控件录制轨道已经收口为 3 组 reference 快照和最终稳定帧：

1. `Overview`
   `Core view / Project overview / Goals, owner and next step.`
2. `Activity`
   `Daily feed / Recent activity / Ship notes and open reviews.`
3. `History`
   `Past work / Change history / Milestones and archived updates.`
4. `Overview`
   回到默认 `Core view / Project overview / Goals, owner and next step.`

底部 preview 在整条轨道中保持固定：

1. `compact`
   `Home / Quick / Pinned work`
   `compact_mode=1`
   `current_index=0`
2. `read only`
   `Audit / Static / Read only`
   `compact_mode=1`
   `read_only_mode=1`
   `current_index=1`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 206`
- 主控件尺寸：`196 x 108`
- 底部行容器尺寸：`216 x 72`
- 单个 preview 尺寸：`104 x 72`
- 页面结构：标题 -> 主 `pivot` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色主 surface、低噪音边框、轻量 active fill 与 underline

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | 是 | 是 | 是 |
| `Overview / Activity / History` 三态切换 | 是 | 否 | 否 |
| `DOWN(A) -> MOVE(B) -> UP(B)` 不提交 | 是 | 否 | 否 |
| `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交 | 是 | 否 | 否 |
| `Left / Right / Up / Down / Home / End / Tab` 键盘切换 | 是 | 否 | 否 |
| `Enter / Space` consume 但不切换 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |
| `read_only_mode / !enable` 先清 pressed 再拒绝输入 | 是 | 否 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_pivot.c` 当前覆盖 `4` 条用例：

1. 样式 helper 与 setter 状态清理。
   覆盖 `apply_compact_style()`、`apply_read_only_style()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_items()`、`set_current_index()` 对残留 `pressed` 的清理。
2. touch same-target release 与 cancel。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到 `A` 后才提交。
3. 键盘导航与 guard。
   覆盖 `Right / End / Home / Tab / Enter / Space`，以及 `read_only_mode / !enable` 下的拒绝输入与 `pressed` 清理。
4. static preview 不变性断言。
   通过 `pivot_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验 `region_screen / background / items / font / meta_font / on_changed / api / surface_color / border_color / text_color / muted_text_color / accent_color / card_surface_color / item_count / current_index / compact_mode / read_only_mode / pressed_index / alpha / enable / is_focused / is_pressed / padding` 保持不变。

说明：
- 主区真实交互继续保留 `same-target release`、section 切换和键盘导航闭环；`Enter / Space` 仍然 consume 但不触发切换。
- `read_only_mode / !enable`、样式 helper、setter 和 static preview 路径都统一要求先清理残留 `pressed_index / is_pressed`，再处理后续状态。
- 底部 `Compact / Read only` preview 统一通过 `hcw_pivot_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不触发 `on_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Overview`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `PIVOT_RECORD_FRAME_WAIT`。
2. 切到 `Activity`，等待 `PIVOT_RECORD_WAIT`。
3. 抓取 `Activity` 主区快照，等待 `PIVOT_RECORD_FRAME_WAIT`。
4. 切到 `History`，等待 `PIVOT_RECORD_WAIT`。
5. 抓取 `History` 主区快照，等待 `PIVOT_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Overview`，等待 `PIVOT_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `PIVOT_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区 `Overview / Activity / History` 的状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证默认态、两组切换态和最终稳定帧的布局口径一致。
- `apply_primary_default_state()` 与 `apply_preview_states()` 仅在 `ui_ready` 后触发布局，避免 root view 挂载前后出现布局口径分叉。
- README 这里按当前 `test.c` 如实保留 `PIVOT_RECORD_WAIT / PIVOT_RECORD_FRAME_WAIT / PIVOT_RECORD_FINAL_WAIT` 三档等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pivot PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pivot --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pivot
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pivot
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制必须清晰覆盖默认 `Overview`、`Activity`、`History` 三组状态，最终稳定帧必须回到默认 `Overview`。
- 主区真实交互仍需保留 `same-target release`、键盘切换和 `read_only / !enable` guard 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key`，不能改写 `current_index / compact_mode / read_only_mode`。
- 样式 helper、setter、guard 和 static preview 都必须统一遵守“先清理残留 `pressed_index / is_pressed` 再处理后续状态”的语义。
- WASM demo 必须能以 `HelloCustomWidgets_navigation_pivot` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_pivot/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(54, 121) - (425, 264)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 265` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Overview`

## 12. 与现有控件的边界
- 相比 `tab_strip`：这里补齐 body 区域，不只是 header 切换条。
- 相比 `selector_bar`：这里表达页内 section 切换，而不只是选择入口。
- 相比 `flip_view`：这里保留顶部 header 导航，不强调翻页手势。
- 相比 `tab_view`：这里不绑定页签壳层，也不处理 `close / add`。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Overview`
  - `Activity`
  - `History`
- 保留的底部对照：
  - `Compact`
  - `Read only`
- 保留的交互：
  - `same-target release`
  - 键盘 `Left / Right / Up / Down / Home / End / Tab / Enter / Space`
  - 主区 section 切换
- 删减的旧桥接与旧装饰：
  - `compact_panel / read_only_panel` 包装
  - preview 轮换
  - 录制里的 preview 切换桥接
  - 手势翻页动画
  - 场景化页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/pivot PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `pivot` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pivot --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_pivot/default`
  - 共捕获 `9` 帧
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - navigation `13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pivot`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pivot`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1581 colors=171`
- 截图复核结论：
  - 主区覆盖默认 `Overview`、`Activity`、`History`
  - 最终稳定帧显式回到默认 `Overview`
  - 主区 RGB 差分边界收敛到 `(54, 121) - (425, 264)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact / Read only` preview 以 `y >= 265` 裁切后全程保持单哈希静态
