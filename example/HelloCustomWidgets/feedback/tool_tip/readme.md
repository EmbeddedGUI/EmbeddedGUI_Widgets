# ToolTip 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ToolTip`
- 对应组件名：`ToolTip`
- 当前保留状态：`delayed open`、`top / bottom placement`、`warning tone`、`Esc close`、`compact`、`read only`
- 当前移除内容：旧版 preview snapshot 切换、preview click/focus 桥接、与主区 reference 语义无关的额外录制动作
- EGUI 适配说明：继续使用仓库内 `tool_tip` custom 实现，本轮只收口 `reference` 页面结构、静态 preview、README、单测和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`tool_tip` 对齐的是 Fluent / WinUI 里的轻量目标提示语义，用来围绕单个按钮或热点区域提供短提示、快捷说明和轻量风险提醒。它不是 `teaching_tip` 那种引导卡片，也不是 `message_bar` 那种页内横幅，而是更贴近悬停提示气泡的 reference 对照。

## 2. 为什么现有控件不够
- `teaching_tip` 更偏引导和教学，信息密度、动作层级和占位都更重。
- `message_bar` 是页内横向反馈，不围绕单个目标锚定。
- `toast_stack` 是瞬时通知，不承担目标解释和悬停提示语义。
- 仓库仍需要一版贴近 Fluent / WPF UI `ToolTip` 的 `reference` 页面，用来和其他 feedback 控件划清边界。

## 3. 当前页面结构
- 标题：`ToolTip`
- 主区：一个主 `tool_tip`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `tool_tip` -> `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/tool_tip/`

## 4. 主区 reference 轨道
主区录制轨道当前固定为六步：

1. `Save / closed`
   默认闭合态，作为首帧基线
2. `Save / delayed open`
   通过 `begin_show_delay()` 触发稳定的延时打开参考帧
3. `Search / top placement`
   程序化切到顶部气泡
4. `Publish / warning open`
   程序化切到 warning 语义并保持展开
5. `Publish / Esc close`
   在 warning 态发送 `Esc`，验证关闭后的收口状态
6. `Save / closed`
   回到默认态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Filter`
2. `read only`
   `Preview`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 238`
- 主 `tool_tip` 尺寸：`196 x 118`
- 底部容器尺寸：`216 x 82`
- 单个 preview 尺寸：`104 x 82`
- 页面结构：标题 -> 主 `tool_tip` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色 `surface`、低噪音边框、轻量阴影与克制的 accent / warning 色带

## 6. 状态矩阵
| 状态 / 区域 | 主 `tool_tip` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `delayed open` | 是 | 否 | 否 |
| `top placement` | 是 | 否 | 否 |
| `warning tone` | 是 | 否 | 否 |
| `Esc close` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `tool_tip` 继续保留真实触摸与键盘延时打开语义：同目标 release 才触发、`Enter / Space` 延时打开、`Esc` 关闭。
- 触摸仍遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才生效。
- `compact / read only` preview 统一通过 `egui_view_tool_tip_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- `read only` preview 继续显式设置 `set_read_only_mode(..., 1)`，避免参与真实交互。
- 交互细节由 `HelloUnitTest` 覆盖，runtime 录制只保留稳定 reference 主状态。

## 8. 本轮收口内容
- `example/HelloCustomWidgets/feedback/tool_tip/test.c`
  新增 `ui_ready`、`layout_local_views()`、`layout_page()` 和统一 `request_page_snapshot()`，把录制轨道收口为主区 reference 状态与最终稳定帧；底部 preview 全程保持真正静态。
- `example/HelloUnitTest/test/test_tool_tip.c`
  新增 `tool_tip_preview_snapshot_t`、`assert_region_equal()`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()`，把静态 preview 用例收口为完整状态不变断言。
- `example/HelloCustomWidgets/feedback/tool_tip/readme.md`
  按当前 workflow 模板重写，并回填 compile / unit / runtime / web 结果和 runtime 复核数字。

