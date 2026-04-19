# font_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`FontIcon`
- 当前保留形态：主区 `Search / MS24`、`Favorite / MS20`、`Settings / MS16` 三组 reference 快照，底部 `subtle / accent` 双静态 preview
- 当前保留交互：主区保留程序化 `glyph / font / palette` 切换；`glyph / icon_font / palette` setter 与 style helper 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `panel / heading / note`、底部 preview 包装和说明文案、场景化叙事文案、录制末尾回切默认态的额外桥接帧
- EGUI 适配说明：继续复用 custom 层 `egui_view_font_icon`，在 custom view 内收口默认字形回退、显式图标字体句柄切换、颜色 helper 与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`font_icon` 用来表达“图标就是一个字形”的轻量显示语义。它适合挂在设置入口、摘要条目、状态提示和小型操作入口上，让调用方直接用 `glyph + font` 控制图标，而不是被固定语义枚举绑定。

## 2. 为什么现有控件不够用

- `symbol_icon` 更偏固定语义图标入口，不适合表达“任意字形 + 任意图标字体”的通用组合。
- 直接使用 SDK `egui_view_label` 虽然能画字形，但缺少 `FontIcon` 层的默认回退、palette helper 和静态 preview 输入抑制。
- `image_icon`、`bitmap_icon` 面向位图资源，不适合字体字形图标。

## 3. 当前页面结构

- 标题：`FontIcon`
- 主区：一个主 `font_icon` 和一个当前 `glyph / font` label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`subtle`
- 右侧 preview：`accent`
- 页面结构：标题 -> 主 `font_icon` -> `glyph / font` label -> 底部 `subtle / accent`

目录：
- `example/HelloCustomWidgets/display/font_icon/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

1. 默认态
   glyph：`Search`
   font：`MS24`
   palette：蓝色主线
2. 快照 2
   glyph：`Favorite`
   font：`MS20`
   palette：暖色强调
3. 快照 3
   glyph：`Settings`
   font：`MS16`
   palette：绿色语义
4. 最终稳定帧
   glyph：`Search`
   font：`MS24`
   palette：蓝色主线

底部 preview 在整条轨道中始终固定：

1. `subtle`
   glyph：`Search`
   font：`MS16`
2. `accent`
   glyph：`Favorite`
   font：`MS20`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主图标槽：`48 x 48`
- 主状态 label：`224 x 12`
- 底部 preview 行：`64 x 28`
- 单个 preview 图标槽：`28 x 28`
- 页面风格：浅色 page panel、主区只保留字形 / 字号 / palette 变化，底部 preview 只做静态 reference，对比集中在 glyph、字号和颜色本身

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认 `Search / MS24` | 是 | 否 | 否 |
| `Favorite / MS20` | 是 | 否 | 否 |
| `Settings / MS16` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `Search / MS16` + subtle | 否 | 是 | 否 |
| `Favorite / MS20` + accent | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_font_icon.c` 当前覆盖 `3` 条用例：

1. `init uses default glyph font and palette`
   覆盖默认 glyph、默认图标字体句柄和默认 palette。
2. `style helpers and setters clear pressed state`
   覆盖 `apply_subtle_style()`、`apply_accent_style()`、`set_glyph()`、`set_icon_font()`、`set_palette()`、`set_glyph(NULL)` 与 `set_icon_font(NULL)` 对 `pressed` 状态的清理和默认回退。
3. `static preview consumes input and keeps state`
   通过 `font_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`line_space`、`align_type`、`label_alpha`、`color`、`font`、`text`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `font_icon` 是 display-first 的字体字形图标控件，重点在 `glyph + font` 组合和 palette 切换，不承担 click 提交职责。
- 底部 `subtle / accent` preview 统一通过 `egui_view_font_icon_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Search / MS24` 和底部 preview 固定状态，请求首帧并等待 `FONT_ICON_RECORD_FRAME_WAIT = 170`
2. 切到 `Favorite / MS20`，等待 `FONT_ICON_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Settings / MS16`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Search / MS24`，等待 `FONT_ICON_RECORD_WAIT = 90`
7. 请求最终稳定帧，并继续等待 `FONT_ICON_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧版 panel、heading、note 和底部 preview 说明文字。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/font_icon PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/font_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/font_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_font_icon
```

## 10. 验收重点

- 主区 `font_icon` 与底部 `subtle / accent` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Search / MS24`、`Favorite / MS20`、`Settings / MS16` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `apply_subtle_style()`、`apply_accent_style()`、`set_glyph()`、`set_icon_font()`、`set_palette()` 与默认回退路径都不能残留 `pressed`。
- 底部 `subtle / accent` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_font_icon/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(198, 183) - (282, 255)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 256` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Search / MS24`

## 12. 与现有控件的边界

- 相比 `symbol_icon`：这里强调调用方直接提供 `glyph + font`，而不是固定语义入口。
- 相比 `bitmap_icon`、`image_icon`：这里聚焦字体字形，而不依赖位图资源。
- 相比普通 `label`：这里补齐默认回退、palette helper 和静态 preview 输入抑制，统一成图标控件语义。

## 13. 本轮保留与删减

- 保留的主区状态：`Search / MS24`、`Favorite / MS20`、`Settings / MS16`
- 保留的底部对照：`subtle`、`accent`
- 保留的交互与实现约束：`glyph + font` 组合、palette helper、默认回退、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `panel / heading / note`、底部 preview 包装和说明文案、场景化叙事文案、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/font_icon PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `font_icon` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/font_icon --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_font_icon/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 256` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_font_icon`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1159 colors=92`
  - `web/demos/HelloCustomWidgets_display_font_icon` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(198, 183) - (282, 255)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 256` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Search / MS24`、`Favorite / MS20`、`Settings / MS16` 三组 reference 快照，最终稳定帧已回到默认态，底部 `subtle / accent` preview 全程静态
