# hyperlink_button 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`HyperlinkButton`
- 当前保留形态：`Project updates / Open release notes`、`Policy changes / Review change summary`、`Deployment checklist / Browse final checklist`、`inline`、`disabled`
- 当前保留交互：主区保留真实 `touch`、`same-target release` 与 `Space / Enter` 提交；底部 `inline / disabled` 统一收口为静态 preview 对照
- 当前移除内容：页面级说明面板、preview 清焦桥接、录制阶段真实 `click / key` 驱动、额外收尾态、visited / hover 延展、showcase 化装饰，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `hcw_hyperlink_button` 包装层与 SDK `button` 基础点击能力，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`hyperlink_button` 用于承载“跳转到说明、详情、文档、补充动作”的轻量入口。它和仓库中的实体按钮类控件不同，重点是保留链接语义、低视觉重量和文本导向的交互反馈，适合放在内容区、设置页或说明区里做次级动作。

## 2. 为什么现有控件不够用
- `button`、`toggle_button` 更偏主命令或状态切换，视觉重量更高。
- `split_button`、`drop_down_button` 自带分段或菜单语义，不适合最小链接动作。
- `text_block` 只能展示文本，不承担 click 语义和键盘提交闭环。

## 3. 当前页面结构
- 标题：`HyperlinkButton`
- 主区：一个可真实交互的主链接面板
- 底部：两个静态 preview
- 左侧 preview：`inline`，固定文案 `Inline article`
- 右侧 preview：`disabled`，固定文案 `Archived link`

目录：
- `example/HelloCustomWidgets/input/hyperlink_button/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组 reference 快照，最终稳定帧回到默认 `Project updates / Open release notes`；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   `Project updates / Open release notes`
2. 快照 2
   `Policy changes / Review change summary`
3. 快照 3
   `Deployment checklist / Browse final checklist`
4. 最终稳定帧
   `Project updates / Open release notes`

底部 preview 在整条轨道中始终固定：
1. `inline`
   `Inline article`
2. `disabled`
   `Archived link`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 170`
- 主面板：`196 x 92`
- 主链接：`164 x 24`
- 底部 preview 行：`200 x 24`
- 单个 preview：`96 x 24`
- 页面结构：标题 -> 主链接面板 -> 底部 `inline / disabled`
- 视觉约束：浅色 `page panel`、白色主面板、低噪音下划线链接、轻量 `pressed / focused` 反馈，不回退到实体按钮化视觉

## 6. 状态矩阵
| 状态 | 主链接 | Inline preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | `Project updates / Open release notes` | `Inline article` | `Archived link` |
| `same-target release` 提交 | 是 | 否 | 否 |
| `Space / Enter` 提交 | 是 | 否 | 否 |
| 快照 2 | `Policy changes / Review change summary` | 保持不变 | 保持不变 |
| 快照 3 | `Deployment checklist / Browse final checklist` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到默认 `Project updates / Open release notes` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_hyperlink_button.c` 当前覆盖 `7` 条用例：

1. 样式 helper 更新背景并清理 `pressed`。
   覆盖 `apply_standard_style()`、`apply_inline_style()` 与 `apply_disabled_style()` 的背景、文字颜色与按下态清理。
2. setter 清理 `pressed` 并更新内容。
   覆盖 `set_text()`、`set_font()` 的内容更新与按下态清理。
3. 触摸 `same-target release` 只提交一次。
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到原目标释放后单次点击提交。
4. 键盘 `Space / Enter` 点击。
   覆盖主区链接的键盘提交闭环。
5. disabled 输入不触发点击。
   覆盖禁用态下的触摸保护、释放保护与点击计数保持不变。
6. 未处理按键清理 `pressed`。
   覆盖收到无效按键后的状态复位。
7. static preview 吞输入且保持状态不变。
   通过 `hyperlink_button_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`base.text`、`base.font`、`base.color`、`base.alpha`、`base.align_type`、`background`、`api`、`on_click_listener`、`padding`、`icon`、`icon_text_gap`、`shadow`、`alpha`、`enable` 不变，并要求 `is_pressed == false`、`g_click_count == 0`。