## 9. 录制动作设计
`egui_port_get_recording_action()` 当前轨道如下：

1. 还原 `Save / closed`，同步底部两个静态 preview，并请求首帧。
2. 对主区 `Save` 触发程序化 delay，等待 `560ms` 后请求 `Save / delayed open`。
3. 切到 `Search / top placement` 并请求快照。
4. 切到 `Publish / warning open` 并请求快照。
5. 发送 `Esc`，请求 `Publish / Esc close` 快照。
6. 回到默认 `Save / closed` 并请求最终稳定帧。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，在请求前统一布局和刷新。
- 底部 preview 在整条轨道中不承接任何状态切换职责。
- 主区变化只来自 reference snapshot 切换、delay 打开和 `Esc` 关闭。

## 10. 单元测试口径
`example/HelloUnitTest/test/test_tool_tip.c` 当前覆盖九部分：

1. 默认初始化状态
2. `show_delay / snapshot / compact / read_only / open / font / meta_font / palette` setter 清理 pending 交互状态
3. 触摸点击启动 delay，第二次点击关闭
4. 触摸移出目标后取消 delay
5. 键盘 `Enter` 延时打开，`Esc` 关闭
6. 未处理按键清理 pressed / key_active
7. `read_only` 与 `!enable` guard
8. 静态 preview 吞掉输入后保持状态不变
9. attach / detach 恢复 pending timer

其中静态 preview 用例通过 `tool_tip_preview_snapshot_t` 固定校验：
`region_screen / background / snapshots / font / meta_font / on_click_listener / api / palette / show_delay_ms / snapshot_count / current_snapshot / current_part / compact_mode / read_only_mode / open / timer_started / pending_show / touch_active / key_active / toggle_on_release / alpha / enable / is_focused / is_pressed / padding`

## 11. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/tool_tip --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/tool_tip
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_tool_tip
```

## 12. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`
  `make all APP=HelloCustomWidgets APP_SUB=feedback/tool_tip PORT=pc`
- `HelloUnitTest`：`PASS`
  在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `tool_tip` suite `9 / 9`
- `sync_widget_catalog.py`：`PASS`
  同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`
  `custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`
  `134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`
  `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`
  `12 frames captured -> runtime_check_output/HelloCustomWidgets_feedback_tool_tip/default`
- feedback 分类 compile/runtime 回归：`PASS`
- wasm 构建：`PASS`
  `web/demos/HelloCustomWidgets_feedback_tool_tip`
- web smoke：`PASS`
  `status=Running canvas=480x480 ratio=0.1815 colors=127`

## 13. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_tool_tip/default`

- 总帧数：`12`
- 主区 RGB 差分边界：`(54, 97) - (426, 202)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`5`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `tool_tip` reference 主体；遮罩差分边界后，主区外页面 chrome 在整条录制轨道中保持静态。
- `12` 帧里主区共出现 `5` 组唯一状态，对应 `Save / closed`、`Save / delayed open`、`Search / top placement`、`Publish / warning open` 和 `Publish / Esc close`；最终稳定帧回到默认 `Save / closed`。
- 按 `y >= 203` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条轨道中始终静态一致。

## 14. 已知限制
- 当前版本仍是页内固定锚点的 reference，不做系统级 popup 跟随。
- 当前不做自动避让、屏幕边缘翻转和复杂动画。
- 当前以固定 snapshot 数据保证录制稳定，不接业务侧动态文案来源。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部提示容器。

## 15. 与现有控件的边界
- 相比 `teaching_tip`：这里是轻量目标提示，不承担引导式布局和多动作卡片职责。
- 相比 `message_bar`：这里是围绕目标的锚定气泡，不是页内横向反馈条。
- 相比 `toast_stack`：这里不承担瞬时通知队列，只表达目标提示和快捷说明。

## 16. EGUI 适配说明
- `tool_tip` custom 实现继续留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用 static preview API，吞掉输入并清理残留交互状态。
- README、demo、单测、runtime 验收和 web 发布口径已经对齐到当前 `reference` workflow。
