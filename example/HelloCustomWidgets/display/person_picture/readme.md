# person_picture 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`PersonPicture`
- 当前保留语义：头像圆形承载、`display_name -> initials` 回退、显式 `initials`、`fallback_glyph`、可选 `image`、`presence` 状态点、`tone` 语义色、静态 preview 输入抑制
- 当前移除内容：主 panel / heading / note、preview panel / heading / body、联系人数据源桥接、场景化叙事文案、SDK 改动
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_person_picture`，在控件内部完成圆形头像 mask、initials 回退链路与 presence 点绘制，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`person_picture` 用来表达“这里代表一个人或一个联系人槽位”的标准头像语义。它和单纯的 `image_icon` 不同，核心价值不是展示任意图片，而是在图片缺失时仍然能稳定回退到 initials 或 `person` glyph，并附带 presence 点来承载在线状态。

## 2. 为什么现有控件不够用
- `image_icon` 只负责图片本身，没有 `display_name / initials / fallback_glyph` 的回退链路。
- `font_icon`、`symbol_icon` 只能展示图标，不能表达头像的 tone 和 presence 语义。
- `persona`、`persona_group` 面向带文本信息的组合展示，不适合作为单个头像的基础控件。

## 3. 当前页面结构
- 标题：`PersonPicture`
- 主区：一个主 `person_picture` 和一个当前状态 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/person_picture/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   name：`Lena Marsh`
   回退：`display_name -> LM`
   label：`LM / live`
2. 快照 2
   name：`Aria Rowan`
   initials：`AR`
   label：`AR / busy`
3. 快照 3
   name：空
   回退：`fallback_glyph`
   label：`Person / empty slot`

底部 preview 在整条轨道中始终固定：

1. `compact`
   name：`Maya Yu`
   mode：`compact`
2. `read_only`
   name：`Mina Brooks`
   initials：`MB`
   mode：`compact + read_only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 170`
- 标题：`224 x 18`
- 主头像：`72 x 72`
- 主状态 label：`224 x 12`
- 底部 preview 行：`84 x 38`
- 单个 preview 头像：`38 x 38`
- 页面结构：标题 -> 主 `person_picture` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、主区只保留头像回退链路与 presence/tone 变化，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `display_name -> initials` | 是 | 是 | 是 |
| 显式 `initials` | 是 | 否 | 是 |
| `fallback_glyph` | 是 | 是 | 是 |
| `set_image()` 圆形遮罩 | 支持 | 支持 | 支持 |
| `tone` | 是 | 是 | 是 |
| `presence` | 是 | 是 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `AR / busy`
4. 抓取第二组主区快照
5. 切到 `Person / empty slot`
6. 抓取第三组主区快照
7. 回到默认 `LM / live`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧主 panel、heading、note 和 preview body。
- 底部 preview 统一通过 `egui_view_person_picture_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `PERSON_PICTURE_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_person_picture.c` 当前覆盖四部分：

1. 主控件初始化与默认语义
   覆盖默认 `fallback_glyph`、默认 `tone / presence`、默认字体与 icon font 回退规则
2. setter 与 initials 回退守卫
   覆盖 `set_display_name()`、`set_initials()`、`set_fallback_glyph()`、`set_image()`、`set_tone()`、`set_presence()`、`set_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed` 状态的清理和默认回退
3. helper 与 region/color 计算
   覆盖 `resolve_initials()`、`get_avatar_region()`、`get_presence_region()`、`tone_color()`、`presence_color()` 与 disabled 混色 helper
4. 静态 preview 不变性断言
   通过 `person_picture_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`display_name`、`initials`、`fallback_glyph`、`image`、`font`、`icon_font`、`surface_color`、`border_color`、`foreground_color`、`accent_color`、`success_color`、`warning_color`、`neutral_color`、`muted_color`、`on_click_listener`、`api`、`alpha`、`tone`、`presence`、`compact_mode`、`read_only_mode`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/person_picture PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/person_picture --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/person_picture
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_person_picture
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/person_picture PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `person_picture` suite `4 / 4`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_person_picture/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`
  runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_person_picture`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1331 colors=144`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_person_picture/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(187, 140) - (294, 275)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 275` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `person_picture` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态：`[0,1,6,7,8]` 对应默认 `LM / live`，`[2,3]` 对应 `AR / busy`，`[4,5]` 对应 `Person / empty slot`；最终稳定帧已显式回到默认态。
- 按 `y >= 275` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read_only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前 demo 不内置 portrait 资源，主区重点放在 initials / glyph 回退链路。
- `set_image()` 已保留并支持圆形遮罩，但需要业务层自行提供稳定的 `egui_image_t *`。
- initials 解析当前按 ASCII 规则处理，优先覆盖 reference demo 与现有英文命名场景。
- 底部 `compact / read_only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `image_icon`：这里承载头像语义和回退链路，而不是任意图片内容。
- 相比 `font_icon`、`symbol_icon`：这里不仅能画 glyph，还要表达头像 tone、presence 与 initials 回退。
- 相比 `persona`、`persona_group`：这里聚焦单个头像本身，不携带组合文本布局。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_person_picture` custom view，不修改 SDK。
- 主区保留 `LM / live`、`AR / busy`、`Person / empty slot` 三组 reference 快照。
- 底部 preview 通过 `egui_view_person_picture_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展 portrait image 示例。
