# check_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 CheckBox`
- 开源母本：`WPF UI`
- 对应组件：`CheckBox`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留真实 `touch` 勾选与 `Space / Enter` 键盘切换；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 快照切换、旧录制轨道里的额外收尾态

## 1. 为什么需要这个控件
`check_box` 用来表达某个布尔选项当前是否被选中，适合通知订阅、离线同步、权限确认、同意条款和批量选择等场景。它和 `toggle_button` 的区别在于这里强调的是表单字段语义，而不是按钮化命令入口。

## 2. 为什么现有控件不够用
- `toggle_button` 更接近按钮语义，视觉重心也更像命令触发器。
- `switch` 更偏即时开关轨道，不是标准复选框字段。
- `radio_button` 表达的是互斥选择，不适合独立布尔字段。
- 仓库里当前 `input/check_box` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Check Box`
- 主区：一个保留真实勾选闭环的主 `check_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Auto`
- 右侧 preview：`read only`，固定显示 `Accepted`

目录：
- `example/HelloCustomWidgets/input/check_box/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Email alerts` / unchecked
2. 触摸勾选后
   `Email alerts` / checked
3. 第二组主快照
   `Offline sync` / unchecked
4. `Enter` 键盘切换后
   `Offline sync` / checked

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Auto`
2. `read only`
   `Accepted`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 158`
- 主控件：`196 x 34`
- 底部对照行：`216 x 28`
- 单个 preview：`104 x 28`
- 页面结构：标题 -> 主 `check_box` -> 底部 `compact / read only`
- 风格约束：使用浅色 page panel、轻边框和低噪音勾选填充；主控件保留轻量 focus ring，不叠加厚重阴影；`compact` 只压缩尺寸和间距；`read only` 保留勾选结果展示但不承担真实输入职责

## 6. 状态矩阵
| 状态 | 主 `check_box` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Email alerts` / unchecked | `Auto` / checked | `Accepted` / checked |
| 快照 2 | `Email alerts` / checked | 保持不变 | 保持不变 |
| 快照 3 | `Offline sync` / unchecked | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Offline sync` / checked | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件交互，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Email alerts` 未选中，同时恢复底部 `compact / read only` 固定状态
2. 抓取首帧
3. 触摸点击主控件，切到 `Email alerts` / checked
4. 抓取第二组主区快照
5. 切换到 `Offline sync` / unchecked
6. 抓取第三组主区快照
7. 发送 `Enter`，切到 `Offline sync` / checked
8. 抓取第四组主区快照
9. 保持 `Offline sync` / checked 不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `hcw_check_box_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立即清理残留 `pressed`
- preview 不改 `text / checked / region_screen / palette / font / mark_style`

当前 `test.c` 已保持统一 finalize 模板：保留 `CHECK_BOX_RECORD_FINAL_WAIT`、`CHECK_BOX_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_snapshot()`、`apply_preview_states()`、`focus_primary_box()`、`layout_page()` 与 `request_page_snapshot()`，初始化阶段在 root view 挂载前后重放默认态与 preview，确保主区 `Email alerts / Offline sync` 两组快照与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_check_box.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖样式 helper、`set_text()`、`set_mark_style()`、`set_mark_icon()`、`set_icon_font()`、`set_checked()`、`touch` same-target release、`Space / Enter` 键盘切换与 disabled guard 的 `pressed` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `text`、`checked`、`font`、`icon_font`、`mark_style`、`region_screen`、`palette`、`text_gap`、`alpha`、`background` 不变，并要求 `g_checked_count == 0`、`g_last_checked == 0xFF` 且 `is_pressed == false`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/check_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/check_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/check_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_check_box
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Email alerts` unchecked / checked 与 `Offline sync` unchecked / checked 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、`Space / Enter` 与 setter / 样式 helper 的状态清理链路收口后不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_check_box/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(45, 148) - (168, 198)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 199` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `toggle_button`：这里保留表单字段语义，而不是按钮式命令语义。
- 相比 `switch`：这里强调复选框视觉与清单场景，而不是立即生效的开关轨道。
- 相比 `radio_button`：这里支持独立布尔值，不承担互斥组职责。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Email alerts` / unchecked
  - `Email alerts` / checked
  - `Offline sync` / unchecked
  - `Offline sync` / checked
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 快照切换
  - 旧录制轨道中的额外收尾态
  - 富文本标签、嵌入链接、多列表单壳与非必要页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/check_box PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `check_box` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/check_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_check_box/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/check_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_check_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1205 colors=154`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(45, 148) - (168, 198)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 199` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Email alerts` 未选中、触摸勾选、`Offline sync` 未选中与 `Offline sync` 已选中四组 reference 状态，最终稳定帧保持 `Offline sync` 已选中，底部 `compact / read only` preview 全程静态
