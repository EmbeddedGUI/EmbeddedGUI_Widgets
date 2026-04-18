# parallax_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ParallaxView`
- 对应组件语义：`ParallaxView`
- 本轮保留语义：`Hero Banner / Pinned Deck / Quiet Layer / System Cards / compact / read only`
- 本轮移除内容：页面级 guide、状态文案、preview 点击桥接、旧录制轨道里的 compact preview 切换与 preview click 收尾动作
- EGUI 适配说明：继续复用仓库内 `layout/parallax_view` 控件实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 口径与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`parallax_view` 用来表达“前景内容滚动时，背景 hero 区域以更慢速度位移”的标准景深语义。它适合 onboarding、内容导览、长页面摘要和 dashboard hero 这类需要保留内容轨道与背景层级关系的场景。

## 2. 为什么现有控件不够用
- `split_view`、`master_detail` 强调双栏结构，不表达单卡内部的景深滚动。
- `flip_view` 强调分页切换，不表达连续 offset。
- `scroll_bar` 只表达滚动位置，不承担 hero depth 的视觉反馈。
- `card_panel` 更偏静态摘要卡，不处理前景 rows 与背景 hero 的联动。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `parallax_view` -> 底部 `compact / read only` 双静态 preview。
- 主区保留四组录制状态：
  - `Hero Banner`
  - `Pinned Deck`
  - `Quiet Layer`
  - `System Cards`
- 底部左侧是 `compact` 静态 preview，固定展示 `Depth Strip`。
- 底部右侧是 `read only` 静态 preview，固定展示 `Review Shelf`。
- 两个 preview 都通过 `egui_view_parallax_view_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / dispatch_key_event()`
  - 只清理残留 `pressed`
  - 不改 `offset / active_row / compact_mode / read_only_mode / row_count`
  - 不触发 `on_changed`

目标目录：`example/HelloCustomWidgets/layout/parallax_view/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 304`
- 主控件：`194 x 136`
- 底部对照行：`218 x 82`
- `compact` preview：`106 x 82`
- `read only` preview：`106 x 82`
- 视觉约束：
  - 使用浅色 Fluent 卡片，不回退到 showcase 式的重装饰 hero。
  - hero 区域保留低对比三层 background strips，通过 offset 和 vertical shift 表达景深。
  - 前景 rows 以低噪音列表卡呈现，active row 只保留轻量 tone bar 和弱化边框强调。
  - `read only` 同时满足视觉弱化与输入抑制。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 304` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Parallax View` | 页面标题 |
| `parallax_primary` | `egui_view_parallax_view_t` | `194 x 136` | `Hero Banner` | 主区标准 `ParallaxView` |
| `parallax_compact` | `egui_view_parallax_view_t` | `106 x 82` | `Depth Strip` | 紧凑静态 preview |
| `parallax_read_only` | `egui_view_parallax_view_t` | `106 x 82` | `Review Shelf` | 只读静态 preview |
| `primary_rows` | `egui_view_parallax_view_row_t[4]` | - | `Hero Banner / Pinned Deck / Quiet Layer / System Cards` | 主区录制轨道 |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Hero Banner` | 默认 reference 状态 |
| 主控件 | `Pinned Deck` | 验证 success tone 与中段 offset |
| 主控件 | `Quiet Layer` | 验证 neutral tone 与静态层次 |
| 主控件 | `System Cards` | 验证 warning tone 与尾态 offset |
| `compact` preview | `Depth Strip` | 固定静态对照，不随录制轨道变化 |
| `read only` preview | `Review Shelf` | 固定静态对照，不随录制轨道变化 |

## 7. 交互语义与单测要求
- 主控件继续保留真实 row 选中、offset 切换与键盘导航闭环。
- 单测覆盖：
  - `set_font / set_meta_font / set_content_metrics / set_compact_mode / set_read_only_mode` 的 pressed 清理语义
  - `touch` 选中、same-target release / cancel、`read only`、`!enable` 守卫
  - `Up / Down / Home / End / Plus / Minus` 键盘切换
  - 静态 preview 用例改为 “consumes input and keeps state”
- preview 键盘入口统一走 `dispatch_key_event()`，不再使用旧的 `on_key_event()` 直连路径。
- 静态 preview 用例必须验证：
  - 输入前后的 `offset / active_row / content_length / viewport_length / vertical_shift / line_step / page_step / compact_mode / read_only_mode / row_count` 保持不变
  - `content_region / hero_region / title_region / subtitle_region / progress_region / footer_region / row_regions` 保持不变
  - `pressed_row / is_pressed` 被清理
  - `on_changed` 不触发

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 恢复主控件默认 `Hero Banner`，同时恢复底部 `compact / read only` preview，并直接输出首帧
2. 切到主区 `Pinned Deck`
3. 输出第二组主区帧
4. 切到主区 `Quiet Layer`
5. 输出第三组主区帧
6. 切到主区 `System Cards`
7. 输出第四组主区帧
8. 恢复主区默认 `Hero Banner`
9. 输出最终稳定帧

录制只导出主区状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致。

## 9. 编译、单测、运行时与文档检查
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
- 主区和底部双 preview 必须完整可见，不能裁切、黑屏或白屏。
- 主区录制只允许出现 `Hero Banner / Pinned Deck / Quiet Layer / System Cards` 四组可识别状态。
- 底部 `compact / read only` preview 必须在全程 runtime 帧里保持静态一致。
- 静态 preview 输入后不能改变 `offset`、`active_row`、布局区域或 listener 状态。
- README、demo 录制轨道、单测入口与验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_parallax_view/default`
- 复核目标：
  - 主区裁剪后只出现 `4` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `split_view` / `master_detail`：这里是单卡内部的景深滚动，不是双栏布局。
- 相比 `flip_view`：这里是连续 offset 语义，不是分页翻页。
- 相比 `scroll_bar`：这里重点是 hero depth 反馈，不是标准滚动条输入。
- 相比 `card_panel`：这里强调 offset 驱动的层位移，而不是静态摘要卡。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `hero banner`
  - `pinned deck`
  - `quiet layer`
  - `system cards`
  - `compact`
  - `read only`
  - `offset / active row`
- 保留的交互：
  - same-target touch release
  - 键盘 `Up / Down / Home / End / Plus / Minus`
- 删除的装饰或桥接：
  - 页面级 guide 与状态文案
  - preview 点击桥接
  - `compact` preview 切换轨道
  - 录制里的 preview click 收尾动作

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `parallax_view` suite `12 / 12`
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
  - `11 frames captured -> runtime_check_output/HelloCustomWidgets_layout_parallax_view/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/parallax_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_parallax_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2359 colors=190`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,8,9,10]`、`[2,3]`、`[4,5]`、`[6,7]`
  - 主区变化边界保持在 `(57, 54) - (416, 232)`
  - 按 `y >= 233` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Hero Banner / Pinned Deck / Quiet Layer / System Cards` 四组 reference 状态，最终稳定帧已显式回到默认快照
