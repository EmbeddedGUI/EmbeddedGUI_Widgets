# info_label 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`InfoLabel`
- 当前保留语义：`label`、`info button`、`anchored bubble`、`compact`、`read only`、静态 preview 输入抑制
- 当前移除内容：主面板包装、preview panel / heading、录制阶段真实 icon click、额外故事化说明与高噪音页面壳层
- EGUI 适配说明：继续使用 `HelloCustomWidgets` custom 层的轻量 `wrapper`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`info_label` 用来在正文标签旁边提供一个低打扰的信息入口。它适合出现在设置项、表单字段、只读说明和摘要行附近，让用户按需展开一段短说明，而不是被 `TeachingTip` 或块级反馈容器打断。

## 2. 为什么现有控件不够用
- `tool_tip` 更偏悬停提示，不承担标签旁常驻解释入口的语义。
- `teaching_tip` 层级更高、信息量更大，不适合这里的轻量说明。
- `text_block` 只能静态展示文本，无法表达按需展开的解释气泡。
- `info_badge` 表达的是状态或数量提醒，不表达“标签解释”。

## 3. 当前页面结构
- 标题：`InfoLabel`
- 主区：一个主 `info_label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`

目录：
- `example/HelloCustomWidgets/display/info_label/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照：

1. 默认态
   `Project policy`
   `Versioning`
   `Keep release notes aligned with the approved branch.`
   主区保持 `closed accent`
2. 快照 2
   `Export guidance`
   `Sensitive content`
   `Mask personal data before sharing outside the tenant.`
   主区保持 `open warning`
3. 快照 3
   `Reading help`
   `Reference note`
   `Use the compact preview when the layout has limited width.`
   主区保持 `open neutral`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Compact help`
   `Inline note`
   `Compact mode keeps the bubble short.`
2. `read only`
   `Audit note`
   `Read only`
   `Static preview keeps input disabled.`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 194`
- 标题：`224 x 18`
- 主控件：`196 x 96`
- 底部 preview 行：`176 x 54`
- 单个 preview：`84 x 54`
- 页面结构：标题 -> 主 `info_label` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、主区保留 `closed / open` 与 palette 语义变化，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `closed` | 是 | 否 | 否 |
| `open` | 是 | 是 | 是 |
| `accent / warning / neutral` | 是 | 是 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `touch / key` 真实开关 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `open warning`
4. 抓取第二组主区快照
5. 切到 `open neutral`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再依赖真实 icon click 来驱动主区展开。
- 页面层不再保留旧 preview panel、heading 和额外说明文案。
- 底部 preview 统一通过 `hcw_info_label_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_info_label.c` 当前覆盖四部分：

1. 样式 helper 与 setter
   覆盖 `apply_compact_style()`、`apply_read_only_style()`、`set_text()`、`set_info_title()`、`set_info_body()`、`set_font()`、`set_meta_font()`、`set_icon_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`
2. touch 语义
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到同一 target 后 `UP(A)` 才提交
3. 键盘开关
   覆盖 `Enter / Space` 开关和 `Esc` 关闭
4. 静态 preview 不变性断言
   通过 `info_label_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`label`、`info_title`、`info_body`、`font`、`meta_font`、`icon_font`、`on_open_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`bubble_surface_color`、`shadow_color`、`compact_mode`、`read_only_mode`、`open`、`pressed_part`、`icon_region`、`bubble_region`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/info_label PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_label --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_label
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_label
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/info_label PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `info_label` suite `4 / 4`
- `sync_widget_catalog.py`：已通过，`widget_catalog.json` 与 `web/catalog-policy.json` 保持同步，本轮无额外目录漂移
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_label --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- `display` 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_label`，输出 `web/demos/HelloCustomWidgets_display_info_label`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_label`，结果 `PASS status=Running canvas=480x480 ratio=0.1479 colors=136`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_info_label/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(44, 121) - (436, 265)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 265` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前 demo 只覆盖 `closed / open` 与 `accent / warning / neutral` 的最小 `InfoLabel` 语义。
- 气泡使用控件内 anchored bubble，而不是系统级 popup。
- 当前页面优先保证 reference 录制稳定，不额外扩展更复杂的定位避让和多气泡管理。
- 底部 `compact / read only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `tool_tip`：这里强调标签旁解释入口，而不是悬停提示。
- 相比 `teaching_tip`：这里不承担教学卡片或高层级引导。
- 相比 `text_block`：这里保留按需展开的轻交互语义。
- 相比 `info_badge`：这里表达的是解释信息，而不是状态角标。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_info_label` custom view，不修改 SDK。
- 主区保留 3 组 reference 快照：`closed accent`、`open warning`、`open neutral`。
- 底部 preview 通过 `hcw_info_label_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展更多 `InfoLabel` 页面场景。
