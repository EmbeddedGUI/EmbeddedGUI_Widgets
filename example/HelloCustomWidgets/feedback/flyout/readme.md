# Flyout 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI Flyout`
- 补充参考控件：`tool_tip`、`teaching_tip`
- 对应组件名：`Flyout`
- 当前保留状态：`bottom placement`、`top placement`、`warning`、`closed`、`compact`、`disabled`
- 当前移除内容：旧录制轨道里的键盘切换、target 点击 dismiss 动作、preview 焦点桥接和与 reference 页面无关的额外收尾动作
- EGUI 适配说明：继续使用仓库内 `flyout` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`flyout` 用来表达围绕单一目标展开的轻量操作面板。它比 `tool_tip` 更重，因为可以承载标题、正文和动作；又比 `teaching_tip` 更轻，因为不承担教学卡片和长篇引导叙事。仓库需要一版贴近 Fluent / WPF UI 的 `Flyout` reference，实现与其它 feedback 控件的清晰边界。

## 2. 为什么现有控件不够用
- `tool_tip` 只有提示，不承载动作按钮。
- `teaching_tip` 偏教学和 coachmark，视觉与语义都更重。
- `menu_flyout` 偏菜单列表，不强调 target + bubble 的锚定关系。
- `dialog_sheet` 是更高层级的确认层，不适合作为局部上下文面板。

## 3. 当前页面结构
- 标题：`Flyout`
- 主区：一个主 `flyout`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`disabled`
- 页面结构统一收口为：标题 -> 主 `flyout` -> `compact / disabled`

当前目录：`example/HelloCustomWidgets/feedback/flyout/`

## 4. 主区 reference 轨道
主控件录制轨道只保留四组主区 snapshot 和最终稳定帧：

1. `Review`
   `bottom placement / open`，默认落在 `primary`
2. `Search`
   `top placement / open`，默认落在 `secondary`
3. `Sync`
   `warning / bottom placement / open`
4. `Pinned`
   `closed`，主区只保留 target
5. `Review`
   回到默认 `bottom placement / open`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Filter`
   紧凑尺寸、accent tone、静态打开态
2. `disabled`
   `Locked`
   禁用弱化配色、静态关闭态

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 252`
- 主 `flyout` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 -> 主 `flyout` -> 底部 `compact / disabled`
- 页面风格：浅灰 page panel、白色 surface、低噪音阴影和明确但克制的 target + bubble 锚定关系

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Disabled preview |
| --- | --- | --- | --- |
| `bottom placement` | 是 | 是 | 是 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `open_state = 1` | 是 | 是 | 否 |
| `open_state = 0` | 是 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `disabled_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `flyout` 保留 target、bubble、`primary / secondary action`、`top / bottom placement` 和关闭态语义。
- 交互仍遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `compact / disabled` preview 统一通过 `egui_view_flyout_override_static_preview_api()` 吞掉 `touch / key`，不再承担任何 snapshot 切换、target dismiss 或焦点桥接职责。
- `disabled` preview 保持 `set_disabled_mode(..., 1)`，只作为静态对照，不参与真实交互。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主控件默认 `Review` 和底部 preview 固定状态
2. 请求首帧
3. 切到 `Search`
4. 请求第二帧
5. 切到 `Sync`
6. 请求第三帧
7. 切到 `Pinned`
8. 请求第四帧
9. 回到默认 `Review` 并请求最终稳定帧

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自四组 snapshot 以及最终回落到默认帧。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_flyout.c` 当前覆盖六部分：

1. setter 钳制与 `pressed` 清理
2. 默认 part、snapshot guard 和空数据 guard
3. 触摸 same-target release、action 触发和 dismiss 语义
4. 键盘导航、激活与关闭
5. `disabled_mode / !enable` guard
6. 静态 preview 不变性断言

其中静态 preview 用例通过 `flyout_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / snapshots / font / meta_font / on_action / api / palette / snapshot_count / current_snapshot / current_part / open_state / compact_mode / disabled_mode / pressed_part / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/flyout --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/flyout
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_flyout
```

## 11. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=feedback/flyout PORT=pc`
- `HelloUnitTest`：`PASS`，在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `flyout` suite `6 / 6`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`11 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_flyout/default`
- feedback 分类 compile/runtime 回归：`PASS`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_feedback_flyout`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1921 colors=153`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_flyout/default`

- 总帧数：`11`
- 主区 RGB 差分边界：`(60, 86) - (420, 207)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`4`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `flyout` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `11` 帧里主区只出现 `4` 组唯一状态，对应 `Review / Search / Sync / Pinned` 四组 snapshot，最终稳定帧回到默认 `Review`。
- 按 `y >= 208` 裁剪底部 preview 区域后保持单哈希，确认 `compact / disabled` preview 在整条录制轨道中始终静态一致。

## 13. 已知限制
- 当前版本是页内固定锚点 reference，不做系统级 popup 跟随。
- 当前不实现复杂避让、边缘翻转和额外过渡动画。
- 当前动作由固定 snapshot 驱动，不接业务动态数据源。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部提示容器。

## 14. 与现有控件的边界
- 相比 `tool_tip`：这里承载动作，不是纯提示。
- 相比 `teaching_tip`：这里是轻量局部面板，不是教学卡片。
- 相比 `menu_flyout`：这里强调 target + bubble 关系，不是命令列表。
- 相比 `dialog_sheet`：这里是局部上下文面板，不是模态确认层。

## 15. EGUI 适配说明
- 保持 `flyout` custom 实现继续驻留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用静态 preview API，吞掉输入并清理残留 `pressed`。
- README、demo、单测和 runtime 验收口径已经对齐到当前 reference workflow。
