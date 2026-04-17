# path_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`PathIcon`
- 当前保留语义：预解析 path 数据切换、单色 palette helper、默认路径回退、静态 preview 输入抑制
- 当前移除内容：主 panel / heading / note、preview panel / body、场景化叙事文案、完整 SVG/XAML path 字符串解析、SDK 改动
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_path_icon`，通过 `MOVE_TO / LINE_TO / QUAD_TO / CUBIC_TO / CLOSE` 命令数组和 `on_draw` 内有限步数 flatten 完成绘制，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`path_icon` 用来表达“图标来源是一段单色矢量路径数据”的显示语义。它适合承载没有位图资源、也不依赖图标字体映射的轻量图标场景，让调用方直接通过 path 数据和前景色控制图形本身，而不是退化成位图或字体字形。

## 2. 为什么现有控件不够用
- `bitmap_icon` 依赖 alpha-only 位图资源，缩放后仍然是位图重采样，不适合表达纯路径数据。
- `image_icon` 面向完整图片内容，不适合单色几何图标。
- `font_icon`、`symbol_icon` 依赖字体和字形映射，无法承载任意业务自定义路径。
- 直接使用 SDK canvas API 虽然能画多边形和折线，但缺少 `PathIcon` 这一层默认回退、样式 helper 和静态 preview 输入抑制。

## 3. 当前页面结构
- 标题：`PathIcon`
- 主区：一个主 `path_icon` 和一个当前图标名称 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Heart` + `subtle`
- 右侧 preview：`Send` + `accent`

目录：
- `example/HelloCustomWidgets/display/path_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照和最终稳定帧：

1. 默认态
   path：`Bookmark`
   palette：蓝色主线
2. 快照 2
   path：`Heart`
   palette：玫红强调
3. 快照 3
   path：`Send`
   palette：绿色语义
4. 最终稳定帧
   path：`Bookmark`
   palette：蓝色主线

底部 preview 在整条轨道中始终固定：

1. `subtle`
   path：`Heart`
2. `accent`
   path：`Send`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主图标：`56 x 56`
- 主状态 label：`224 x 12`
- 底部 preview 行：`68 x 30`
- 单个 preview 图标：`30 x 30`
- 页面结构：标题 -> 主 `path_icon` -> 名称 label -> 底部 `subtle / accent`
- 风格约束：浅色 page panel、主区只保留路径数据与 palette 变化，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认显示 | `Bookmark` | `Heart` | `Send` |
| 快照 2 | `Heart` | 保持不变 | 保持不变 |
| 快照 3 | `Send` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Bookmark` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Heart`
4. 抓取第二组主区快照
5. 切到 `Send`
6. 抓取第三组主区快照
7. 回到默认 `Bookmark`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧主 panel、heading、note 和 preview body。
- 底部 preview 统一通过 `egui_view_path_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `PATH_ICON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_path_icon.c` 当前覆盖三部分：

1. 主控件初始化与默认语义
   覆盖默认 `Bookmark` 数据、内置三组路径 getter 和默认 palette
2. setter 与样式 helper 守卫
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()`、`set_data()`、`set_palette()`、`set_data(NULL)` 对 `pressed` 状态的清理和默认回退
3. 静态 preview 不变性断言
   通过 `path_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`data`、`icon_color`、`on_click_listener`、`api`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/path_icon PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/path_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/path_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_path_icon
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/path_icon PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `path_icon` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_path_icon/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_path_icon`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1159 colors=136`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_path_icon/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(213, 170) - (268, 265)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 266` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `path_icon` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应 `Bookmark`、`Heart`、`Send` 三组主区快照；最终稳定帧已显式回到默认 `Bookmark`。
- 按 `y >= 266` 裁剪底部 preview 区域后保持单哈希，确认 `subtle / accent` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前 demo 只覆盖 `Bookmark / Heart / Send` 三组内置预解析路径数据。
- 当前不支持完整 SVG/XAML path 字符串解析。
- filled path 依赖 `egui_canvas_draw_polygon_fill()`，单个 contour 最终顶点数会压缩到 `16` 以内。
- 曲线路径使用有限步数 flatten，目标是 reference 图标稳定可读，不追求高精度矢量拟合。
- 底部 `subtle / accent` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `bitmap_icon`：这里表达矢量路径数据，而不是 alpha-only 位图遮罩。
- 相比 `image_icon`：这里不承载完整图片内容，而是单色几何图标。
- 相比 `font_icon`、`symbol_icon`：这里不依赖字体和字形映射，调用方直接传入 path 数据。
- 相比直接使用 SDK canvas：这里补齐默认回退、palette helper 和静态 preview 输入抑制。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_path_icon` custom view，不修改 SDK。
- 主区保留 `Bookmark`、`Heart`、`Send` 三组 reference 快照。
- 底部 preview 通过 `egui_view_path_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展更多 path 数据来源。
