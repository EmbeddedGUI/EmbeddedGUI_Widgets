# image_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`ImageIcon`
- 当前保留语义：完整图片图标显示、默认图片回退、图片源切换、静态 preview 输入抑制
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、轮换 preview 轨道、SDK 改动
- EGUI 适配说明：继续复用 custom 层 `egui_view_image_icon`，当前 demo 固定收口 `default / warm / fresh` 三张内置 `RGB565` 图片资源，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`image_icon` 用来表达“图标本身就是一张完整图片”的只读展示语义。它适合承载缩略图、品牌图块、场景图片入口和带色块构图的状态图标，这类内容不能退化成单色位图遮罩，也不适合用字体字形表达。

## 2. 为什么现有控件不够用
- `bitmap_icon` 面向单色位图遮罩 + palette 着色，不适合完整彩色图片内容。
- `font_icon`、`symbol_icon` 依赖字体字形，无法表达图片构图和色块层次。
- 直接使用 SDK `egui_view_image` 虽然能显示图片，但缺少 `ImageIcon` 这层默认图回退、静态 preview 输入抑制和 reference 页面约束。

## 3. 当前页面结构
- 标题：`ImageIcon`
- 主区：一个主 `image_icon` 和一个当前图片名 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Warm tone`
- 右侧 preview：`Fresh tone`

目录：
- `example/HelloCustomWidgets/display/image_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   图片：`Thumbnail`
   label 颜色：蓝色主线
2. 快照 2
   图片：`Warm tone`
   label 颜色：琥珀强调
3. 快照 3
   图片：`Fresh tone`
   label 颜色：绿色语义

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
- 页面结构：标题 -> 主 `image_icon` -> 图片名 label -> 底部 `warm / fresh`
- 风格约束：浅色 page panel、主区只保留图片内容与 label 颜色切换，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Warm preview | Fresh preview |
| --- | --- | --- | --- |
| 默认 `Thumbnail` | 是 | 否 | 否 |
| `Warm tone` | 是 | 是 | 否 |
| `Fresh tone` | 是 | 否 | 是 |
| `set_image(NULL)` 回退默认图 | 支持 | 支持 | 支持 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Warm tone`
4. 抓取第二组主区快照
5. 切到 `Fresh tone`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切默认态后额外抓帧。
- 页面层不再保留旧版 panel、heading、note 和 preview 包装容器。
- 底部 preview 统一通过 `egui_view_image_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_image_icon.c` 当前覆盖三部分：

1. 主控件初始化与默认语义
   覆盖默认 `resize` 模式、默认 `Thumbnail` 图片，以及三张内置资源 getter 的区分。
2. setter 守卫
   覆盖 `set_image()`、`set_image(NULL)` 对 `pressed` 状态的清理和默认资源回退。
3. 静态 preview 不变性断言
   通过 `image_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`image_type`、`image`、`image_color`、`image_color_alpha`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

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

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `image_icon` suite `3 / 3`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`，本轮无额外变更
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_icon --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/image_icon`，输出 `web/demos/HelloCustomWidgets_display_image_icon`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_icon`，结果 `PASS status=Running canvas=480x480 ratio=0.1441 colors=77`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_image_icon/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(168, 129) - (311, 262)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 263` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前 demo 只覆盖 `default`、`warm`、`fresh` 三张内置 `RGB565` 图片资源。
- 当前默认使用 `resize` 模式绘制，不扩展裁切、平铺或更复杂的缩放策略。
- 当前优先保证 PC runtime / web 的稳定性，不额外引入 alpha resize 路径。
- 底部 `warm / fresh` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `bitmap_icon`：这里表达完整图片内容，而不是单色位图遮罩着色。
- 相比 `font_icon`、`symbol_icon`：这里聚焦图片资源，不依赖字体字形。
- 相比直接使用 `egui_view_image`：这里补齐默认图回退与静态 preview 输入抑制，统一成 `ImageIcon` 语义。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_image_icon` custom view，不修改 SDK。
- 主区保留 `Thumbnail`、`Warm tone`、`Fresh tone` 三组 reference 快照。
- 底部 preview 通过 `egui_view_image_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展更多图片资源来源。
