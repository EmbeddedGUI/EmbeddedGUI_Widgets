# info_badge 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI / WPF UI`
- 对应组件：`InfoBadge`
- 当前保留语义：`count`、`icon`、`attention dot` 三种核心附着提醒语义、palette setter、静态 preview 输入抑制
- 当前移除内容：主面板包装、页面级 guide / note、preview panel / heading / body、轮换 preview 轨道、SDK 改动
- EGUI 适配说明：继续复用 SDK `notification_badge`，custom 层只补样式 helper、attention dot draw 语义与静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`info_badge` 用来表达“附着在宿主元素旁边的轻量状态提醒”。它适合用于设置项、摘要条目、导航入口或列表行上的数量、信息提示和 attention dot，而不是块级反馈或故事化告警容器。

## 2. 为什么现有控件不够用
- SDK `notification_badge` 只有基础计数/图标绘制，没有当前仓库要求的 `InfoBadge` 语义收口、attention dot draw 语义和静态 preview API。
- `badge`、`badge_group` 更偏独立标签或组合展示，不是附着型信息角标。
- `message_bar`、`toast_stack` 这类反馈控件层级更高、噪音更大，不适合替代行内提醒。

## 3. 当前页面结构
- 标题：`InfoBadge`
- 主区：三行并列的 `count / icon / attention dot`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`overflow`
- 右侧 preview：`attention`

目录：
- `example/HelloCustomWidgets/display/info_badge/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组目标快照和最终稳定帧：

1. 默认态
   `Inbox updates` + `18`
   `Policy note` + `Info`
   `Review pending` + red dot
2. 快照 2
   `Build blockers` + `7`
   `QA warning` + `Warning`
   `Deployment paused` + red dot
3. 快照 3
   `Finished checks` + `2`
   `Published` + `Done`
   `Watch list` + blue dot
4. 最终稳定帧
   `Inbox updates` + `18`
   `Policy note` + `Info`
   `Review pending` + red dot

底部 preview 在整条轨道中始终固定：

1. `overflow`
   `99+`
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
- 页面结构：标题 -> 三行主语义 -> 底部 `overflow / attention`
- 风格约束：浅色 page panel、主区只保留行标签与 badge 语义切换，底部 preview 只做静态 reference

## 6. 状态矩阵
| 状态 | 主控件 | Overflow preview | Attention preview |
| --- | --- | --- | --- |
| 默认显示 | `Inbox updates + 18` / `Policy note + Info` / `Review pending + red dot` | `99+` | red dot |
| 快照 2 | `Build blockers + 7` / `QA warning + Warning` / `Deployment paused + red dot` | 保持不变 | 保持不变 |
| 快照 3 | `Finished checks + 2` / `Published + Done` / `Watch list + blue dot` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Inbox updates + 18` / `Policy note + Info` / `Review pending + red dot` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Build blockers / QA warning / Deployment paused`
4. 抓取第二组主区快照
5. 切到 `Finished checks / Published / Watch list`
6. 抓取第三组主区快照
7. 回到默认 `Inbox updates / Policy note / Review pending`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区默认态，并走统一布局重放路径。
- 录制阶段不再保留旧版主面板、heading、note 和 preview 包装容器。
- 页面层不再为 preview 保留额外说明文案。
- 底部 preview 统一通过 `hcw_info_badge_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `INFO_BADGE_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_info_badge.c` 当前覆盖三部分：

1. 样式 helper
   覆盖 `apply_count_style()`、`apply_icon_style()`、`apply_attention_style()` 的 `content_style`、默认 icon 与 palette。
2. setter 守卫
   覆盖 `set_count()`、`set_icon()`、`set_icon(NULL)`、`set_palette()` 对 `pressed` 状态的清理和模式切换。
3. 静态 preview 不变性断言
   通过 `info_badge_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`count`、`max_display`、`badge_color`、`text_color`、`font`、`content_style`、`icon`、`icon_font`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

补充说明：
- 静态 preview 用例已收口为 “consumes input and keeps state”。
- 为兼容当前 `HelloUnitTest` harness，preview 用例继续直接调用 `on_touch_event()` / `on_key_event()`。

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

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/info_badge PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `info_badge` suite `3 / 3`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_info_badge/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_info_badge`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1099 colors=95`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_info_badge/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(64, 163) - (366, 271)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 272` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `info_badge` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应默认、`Build blockers / QA warning / Deployment paused` 与 `Finished checks / Published / Watch list` 三组主区快照；最终稳定帧已显式回到默认态。
- 按 `y >= 272` 裁剪底部 preview 区域后保持单哈希，确认 `overflow / attention` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前 demo 只覆盖 `count`、`icon`、`attention dot` 三种最小 `InfoBadge` 语义。
- `attention dot` 通过 custom draw 收口为 `icon == NULL && style == ICON` 的无字圆点，不扩展到更复杂的宿主附着定位。
- 当前页面优先保证 reference 录制稳定，不额外扩展 hover、动画或组合式交互容器。
- 底部 `overflow / attention` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `badge`：这里强调附着型信息提醒，而不是独立标签展示。
- 相比 `badge_group`：这里是单个 `InfoBadge` 语义，不是多 badge 组合容器。
- 相比 SDK `notification_badge`：这里补齐 `InfoBadge` 语义 helper、attention dot 和静态 preview 输入抑制。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_info_badge` custom view，不修改 SDK。
- 主区保留三行 `count / icon / attention dot` 与 3 组 reference 快照。
- 底部 preview 通过 `hcw_info_badge_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制稳定，再评估是否需要扩展更多宿主布局场景。
