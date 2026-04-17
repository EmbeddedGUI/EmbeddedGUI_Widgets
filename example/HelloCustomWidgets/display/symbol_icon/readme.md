# SymbolIcon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`SymbolIcon`
- 当前保留语义：只读符号图标显示、主区 reference 快照、`subtle / accent` 静态 preview、静态 preview 输入抑制
- 当前移除内容：旧主 `panel / heading / note`、底部 preview 标题和说明文案、录制末尾回切默认态恢复帧、动画、hover 效果、额外装饰边框和场景化叙事
- EGUI 适配说明：仓库没有单独的 Fluent Symbols 资源，本控件继续复用内置 `Material Symbols` 字体子集表达 `SymbolIcon` 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`symbol_icon` 适合在导航、状态提示和设置入口里承载低噪音图标语义。它比图片资源更轻量，也比组合型 badge 更纯粹，适合作为 reference 主线里的基础显示控件。

## 2. 为什么现有控件不够用
- `font_icon` 更偏通用 glyph 容器，`symbol_icon` 负责把 Material Symbols 的语义图标显示收口成更小的 facade。
- `image_icon` 依赖位图资源，不适合这类纯字体图标的轻量展示。
- `info_badge`、`badge` 关注的是徽标或状态提醒，不是单个只读符号图标。

## 3. 当前页面结构
- 标题：`SymbolIcon`
- 主区：一个主 `symbol_icon` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`subtle`
- 右侧 preview：`accent`

目录：
- `example/HelloCustomWidgets/display/symbol_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照和最终稳定帧：

1. 默认态
   文案：`Home / standard`
   图标：`EGUI_ICON_MS_HOME`
   颜色：`#0F6CBD`
2. 快照 2
   文案：`Notifications / accent`
   图标：`EGUI_ICON_MS_NOTIFICATIONS`
   颜色：`#A15C00`
3. 快照 3
   文案：`Settings / success`
   图标：`EGUI_ICON_MS_SETTINGS`
   颜色：`#0F7B45`
4. 最终稳定帧
   文案：`Home / standard`
   图标：`EGUI_ICON_MS_HOME`
   颜色：`#0F6CBD`

底部 preview 在整条轨道中始终固定：

1. `subtle`
   图标：`EGUI_ICON_MS_SEARCH`
   palette：`subtle`
2. `accent`
   图标：`EGUI_ICON_MS_INFO`
   palette：`accent`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 164`
- 标题：`224 x 18`
- 主 `symbol_icon`：`64 x 64`
- 主状态 label：`224 x 12`
- 底部 preview 行：`72 x 32`
- 单个 preview：`32 x 32`
- 页面结构：标题 -> 主 `symbol_icon` -> 状态 label -> 底部 `subtle / accent`
- 风格约束：浅色 page panel、主图标居中、状态色只通过 glyph 颜色表达，不再依赖额外卡片包装或说明文案

## 6. 状态矩阵
| 状态 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认显示 | `Home / standard` | `EGUI_ICON_MS_SEARCH` + `subtle` | `EGUI_ICON_MS_INFO` + `accent` |
| 快照 2 | `Notifications / accent` | 保持不变 | 保持不变 |
| 快照 3 | `Settings / success` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Home / standard` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Notifications / accent`
4. 抓取第二组主区快照
5. 切到 `Settings / success`
6. 抓取第三组主区快照
7. 回到默认 `Home / standard`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧 `primary_panel`、`heading / note`、底部 preview 标题与说明文案。
- 底部 preview 统一通过 `egui_view_symbol_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `SYMBOL_ICON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_symbol_icon.c` 当前覆盖三部分：

1. style helper 与 palette 清理
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()` 对 `pressed` 状态的清理和颜色切换。
2. setter 更新与默认回退
   覆盖 `set_symbol()`、`set_icon_font()`、`set_palette()` 与 `NULL` 符号回退到默认 `info` 图标。
3. 静态 preview 不变性断言
   通过 `symbol_icon_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / symbol / icon_font / icon_color / on_click_listener / api / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/symbol_icon PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/symbol_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/symbol_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_symbol_icon
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/symbol_icon PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `symbol_icon` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_symbol_icon/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_symbol_icon`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1284 colors=83`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_symbol_icon/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(183, 182) - (296, 267)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 268` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `symbol_icon` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应 `Home / standard`、`Notifications / accent`、`Settings / success` 三组主区快照；最终稳定帧已显式回到默认 `Home / standard`。
- 按 `y >= 268` 裁剪底部 preview 区域后保持单哈希，确认 `subtle / accent` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖只读 `SymbolIcon` 显示语义，不承载按钮点击、宿主布局或动画职责。
- 当前继续复用 `Material Symbols` 字体子集，不追求完整 Fluent Symbols 资源覆盖。
- 底部 `subtle / accent` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `font_icon`：这里收口的是固定符号语义和标准 palette，不是通用 glyph 文本容器。
- 相比 `image_icon`：这里不依赖位图资源，适合纯字体图标场景。
- 相比 `info_badge`：这里表达的是独立图标语义，不是徽标或提醒状态。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_symbol_icon` custom view，不修改 SDK。
- 主区保留 `Home / standard`、`Notifications / accent`、`Settings / success` 三组 reference 快照。
- 底部 preview 通过 `egui_view_symbol_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制不再保留旧 panel 级说明 chrome。
