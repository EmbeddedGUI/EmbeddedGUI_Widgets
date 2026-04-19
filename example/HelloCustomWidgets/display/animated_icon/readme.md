# animated_icon 自定义控件设计说明

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 补充对照控件：`symbol_icon`、`font_icon`、`path_icon`
- 对应组件：`AnimatedIcon`
- 当前保留形态：主区 `Normal`、`PointerOver`、`Pressed` 三组目标 reference 快照，以及底部 `chevron / fallback` 静态 preview
- 当前保留交互：状态驱动图标过渡、内置 source 切换、fallback glyph、主区动画推进、静态 preview 输入抑制
- 当前移除内容：`Lottie / JSON` 解析、任意 marker 导入、系统级动画偏好桥接、主区说明 note、preview 面板包装与额外页面 chrome
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_animated_icon`，当前只收口 `AnimatedBackVisualSource` 与 `AnimatedChevronDownSmallVisualSource` 两个内置 source，以及 `source = NULL` 时的 fallback glyph 路径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`animated_icon` 用来表达“图标不是静态资源，而是根据控件状态在几个关键姿态之间过渡”的语义。它适合挂在按钮、导航入口、折叠入口等轻量状态反馈上，避免为了一个小图标引入额外场景容器或位图序列。

## 2. 为什么现有控件不够用

- `symbol_icon`、`font_icon`、`path_icon` 只能直接切换最终图形，没有状态过渡。
- `animated_image` 面向帧序列图片，不适合表达由 `Normal / PointerOver / Pressed` 驱动的单色图标动画。
- 直接在业务页手写计时器与 source / fallback / preview 桥接，会把 reference 语义散落到页面代码里，缺少统一收口。

## 3. 当前页面结构

- 标题：`AnimatedIcon`
- 主区：一个 `AnimatedBackVisualSource` 主图标与一个状态 label
- 底部：一行并排的两个真正静态的 preview
- 左侧 preview：`AnimatedChevronDownSmallVisualSource / Pressed`
- 右侧 preview：`source = NULL` 时的 `Settings` fallback glyph
- 页面结构统一收口为：标题 -> 主 `animated_icon` -> 状态 label -> 底部 `chevron / fallback`

目录：`example/HelloCustomWidgets/display/animated_icon/`

## 4. 主区 reference 快照

主区录制轨道只保留 3 组目标快照和最终稳定帧：

1. `Normal`
   source：`AnimatedBackVisualSource`
2. `PointerOver`
   source：`AnimatedBackVisualSource`
3. `Pressed`
   source：`AnimatedBackVisualSource`
4. `Normal`
   source：`AnimatedBackVisualSource`
   作为最终稳定帧

底部 preview 在整条轨道中始终固定：

1. `chevron`
   source：`AnimatedChevronDownSmallVisualSource`
   状态：`Pressed`
2. `fallback`
   source：`NULL`
   glyph：`Settings`
   状态：`Normal`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 160`
