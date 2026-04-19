# bitmap_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`BitmapIcon`
- 当前保留形态：主区 `Document`、`Mail`、`Alert` 三组 reference 快照，底部 `subtle / accent` 双静态 preview
- 当前保留交互：主区保留程序化资源切换与 palette 更新；`style / image / palette` setter 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、旧 preview 输入桥接、资源生成链依赖、SDK 改动
- EGUI 适配说明：继续复用 custom 层 `egui_view_bitmap_icon`，当前只收口 `Document`、`Mail`、`Alert` 三张内置 alpha-only 位图资源与 `egui_view_bitmap_icon_override_static_preview_api()` 静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`bitmap_icon` 用来表达“图形来源是位图遮罩，但颜色来自宿主前景色”的单色图标语义。它适合挂在设置页、文档列表、状态摘要和告警提示这类轻量 UI 上，用固定像素形状表达文档、邮件、告警等语义，而不引入多色图片资源。

## 2. 为什么现有控件不够用

- `image_icon` 面向完整图片内容，不强调“位图遮罩 + 前景色着色”的单色图标模型。
- `font_icon`、`symbol_icon` 依赖字体字形，不适合表达来源固定的位图 mask 资源。
- 直接在业务页操作 `egui_view_image` 会把默认资源回退、palette helper 和静态 preview 输入抑制散落在页面代码里，缺少统一收口。

## 3. 当前页面结构

- 标题：`BitmapIcon`
- 主区：一个主 `bitmap_icon` 和一个当前图标名 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Mail` + `subtle`
- 右侧 preview：`Alert` + `accent`
- 页面结构：标题 -> 主 `bitmap_icon` -> 图标名 label -> 底部 `subtle / accent`

目录：
- `example/HelloCustomWidgets/display/bitmap_icon/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组目标快照和最终稳定帧：

1. 默认态
   图标：`Document`
   palette：蓝色主线
2. 快照 2
   图标：`Mail`
   palette：绿色语义
3. 快照 3
   图标：`Alert`
   palette：红色告警
4. 最终稳定帧
   图标：`Document`
   palette：蓝色主线

底部 preview 在整条轨道中始终固定：

1. `subtle`
   图标：`Mail`
   样式：`subtle`
2. `accent`
   图标：`Alert`
   样式：`accent`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主图标：`52 x 52`
- 图标名 label：`224 x 12`
- 底部 preview 行：`64 x 28`
- 单个 preview 图标：`28 x 28`
- 页面风格：浅色 page panel、单色 palette 切换，主区只保留资源与前景色变化，底部 preview 只做静态 reference

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认 `Document` | 是 | 否 | 否 |
| `Mail` | 是 | 否 | 否 |
| `Alert` | 是 | 否 | 否 |
| 最终稳定帧回到 `Document` | 是 | 否 | 否 |
| `Mail + subtle` | 否 | 是 | 否 |
| `Alert + accent` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_bitmap_icon.c` 当前覆盖 `3` 条用例：

1. `init uses resize mode default image and standard palette`
   覆盖默认 `resize` 模式、默认 `Document` 图标、默认 palette，以及三张内置资源 getter 的区分。
2. `style helpers and setters clear pressed state`
   覆盖 `apply_subtle_style()`、`apply_accent_style()`、`set_image()`、`set_palette()` 与 `set_image(NULL)` 对 `pressed` 状态的清理，以及默认资源回退。
3. `static preview consumes input and keeps state`
   通过 `bitmap_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`image_type`、`image`、`image_color`、`image_color_alpha`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `bitmap_icon` 是 display-first 的只读图标控件，重点在资源切换与单色 palette，不承担 click 提交语义。
- 底部 `subtle / accent` preview 统一通过 `egui_view_bitmap_icon_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Document` 和底部 preview 固定状态，请求首帧并等待 `BITMAP_ICON_RECORD_FRAME_WAIT = 170`
2. 切到 `Mail`，等待 `BITMAP_ICON_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Alert`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Document`，等待 `90`
7. 请求最终稳定帧，并继续等待 `BITMAP_ICON_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区 `Document / Mail / Alert` 三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留 preview 包装面板与辅助说明文案。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/bitmap_icon PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/bitmap_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/bitmap_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_bitmap_icon
```

## 10. 验收重点

- 主区与底部 `subtle / accent` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Document / Mail / Alert` 三组资源与 palette 状态必须能从截图中稳定区分，且最终稳定帧显式回到默认 `Document`。
- `apply_subtle_style()`、`apply_accent_style()`、`set_image()`、`set_palette()` 与 `set_image(NULL)` 不能残留 `pressed`。
- 底部 `subtle / accent` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_bitmap_icon/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(201, 156) - (285, 259)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 260` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Document`

## 12. 与现有控件的边界

- 相比 `image_icon`：这里表达单色位图遮罩着色，不展示完整彩色图片内容。
- 相比 `font_icon`、`symbol_icon`：这里聚焦固定位图资源，而不是字体字形。
- 相比 `path_icon`：这里依赖位图 mask，而不是矢量路径描述。

## 13. 本轮保留与删减

- 保留的主区状态：`Document`、`Mail`、`Alert`
- 保留的底部对照：`subtle`、`accent`
- 保留的交互与实现约束：alpha-only 位图遮罩着色、默认资源回退、`style / image / palette` setter 清理 `pressed`、static preview 输入抑制
- 删减的旧桥接与装饰：主 panel / heading / note、preview panel / heading / body、旧 preview 输入桥接、资源生成链依赖与 SDK 改动

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/bitmap_icon PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `bitmap_icon` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/bitmap_icon --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_bitmap_icon/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 260` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_bitmap_icon`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1159 colors=66`
  - `web/demos/HelloCustomWidgets_display_bitmap_icon` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(201, 156) - (285, 259)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 260` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Document / Mail / Alert` 三组 reference 快照，最终稳定帧已回到默认 `Document`，底部 `subtle / accent` preview 全程静态
