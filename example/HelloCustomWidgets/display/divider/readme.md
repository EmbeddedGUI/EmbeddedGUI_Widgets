# Divider 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`Separator`
- 当前保留形态：主区 `standard / accent / subtle` 三组 reference 快照，底部 `subtle / accent` 双静态 preview
- 当前保留交互：主区保留程序化样式切换；`style helper / palette setter` 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `primary_panel / heading / body / note`、底部 `subtle_panel / accent_panel` 包装与说明文案、录制末尾回切默认态的额外桥接帧
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
- 页面结构：标题 -> 主 `divider` -> 状态 label -> 底部 `subtle / accent`

目录：
- `example/HelloCustomWidgets/display/divider/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

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
- 页面风格：浅色 page panel、极简分隔线、状态差异只通过主线颜色和 label 文案表达，不再保留额外面板包裹或叙事性说明文案

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Subtle preview | Accent preview |
| --- | --- | --- | --- |
| 默认 `Standard / neutral` | 是 | 否 | 否 |
| `Accent / emphasis` | 是 | 否 | 否 |
| `Subtle / muted` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `subtle` | 否 | 是 | 否 |
| `accent` | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_divider.c` 当前覆盖 `3` 条用例：

1. `style helpers apply expected palette and clear pressed state`
   覆盖 `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()` 对 `pressed` 状态的清理，以及颜色和 alpha 变化。
2. `palette setter clears pressed state and resets background`
   覆盖 `hcw_divider_set_palette()` 对 `pressed`、`background` 和 `padding` 的清理。
3. `static preview consumes input and keeps state`
   通过 `divider_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`color`、`alpha`、`on_click_listener`、`api`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `divider` 是 display-first 的只读分隔控件，重点在分隔线配色和 alpha 语义，不承担 click 提交职责。
- 底部 `subtle / accent` preview 统一通过 `hcw_divider_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Standard / neutral` 和底部 preview 固定状态，请求首帧并等待 `DIVIDER_RECORD_FRAME_WAIT = 170`
2. 切到 `Accent / emphasis`，等待 `DIVIDER_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Subtle / muted`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Standard / neutral`，等待 `90`
7. 请求最终稳定帧，并继续等待 `DIVIDER_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧 `primary_panel`、`heading / body / note` 与底部 preview panel 包装。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/divider PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
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

## 10. 验收重点

- 主区与底部 `subtle / accent` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Standard / neutral`、`Accent / emphasis`、`Subtle / muted` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `apply_standard_style()`、`apply_subtle_style()`、`apply_accent_style()` 与 `hcw_divider_set_palette()` 不能残留 `pressed`。
- 底部 `subtle / accent` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_divider/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(64, 168) - (416, 199)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 199` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Standard / neutral`

## 12. 与现有控件的边界

- 相比 `card_panel`：这里不承载结构化容器，只负责最轻量的节奏切分。
- 相比 `info_label`：这里不表达语义提示文本，只表达边界和层级。
- 相比 `badge`、`info_badge`：这里不承担提醒或状态计数，只负责页面分隔。

## 13. 本轮保留与删减

- 保留的主区状态：`Standard / neutral`、`Accent / emphasis`、`Subtle / muted`
- 保留的底部对照：`subtle`、`accent`
- 保留的交互与实现约束：`standard / accent / subtle` 样式 helper、palette setter、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `primary_panel / heading / body / note`、底部 `subtle_panel / accent_panel` 包装与说明文案、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/divider PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `divider` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/divider --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_divider/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 199` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_divider`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1034 colors=65`
  - `web/demos/HelloCustomWidgets_display_divider` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(64, 168) - (416, 199)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 199` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Standard / neutral`、`Accent / emphasis`、`Subtle / muted` 三组 reference 快照，最终稳定帧已回到默认态，底部 `subtle / accent` preview 全程静态
