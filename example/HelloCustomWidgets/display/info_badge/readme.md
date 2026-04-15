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
主区录制轨道只保留 3 组目标快照：

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
| 状态 / 区域 | 主控件 | Overflow preview | Attention preview |
| --- | --- | --- | --- |
| `count` | 是 | 是 | 否 |
| `icon` | 是 | 否 | 否 |
| `attention dot` | 是 | 否 | 是 |
| `set_count()` 切换 | 支持 | 支持 | 否 |
| `set_icon()` / `set_icon(NULL)` 切换 | 支持 | 否 | 支持 |
| `set_palette()` 改色 | 支持 | 支持 | 支持 |
| 静态 preview 吞掉 `touch / key` | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Release board` 对应快照
4. 抓取第二组主区快照
5. 切到 `Calm summary` 对应快照
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再保留旧版主面板、heading、note 和 preview 包装容器。
- 页面层不再为 preview 保留额外说明文案。
- 底部 preview 统一通过 `hcw_info_badge_override_static_preview_api()` 吞掉 `touch / key`，只负责静态 reference 对照。

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

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/info_badge PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `info_badge` suite `3 / 3`
- `sync_widget_catalog.py`：已通过，`widget_catalog.json` 与 `web/catalog-policy.json` 保持同步，本轮无额外目录漂移
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_badge --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- `display` 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 与 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_badge`，输出 `web/demos/HelloCustomWidgets_display_info_badge`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_badge`，结果 `PASS status=Running canvas=480x480 ratio=0.1099 colors=95`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_info_badge/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(64, 163) - (366, 271)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 272` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

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
