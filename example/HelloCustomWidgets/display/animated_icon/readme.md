# animated_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`AnimatedIcon`
- 当前保留语义：状态驱动图标过渡、内置 source 切换、fallback glyph、静态 preview 输入抑制
- 当前移除内容：`Lottie / JSON` 解析、任意 marker 导入、系统级动画偏好桥接、SDK 改动
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_animated_icon`，当前只收口 `AnimatedBackVisualSource` 与 `AnimatedChevronDownSmallVisualSource` 两个内置 source，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`animated_icon` 用来表达“图标不是静态资源，而是根据控件状态在几个关键姿态之间过渡”的语义。它适合挂在按钮、导航入口、折叠入口等轻量状态反馈上，避免为了一个小图标引入额外场景容器或位图序列。

## 2. 为什么现有控件不够用
- `symbol_icon`、`font_icon`、`path_icon` 只能直接切换最终图形，没有状态过渡。
- `animated_image` 面向帧序列图片，不适合表达由 `Normal / PointerOver / Pressed` 驱动的单色图标动画。
- 直接在业务页手写计时器与 source / fallback / preview 桥接，会把 reference 语义散落到页面代码里，缺少统一收口。

## 3. 当前页面结构
- 标题：`AnimatedIcon`
- 主区：一个 `AnimatedBackVisualSource` 主图标与一个状态 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`AnimatedChevronDownSmallVisualSource / Pressed`
- 右侧 preview：`source = NULL` 时的 `Settings` fallback glyph

目录：
- `example/HelloCustomWidgets/display/animated_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   状态：`Normal`
   source：`AnimatedBackVisualSource`
2. 快照 2
   状态：`PointerOver`
   source：`AnimatedBackVisualSource`
3. 快照 3
   状态：`Pressed`
   source：`AnimatedBackVisualSource`

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
- 风格约束：浅色 page panel、单色 icon palette、仅主区保留状态过渡，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Chevron preview | Fallback preview |
| --- | --- | --- | --- |
| `Normal` | 是 | 否 | 是 |
| `PointerOver` | 是 | 否 | 否 |
| `Pressed` | 是 | 是 | 否 |
| `NormalToPressed` 等过渡字符串 | 是 | 否 | 否 |
| `source = NULL` 使用 fallback glyph | 否 | 否 | 是 |
| `set_animation_enabled(0)` 直接落最终帧 | 支持 | 支持 | 支持 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `PointerOver`，等待动画完成
4. 抓取第二组主区快照
5. 切到 `Pressed`，等待动画完成
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切 `Normal` 后额外抓帧。
- 页面层不再保留主区说明 note、preview 说明文字和面板包装。
- 底部 preview 统一通过 `egui_view_animated_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_animated_icon.c` 当前覆盖四部分：

1. 主控件初始化与默认语义
   覆盖 `AnimatedBackVisualSource`、默认 `Normal` 状态、默认 fallback glyph、默认 palette、默认 progress 与字体解析路径。
2. setter 与状态解析守卫
   覆盖 `apply_subtle_style()`、`set_source()`、`set_state()`、`set_fallback_glyph()`、`set_icon_font()`、`set_animation_enabled()` 对 `pressed` 状态的清理和目标状态解析。
3. attached 动画与 hard cut
   覆盖 `NormalToPressed` 过渡、attach 后计时器启动、`tick()` 推进以及禁用动画后的直接落终态。
4. 静态 preview 不变性断言
   通过 `animated_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`source`、`icon_font`、`fallback_glyph`、`icon_color`、`current_progress`、`from_progress`、`to_progress`、`current_state`、`animation_enabled`、`fallback_overridden`、`anim_step`、`anim_steps`、`timer_started`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

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

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/animated_icon PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `animated_icon` suite `4 / 4`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/animated_icon --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/animated_icon`，输出 `web/demos/HelloCustomWidgets_display_animated_icon`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_animated_icon`，结果 `PASS status=Running canvas=480x480 ratio=0.1253 colors=93`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_animated_icon/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(201, 162) - (270, 262)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`5`
- 按 `y >= 280` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

说明：
- 主区 `5` 组唯一状态对应 `Normal`、`PointerOver`、`Pressed` 三个目标状态，以及两段保留下来的中间过渡帧。
- 主区外唯一哈希数与底部 preview 区唯一哈希数都为 `1`，确认页面 chrome 与底部 preview 在整条录制轨道中保持静态。

## 12. 已知限制
- 当前不解析 `Lottie / JSON` 动画资源，只支持内置 source。
- 当前只覆盖常见的 `Normal / PointerOver / Pressed` 族及对应过渡字符串。
- 当前动画由固定步长定时器推进，目标是 reference 演示稳定，不追求复杂 easing。
- 当前 fallback 只支持静态 glyph，不扩展完整 `IconSource` 对象树。

## 13. 与现有控件的边界
- 相比 `symbol_icon`、`font_icon`、`path_icon`：这里表达状态驱动的图标过渡，不是静态图标切换。
- 相比 `animated_image`：这里强调图标语义和状态过渡，不承载位图帧序列播放。
- 相比 `bitmap_icon`：这里聚焦单色动态图标，不承担多色位图资源展示。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_animated_icon` custom view，不修改 SDK。
- 主区保留 `Normal / PointerOver / Pressed` 三组目标 reference 快照。
- 底部 preview 通过 `egui_view_animated_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证 `Back` 与 `Chevron` 两个内置 source、fallback glyph 路径、静态 preview 与 runtime 录制稳定，再评估是否需要扩展到更多 source。
