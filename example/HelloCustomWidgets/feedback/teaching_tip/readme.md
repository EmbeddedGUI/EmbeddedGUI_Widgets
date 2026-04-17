# TeachingTip 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI TeachingTip`
- 补充对照实现：`ModernWpf`
- 对应组件名：`TeachingTip`
- 当前保留状态：`bottom placement`、`top placement`、`warning tone`、`closed / reopen`、`compact`、`read only`
- 当前移除内容：旧版强交互录制轨道、preview 桥接点击、与主区 reference 语义无关的额外切换动作
- EGUI 适配说明：继续使用仓库内 `teaching_tip` custom 实现，本轮只收口 `reference` 页面结构、静态 preview、README、单测与验收链，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`teaching_tip` 用于围绕具体目标表达带上下文的引导信息，适合首次引导、快捷操作提示、功能解释和风险提醒。它不是页内横幅，也不是阻塞式弹层，而是更接近 Fluent / WPF UI 的 anchored callout 语义。

## 2. 为什么现有控件不够
- `tool_tip` 更偏轻量悬浮提示，不承担多动作引导卡片语义。
- `message_bar` 是页内横向反馈，不围绕具体目标锚定。
- `dialog_sheet` 是收口式对话层，不适合贴近目标的上下文提示。
- `toast_stack` 偏瞬时通知，不负责教学式引导。
- 仓库仍需要一版贴近 Fluent / WPF UI `TeachingTip` 的 `reference` 页面，和其他 feedback 控件划清边界。

## 3. 当前页面结构
- 标题：`Teaching Tip`
- 主区：一个主 `teaching_tip`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `teaching_tip` -> 底部 `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/teaching_tip/`

## 4. 主区 reference 轨道
主区录制轨道当前固定为五步：

1. `Quick filters / bottom open`
   默认 `accent / bottom placement`
2. `Cmd palette / top open`
   程序化切到顶部气泡
3. `Sync draft / warning open`
   程序化切到 warning 语义
4. `Tip hidden / closed`
   使用同步后的 closed snapshot，保留主区关闭态
5. `Quick filters / bottom open`
   回到默认态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：
1. `compact`
   `Quick tip`
2. `read only`
   `Preview`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 252`
- 主 `teaching_tip` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 -> 主 `teaching_tip` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 `surface`、低噪音边框、克制的 `accent / warning` 色带

## 6. 状态矩阵
| 状态 / 区域 | 主 `teaching_tip` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `bottom placement` | 是 | 是 | 是 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `closed / reopen` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `teaching_tip` 继续保留 `target / primary / secondary / close` 的真实交互语义。
- 主区继续保留 `top / bottom placement`、关闭态和重新打开态。
- 主区继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才提交。
- `compact / read only` preview 统一通过 `egui_view_teaching_tip_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- `read only` preview 继续显式设置 `set_read_only_mode(..., 1)`，避免参与真实交互。
- 更细的交互闭环由 `HelloUnitTest` 覆盖，runtime 录制只保留稳定 reference 主状态。

## 8. 本轮收口内容
- `example/HelloCustomWidgets/feedback/teaching_tip/test.c`
  新增 `TEACHING_TIP_DEFAULT_SNAPSHOT`、`apply_primary_default_state()`、`ui_ready`、`layout_local_views()`、`layout_page()` 和统一 `request_page_snapshot()`，把 root view 挂载前后的默认态恢复与录制轨道统一收口为主区四组 reference 状态加最终稳定帧；底部 preview 全程保持真正静态。
