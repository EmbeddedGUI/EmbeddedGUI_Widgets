# Divider 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`Separator`
- 当前保留语义：`standard / accent / subtle` 主区 reference 快照、`subtle / accent` 静态 preview、静态 preview 输入抑制
- 当前移除内容：旧主 `primary_panel / heading / body / note`、底部 `subtle_panel / accent_panel` 包装与说明文案
- EGUI 适配说明：继续复用 SDK `divider` 的基础绘制，在 custom 层补齐 `Separator` 的 style helper、palette setter 和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`divider` 用于在低噪音界面里切分相邻内容段落。它只负责表达“这里有一个层级边界”，不承担卡片、标题栏、说明块或其它宿主 chrome 的职责，适合作为 `display` 类别里的基础 reference 控件保留。

## 2. 为什么现有控件不够用
- `card_panel` 承担的是结构化容器语义，不是最轻量的分隔语义。
- `badge`、`info_badge`、`presence_badge` 关注的是提醒或状态表达，不负责页面节奏切分。
- SDK 自带 `divider` 只有基础绘制，没有当前仓库要求的 Fluent reference 页面、静态 preview API 和验收闭环。

## 3. 当前页面结构
- 标题：`Separator`
- 主区：一个主 `divider` 和一个主状态 `label`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`subtle`
- 右侧 preview：`accent`

目录：
- `example/HelloCustomWidgets/display/divider/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组 reference 快照和最终稳定帧：

1. 默认态
   文案：`Standard / neutral`
   表现：标准灰色分隔线
2. 快照 2
   文案：`Accent / emphasis`
   表现：Fluent 蓝色强调分隔线
3. 快照 3
   文案：`Subtle / muted`
   表现：更浅、更低 alpha 的弱化分隔线
4. 最终稳定帧
   文案：`Standard / neutral`
   表现：恢复默认标准分隔线

底部 preview 在整条轨道中始终固定：

1. `subtle`
   palette：`subtle`
2. `accent`
   palette：`accent`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 132`
- 标题：`224 x 18`
- 主 `divider`：`176 x 2`
- 主状态 label：`224 x 12`
- 底部 preview 行：`80 x 2`
- 单个 preview：`36 x 2`
- 页面结构：标题 -> 主 `divider` -> 状态 label -> 底部 `subtle / accent`
- 风格约束：浅色 page panel、极简分隔线、状态差异只通过主线颜色和 label 文案表达，不再保留额外面板包裹或叙事性说明文案

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认显示 | `Standard / neutral` | `subtle` | `accent` |
| 快照 2 | `Accent / emphasis` | 保持不变 | 保持不变 |
| 快照 3 | `Subtle / muted` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Standard / neutral` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Accent / emphasis`
4. 抓取第二组主区快照
5. 切到 `Subtle / muted`
6. 抓取第三组主区快照
7. 回到默认 `Standard / neutral`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 页面层不再保留旧 `primary_panel`、`heading / body / note` 与底部 preview panel 包装。
- 底部 preview 统一通过 `hcw_divider_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `DIVIDER_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_divider.c` 当前覆盖三部分：

1. style helper 与 palette
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()` 对 `pressed` 状态的清理，以及颜色和 alpha 变化。
2. palette setter 守卫
   覆盖 `hcw_divider_set_palette()` 对 `pressed`、`background` 和 `padding` 的清理。
3. 静态 preview 不变性断言
   通过 `divider_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / color / alpha / on_click_listener / api / enable / is_focused / is_pressed / padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/divider PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/divider --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/divider
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_divider
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/divider PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `divider` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_divider/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_divider`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1034 colors=65`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_divider/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(64, 168) - (416, 199)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 199` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `divider` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应默认 `Standard / neutral`、`Accent / emphasis` 与 `Subtle / muted` 三组主区快照；最终稳定帧已显式回到默认态。
- 按 `y >= 199` 裁剪底部 preview 区域后保持单哈希，确认 `subtle / accent` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖横向 `Separator`，不扩展纵向分隔线或带标题的复合 section header。
- `subtle` 变体优先保持低噪音，不追求在强对比背景下的独立强调。
- 底部 `subtle / accent` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `card_panel`：这里不承载结构化容器，只负责最轻量的节奏切分。
- 相比 `info_label`：这里不表达语义提示文本，只表达边界和层级。
- 相比 `badge`、`info_badge`：这里不承担提醒或状态计数，只负责页面分隔。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_divider` custom facade，不修改 SDK。
- 主区保留 `Standard / neutral`、`Accent / emphasis`、`Subtle / muted` 三组 reference 快照。
- 底部 preview 通过 `hcw_divider_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制不再保留旧 panel 级说明 chrome。
