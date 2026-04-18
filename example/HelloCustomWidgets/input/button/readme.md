# Button 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`Button`
- 当前保留形态：`standard`、`compact`、`disabled`
- 当前保留交互：主区保留真实 `touch` 与 `Space / Enter` 提交；底部 `compact / disabled` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 清焦桥接、录制阶段 `compact` preview 快照轮换与额外 showcase 化装饰

## 1. 为什么需要这个控件
`button` 是最基础的命令触发控件，用来表达一次性动作，例如部署、同步、确认和派发。`HelloCustomWidgets` 需要一个与 `Fluent 2 / WPF UI` 主线一致的 `Button` reference 页面，作为其它输入控件的最小语义基准。

## 2. 为什么现有控件不够用
- `toggle_button` 表达的是状态切换，不是一次性命令提交。
- `split_button` 与 `drop_down_button` 带有分裂动作或菜单，不是最小 `Button` 语义。
- SDK 虽然已有基础 `button`，但仓库内当前 `input/button` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Button`
- 主区：一个保留真实提交链路的主 `button`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Open`
- 右侧 preview：`disabled`，固定显示 `Queued`

目录：
- `example/HelloCustomWidgets/input/button/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Deploy`
2. 触摸提交后
   `Sync`
3. `Space` 键盘提交后
   `Confirm`
4. `Enter` 键盘提交后
   `Dispatch`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Open`
2. `disabled`
   `Queued`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主按钮：`140 x 40`
- 底部 preview 行：`200 x 32`
- 单个 preview：`96 x 32`
- 页面结构：标题 -> 主 `button` -> 底部 `compact / disabled`
- 风格约束：主按钮使用 Fluent 风格蓝色主动作视觉；`compact` preview 只压缩尺寸与间距；`disabled` preview 使用浅灰低对比视觉；焦点态只保留轻量 ring，不引入厚阴影或额外帮助文案

## 6. 状态矩阵
| 状态 | 主按钮 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Deploy` | `Open` | `Queued` |
| 快照 2 | `Sync` | 保持不变 | 保持不变 |
| 快照 3 | `Confirm` | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Dispatch` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主按钮交互，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主按钮默认 `Deploy`，同时恢复底部 `compact / disabled` 固定状态
2. 抓取首帧
3. 触摸点击主按钮，切到 `Sync`
4. 抓取第二组主区快照
5. 发送 `Space`，切到 `Confirm`
6. 抓取第三组主区快照
7. 发送 `Enter`，切到 `Dispatch`
8. 抓取第四组主区快照
9. 保持 `Dispatch` 不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `hcw_button_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立即清理残留 `pressed`
- preview 不改 `text / icon / icon_font / icon_text_gap / region_screen / background / color`

当前 `test.c` 已保持统一 finalize 模板：保留既有 `BUTTON_RECORD_FINAL_WAIT`、`BUTTON_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_default_state()`、`apply_preview_states()`、`layout_local_views()`、`layout_page()` 与 `request_page_snapshot()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，确保主区 `Deploy / Sync / Confirm / Dispatch` 与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_button.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `touch` same-target release、`Space / Enter` 提交、`set_text()`、`set_icon()`、`set_icon_font()`、`set_icon_text_gap()` 与样式 helper 的 `pressed` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `text`、`icon`、`icon_font`、`icon_text_gap`、`region_screen`、`background`、`color`、`alpha` 不变，并要求 `g_click_count == 0` 且 `is_pressed == false`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_button
```

## 10. 验收重点
- 主按钮和底部 `compact / disabled` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Deploy`、`Sync`、`Confirm`、`Dispatch` 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、`Space / Enter` 与 setter / 样式 helper 的状态清理链路收口后不能残留 `pressed` 污染。
- 底部 `compact / disabled` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_button/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(206, 195) - (276, 207)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 208` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `toggle_button`：这里强调一次性命令触发，不是状态开关。
- 相比 `split_button`：这里没有主次动作分裂。
- 相比 `drop_down_button`：这里没有展开菜单语义。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Deploy`
  - `Sync`
  - `Confirm`
  - `Dispatch`
  - `compact`
  - `disabled`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 清焦桥接
  - 录制阶段 `compact` preview 快照轮换
  - showcase 化容器、额外说明标签与非必要页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/button PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `button` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_button/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0977 colors=140`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(206, 195) - (276, 207)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 208` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Deploy`、`Sync`、`Confirm` 与 `Dispatch` 四组 reference 状态，最终稳定帧保持 `Dispatch`，底部 `compact / disabled` preview 全程静态
