# presence_badge 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`PresenceBadge`
- 当前保留语义：`available / busy / away / do_not_disturb / offline` 五种在线状态、独立小尺寸状态点、`compact / read_only` 静态 preview、静态 preview 输入抑制
- 当前移除内容：旧主 panel / heading / summary / note、底部 preview panel / heading / note、场景化故事外壳、录制阶段额外恢复帧、SDK 改动
- EGUI 适配说明：继续复用 `custom` 层 `egui_view_presence_badge`，在控件内部完成状态点几何、离线 ring、`do_not_disturb` 减号与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`presence_badge` 用来表达“某个对象当前是否可达、忙碌或离线”的极小面积状态语义。它不是 `person_picture` 的附属装饰，而是可以单独挂在列表项、卡片角落、消息线程或工作区入口旁边的标准在线状态点。

## 2. 为什么现有控件不够用
- `person_picture` 的 presence 只是头像内部的附属绘制，不提供独立控件 API。
- `info_badge` 更偏数量、图标和 attention dot，不区分在线状态、阻止打扰和离线 ring。
- `badge` 承载的是文本状态标签，不适合替代极小面积的 presence 信号。

## 3. 当前页面结构
- 标题：`PresenceBadge`
- 主区：一个主 `presence_badge` 和一个当前状态 label
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read_only`

目录：
- `example/HelloCustomWidgets/display/presence_badge/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   status：`available`
   label：`Available`
2. 快照 2
   status：`do_not_disturb`
   label：`Do not disturb`
3. 快照 3
   status：`offline`
   label：`Offline`

底部 preview 在整条轨道中始终固定：

1. `compact`
   status：`away`
   mode：`compact`
2. `read_only`
   status：`busy`
   mode：`compact + read_only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 148`
- 标题：`224 x 18`
- 主状态点：`32 x 32`
- 主状态 label：`224 x 12`
- 底部 preview 行：`44 x 18`
- 单个 preview：`18 x 18`
- 页面结构：标题 -> 主 `presence_badge` -> 状态 label -> 底部 `compact / read_only`
- 风格约束：浅色 page panel、主区只保留状态点几何和状态 label 差异，底部 preview 只做静态 reference；`offline` 继续使用 ring-only，`do_not_disturb` 继续使用减号语义

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `available` | 是 | 否 | 否 |
| `busy` | API 保留 | 否 | 是 |
| `away` | API 保留 | 是 | 否 |
| `do_not_disturb` | 是 | 否 | 否 |
| `offline` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Do not disturb`
4. 抓取第二组主区快照
5. 切到 `Offline`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再回切 `Available` 后额外补一帧。
- 页面层不再保留旧主 panel、summary、note 和底部 preview 包装文案。
- 底部 preview 统一通过 `egui_view_presence_badge_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_presence_badge.c` 当前覆盖四部分：

1. 主控件初始化与默认语义
   覆盖默认 `status`、默认 `compact / read_only`、默认 palette
2. setter 与 palette 守卫
   覆盖 `set_status()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed` 状态的清理与非法值回退
3. helper 与 region/color 计算
   覆盖 `get_indicator_region()`、`resolve_status_color()` 与 disabled 混色 helper
4. 静态 preview 不变性断言
   通过 `presence_badge_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`surface_color`、`outline_color`、`available_color`、`busy_color`、`away_color`、`do_not_disturb_color`、`offline_color`、`glyph_color`、`muted_color`、`on_click_listener`、`api`、`alpha`、`status`、`compact_mode`、`read_only_mode`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/presence_badge PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/presence_badge --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/presence_badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_presence_badge
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/presence_badge PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `presence_badge` suite `4 / 4`
- `sync_widget_catalog.py`：已通过，重新同步 `example/HelloCustomWidgets/widget_catalog.json` 与 `web/catalog-policy.json`，本轮无额外变更
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/presence_badge --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/presence_badge`，输出 `web/demos/HelloCustomWidgets_display_presence_badge`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_presence_badge`，结果 `PASS status=Running canvas=480x480 ratio=0.1159 colors=83`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_presence_badge/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(202, 160) - (278, 229)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 230` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前只覆盖独立状态点，不负责和头像、列表项或命令入口做宿主绑定。
- `away` 先收口为纯色 amber dot，不继续补更复杂的月牙或动画语义。
- 底部 `compact / read_only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `person_picture`：这里提供独立控件 API，而不是头像内部附属绘制。
- 相比 `info_badge`：这里表达在线状态，不承载数量或 attention dot。
- 相比 `badge`：这里是极小面积状态点，不表达文本状态标签。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_presence_badge` custom view，不修改 SDK。
- 主区保留 `Available`、`Do not disturb`、`Offline` 三组 reference 快照。
- 底部 preview 通过 `egui_view_presence_badge_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展宿主绑定示例。