说明：
- 主区继续复用 `button` 的点击基础能力，但当前 reference 明确强调链接语义和轻量反馈，不回退到实体按钮样式。
- 非拖拽点击继续遵循 `same-target release`：`DOWN(A) -> UP(B)` 不提交，只有回到原命中目标释放才提交。
- 底部 `inline / disabled` preview 统一通过 `hcw_hyperlink_button_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主区默认 `Project updates / Open release notes` 与底部双静态 preview，请求首帧截图，等待 `HYPERLINK_RECORD_FRAME_WAIT`。
2. 程序化切到 `Policy changes / Review change summary`，等待 `HYPERLINK_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `HYPERLINK_RECORD_FRAME_WAIT`。
4. 程序化切到 `Deployment checklist / Browse final checklist`，等待 `HYPERLINK_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `HYPERLINK_RECORD_FRAME_WAIT`。
6. 程序化回到默认 `Project updates / Open release notes`，等待 `HYPERLINK_RECORD_WAIT`。
7. 请求最终稳定帧，并继续等待 `HYPERLINK_RECORD_FINAL_WAIT`。

说明：
- 录制阶段不再调用真实 `touch click`、`Space` 或 `Enter`。
- preview 只负责静态对照，不再承担清焦或额外页面状态切换职责。
- `request_page_snapshot()` 会统一执行 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/hyperlink_button PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/hyperlink_button --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/hyperlink_button
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_hyperlink_button
```

## 10. 验收重点
- 主链接面板和底部 `inline / disabled` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Project updates / Open release notes`、`Policy changes / Review change summary`、`Deployment checklist / Browse final checklist` 三组 reference 快照必须能从截图中稳定区分。
- 主控件 `touch`、`same-target release` 与 `Space / Enter` 提交链路收口后，不能残留 `pressed` 脏态。
- 最终稳定帧必须显式回到默认 `Project updates / Open release notes`。
- 底部 `inline / disabled` preview 必须保持静态 reference，对输入只吞掉而不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_hyperlink_button` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_hyperlink_button/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(44, 143) - (287, 211)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 212` 裁切底部 preview 后，preview 区域唯一哈希数为 `1`

## 12. 与现有控件的边界
- 对比 `button`：这里强调链接语义和轻量反馈，而不是实体命令按钮。
- 对比 `toggle_button` / `switch`：这里不表达状态切换，只表达一次性跳转或打开动作。
- 对比 `text_block`：这里保留 `click`、`pressed` 和键盘提交闭环。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Project updates / Open release notes`
  - `Policy changes / Review change summary`
  - `Deployment checklist / Browse final checklist`
- 保留的底部对照：
  - `inline`
  - `disabled`
- 保留的交互：
  - 主区 `touch` 点击
  - `same-target release`
  - `Space / Enter` 键盘提交
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级说明面板与 guide
  - preview 清焦桥接
  - 录制阶段真实 `click / Space / Enter` 驱动
  - 额外收尾态、visited / hover 延展与 showcase 化 chrome
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/hyperlink_button PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `hyperlink_button` suite `7 / 7`
- catalog / 文档 / 触控语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触控语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/hyperlink_button --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_hyperlink_button/default`
  - 共捕获 `9` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/hyperlink_button`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_hyperlink_button`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1297 colors=132`
- 截图复核结论：
  - 主区只覆盖默认 `Project updates / Open release notes`、`Policy changes / Review change summary` 与 `Deployment checklist / Browse final checklist` 三组 reference 快照
  - 最终稳定帧已显式回到默认 `Project updates / Open release notes`
  - 主区 RGB 差分边界收敛到 `(44, 143) - (287, 211)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `inline / disabled` preview 以 `y >= 212` 裁切后全程保持单哈希静态
