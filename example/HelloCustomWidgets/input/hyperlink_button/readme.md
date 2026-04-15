# HyperlinkButton 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`HyperlinkButton`
- 当前保留形态：`standard`、`inline`、`disabled`
- 当前保留交互：主区保留真实 `touch` 与 `Space / Enter` 提交；底部两个 preview 统一改为静态对照
- 当前移除内容：页面级说明面板、preview 清焦桥接、录制阶段真实 click / key 驱动、额外收尾态和 showcase 化装饰

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
主区录制轨道只保留 3 组程序化快照，不再在录制阶段发送真实触摸或键盘事件：

1. `Project updates`
   主链接：`Open release notes`
   说明：`Primary action keeps the lighter link affordance.`
2. `Policy changes`
   主链接：`Review change summary`
   说明：`Touch and keyboard still reuse the button submit path.`
3. `Deployment checklist`
   主链接：`Browse final checklist`
   说明：`Reference shell stays compact without extra chrome.`

底部 preview 在整条轨道中始终保持不变：

1. `inline`
   文案：`Inline article`
2. `disabled`
   文案：`Archived link`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 170`
- 主面板：`196 x 92`
- 主链接：`164 x 24`
- 底部 preview 行：`200 x 24`
- 单个 preview：`96 x 24`
- 视觉约束：浅灰 page panel、白色主面板、低噪音下划线链接、轻量 pressed / focused 反馈，不回退到实体按钮化视觉

## 6. 状态矩阵
| 状态 | 主链接 | Inline preview | Disabled preview |
| --- | --- | --- | --- |
| 默认显示 | 是 | 是 | 是 |
| same-target release 提交 | 是 | 否 | 否 |
| `Space / Enter` 提交 | 是 | 否 | 否 |
| 静态 reference 对照 | 否 | 是 | 是 |
| 录制阶段程序化切换 | 是 | 否 | 否 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 现在收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Policy changes`
4. 抓取第二组主区快照
5. 切到 `Deployment checklist`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再调用真实 `touch click`、`Space`、`Enter`
- 底部 preview 统一通过 `hcw_hyperlink_button_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态对照，不再承担清焦或额外页面状态切换职责

## 8. 单元测试口径
`example/HelloUnitTest/test/test_hyperlink_button.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖样式 helper、`set_text()` / `set_font()` 清理、same-target release、`Space / Enter` 键盘提交、禁用态和未处理按键收口。
2. 静态 preview 不变性断言
   通过 `hyperlink_button_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`base.text`、`base.font`、`base.color`、`base.alpha`、`base.align_type`、`background`、`api`、`on_click_listener`、`padding`、`icon`、`icon_text_gap`、`shadow`、`alpha`、`enable`

同时要求：
- `is_pressed == false`
- `g_click_count == 0`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/hyperlink_button PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

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

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：PASS
- `HelloUnitTest`：`845 / 845` 通过，其中 `hyperlink_button` suite `7 / 7`
- `sync_widget_catalog.py`：PASS（`106` entries）
- `touch release semantics`：PASS（`custom_audited=28`，`custom_skipped_allowlist=5`）
- `docs encoding`：PASS（`134 files`）
- `widget catalog check`：PASS（`106 widgets: reference=106, showcase=0, deprecated=0`）
- 单控件 runtime：PASS
- input 分类 compile/runtime 回归：PASS
- wasm 构建：PASS
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1297 colors=132`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_input_hyperlink_button/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(44, 143) - (287, 211)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区差分边界裁切后，主区唯一状态数：`3`
- 按 `y >= 289` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 变化只发生在主面板内的标题、链接文本和说明文案，符合 3 组 reference 快照轨道。
- 主区外区域全程静态，没有 preview 污染、黑屏、白屏或布局抖动。
- 底部 `inline / disabled` preview 在整条录制链路中保持静态一致。

## 12. 已知限制
- 当前只覆盖最小 `HyperlinkButton` reference，不扩展 visited 态、外链图标或 hover 动画。
- 底部 preview 只作为静态 reference，对外不承担交互职责。
- 页面不承载复杂内容排版，只验证控件级视觉与交互闭环。

## 13. 与现有控件的边界
- 相比 `button`：这里强调链接语义和轻量反馈，而不是实体命令按钮。
- 相比 `toggle_button` / `switch`：这里不表达状态切换，只表达一次性跳转或打开动作。
- 相比 `text_block`：这里保留 click、pressed 和键盘提交闭环。

## 14. EGUI 适配说明
- 继续复用 SDK `button` 的 same-target release 与 click listener 语义，不修改 SDK。
- 在 custom 层通过 `egui_view_hyperlink_button.c` 增加下划线绘制、样式 helper 和 `Space / Enter` 键盘闭环。
- 通过 `override_static_preview_api()` 把底部 preview 明确收口为静态 reference，保证 runtime 和 unit test 都只验证“不变性”，不再验证清焦桥接。