- 标题：`224 x 18`
- 主图标：`60 x 60`
- 状态 label：`224 x 12`
- 底部 preview 行：`64 x 28`
- 单个 preview 图标：`28 x 28`
- 页面结构：标题 -> 主 `animated_icon` -> 状态 label -> 底部 `chevron / fallback`
- 页面风格：浅色 page panel、单色 icon palette、仅主区保留状态过渡，底部 preview 只做静态 reference

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Chevron preview | Fallback preview |
| --- | --- | --- | --- |
| `Normal` | 是 | 否 | 否 |
| `PointerOver` | 是 | 否 | 否 |
| `Pressed` | 是 | 否 | 否 |
| 最终稳定帧回到 `Normal` | 是 | 否 | 否 |
| `Pressed` chevron | 否 | 是 | 否 |
| `Normal` fallback | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_animated_icon.c` 当前覆盖 `4` 部分：

1. 主控件初始化与默认语义
   覆盖 `AnimatedBackVisualSource`、默认 `Normal` 状态、默认 fallback glyph、默认 palette、默认 progress 与字体解析路径
2. setter 与状态解析守卫
   覆盖 `apply_subtle_style()`、`set_source()`、`set_state()`、`set_fallback_glyph()`、`set_icon_font()`、`set_animation_enabled()` 对 `pressed` 状态的清理和目标状态解析
3. attached 动画与 hard cut
   覆盖 `NormalToPressed` 过渡、attach 后计时器启动、`tick()` 推进以及禁用动画后的直接落终态
4. 静态 preview 不变性断言
   通过 `animated_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`source`、`icon_font`、`fallback_glyph`、`icon_color`、`current_progress`、`from_progress`、`to_progress`、`current_state`、`animation_enabled`、`fallback_overridden`、`anim_step`、`anim_steps`、`timer_started`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding`

补充说明：

- 静态 preview 用例已收口为 “consumes input and keeps state”
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`
- 主区重点不在点击提交，而在状态字符串解析、动画推进和 fallback glyph 路径

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Normal` 和底部 preview 固定状态，请求首帧并等待 `ANIMATED_ICON_RECORD_FRAME_WAIT = 170`
2. 切到 `PointerOver`，等待 `ANIMATED_ICON_RECORD_ANIM_WAIT = 260`
3. 请求第二组主区快照，并继续等待 `170`
4. 切到 `Pressed`，等待 `260`
5. 请求第三组主区快照，并继续等待 `170`
6. 回到默认 `Normal`，等待 `260`
7. 请求最终稳定帧，并继续等待 `ANIMATED_ICON_RECORD_FINAL_WAIT = 280`

说明：

- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径
- 页面层不再保留主区说明 note、preview 说明文字和面板包装
- 底部 preview 统一通过 `egui_view_animated_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/animated_icon PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/animated_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/animated_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_animated_icon
```

## 10. 验收重点

- 主区和底部 `chevron / fallback` preview 必须完整可见，不能黑屏、白屏或裁切
- 主区 `Normal / PointerOver / Pressed` 三组目标状态，以及中间过渡帧，必须能从截图中稳定区分
- 最终稳定帧必须显式回到默认 `Normal`
- 主区 `source / state / fallback / icon_font / animation_enabled` 收口后不能残留错误 timer 状态或异常 progress
- 底部 `chevron / fallback` preview 必须保持静态 reference，对输入只吞不改状态

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_animated_icon/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(201, 162) - (270, 262)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`5`
  - 按 `y >= 280` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Normal`

## 12. 与现有控件的边界

- 相比 `symbol_icon`、`font_icon`、`path_icon`：这里表达状态驱动的图标过渡，不是静态图标切换
- 相比 `animated_image`：这里强调图标语义和状态过渡，不承载位图帧序列播放
- 相比 `bitmap_icon`：这里聚焦单色动态图标，不承担多色位图资源展示

## 13. 本轮保留与删减

- 保留的主区状态：`Normal`、`PointerOver`、`Pressed`
- 保留的底部对照：`chevron`、`fallback`
- 保留的交互与实现约束：状态驱动过渡、内置 source 切换、fallback glyph、动画推进、static preview 输入抑制
- 删减的旧桥接与旧装饰：`Lottie / JSON` 解析、任意 marker 导入、系统级动画偏好桥接、主区说明 note、preview 面板包装与额外页面 chrome

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/animated_icon PORT=pc`
  - 本轮沿用 `2026-04-18` 已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `animated_icon` suite `4 / 4`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 本轮重新执行文档编码与 display 类触摸语义检查，其余结果沿用已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/animated_icon --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_animated_icon/default`
  - 本轮沿用已归档 runtime 结果，并按 tracker 最新记录采用 `8` 帧 / `5` 组主区状态的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-18` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_animated_icon`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1253 colors=93`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(201, 162) - (270, 262)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`5`
  - 按 `y >= 280` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Normal / PointerOver / Pressed` 三个目标状态与两段中间过渡帧，最终稳定帧已回到默认 `Normal`，底部 `chevron / fallback` preview 全程静态
