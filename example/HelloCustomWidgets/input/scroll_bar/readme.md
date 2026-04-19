# ScrollBar 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI ScrollBar`
- 参考开源库：`WPF UI`
- 对应组件：`ScrollBar`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留 `decrease / track / thumb / increase`、viewport 比例表达与 `Tab / Up / Down / Home / End / +/-` 键盘闭环；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：旧版 preview 焦点桥接、preview 点击驱动录制、第二组 compact preview 切换、额外 guide / section divider / preview 标签
- EGUI 适配说明：继续使用当前目录下的 `egui_view_scroll_bar` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下完成 reference 页面、README、录制轨道和单测收口

## 1. 为什么需要这个控件
`scroll_bar` 用来表达长内容窗口在整段内容中的位置和可视比例，适合文档轨道、日志浏览、时间轴和属性面板等场景。它不是抽象数值选择器，而是围绕 `content_length / viewport_length / offset` 表达“当前位置 + 可视窗口比例”。

仓库里虽然已经有 `slider`、`number_box` 和 `progress_bar` 一类数值控件，但仍需要一颗与 `Fluent 2 / WinUI ScrollBar` 语义对齐的独立标准滚动条 reference。

## 2. 为什么现有控件不够用
- `slider` 更偏抽象数值选择，thumb 大小固定，不能表达 viewport 比例。
- `progress_bar` 只有展示语义，没有按钮、track 分页和 thumb drag。
- `scroll` 是容器行为，不是独立的标准输入控件。
- `number_box` / `stepper` 偏离散值编辑，不适合连续内容定位。

## 3. 当前页面结构
- 标题：`Scroll Bar`
- 主区：1 个标准 `scroll_bar`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`

目录：
- `example/HelloCustomWidgets/input/scroll_bar/`

## 4. 主区 reference 快照
主区数据集中继续保留 3 组 reference 数据：

1. `Document rail`
2. `Timeline rail`
3. `Audit rail`

当前录制轨道只保留 5 组可识别状态：

1. 默认 `Document rail`
2. `Down`
3. `+`
4. `End`
5. `Timeline rail`

最终稳定帧会显式回到默认 `Document rail`。`Audit rail` 继续作为实现内保留数据，不进入本轮 runtime 录制轨道。

底部 preview 在整条轨道中始终固定：

1. `compact`
2. `read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 238`
- 主控件：`196 x 146`
- 底部 preview 行：`216 x 52`
- 单个 preview：`104 x 52`
- 页面结构：标题 -> 主 `scroll_bar` -> 底部 `compact / read only`
- 风格约束：浅灰 `page panel`、低噪音白色 surface、清晰的 `label + helper + viewport preview + rail` 层级，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `Document rail` | 是 | 是 | 是 |
| `Down` | 是 | 否 | 否 |
| `+` | 是 | 否 | 否 |
| `End` | 是 | 否 | 否 |
| `Timeline rail` | 是 | 否 | 否 |
| `Audit rail` 数据保留 | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认 `Document rail` 和底部 preview 固定状态
2. 抓取首帧
3. 派发 `Down`
4. 抓取 line step 结果
5. 派发 `+`
6. 抓取 page step 结果
7. 派发 `End`
8. 抓取末尾状态
9. 程序化切到 `Timeline rail`
10. 抓取 `Timeline rail`
11. 回到默认 `Document rail`
12. 抓取最终稳定帧

说明：
- 主控件仍保留真实触摸语义：`decrease` 按钮、`track` 分页和 `thumb` 拖拽。
- 底部 `compact / read only` preview 统一通过 `egui_view_scroll_bar_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证整条轨道口径一致。
- 当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，录制键盘入口统一通过 `focus_primary_scroll_bar()` 收敛焦点。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_scroll_bar.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `content_metrics / step_size / offset / current_part` setter 的 clamp 与 pressed 清理、`Tab / Up / Down / Home / End / +/-` 键盘闭环、`decrease` 点击、`track` 分页、`thumb` 拖拽、`touch cancel` 清理，以及 `compact_mode / read_only_mode / !enable` guard。
2. 静态 preview 不变性断言
   通过 `scroll_bar_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`on_changed`、`font`、`meta_font`、`label`、`helper`、`api`、`palette`、`content_length`、`viewport_length`、`offset`、`line_step`、`page_step`、`compact_mode`、`read_only_mode`、`current_part`、`pressed_part`、`pressed_track_direction`、`thumb_dragging`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/scroll_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/scroll_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_scroll_bar
```

## 10. 验收重点
- 主区和底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现默认 `Document rail`、`Down`、`+`、`End`、`Timeline rail` 5 组可识别状态。
- 主控件的 `decrease / track / thumb / increase`、setter、模式切换和禁用 guard 不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `region_screen / background / on_changed / font / meta_font / label / helper / api / palette / content_length / viewport_length / offset / line_step / page_step / compact_mode / read_only_mode / current_part / pressed_part / pressed_track_direction / thumb_dragging / alpha / enable / is_focused / is_pressed / padding`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_scroll_bar/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6,7] / [8,9]`
  - 主区 RGB 差分边界收敛到 `(54, 98) - (289, 298)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 299` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `slider`：这里的 thumb 尺寸由 viewport 决定，目标是定位内容窗口，而不是选择抽象数值。
- 相比 `scroll`：这里是独立标准控件，不负责 child layout、惯性或内容实际滚动。
- 相比 `progress_bar`：这里保留按钮、track page、thumb drag 和 focus part 语义。
- 相比 `number_box` / `stepper`：这里强调连续内容定位，不是离散数值编辑。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Document rail`
  - `Down`
  - `+`
  - `End`
  - `Timeline rail`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 旧版 preview 焦点桥接
  - preview 点击驱动录制
  - 第二组 compact preview 切换
  - 额外 guide / section divider / preview 标签
  - 让 preview 参与主区状态切换的旧链路

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `scroll_bar` suite `14 / 14`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/scroll_bar --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_scroll_bar/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/scroll_bar`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_scroll_bar`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1815 colors=157`
- 截图复核结论：`PASS`
  - 共捕获 `13` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6,7] / [8,9]`
  - 主区 RGB 差分边界为 `(54, 98) - (289, 298)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 299` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Document rail`、`Down`、`+`、`End` 与 `Timeline rail` 五组 reference 状态，最终稳定帧已显式回到默认态，底部 `compact / read only` preview 全程静态
