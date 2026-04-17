# bitmap_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`BitmapIcon`
- 当前保留语义：alpha-only 位图遮罩着色、内置图标资源切换、静态 preview 输入抑制
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、资源生成链依赖、SDK 改动
- EGUI 适配说明：继续复用 custom 层 `egui_view_bitmap_icon`，当前只收口 `Document`、`Mail`、`Alert` 三张内置 alpha-only 位图资源，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`bitmap_icon` 用来表达“图形来源是位图遮罩，但颜色来自宿主前景色”的单色图标语义。它适合挂在设置页、文档列表、状态摘要和告警提示这类轻量 UI 上，用固定像素形状表达文档、邮件、告警等语义，而不引入多色图片资源。

## 2. 为什么现有控件不够用
- `image_icon` 面向完整图片内容，不强调“位图遮罩 + 前景色着色”的单色图标模型。
- `font_icon`、`symbol_icon` 依赖字体字形，不适合表达来源固定的位图掩码资源。
- 直接在业务页操作 `egui_view_image` 会把默认资源回退、palette helper 和静态 preview 输入抑制散落在页面代码里，缺少统一收口。

## 3. 当前页面结构
- 标题：`BitmapIcon`
- 主区：一个主 `bitmap_icon` 和一个当前图标名 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Mail` + `subtle`
- 右侧 preview：`Alert` + `accent`

目录：
- `example/HelloCustomWidgets/display/bitmap_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照和最终稳定帧：

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
- 页面结构：标题 -> 主 `bitmap_icon` -> 图标名 label -> 底部 `subtle / accent`
- 风格约束：浅色 page panel、单色 palette 切换、主区只保留资源与前景色变化，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认显示 | `Document` | `Mail` + `subtle` | `Alert` + `accent` |
| 快照 2 | `Mail` | 保持不变 | 保持不变 |
| 快照 3 | `Alert` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Document` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Mail`
4. 抓取第二组主区快照
5. 切到 `Alert`
6. 抓取第三组主区快照
7. 回到默认 `Document`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留 preview 包装面板与辅助说明文案。
- 底部 preview 统一通过 `egui_view_bitmap_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `BITMAP_ICON_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_bitmap_icon.c` 当前覆盖三部分：

1. 主控件初始化与默认语义
   覆盖默认 `resize` 模式、默认 `Document` 图标、默认 palette，以及三张内置资源 getter 的区分。
2. setter 与样式 helper 守卫
   覆盖 `apply_subtle_style()`、`apply_accent_style()`、`set_image()`、`set_palette()`、`set_image(NULL)` 对 `pressed` 状态的清理与默认资源回退。
3. 静态 preview 不变性断言
   通过 `bitmap_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`image_type`、`image`、`image_color`、`image_color_alpha`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

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

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/bitmap_icon PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `bitmap_icon` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_bitmap_icon/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_bitmap_icon`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1159 colors=66`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_bitmap_icon/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(201, 156) - (285, 259)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 260` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `bitmap_icon` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应 `Document / Mail / Alert` 三组主区快照；最终稳定帧已显式回到默认 `Document`。
- 按 `y >= 260` 裁切底部 preview 区域后保持单哈希，确认 `subtle / accent` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖 `Document`、`Mail`、`Alert` 三张内置 alpha-only 位图资源。
- 当前只支持单色 palette 着色，不扩展多通道分层位图。
- 当前默认使用 `resize` 模式绘制，不额外提供裁切、平铺或复杂缩放策略。
- 底部 `subtle / accent` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `image_icon`：这里表达单色位图遮罩着色，不展示完整彩色图片内容。
- 相比 `font_icon`、`symbol_icon`：这里聚焦固定位图资源，而不是字体字形。
- 相比 `path_icon`：这里依赖位图 mask，而不是矢量路径描述。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_bitmap_icon` custom view，不修改 SDK。
- 主区保留 `Document`、`Mail`、`Alert` 三组 reference 快照。
- 底部 preview 通过 `egui_view_bitmap_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展到更多位图资源。
