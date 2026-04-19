# SplitButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI SplitButton`
- 参考开源库：`WPF UI`
- 对应组件：`SplitButton`
- 当前保留形态：`standard`、`compact`、`disabled`
- 当前保留交互：主区保留 `primary action / menu action` 双段语义、same-target release、键盘 part 导航与静态 preview 对照
- 当前移除内容：页面级 guide、preview 快照轮换、preview 清焦桥接、额外收尾态、真实 flyout 弹层、多级菜单与 showcase 包装
- EGUI 适配说明：继续使用当前目录下的 `egui_view_split_button` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下完成 reference 页面、README、录制轨道和单测收口

## 1. 为什么需要这个控件
`split_button` 用来表达“默认主动作 + 更多动作入口”并存的复合命令按钮，适合保存、分享、导出和归档这类场景：用户可以直接触发主动作，也可以进入菜单段选择扩展动作。

仓库里已有普通 `button` 和 `drop_down_button`，但仍缺少一个能稳定承接 `SplitButton` 双入口语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `button` 只有单一动作，无法表达 split 双段结构。
- `drop_down_button` 只有统一菜单入口，没有主动作段。
- `menu_flyout` 是独立弹出菜单容器，不是页内复合按钮。
- `command_bar` 承担工具栏语义，不是单个复合命令按钮。

## 3. 当前页面结构
- 标题：`Split Button`
- 主区：1 个保留真实 split 语义的 `split_button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick / Save`
- 右侧 preview：`disabled`，固定显示 `Locked / Publish`

目录：
- `example/HelloCustomWidgets/input/split_button/`

## 4. 主区 reference 快照
主区录制轨道只保留 4 组程序化快照，不再在录制阶段真实驱动 touch / key，也不再让底部 preview 参与轮换：

1. 默认态
   `Save draft / primary`
2. 快照 2
   `Share handoff / menu`
3. 快照 3
   `Export file / menu`
4. 快照 4
   `Archive page / primary`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Quick / Save`
2. `disabled`
   `Locked / Publish`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 154`
- 主控件：`196 x 74`
- 底部 preview 行：`216 x 44`
- 单个 preview：`104 x 44`
- 页面结构：标题 -> 主 `split_button` -> 底部 `compact / disabled`
- 风格约束：浅色 page panel、低噪音描边、明确的主动作段与菜单段边界，以及稳定的 helper 文案层级，不回退到旧 demo 的 guide 与场景包装

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Save draft / primary` | `Quick / Save` | `Locked / Publish` |
| 快照 2 | `Share handoff / menu` | 保持不变 | 保持不变 |
| 快照 3 | `Export file / menu` | 保持不变 | 保持不变 |
| 快照 4 | `Archive page / primary` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Archive page / primary` | 保持不变 | 保持不变 |
| same-target release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Share handoff`
4. 抓取第二组主区快照
5. 切到 `Export file`
6. 抓取第三组主区快照
7. 切到 `Archive page`
8. 抓取第四组主区快照
9. 保持最终状态不变并等待稳定
10. 抓取最终稳定帧

说明：
- 主区仍保留真实 `primary / menu` 交互、same-target release 和键盘 part 导航，供运行时手动交互。
- runtime 录制阶段不再真实发送主区点击或菜单切换输入。
- 底部 preview 统一通过 `egui_view_split_button_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `4` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `SPLIT_BUTTON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_split_button.c` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖 setter 清理 pressed、snapshot / current_part guard、same-target release、touch cancel、`compact / disabled / !enable` guard 与键盘导航。
2. 静态 preview 不变性断言
   通过 `split_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`snapshots`、`font`、`meta_font`、`on_part_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`danger_color`、`neutral_color`、`snapshot_count`、`current_snapshot`、`current_part`、`compact_mode`、`disabled_mode`、`alpha`

同时要求：
- `changed_count == 0`
- `last_part == PART_NONE`
- `is_pressed == false`
- `pressed_part == PART_NONE`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/split_button PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/split_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/split_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_split_button
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Save draft`、`Share handoff`、`Export file`、`Archive page` 4 组可识别状态。
- 主区真实交互仍需保留 `primary / menu` 双段 same-target release 与键盘 part 导航语义。
- 底部 `compact / disabled` preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `snapshots / current_snapshot / current_part / compact_mode / disabled_mode / region_screen / palette`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_split_button/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界收敛到 `(52, 157) - (427, 255)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 256` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `button`：这里强调主动作段和菜单段并存，而不是单动作按钮。
- 相比 `drop_down_button`：这里同时保留主动作段和菜单段。
- 相比 `menu_flyout`：这里是页内复合按钮，不是独立弹出菜单容器。
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Save draft / primary`
  - `Share handoff / menu`
  - `Export file / menu`
  - `Archive page / primary`
  - `compact`
  - `disabled`
- 删减的装饰或桥接：
  - 页面级 guide
  - preview 快照轮换
  - preview 清焦桥接
  - 额外收尾态
  - 真实 flyout 弹层与多级菜单

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/split_button PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `split_button` suite `11 / 11`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/split_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_split_button/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/split_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_split_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1175 colors=139`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(52, 157) - (427, 255)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 256` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Save draft / primary`、`Share handoff / menu`、`Export file / menu` 与 `Archive page / primary` 4 组 reference 快照，最终稳定帧保持 `Archive page / primary`，底部 `compact / disabled` preview 全程静态