- `example/HelloUnitTest/test/test_teaching_tip.c`
  新增 `teaching_tip_preview_snapshot_t`、`assert_region_equal()`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()`，把静态 preview 用例收口为完整状态不变断言，并统一改成 `dispatch_touch_event()` / `dispatch_key_event()`。
- `example/HelloCustomWidgets/feedback/teaching_tip/readme.md`
  按当前 workflow 模板重写，并在验收完成后回填 compile / unit / runtime / web 结果与 runtime 复核数字。

## 9. 录制动作设计
`egui_port_get_recording_action()` 当前轨道如下：

1. 还原 `Quick filters / bottom open`，同步底部两个静态 preview，并请求首帧。
2. 切到 `Cmd palette / top open` 并请求快照。
3. 切到 `Sync draft / warning open` 并请求快照。
4. 同步 closed snapshot，进入 `Tip hidden / closed` 并请求快照。
5. 回到默认 `Quick filters / bottom open` 并请求最终稳定帧。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- 底部 preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 reference snapshot 切换与 closed snapshot 同步。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 与最终稳定帧入口统一回到 `apply_primary_default_state()`，确保录制首尾都走同一条显式布局路径。

## 10. 单元测试口径
`example/HelloUnitTest/test/test_teaching_tip.c` 当前覆盖十部分：

1. `set_snapshots()` 的 clamp、默认 part 解析和空数据保护
2. `current_snapshot / current_part` guard 与 pressed 清理
3. `font / meta_font / compact / read_only / palette` setter 与内部 helper
4. metrics 计算、placement 变化与 hit testing
5. 触摸 same-target release、cancel 和 close / reopen
6. keyboard navigation 与 `Left / Right / Home / End / Tab / Escape / Enter / Space`
7. `compact_mode` 切换后的 pressed 清理和输入行为
8. `read_only_mode` 的输入忽略与恢复
9. `!enable` guard 与 disabled 状态下的 pressed 清理
10. 静态 preview 吞掉输入后保持完整状态不变

其中静态 preview 用例通过 `teaching_tip_preview_snapshot_t` 固定校验：
`region_screen / background / snapshots / font / meta_font / on_part_changed / api / palette / snapshot_count / current_snapshot / current_part / compact_mode / read_only_mode / pressed_part / alpha / enable / is_focused / is_pressed / padding`

## 11. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/teaching_tip --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/teaching_tip
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_teaching_tip
```

## 12. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`
  `make all APP=HelloCustomWidgets APP_SUB=feedback/teaching_tip PORT=pc`
- `HelloUnitTest`：`PASS`
  在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `teaching_tip` suite `10 / 10`
- `sync_widget_catalog.py`：`PASS`
  同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`
  `custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`
  `134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`
  `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`
  `10 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_teaching_tip/default`
- feedback 分类 compile/runtime 回归：`PASS`
  compile `10 / 10`，runtime `10 / 10`
- wasm 构建：`PASS`
  `web/demos/HelloCustomWidgets_feedback_teaching_tip`
- web smoke：`PASS`
  `status=Running canvas=480x480 ratio=0.1921 colors=162`

## 13. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_teaching_tip/default`

- 总帧数：`10`
- 主区 RGB 差分边界：`(68, 81) - (413, 207)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`4`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `teaching_tip` reference 主体，遮罩主区差分边界后，主区外页面 chrome 在整条录制轨道中保持静态。
- `10` 帧里主区共出现 `4` 组唯一状态，对应 `Quick filters / bottom open`、`Cmd palette / top open`、`Sync draft / warning open` 和 `Tip hidden / closed`；最终稳定帧回到默认 `Quick filters / bottom open`。
- 按 `y >= 207` 裁切底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条轨道中始终静态一致。

## 14. 已知限制
- 当前版本仍是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做自动避让、屏幕边缘翻转和复杂入场动画。
- 当前以固定 snapshot 数据保证录制稳定，不接业务侧动态文案来源。
- 当前优先保证 `reference` 页面、单测、runtime 验收和 web 发布链闭环，不扩展成多目标引导系统。

## 15. 与现有控件的边界
- 相比 `tool_tip`：这里是带动作的引导卡片，不是轻量悬浮提示。
- 相比 `message_bar`：这里是围绕目标的锚定 callout，不是页内横向反馈条。
- 相比 `toast_stack`：这里不承担瞬时通知队列，只表达目标引导和功能提醒。

## 16. EGUI 适配说明
- `teaching_tip` custom 实现继续留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用 static preview API，吞掉输入并清理残留 pressed 状态。
- README、demo、单测、runtime 验收和 web 发布口径已经对齐到当前 `reference` workflow。
