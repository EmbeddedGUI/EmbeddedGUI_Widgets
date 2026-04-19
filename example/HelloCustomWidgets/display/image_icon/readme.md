# image_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`ImageIcon`
- 当前保留形态：主区 `Thumbnail`、`Warm tone`、`Fresh tone` 三组 reference 快照，底部 `warm / fresh` 双静态 preview
- 当前保留交互：主区保留程序化图片源切换；`set_image()` 清理 `pressed` 并支持默认图回退；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `panel / heading / note`、底部 preview 包装和说明文案、轮换 preview 轨道、录制末尾回切默认态的额外桥接帧
- EGUI 适配说明：继续复用 custom 层 `egui_view_image_icon`，在 custom view 内收口默认图回退、图片源切换与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`image_icon` 用来表达“图标本身就是一张完整图片”的只读展示语义。它适合承载缩略图、品牌图块、场景图片入口和带色块构图的状态图标，这类内容不能退化成单色位图遮罩，也不适合用字体字形表达。

## 2. 为什么现有控件不够用

- `bitmap_icon` 面向单色位图遮罩加 palette 着色，不适合完整彩色图片内容。
- `font_icon`、`symbol_icon` 依赖字体字形，无法表达图片构图和色块层次。
- 直接使用 SDK `egui_view_image` 虽然能显示图片，但缺少 `ImageIcon` 这层默认图回退、静态 preview 输入抑制和 reference 页面约束。

## 3. 当前页面结构

- 标题：`ImageIcon`
- 主区：一个主 `image_icon` 和一个当前图片名 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`warm`
- 右侧 preview：`fresh`
- 页面结构：标题 -> 主 `image_icon` -> 图片名 label -> 底部 `warm / fresh`

目录：
- `example/HelloCustomWidgets/display/image_icon/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

1. 默认态
   图片：`Thumbnail`
   label 颜色：蓝色主线
2. 快照 2
   图片：`Warm tone`
   label 颜色：暖色强调
3. 快照 3
   图片：`Fresh tone`
   label 颜色：绿色语义
4. 最终稳定帧
   图片：`Thumbnail`
   label 颜色：蓝色主线

底部 preview 在整条轨道中始终固定：

1. `warm`
   图片：`Warm tone`
2. `fresh`
   图片：`Fresh tone`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 184`
- 标题：`224 x 18`
- 主图片槽：`72 x 72`
- 主状态 label：`224 x 12`
- 底部 preview 行：`88 x 40`
- 单个 preview 图片槽：`40 x 40`
- 页面风格：浅色 page panel、主区只保留图片内容和 label 颜色切换，底部 preview 只做静态 reference，对比集中在图片本身和当前 label 上

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Warm preview | Fresh preview |
| --- | --- | --- | --- |
| 默认 `Thumbnail` | 是 | 否 | 否 |
| `Warm tone` | 是 | 否 | 否 |
| `Fresh tone` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `Warm tone` 预览 | 否 | 是 | 否 |
| `Fresh tone` 预览 | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_image_icon.c` 当前覆盖 `3` 条用例：

1. `init uses resize mode and default image`
   覆盖默认 `resize` 模式、默认 `Thumbnail` 图片，以及三张内置资源 getter 的区分。
2. `set image updates source and falls back to default`
   覆盖 `set_image()`、`set_image(NULL)` 对 `pressed` 状态的清理和默认资源回退。
3. `static preview consumes input and keeps state`
   通过 `image_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`image_type`、`image`、`image_color`、`image_color_alpha`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `image_icon` 是 display-first 的只读图片图标控件，重点在图片资源切换和默认图回退，不承担 click 提交职责。
- 底部 `warm / fresh` preview 统一通过 `egui_view_image_icon_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Thumbnail` 和底部 preview 固定状态，请求首帧并等待 `IMAGE_ICON_RECORD_FRAME_WAIT = 170`
2. 切到 `Warm tone`，等待 `IMAGE_ICON_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Fresh tone`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Thumbnail`，等待 `IMAGE_ICON_RECORD_WAIT = 90`
7. 请求最终稳定帧，并继续等待 `IMAGE_ICON_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧版 panel、heading、note 和底部 preview 包装。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/image_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_icon
```

## 10. 验收重点

- 主区 `image_icon` 与底部 `warm / fresh` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Thumbnail`、`Warm tone`、`Fresh tone` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `set_image()` 在图片切换和默认图回退路径上都不能残留 `pressed`。
- 底部 `warm / fresh` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_image_icon/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(168, 129) - (311, 262)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 263` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Thumbnail`

## 12. 与现有控件的边界

- 相比 `bitmap_icon`：这里表达完整图片内容，而不是单色位图遮罩着色。
- 相比 `font_icon`、`symbol_icon`：这里聚焦图片资源，不依赖字体字形。
- 相比直接使用 `egui_view_image`：这里补齐默认图回退与静态 preview 输入抑制，统一成 `ImageIcon` 语义。

## 13. 本轮保留与删减

- 保留的主区状态：`Thumbnail`、`Warm tone`、`Fresh tone`
- 保留的底部对照：`warm`、`fresh`
- 保留的交互与实现约束：默认图回退、图片源切换、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `panel / heading / note`、底部 preview 包装和说明文案、轮换 preview 轨道、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `image_icon` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_icon --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_image_icon/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 263` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_icon`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1441 colors=77`
  - `web/demos/HelloCustomWidgets_display_image_icon` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(168, 129) - (311, 262)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 263` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Thumbnail`、`Warm tone`、`Fresh tone` 三组 reference 快照，最终稳定帧已回到默认态，底部 `warm / fresh` preview 全程静态
