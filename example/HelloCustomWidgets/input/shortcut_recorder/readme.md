# shortcut_recorder 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级参考：`WinUI KeyboardAccelerator / shortcut editor`
- 对应组件语义：`Keyboard shortcut field / accelerator recorder`
- 当前保留形态：`Ctrl + K`、`listening`、`Ctrl + Shift + P`、`Ctrl + 1 preset`、`clear binding`、`compact`、`read only`
- 当前保留交互：主区保留 `same-target release`、监听态捕获、preset 应用、clear binding、`Tab / Left / Right / Home / End / Up / Down` 导航、`Enter / Space` 提交与 `Escape` 取消；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：preview 清焦桥接、preview click 录制轨道、`compact` preview 状态切换、旧版 guide / 状态回显 / 外部 preview 标签，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `egui_view_shortcut_recorder` 包装层，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`shortcut_recorder` 用来表达“进入监听态后捕获一组快捷键并保存”的输入语义，适合命令面板、全局搜索、常用动作加速键和工作区设置页。它不是自由文本输入，也不是普通按钮集合，而是一个带监听态、预设项和清空动作的快捷键编辑器。

## 2. 为什么现有控件不够用
- `textinput` 关注自由文本编辑，不表达快捷键监听与捕获。
- `token_input` 关注多 token 管理，不表达单个组合键绑定。
- `command_bar` 和 `command_palette` 负责触发命令，不负责录入并保存快捷键。
- 当前 reference 主线仍需要一个更接近 Fluent / WPF UI 语义的快捷键录入控件。

## 3. 当前页面结构
- 标题：`Shortcut Recorder`
- 主区：一个标准 `shortcut_recorder`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Ctrl + Shift + P`
- 右侧 preview：`read only`，固定显示 `Ctrl + S`
- 页面结构：标题 -> 主控件 -> `compact / read only` 双 preview

目录：
- `example/HelloCustomWidgets/input/shortcut_recorder/`

## 4. 主区 reference 快照
主区录制轨道保留 `5` 组 reference 状态，最终稳定帧回到默认 `Ctrl + K`；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   绑定：`Ctrl + K`
   状态：默认 field
2. 快照 2
   绑定：`Ctrl + K`
   状态：`listening`
3. 快照 3
   绑定：`Ctrl + Shift + P`
   状态：capture 新绑定
4. 快照 4
   绑定：`Ctrl + 1`
   状态：preset apply
5. 快照 5
   绑定：空
   状态：`clear binding`
6. 最终稳定帧
   绑定：`Ctrl + K`
   状态：默认 field

底部 preview 在整条轨道中始终固定：

1. `compact`
   绑定：`Ctrl + Shift + P`
2. `read only`
   绑定：`Ctrl + S`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 264`
- 主控件：`194 x 138`
- 底部容器：`218 x 82`
- 单个 preview：`106 x 82`
- 页面结构：标题 -> 主控件 -> `compact / read only`
风格约束：
- 使用低噪音浅色 page panel 和白色 surface，不保留旧版 guide、状态回显和外部标签。
- 主控件保留标题、辅助文案、快捷键 token、监听态 pill、preset 行和 clear 动作。
- 焦点 ring 只在主控件真实获得 focus、控件可用且非 `read only` 时绘制，底部 preview 不再伪装成可交互焦点目标。

## 6. 状态矩阵

| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认绑定显示 | 是 | 是 | 是 |
| `listening` | 是 | 否 | 否 |
| capture 新绑定 | 是 | 否 | 否 |
| preset apply | 是 | 否 | 否 |
| clear binding | 是 | 否 | 否 |
| 最终稳定帧回到默认绑定 | 是 | 否 | 否 |
| `same-target release` | 是 | 否 | 否 |
| 静态 preview 吞 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_shortcut_recorder.c` 当前覆盖 `10` 条用例：

1. 进入监听态并捕获新绑定。
   覆盖 `Enter` 进入 `listening`，再通过 `Ctrl + Shift + P` 捕获新绑定并退出监听态。
2. 键盘切换 preset 并应用。
   覆盖 `Tab -> Down -> Space` 把当前 binding 切到第二个 preset。
3. 触摸 preset 与 clear。
   覆盖触摸 preset 应用 `Ctrl + 1`，以及触摸 `Clear` 清空当前 binding。
4. `Escape` 取消监听。
   覆盖监听态下 `Escape` 退出监听，并保持原始 `Ctrl + K` 绑定。
5. setter 清理 `pressed`。
   覆盖 `set_header()`、`set_palette()`、`set_presets()`、`set_binding()`、`clear_binding()`、`set_listening()`、`apply_preset()`、`set_current_part()`、`set_current_preset()`、`set_compact_mode()` 与 `set_read_only_mode()` 的残留 pressed 清理。
6. `same-target release` 语义。
   覆盖 `DOWN(A) -> UP(B)` 不提交，以及 field / preset / clear 三类区域都必须同目标释放才提交。
7. `ACTION_CANCEL` 清理 pressed 且不 notify。
   覆盖触摸取消时 `pressed_part / pressed_preset / is_pressed` 全部清空，且不改 binding。
8. `compact / read only / disabled` guard 清理 pressed。
   覆盖三类 guard 下的 `touch / key` 输入先清 pressed 再返回，不改状态。
9. static preview 吞输入且清理 pressed。
   覆盖 preview 侧 `touch` 与 `Tab` 输入不会改写 `binding / listening / current part / current preset`。
10. region 暴露随状态变化。
   覆盖 `field / preset / clear` 区域在有无 binding 场景下的 region 是否正确暴露。

说明：
- 主控件保留真实 `shortcut_recorder` 语义：触摸 `field` 切换 `listening`，触摸 preset 行应用对应快捷键绑定，触摸 `Clear` 清空当前绑定。
- `Tab / Left / Right` 在 `field / preset / clear` 间循环；`Up / Down / Minus / Plus / Home / End` 在 preset 列表中移动；`Enter / Space` 按当前 part 提交；`Escape` 在监听态取消捕获，非监听态下收回到 `field`。
- `shortcut_recorder` 不是拖拽控件，触摸提交必须满足同目标释放：`DOWN(A) -> UP(B)` 不提交，`ACTION_MOVE` 只保持手势占用，不改写按下目标。
- `compact` 与 `read only` preview 统一通过 `egui_view_shortcut_recorder_override_static_preview_api()` 覆盖为静态 API；preview 吞掉 `touch / key` 输入，并在入口先清理残留 `pressed_part / pressed_preset / is_pressed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主控件默认绑定、同步底部双静态 preview、聚焦主控件并请求首帧截图，等待 `SHORTCUT_RECORD_FRAME`。
2. 通过 `Enter` 进入 `listening`，等待 `SHORTCUT_RECORD_WAIT`。
3. 请求 listening 帧，等待 `SHORTCUT_RECORD_FRAME`。
4. 通过 `Ctrl + Shift + P` 捕获新绑定，等待 `SHORTCUT_RECORD_WAIT`。
5. 请求 capture 帧，等待 `SHORTCUT_RECORD_FRAME`。
6. 通过 `Tab + End + Enter` 应用最后一个 preset，等待 `SHORTCUT_RECORD_WAIT`。
7. 请求 preset apply 帧，等待 `SHORTCUT_RECORD_FRAME`。
8. 通过 `Tab + Enter` 清空 binding，等待 `SHORTCUT_RECORD_WAIT`。
9. 请求 clear binding 帧，等待 `SHORTCUT_RECORD_FRAME`。
10. 显式恢复默认 `Ctrl + K` 绑定并恢复主控件 focus，等待 `SHORTCUT_RECORD_FINAL_WAIT`。
11. 请求最终稳定帧，并继续等待 `SHORTCUT_RECORD_FINAL_WAIT`。

说明：
- 录制阶段只保留主区真实键盘轨道，不再点击 preview，也不再切换 `compact` preview 状态。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留 `SHORTCUT_RECORD_WAIT`、`SHORTCUT_RECORD_FRAME`、`SHORTCUT_RECORD_FINAL_WAIT`、`apply_primary_default_state()`、`apply_preview_states()`、`focus_primary_recorder()` 与显式布局重放路径。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证五组主区状态和最终稳定帧口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/shortcut_recorder --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/shortcut_recorder
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_shortcut_recorder
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主控件交互结束后不能残留 pressed 污染。
- `Ctrl + K`、listening、`Ctrl + Shift + P` capture、preset apply 与 clear binding 五组 reference 状态必须能从截图中稳定区分。
- listening pill、快捷键 token、preset 行和 clear 行为都要保持 Fluent / WPF UI 的低噪音层级。
- 最终稳定帧必须显式回到默认 `Ctrl + K` 绑定。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_shortcut_recorder` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_shortcut_recorder/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,10,11,12] / [2,3] / [4,5] / [6,7] / [8,9]`
  - 主区 RGB 差分边界收敛到 `(55, 104) - (424, 267)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 268` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `textinput`：这里表达的是组合键捕获，不是自由文本编辑。
- 相比 `token_input`：这里只处理单个快捷键绑定，不做多 token 管理。
- 相比 `command_bar`：这里负责录入和保存快捷键，不负责触发命令。

## 13. 本轮保留与删减
- 保留的主区状态：
  - 默认 `Ctrl + K`
  - `listening`
  - `Ctrl + Shift + P` capture
  - `preset apply`
  - `clear binding`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 监听态捕获
  - preset 应用
  - clear binding
  - `same-target release`
  - `Tab / Left / Right / Home / End / Up / Down` 导航
  - `Enter / Space` 提交与 `Escape` 取消
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - preview 清焦桥接与 preview click 录制轨道
  - `compact` preview 状态切换
  - 旧版 guide、状态回显和外部 preview 标签
  - 系统级快捷键冲突检测、保留键校验和真实持久化写入
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `shortcut_recorder` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/shortcut_recorder --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_shortcut_recorder/default`
  - 共捕获 `13` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/shortcut_recorder`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_shortcut_recorder`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2049 colors=156`
- 截图复核结论：
  - 主区覆盖默认 `Ctrl + K`、listening、`Ctrl + Shift + P` capture、新 preset 应用与 clear binding 五组 reference 状态
  - 最终稳定帧已显式回到默认 `Ctrl + K` 绑定
  - 主区 RGB 差分边界收敛到 `(55, 104) - (424, 267)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 268` 裁切后全程保持单哈希静态
