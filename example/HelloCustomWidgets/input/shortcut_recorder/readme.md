# shortcut_recorder 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级参考：`WinUI KeyboardAccelerator / shortcut editor`
- 对应组件语义：`Keyboard shortcut field / accelerator recorder`

## 1. 为什么需要这个控件
`shortcut_recorder` 用来表达“进入监听态后捕获一组快捷键并保存”的输入语义，适合命令面板、全局搜索、常用动作加速键和工作区设置页。它不是自由文本输入，也不是普通按钮集合，而是一个带监听态、预设项和清空动作的快捷键编辑器。

## 2. 为什么现有控件不够用
- `textinput` 关注自由文本编辑，不表达快捷键监听与捕获。
- `token_input` 关注多 token 管理，不表达单个组合键绑定。
- `command_bar` 和 `command_palette` 负责触发命令，不负责录入并保存快捷键。
- 当前 reference 主线仍需要一个更接近 Fluent / WPF UI 语义的快捷键录入控件。

## 3. 目标场景与示例概览
- 主区域展示标准 `shortcut_recorder`，覆盖默认绑定、监听态、捕获新组合键、应用 preset 和清空绑定。
- 底部左侧展示 `compact` 静态 preview，只保留紧凑摘要。
- 底部右侧展示 `read only` 静态 preview，表达锁定态摘要。
- 页面结构收敛为：标题 -> 主控件 -> `compact / read only` 双 preview。

目录：
`example/HelloCustomWidgets/input/shortcut_recorder/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 264`
- 主控件：`194 x 138`
- 底部容器：`218 x 82`
- 单个 preview：`106 x 82`

视觉约束：
- 使用低噪音浅色 page panel 和白色 surface，不保留旧版 guide、状态回显和外部标签。
- 主控件保留标题、辅助文案、快捷键 token、监听态 pill、preset 行和 clear 动作。
- 焦点 ring 只在主控件真实获得 focus、控件可用且非 `read only` 时绘制，底部 preview 不再伪装成可交互焦点目标。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 | 初始状态 | 用途 |
| --- | --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 264` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Shortcut Recorder` | 页面标题 |
| `recorder_primary` | `egui_view_shortcut_recorder_t` | `194 x 138` | `Ctrl + K` | 主交互控件 |
| `recorder_compact` | `egui_view_shortcut_recorder_t` | `106 x 82` | compact | 紧凑静态 preview |
| `recorder_read_only` | `egui_view_shortcut_recorder_t` | `106 x 82` | compact + read only | 只读静态 preview |

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认绑定显示 | 是 | 是 | 是 |
| listening | 是 | 否 | 否 |
| capture 新绑定 | 是 | 否 | 否 |
| preset apply | 是 | 否 | 否 |
| clear binding | 是 | 否 | 否 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 7. 交互语义

### 7.1 主控件
- 触摸 `field` 切换 `listening`。
- 触摸 preset 行应用对应快捷键绑定。
- 触摸 `Clear` 清空当前绑定。
- `Tab / Left / Right` 在 `field / preset / clear` 之间循环。
- `Up / Down / Minus / Plus / Home / End` 在 preset 列表中移动。
- `Enter / Space` 按当前 part 提交。
- `Escape` 在监听态下取消捕获；非监听态下把当前 part 收回到 `field`。
- `ACTION_DOWN` 命中主控件时会请求 focus。

### 7.2 非拖拽 release 语义
`shortcut_recorder` 不是拖拽控件，触摸提交必须满足同目标释放：
- `DOWN(field) -> UP(field)` 才允许切换监听态。
- `DOWN(preset A) -> UP(preset A)` 才允许应用 preset。
- `DOWN(clear) -> UP(clear)` 才允许清空绑定。
- `DOWN(A) -> UP(B)` 不提交。
- `ACTION_MOVE` 只保持手势占用，不改写按下目标。

### 7.3 静态 preview
- `compact` 与 `read only` preview 统一通过 `egui_view_shortcut_recorder_override_static_preview_api()` 覆盖为静态 API。
- preview 吞掉 touch / key 输入，并在入口先清理残留 `pressed_part / pressed_preset / is_pressed`。
- 点击 preview 只负责清掉主控件 focus，不再承担监听、preset 或 clear 行为。

## 8. 本轮交互收口内容
- 新增统一的 `shortcut_recorder_clear_pressed_state()`。
- 补齐 setter 链路的 pressed 清理：
  - `set_header()`
  - `set_palette()`
  - `set_presets()`
  - `set_binding()`
  - `commit_binding()`
  - `clear_binding()`
  - `set_listening()`
  - `apply_preset()`
  - `set_current_part()`
  - `set_current_preset()`
  - `set_compact_mode()`
  - `set_read_only_mode()`
- touch / key guard 在 `compact / read only / disabled` 下都会先清残留 pressed 再返回。
- `ACTION_UP / ACTION_CANCEL` 收口到统一清理逻辑，避免交互结束后残留 pressed 污染。
- 主控件的 focus ring 改为只在真实 focus 下绘制。

## 9. 交互测试覆盖
`example/HelloUnitTest/test/test_shortcut_recorder.c` 本轮覆盖了以下回归：
- 进入监听态并捕获新绑定
- 键盘切换 preset 并应用
- 触摸 preset 与 clear
- `Escape` 取消监听
- setter 清理 pressed
- 同目标 release 语义
- `ACTION_CANCEL` 清理 pressed 且不 notify
- `compact / read only / disabled` guard 清理 pressed
- static preview 吞输入且不改动 `binding / listening / current part`
- region 暴露随状态变化

## 10. 录制动作设计
`egui_port_get_recording_action()` 当前录制链路：
1. 应用主控件与 `compact` 默认场景。
2. 抓取初始截图。
3. 触摸主控件 field，进入监听态。
4. 抓取 listening 截图。
5. 通过 `Ctrl + Shift + P` 捕获新绑定。
6. 抓取 capture 后截图。
7. 通过 `Tab + End + Enter` 应用最后一个 preset。
8. 抓取 preset 应用截图。
9. 通过 `Tab + Enter` 清空绑定，并切换 `compact` 静态对照。
10. 抓取清空后的截图。
11. 点击 `compact` preview，验证静态 preview 只清主控件 focus。
12. 抓取最终截图。

## 11. 编译、检查与验收命令
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc
make all APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/shortcut_recorder --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主控件交互结束后不能残留 pressed 污染。
- listening pill、快捷键 token、preset 行和 clear 行为都要保持 Fluent / WPF UI 的低噪音层级。
- 底部 preview 必须保持静态 reference，对输入只吞不改状态。

## 12. 已知限制
- 当前只覆盖 `Ctrl` 与 `Shift` 两种修饰键，不扩展到 `Alt / Win`。
- 当前不做系统级快捷键冲突检测、保留键校验和真实持久化写入。
- preset 仍是固定数组，示例不接真实用户配置源。

## 13. 与现有控件的差异边界
- 相比 `textinput`：这里表达的是组合键捕获，不是自由文本编辑。
- 相比 `token_input`：这里只处理单个快捷键绑定，不做多 token 管理。
- 相比 `command_bar`：这里负责录入和保存快捷键，不负责触发命令。

## 14. EGUI 适配说明
- 通过固定 preset 和固定录制脚本保证 `480 x 480` 下的稳定审阅。
- 底部 preview 统一走静态 API，避免为了对照控件再维护第二套可交互实现。
- 当前优先保证状态清理、release 语义和 runtime 渲染稳定，再考虑是否上升为公共框架控件。
