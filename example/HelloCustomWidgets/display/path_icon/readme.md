# path_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`PathIcon`
- 当前保留形态：主区 `Bookmark`、`Heart`、`Send` 三组 reference 快照，底部 `subtle / accent` 双静态 preview
- 当前保留交互：主区保留程序化 path 数据切换；`standard / subtle / accent` 样式 helper、`set_data()` 与 `set_palette()` 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `panel / heading / note`、底部 preview 包装和说明文案、场景化叙事文案、完整 SVG/XAML path 字符串解析、录制末尾回切默认态的额外桥接帧
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
- 左侧 preview：`subtle`
- 右侧 preview：`accent`
- 页面结构：标题 -> 主 `path_icon` -> 名称 label -> 底部 `subtle / accent`

目录：
- `example/HelloCustomWidgets/display/path_icon/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

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
- 页面风格：浅色 page panel、主区只保留路径数据与 palette 变化，底部 preview 只做静态 reference，对比集中在 path 轮廓和前景色本身

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认 `Bookmark` | 是 | 否 | 否 |
| `Heart` | 是 | 否 | 否 |
| `Send` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `Heart` + subtle | 否 | 是 | 否 |
| `Send` + accent | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_path_icon.c` 当前覆盖 `3` 条用例：

1. `init uses default data and standard palette`
   覆盖默认 `Bookmark` 数据、内置三组路径 getter 和默认 palette。
2. `style helpers and setters clear pressed state`
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()`、`set_data()`、`set_palette()`、`set_data(NULL)` 对 `pressed` 状态的清理和默认回退。
3. `static preview consumes input and keeps state`
   通过 `path_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`data`、`icon_color`、`on_click_listener`、`api`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `path_icon` 是 display-first 的路径图标控件，重点在路径数据切换、默认回退与 palette 语义，不承担 click 提交职责。
- 底部 `subtle / accent` preview 统一通过 `egui_view_path_icon_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Bookmark` 和底部 preview 固定状态，请求首帧并等待 `PATH_ICON_RECORD_FRAME_WAIT = 170`
2. 切到 `Heart`，等待 `PATH_ICON_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Send`，等待 `PATH_ICON_RECORD_WAIT = 90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Bookmark`，继续等待 `PATH_ICON_RECORD_FINAL_WAIT = 280`
7. 请求最终稳定帧，并继续等待 `PATH_ICON_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧主 panel、heading、note 和底部 preview body。

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

## 10. 验收重点

- 主区 `path_icon` 与底部 `subtle / accent` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Bookmark`、`Heart`、`Send` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()`、`set_data()`、`set_palette()` 与默认回退路径都不能残留 `pressed`。
- 底部 `subtle / accent` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_path_icon/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(213, 170) - (268, 265)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 266` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Bookmark`

## 12. 与现有控件的边界

- 相比 `bitmap_icon`：这里表达矢量路径数据，而不是 alpha-only 位图遮罩。
- 相比 `image_icon`：这里不承载完整图片内容，而是单色几何图标。
- 相比 `font_icon`、`symbol_icon`：这里不依赖字体和字形映射，调用方直接传入 path 数据。
- 相比直接使用 SDK canvas：这里补齐默认回退、palette helper 和静态 preview 输入抑制。

## 13. 本轮保留与删减

- 保留的主区状态：`Bookmark`、`Heart`、`Send`
- 保留的底部对照：`subtle`、`accent`
- 保留的交互与实现约束：预解析 path 数据切换、palette helper、默认路径回退、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `panel / heading / note`、底部 preview 包装和说明文案、场景化叙事文案、完整 SVG/XAML path 字符串解析、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/path_icon PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `path_icon` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/path_icon --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_path_icon/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 266` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_path_icon`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1159 colors=136`
  - `web/demos/HelloCustomWidgets_display_path_icon` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(213, 170) - (268, 265)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 266` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Bookmark`、`Heart`、`Send` 三组 reference 快照，最终稳定帧已回到默认态，底部 `subtle / accent` preview 全程静态
