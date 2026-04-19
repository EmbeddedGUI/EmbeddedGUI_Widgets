# badge 自定义控件设计说明

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 补充对照控件：`info_badge`、`badge_group`、`tag`
- 对应组件：`Badge`
- 当前保留形态：主区 `Verified`、`Preview`、`Needs review` 三组 reference 状态，以及底部 `compact / read only` 静态 preview
- 当前保留交互：主区保留程序化样式快照切换；文本、图标、palette、font 与 `compact / read_only` 模式 setter 统一清理 `pressed`；底部 `compact / read only` 继续作为静态 preview，对输入只吞不改状态
- 当前移除内容：主面板说明文案、底部 panel 包装与 heading、录制阶段额外恢复帧、preview 说明文字与额外页面 chrome
- EGUI 适配说明：继续复用当前目录下的 `egui_view_badge` custom view，保留 `filled / outline / subtle` 三组主区快照和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`badge` 用来表达附着在内容上的短状态或说明，例如审核状态、发布标记、预览标签和归档标识。仓库虽然已经有 `info_badge`、`badge_group` 和 `tag`，但还缺少一颗与 `Fluent 2 / Fluent UI React Badge` 语义对齐的单个文本 badge reference。

## 2. 为什么现有控件不够用

- `info_badge` 更偏计数、图标和 attention dot，不承载短文本状态。
- `badge_group` 更偏多 badge 汇总，不是单个附着式状态标签。
- `tag` 表达的是用户已选或可撤销的值，不适合承载系统生成的不可编辑状态。
- 当前主线仍需要一个与 `Fluent 2` 语义对齐的 `Badge` reference 页面、单测和 web 验收闭环。

## 3. 当前页面结构

- 标题：`Badge`
- 主区：一个标准 `badge`
- 底部：一行并排的两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Beta`
- 右侧 preview：`read only`，固定显示 `Archived`
- 页面结构统一收口为：标题 -> 主 `badge` -> 底部 `compact / read only`

目录：`example/HelloCustomWidgets/display/badge/`

## 4. 主区 reference 快照

主区录制轨道只保留 3 组程序化快照和最终稳定帧：

1. `Verified`
   图标：`Done`
   样式：`filled`
2. `Preview`
   图标：`Info`
   样式：`outline`
3. `Needs review`
   图标：`Warning`
   样式：`subtle`
4. `Verified`
   图标：`Done`
   样式：`filled`
   作为最终稳定帧

底部 preview 在整条轨道中始终固定：

1. `compact`
   文案：`Beta`
   图标：`Info`
2. `read only`
   文案：`Archived`
   图标：无

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 112`
- 主控件：`126 x 28`
- 底部 preview 行：`184 x 24`
- 单个 preview：`88 x 24`
- 页面结构：标题 -> 主 `badge` -> 底部 `compact / read only`
- 页面风格：浅色 page panel、低噪音边框、清晰的 `filled / outline / subtle` 层级差异，不回退到 showcase 式说明卡片

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Verified` + `filled` | 是 | 否 | 否 |
| `Preview` + `outline` | 是 | 否 | 否 |
| `Needs review` + `subtle` | 是 | 否 | 否 |
| 最终稳定帧回到 `Verified` + `filled` | 是 | 否 | 否 |
| `Beta` + `outline` | 否 | 是 | 否 |
| `Archived` + `read only` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_badge.c` 当前覆盖 `5` 条用例：

1. `style helpers update modes and palette`
   覆盖 `filled / outline / subtle / read_only` 样式 helper 的模式位、palette 和 `pressed` 清理
2. `setters clear pressed state and update content`
   覆盖文本、图标、font、icon font 与 palette setter 的内容更新和 `pressed` 清理
3. `icon region visibility tracks icon and compact mode`
   覆盖 icon region 的可见性、紧凑模式下的 region 收缩，以及 icon 为空时 region 不可解析
4. `mode setters clear pressed state`
   覆盖 `set_compact_mode()` 与 `set_read_only_mode()` 的 `pressed` 清理
5. `static preview consumes input and keeps state`
   通过 `badge_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`font`、`icon_font`、`surface_color`、`border_color`、`text_color`、`accent_color`、`text`、`icon`、`compact_mode`、`read_only_mode`、`outline_mode`、`subtle_mode`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变

补充说明：

- 主区 `badge` 是 display-first 的只读状态标签，不承接点击提交流程
- 底部 `compact / read only` preview 统一通过 `egui_view_badge_override_static_preview_api()` 吞掉 `touch / key`
- preview 输入用例继续走 `dispatch_touch_event()` / `dispatch_key_event()` 口径，与当前 harness 保持一致

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Verified` 和底部 preview 固定状态，请求首帧并等待 `BADGE_RECORD_FRAME_WAIT = 170`
2. 切到 `Preview`，等待 `BADGE_RECORD_WAIT = 90`
3. 请求第二帧并继续等待 `170`
4. 切到 `Needs review`，等待 `90`
5. 请求第三帧并继续等待 `170`
6. 回到默认 `Verified`，等待 `90`
7. 请求最终稳定帧，并继续等待 `BADGE_RECORD_FINAL_WAIT = 280`

说明：

- 录制阶段最终会显式恢复主区到默认 `Verified` 并走统一布局重放路径
- 页面层不再保留主 badge 的说明 note
- preview 只负责静态 reference 对照，不再承担页面桥接职责

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge
```

## 10. 验收重点

- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切
- 主区 `Verified / Preview / Needs review` 三组样式与文案状态必须能从截图中稳定区分
- 最终稳定帧必须显式回到默认 `Verified`
- 样式 helper、文本 / 图标 / palette setter 与 `compact / read_only` 模式切换后不能残留 `pressed`
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_badge/default`
- 已归档复核结果：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界：`(114, 183) - (365, 224)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 252` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Verified`

## 12. 与现有控件的边界

- 相比 `info_badge`：这里表达短文本状态，不是计数或 attention dot
- 相比 `badge_group`：这里聚焦单个附着式 badge，不承担汇总布局
- 相比 `tag`：这里表达系统生成的状态，不是用户可编辑或可撤销的 token

## 13. 本轮保留与删减

- 保留的主区状态：`Verified`、`Preview`、`Needs review`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：程序化样式快照切换、文本 / 图标 / palette / font 与模式 setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：主面板说明文案、底部 panel 包装与 heading、录制阶段额外恢复帧、preview 说明文字与额外页面 chrome

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/badge PORT=pc`
  - 本轮沿用 `2026-04-18` 已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `badge` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 本轮重新执行文档编码与 display 类触摸语义检查，其余结果沿用已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_badge/default`
  - 本轮沿用 `2026-04-18` 已归档 runtime 结果
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-18` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0855 colors=106`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 主区 RGB 差分边界：`(114, 183) - (365, 224)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 252` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Verified / Preview / Needs review` 三组 reference 快照，最终稳定帧已回到默认 `Verified`，底部 `compact / read only` preview 全程静态
