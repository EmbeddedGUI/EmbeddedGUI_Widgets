# font_icon 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`FontIcon`
- 当前保留语义：字形字符串切换、显式图标字体句柄切换、palette helper、静态 preview 输入抑制
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、场景化叙事文案、SDK 改动
- EGUI 适配说明：继续复用 custom 层 `egui_view_font_icon`，当前 demo 固定使用 `Material Symbols` 字体子集做 reference 展示，但控件接口仍允许外部传入任意 `egui_font_t *`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`font_icon` 用来表达“图标就是一个字形”的轻量显示语义。它适合挂在设置入口、摘要条目、状态提示和小型操作入口上，让调用方直接用 `glyph + font` 控制图标，而不是被某套固定语义枚举绑定。

## 2. 为什么现有控件不够用
- `symbol_icon` 更偏固定语义图标入口，不适合表达“任意字形 + 任意图标字体”的通用组合。
- 直接使用 SDK `egui_view_label` 能画出字形，但缺少 `FontIcon` 这层默认回退、样式 helper 和静态 preview 输入抑制。
- `image_icon`、`bitmap_icon` 面向位图资源，不适合字体字形图标。

## 3. 当前页面结构
- 标题：`FontIcon`
- 主区：一个主 `font_icon` 和一个当前 `glyph / font` label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`Search / MS16` + `subtle`
- 右侧 preview：`Favorite / MS20` + `accent`

目录：
- `example/HelloCustomWidgets/display/font_icon/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   glyph：`Search`
   font：`MS24`
   palette：蓝色主线
2. 快照 2
   glyph：`Favorite`
   font：`MS20`
   palette：琥珀强调
3. 快照 3
   glyph：`Settings`
   font：`MS16`
   palette：绿色语义

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
- 页面结构：标题 -> 主 `font_icon` -> `glyph / font` label -> 底部 `subtle / accent`
- 风格约束：浅色 page panel、主区只保留字形 / 字号 / palette 变化，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认 `Search / MS24` | 是 | 否 | 否 |
| `Favorite / MS20` | 是 | 否 | 是 |
| `Settings / MS16` | 是 | 否 | 否 |
| `set_glyph(NULL)` 回退默认字形 | 支持 | 支持 | 支持 |
| `set_icon_font(NULL)` 回退默认字体 | 支持 | 支持 | 支持 |
| `apply_standard_style()` | 是 | 否 | 否 |
| `apply_subtle_style()` | 否 | 是 | 否 |
| `apply_accent_style()` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Favorite / MS20`
4. 抓取第二组主区快照
5. 切到 `Settings / MS16`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切默认态后额外抓帧。
- 页面层不再保留旧版 panel、heading、note 和 preview 说明文字。
- 底部 preview 统一通过 `egui_view_font_icon_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_font_icon.c` 当前覆盖三部分：

1. 主控件初始化与默认语义
   覆盖默认 glyph、默认字体句柄和默认 palette。
2. setter 与样式 helper 守卫
   覆盖 `apply_subtle_style()`、`apply_accent_style()`、`set_glyph()`、`set_icon_font()`、`set_palette()`、`set_glyph(NULL)` 与 `set_icon_font(NULL)` 对 `pressed` 状态的清理和默认回退。
3. 静态 preview 不变性断言
   通过 `font_icon_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`line_space`、`align_type`、`label_alpha`、`color`、`font`、`text`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

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

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/font_icon PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `font_icon` suite `3 / 3`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`，本轮无额外变更
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/font_icon --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/font_icon`，输出 `web/demos/HelloCustomWidgets_display_font_icon`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_font_icon`，结果 `PASS status=Running canvas=480x480 ratio=0.1159 colors=92`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_font_icon/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(198, 183) - (282, 255)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 256` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前 demo 为了确定性固定使用 `Material Symbols` 子集做 reference 展示。
- 当前只覆盖最小 `FontIcon` 语义，不扩展动画、hover 过渡或组合容器语义。
- 当前不提供图标枚举包装，调用方直接传入 glyph 字符串和字体句柄。
- 底部 `subtle / accent` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `symbol_icon`：这里强调调用方直接提供 `glyph + font`，而不是固定语义入口。
- 相比 `bitmap_icon`、`image_icon`：这里聚焦字体字形，不依赖位图资源。
- 相比普通 `label`：这里补齐默认回退、palette helper 和静态 preview 输入抑制，统一成图标控件语义。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_font_icon` custom view，不修改 SDK。
- 主区保留 `Search / MS24`、`Favorite / MS20`、`Settings / MS16` 三组 reference 快照。
- 底部 preview 通过 `egui_view_font_icon_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展更多图标字体或字形来源。
