# ParallaxView 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 / ParallaxView`
- 对应组件：`ParallaxView`
- 当前保留形态：`Hero Banner`、`Pinned Deck`、`Quiet Layer`、`System Cards`、`Depth Strip`、`Review Shelf`
- 当前保留交互：主区保留真实 row same-target release、offset 切换与键盘 `Up / Down / Home / End / Plus / Minus` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview 点击桥接、`compact` preview 切换轨道、录制里的 `preview click` 收尾动作
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_parallax_view`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`parallax_view` 用来表达“前景内容滚动时，背景 hero 区域以更慢速度位移”的景深语义，适合 onboarding、内容导览、长页面摘要和 dashboard hero 这类需要同时保留内容轨道和背景层级关系的场景。

## 2. 为什么现有控件不够用
- `split_view`、`master_detail` 强调双栏结构，不表达单卡内部的景深滚动。
- `flip_view` 强调分页切换，不表达连续 offset。
- `scroll_bar` 只表达滚动位置，不承担 hero depth 的视觉反馈。
- `card_panel` 更偏静态摘要卡，不处理前景 rows 与背景 hero 的联动。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `parallax_view` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `4` 组录制快照：
  - `Hero Banner`
  - `Pinned Deck`
  - `Quiet Layer`
  - `System Cards`
- 录制最终稳定帧显式回到默认 `Hero Banner`。
- 底部左侧是 `Compact` 静态 preview，固定对照 `Depth Strip`。
- 底部右侧是 `Read only` 静态 preview，固定对照 `Review Shelf`。
- 两个 preview 统一通过 `egui_view_parallax_view_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `offset / active_row / content_length / viewport_length / vertical_shift / line_step / page_step / compact_mode / read_only_mode / row_count`
  - 不触发 `on_changed`

目标目录：
- `example/HelloCustomWidgets/layout/parallax_view/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Hero Banner`
2. 快照 2
   `Pinned Deck`
3. 快照 3
   `Quiet Layer`
4. 快照 4
   `System Cards`
5. 最终稳定帧
   回到默认 `Hero Banner`

底部 preview 在整条轨道中固定为：
1. `Depth Strip`
2. `Review Shelf`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 304`
- 主控件：`194 x 136`
- 底部 preview 行：`218 x 82`
- 单个 preview：`106 x 82`
- 页面结构：标题 -> 主 `parallax_view` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 卡片与低对比 hero strips；主区通过 offset 和 vertical shift 表达景深；前景 rows 维持低噪音列表层次；底部 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Hero Banner` | `Depth Strip` | `Review Shelf` |
| 快照 2 | `Pinned Deck` | 保持不变 | 保持不变 |
| 快照 3 | `Quiet Layer` | 保持不变 | 保持不变 |
| 快照 4 | `System Cards` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Hero Banner` | 保持不变 | 保持不变 |
| 主区 row touch / key 导航 | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_parallax_view.c` 当前覆盖 `12` 条用例：

1. `set_font()`、`set_meta_font()` 的 `pressed` 清理语义。
2. `set_content_metrics()` 与 `set_offset()` 的 clamp 行为，以及 `on_changed` 回调更新。
3. `active_row` 随 offset 变化的跟踪结果。
4. 键盘 `Up / Down / Home / End / Plus / Minus` 导航。
5. 触摸 same-target release 选中 row anchor，并更新 offset 与 listener。
6. `ACTION_UP` 落到异目标与 `ACTION_CANCEL` 时不提交，且清理 `pressed`。
7. `compact_mode` 切换时清理 `pressed`，并在紧凑态忽略 `touch / key` 输入；恢复后重新允许提交。
8. `read_only_mode` 直接忽略输入，不改 offset。
9. `read_only_mode` 切换时清理 `pressed`，解锁后恢复交互。
10. `!enable` 时忽略输入并清理 `pressed`，恢复后重新允许提交。
11. `get_row_region()` 可见性与越界返回。
12. static preview 吞掉 `touch / key`，并保持 `offset / active_row / content_length / viewport_length / vertical_shift / line_step / page_step / compact_mode / read_only_mode / row_count / content_region / hero_region / title_region / subtitle_region / progress_region / footer_region / row_regions` 不变，同时不触发 `on_changed`。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- static preview 用例同时验证 `pressed_row / is_pressed` 会被清理，但状态本身不被改写。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Hero Banner`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `PARALLAX_RECORD_FRAME_WAIT`。
2. 切到 `Pinned Deck`，等待 `PARALLAX_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `PARALLAX_RECORD_FRAME_WAIT`。
4. 切到 `Quiet Layer`，等待 `PARALLAX_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `PARALLAX_RECORD_FRAME_WAIT`。
6. 切到 `System Cards`，等待 `PARALLAX_RECORD_WAIT`。
7. 抓取第四组主区快照，等待 `PARALLAX_RECORD_FRAME_WAIT`。
8. 恢复主区默认 `Hero Banner`，同时重放底部 preview 固定状态，等待 `PARALLAX_RECORD_WAIT`。
9. 通过最终抓帧输出稳定的默认态，并继续等待 `PARALLAX_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `4` 组快照和最终稳定帧的布局口径一致。
- README 这里按当前 `test.c` 如实保留中间状态切换使用 `PARALLAX_RECORD_WAIT`、抓帧使用 `PARALLAX_RECORD_FRAME_WAIT`、最终抓帧使用 `PARALLAX_RECORD_FINAL_WAIT` 的等待口径；恢复默认态本身仍使用 `PARALLAX_RECORD_WAIT`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/parallax_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_parallax_view
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Hero Banner / Pinned Deck / Quiet Layer / System Cards` 四组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 row same-target release、键盘导航与 `on_changed` 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `offset / active_row / content_length / viewport_length / vertical_shift / line_step / page_step / compact_mode / read_only_mode / row_count`，也不能触发 `on_changed`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_parallax_view` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_parallax_view/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区唯一状态分组：`[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界：`(57, 54) - (416, 232)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 233` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Hero Banner`

## 12. 与现有控件的边界
- 相比 `split_view`、`master_detail`：这里是单卡内部的景深滚动，不是双栏布局。
- 相比 `flip_view`：这里是连续 offset 语义，不是分页翻页。
- 相比 `scroll_bar`：这里重点是 hero depth 反馈，不是标准滚动条输入。
- 相比 `card_panel`：这里强调 offset 驱动的层位移，而不是静态摘要卡。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Hero Banner`
  - `Pinned Deck`
  - `Quiet Layer`
  - `System Cards`
- 保留的底部对照：
  - `Depth Strip`
  - `Review Shelf`
- 保留的交互：
  - row same-target release
  - 键盘 `Up / Down / Home / End / Plus / Minus`
- 删减的旧桥接与旧轨道：
  - preview 点击桥接
  - `compact` preview 切换轨道
  - 录制里的 `preview click` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `parallax_view` suite `12 / 12`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_parallax_view/default`
  - 共捕获 `11` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/parallax_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_parallax_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2359 colors=190`
- 截图复核结论：
  - 主区覆盖 `Hero Banner / Pinned Deck / Quiet Layer / System Cards` 四组 reference 状态
  - 最终稳定帧显式回到默认 `Hero Banner`
  - 主区 RGB 差分边界收敛到 `(57, 54) - (416, 232)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact / Read only` preview 以 `y >= 233` 裁切后全程保持单哈希静态
