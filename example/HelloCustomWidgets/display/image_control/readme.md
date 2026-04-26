# Image 自定义控件设计说明

## 参考来源

- 参考设计体系：`Fluent 2`
- 平台语义对照：`WinUI / WPF Image`
- 开源母本：`WPF UI`
- 对应组件名：`Image`
- 当前目录：`example/HelloCustomWidgets/display/image_control/`

## 1. 为什么需要这个控件

`Image` 是基础显示控件，用来在界面中呈现位图资源、缩略图、封面、头像以外的图片预览和内容插图。仓库已有 `ImageIcon`，但它更偏向小尺寸图标资源；`Image` 需要单独表达图片源、自然尺寸、拉伸模式和只读预览状态。

## 2. 为什么现有控件不够用

- `image_icon` 关注图标语义，默认尺寸小，适合和 `BitmapIcon`、`FontIcon`、`PathIcon` 并列。
- `person_picture` 关注人物头像、initials、presence badge，不适合作为通用图片容器。
- `card_panel` 只是承载容器，不负责图片源和 stretch 语义。

## 3. 当前页面结构

- 标题：`Image`
- 主区：一个可切换图片源和 stretch 模式的 `image_control`
- 底部：两个静态 preview
- 左侧 preview：`compact / fill`，固定显示 `Square`
- 右侧 preview：`read only / uniform`，固定显示 `Portrait`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`236 x 228`
- 主图片：`166 x 104`
- 底部 preview：`92 x 50` 两个并排
- 样式：浅色 page panel、白色图片面、低噪声描边；不添加额外说明卡片、厚阴影或场景化装饰。

## 5. 状态矩阵

| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 | `Landscape / Uniform` | `Square / Fill` | `Portrait / Uniform` |
| 快照 2 | `Portrait / Uniform` | 保持不变 | 保持不变 |
| 快照 3 | `Square / Fill` | 保持不变 | 保持不变 |
| 快照 4 | `Landscape / None` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到 `Landscape / Uniform` | 保持不变 | 保持不变 |

## 6. 交互与单测口径

`Image` 本身是显示控件，主区录制轨道通过程序切换 reference 快照，不承担读写输入语义。底部 preview 使用静态 API 吞掉 `touch / key`，保持图片源、stretch、样式和区域不变。

`HelloUnitTest` 覆盖：

1. 初始化默认源和内置图片尺寸。
2. `set_source()`、`set_stretch()` 的 fallback / clamp / pressed 清理。
3. `standard / compact / read only` 样式与 palette setter。
4. 静态 preview 吞掉输入并保持状态。

## 7. 录制动作设计

`egui_port_get_recording_action()` 只导出主区四组状态和最终稳定帧：

1. `Landscape / Uniform`
2. `Portrait / Uniform`
3. `Square / Fill`
4. `Landscape / None`
5. 回到 `Landscape / Uniform`

底部 `compact / read only` preview 在整条录制轨道中保持静态。

## 8. EGUI 适配简化点

- 当前版本使用控件内部生成的三组小型 RGB565 位图，不额外引入资源目录。
- 保留 `None / Fill / Uniform` 三种核心 stretch 模式；`UniformToFill` 和裁剪蒙版暂不下沉。
- 不修改 `sdk/EmbeddedGUI`，只在 `HelloCustomWidgets` 中提供 reference 包装。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/image_control PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe image_control
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/sync_widget_catalog.py --check
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_control --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/image_control
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_control
```

## 10. 当前保留与删除

保留：

- 图片源：`Landscape`、`Portrait`、`Square`
- Stretch：`None`、`Fill`、`Uniform`
- 静态 preview：`compact / fill`、`read only / uniform`

删除或暂不保留：

- 额外装饰图文页
- 图片编辑、裁剪、滤镜、加载进度
- 非主线的场景化相册或媒体播放器语义

## 11. 当前验收结果（2026-04-26）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/image_control PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - Windows 长链接 `Error 87` 后 response-file fallback 成功
  - `output\main.exe image_control`，`image_control 4/4`
- catalog / 文档 / 触控语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - `python scripts/sync_widget_catalog.py --check`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/image_control --track reference --timeout 10 --keep-screenshots`
  - 主区覆盖 `Landscape / Uniform`、`Portrait / Uniform`、`Square / Fill`、`Landscape / None`，底部 preview 全程静态。
- display 分类回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/image_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_image_control`
  - smoke：`PASS status=Running canvas=480x480 ratio=0.1863 colors=88`
