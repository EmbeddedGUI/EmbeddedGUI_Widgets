# badge 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`Badge`
- 当前保留形态：`filled`、`outline`、`subtle`、`compact`、`read only`
- 当前保留交互：主区保留程序化样式快照切换；底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：主面板说明文案、底部 panel 包装与 heading、录制阶段额外恢复帧、preview 说明文字与额外页面 chrome

## 1. 为什么需要这个控件
`badge` 用来表达附着在内容上的短状态或说明，例如审核状态、发布标记、预览标签和归档标识。仓库虽然已经有 `info_badge`、`badge_group` 和 `tag`，但还缺少一颗与 `Fluent 2 / Fluent UI React Badge` 语义对齐的单个文本 badge reference。

## 2. 为什么现有控件不够用
- `info_badge` 更偏计数、图标和 attention dot，不承载短文本状态。
- `badge_group` 更偏多 badge 汇总，不是单个附着式状态标签。
- `tag` 表达的是用户已选或可撤销的值，不适合承载系统生成的不可编辑状态。
- 当前主线仍需要一个与 `Fluent 2` 语义对齐的 `Badge` reference 页面、单测和 web 验收闭环。

## 3. 当前页面结构
- 标题：`Badge`
- 主区：一个标准 `badge`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Beta`
- 右侧 preview：`read only`，固定显示 `Archived`

目录：
- `example/HelloCustomWidgets/display/badge/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照和最终稳定帧，不再保留主面板说明文字：

1. 默认态
   文案：`Verified`
   图标：`Done`
   样式：`filled`
2. 快照 2
   文案：`Preview`
   图标：`Info`
   样式：`outline`
3. 快照 3
   文案：`Needs review`
   图标：`Warning`
   样式：`subtle`
4. 最终稳定帧
   文案：`Verified`
   图标：`Done`
   样式：`filled`

底部 preview 在整条轨道中始终固定：
1. `compact`
   文案：`Beta`
   图标：`Info`
2. `read only`
   文案：`Archived`
   图标：无

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 112`
- 主控件：`126 x 28`
- 底部 preview 行：`184 x 24`
- 单个 preview：`88 x 24`
- 页面结构：标题 -> 主 `badge` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音边框、清晰的 `filled / outline / subtle` 层级差异，不回退到 showcase 式说明卡片

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Verified` + `filled` | `Beta` + `outline` | `Archived` + `read only` |
| 快照 2 | `Preview` + `outline` | 保持不变 | 保持不变 |
| 快照 3 | `Needs review` + `subtle` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `Verified` + `filled` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Preview`
4. 抓取第二组主区快照
5. 切到 `Needs review`
6. 抓取第三组主区快照
7. 回到默认 `Verified`
8. 抓取最终稳定帧

说明：
- 录制阶段最终会显式恢复主区到默认 `Verified` 并走统一布局重放路径
- 页面层不再保留主 badge 的说明 note
- 底部 preview 统一通过 `egui_view_badge_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担页面桥接职责

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `BADGE_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_badge.c` 当前覆盖两部分：

1. 主控件样式与 setter 状态守卫
   覆盖样式 helper、文本与图标 setter、palette setter、icon region 与 `compact / read_only` 模式切换时的 `pressed` 清理
2. 静态 preview 不变性断言
   通过 `badge_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`font`、`icon_font`、`surface_color`、`border_color`、`text_color`、`accent_color`、`text`、`icon`、`compact_mode`、`read_only_mode`、`outline_mode`、`subtle_mode`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/badge PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `badge` suite `5 / 5`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`9 frames captured -> runtime_check_output/HelloCustomWidgets_display_badge/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_badge`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.0855 colors=106`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_badge/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(114, 183) - (365, 224)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 252` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `badge` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态，对应 `Verified / Preview / Needs review` 三组主区快照；最终稳定帧已显式回到默认 `Verified`。
- 按 `y >= 252` 裁剪底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖单个 `Badge`，不实现成组布局、dismiss 或容器级状态汇总。
- 当前 leading icon 只支持单个 icon font 字形，不扩展自定义图片或多段内容。
- 底部 `compact / read only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `info_badge`：这里表达短文本状态，不是计数或 attention dot。
- 相比 `badge_group`：这里聚焦单个附着式 badge，不承担汇总布局。
- 相比 `tag`：这里表达系统生成的状态，不是用户可编辑或可撤销的 token。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_badge` custom view，不修改 SDK。
- 主区保留 `filled / outline / subtle` 三组 reference 快照。
- 底部 preview 通过 `egui_view_badge_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制无多余说明 chrome，再评估是否需要扩展成更复杂的 badge 组合形态。
