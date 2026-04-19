# info_badge 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`InfoBadge`
- 当前保留形态：主区 `Inbox updates`、`Build blockers`、`Finished checks` 三组 reference 快照，底部 `overflow / attention` 双静态 preview
- 当前保留交互：主区保留程序化 `count / icon / attention dot` 切换；`count / icon / palette` setter 与 style helper 清理 `pressed`；底部 static preview 吞掉 `touch / key`
- 当前移除内容：旧主 `panel / heading / note`、底部 preview 包装和说明文案、轮换 preview 轨道、录制末尾回切默认态的额外桥接帧
- EGUI 适配说明：继续复用 SDK `notification_badge` 的基础计数与绘制能力，在 custom 层补齐 `InfoBadge` 的 `count / icon / attention dot` 语义、palette setter 和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`info_badge` 用来表达“附着在宿主元素旁边的轻量提醒”。它强调的是数量、信息图标或 attention dot 这类小面积状态提示，适合挂在设置项、摘要条目、导航入口或列表行旁边，而不是扩展成块级反馈面板。

## 2. 为什么现有控件不够用

- SDK `notification_badge` 只有基础计数和图标绘制，没有当前仓库要求的 `InfoBadge` 语义收口、attention dot 绘制语义和静态 preview API。
- `badge`、`badge_group` 更偏独立标签或组合展示，不是附着型信息角标。
- `message_bar`、`toast_stack` 这类反馈控件层级更高、页面噪音更强，不适合替代行内轻提醒。

## 3. 当前页面结构

- 标题：`InfoBadge`
- 主区：三行并列的 `count / icon / attention dot`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`overflow`
- 右侧 preview：`attention`
- 页面结构：标题 -> 三行主语义 -> 底部 `overflow / attention`

目录：
- `example/HelloCustomWidgets/display/info_badge/`

## 4. 主区 reference 快照

主区录制轨道只保留 `3` 组 reference 快照和最终稳定帧：

1. 默认态
   文案：`Inbox updates`、`Policy note`、`Review pending`
   表现：`18` count、`Info` icon、red dot
2. 快照 2
   文案：`Build blockers`、`QA warning`、`Deployment paused`
   表现：`7` count、`Warning` icon、red dot
3. 快照 3
   文案：`Finished checks`、`Published`、`Watch list`
   表现：`2` count、`Done` icon、blue dot
4. 最终稳定帧
   文案：`Inbox updates`、`Policy note`、`Review pending`
   表现：恢复默认 `count / icon / attention dot` 组合

底部 preview 在整条轨道中始终固定：

1. `overflow`
   `count=128`，`max_display=99`，显示 `99+`
2. `attention`
   red dot

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 144`
- 标题：`224 x 18`
- 主区单行：`176 x 24`
- `count` badge：`34 x 20`
- `icon` badge：`20 x 20`
- `dot` badge：`12 x 12`
- 底部 preview 行：`60 x 20`
- `overflow` preview：`40 x 20`
- `attention` preview：`12 x 12`
- 页面风格：浅色 page panel、主区只保留行标签和 `InfoBadge` 语义切换，底部 preview 只做静态 reference，对比集中在 badge 内容和颜色本身

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Overflow preview | Attention preview |
| --- | --- | --- | --- |
| 默认 `Inbox updates / Policy note / Review pending` | 是 | 否 | 否 |
| `Build blockers / QA warning / Deployment paused` | 是 | 否 | 否 |
| `Finished checks / Published / Watch list` | 是 | 否 | 否 |
| 最终稳定帧回到默认态 | 是 | 否 | 否 |
| `99+` overflow | 否 | 是 | 否 |
| red dot attention | 否 | 否 | 是 |
| 静态 preview 对照 | 否 | 是 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_info_badge.c` 当前覆盖 `3` 条用例：

1. `style helpers apply expected modes and palette`
   覆盖 `apply_count_style()`、`apply_icon_style()`、`apply_attention_style()` 的模式切换、默认 icon 与 palette。
2. `setters clear pressed state and switch modes`
   覆盖 `set_count()`、`set_icon()`、`set_icon(NULL)`、`set_palette()` 对 `pressed` 状态的清理和模式切换。
3. `static preview consumes input and keeps state`
   通过 `info_badge_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验 `region_screen`、`background`、`count`、`max_display`、`badge_color`、`text_color`、`font`、`content_style`、`icon`、`icon_font`、`alpha`、`enable`、`is_focused`、`is_pressed` 与 `padding` 不变。

补充说明：

- 主区 `info_badge` 是 display-first 的附着型提醒控件，重点在 `count / icon / attention dot` 语义，不承担 click 提交职责。
- 底部 `overflow / attention` preview 统一通过 `hcw_info_badge_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照职责。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 应用主区默认 `Inbox updates / Policy note / Review pending` 和底部 preview 固定状态，请求首帧并等待 `INFO_BADGE_RECORD_FRAME_WAIT = 170`
2. 切到 `Build blockers / QA warning / Deployment paused`，等待 `INFO_BADGE_RECORD_WAIT = 90`
3. 请求第二组主区快照并继续等待 `170`
4. 切到 `Finished checks / Published / Watch list`，等待 `90`
5. 请求第三组主区快照并继续等待 `170`
6. 回到默认 `Inbox updates / Policy note / Review pending`，等待 `90`
7. 请求最终稳定帧，并继续等待 `INFO_BADGE_RECORD_FINAL_WAIT = 280`

说明：

- 录制轨道只导出主区三态与最终稳定帧。
- 初始化阶段在 root view 挂载前后各重放一次默认态与 preview，统一走 `ui_ready + layout_page + request_page_snapshot` 布局重放路径。
- 页面层不再保留旧主 panel、heading、note 和底部 preview 包装文案。

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=display/info_badge PORT=pc

# 在 X:\ 短路径工作区执行，规避 Windows 命令行长度限制
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_badge --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_badge
```

## 10. 验收重点

- 主区三行 `count / icon / attention dot` 和底部 `overflow / attention` preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区 `Inbox updates`、`Build blockers`、`Finished checks` 三组状态必须能从截图中稳定区分，且最终稳定帧显式回到默认态。
- `apply_count_style()`、`apply_icon_style()`、`apply_attention_style()`、`set_count()`、`set_icon()` 与 `set_palette()` 不能残留 `pressed`。
- 底部 `overflow / attention` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_display_info_badge/default`
- 已归档复核结果：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(64, 163) - (366, 271)`
  - 遮罩主区差分边界后，主区外唯一哈希数：`1`
  - 按主区裁剪后，主区唯一状态数：`3`
  - 按 `y >= 272` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 最终稳定帧显式回到默认 `Inbox updates / Policy note / Review pending`

## 12. 与现有控件的边界

- 相比 `badge`：这里强调附着式信息提醒，不是独立短文本状态标签。
- 相比 `counter_badge`：这里保留 `count / icon / attention dot` 组合语义，不只聚焦数字与 overflow。
- 相比 SDK `notification_badge`：这里补齐 `InfoBadge` 语义 helper、attention dot 和静态 preview 输入抑制。

## 13. 本轮保留与删减

- 保留的主区状态：`Inbox updates`、`Build blockers`、`Finished checks`
- 保留的底部对照：`overflow`、`attention`
- 保留的交互与实现约束：`count / icon / attention dot` 语义、palette setter、style helper 清理 `pressed`、static preview 输入抑制
- 删减的旧桥接与装饰：旧主 `panel / heading / note`、底部 preview 包装和说明文案、轮换 preview 轨道、录制末尾回切默认态的额外桥接帧

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=display/info_badge PORT=pc`
  - 本轮沿用 `2026-04-16` 已归档 acceptance 结果
- `HelloUnitTest`：日志复核 `PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `info_badge` suite `3 / 3`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category display`
  - `python scripts/checks/check_docs_encoding.py`
  - 本轮重新执行文档编码与 display 触摸语义检查；`sync_widget_catalog.py`、`check_widget_catalog.py` 与 widget catalog 结果沿用 `2026-04-16` 已归档 acceptance 数据
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_badge --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_display_info_badge/default`
  - 本轮沿用 `2026-04-16` 已归档 runtime 结果，并按 tracker 最新 static preview 记录采用 `8` 帧 / `3` 组主区状态 / `y >= 272` preview 单哈希的复核口径
- display 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category display --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`
  - 沿用 `2026-04-16` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_badge`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1099 colors=95`
  - `web/demos/HelloCustomWidgets_display_info_badge` 构建结果沿用 `2026-04-16` 已归档 acceptance 数据
- 截图复核结论：
  - 共捕获 `8` 帧
  - 主区 RGB 差分边界：`(64, 163) - (366, 271)`
  - 遮罩主区差分边界后主区外唯一哈希数：`1`
  - 主区唯一状态数：`3`
  - 按 `y >= 272` 裁切底部 preview 后，preview 区唯一哈希数：`1`
  - 结论：主区完整覆盖 `Inbox updates`、`Build blockers`、`Finished checks` 三组 reference 快照，最终稳定帧已回到默认态，底部 `overflow / attention` preview 全程静态
