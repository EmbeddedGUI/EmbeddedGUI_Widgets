# badge_group 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件：`BadgeGroup`
- 当前保留形态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前保留交互：主区保留程序化 snapshot 切换；底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：preview 轮换、preview 点击收尾、页面级 `guide / status` chrome、旧双列包裹壳与额外录制收尾帧

## 1. 为什么需要这个控件
`badge_group` 用来展示一组语义相关的 badge，并让 focus badge 驱动整张卡片的 tone 与 footer summary。它适合出现在概览页、审阅页和状态面板中，用于表达“同一条信息有多个维度，但仍然属于一组”的轻量展示。

## 2. 为什么现有控件不够用
- `notification_badge` 只解决单个角标或计数，不解决多 badge 并列组合。
- `chips` 更偏交互筛选和选中态，不适合作为静态信息组合。
- `tag_cloud` 强调自由分布和权重表达，不强调 focus badge 对摘要的驱动关系。
- `card_panel` 更偏结构化卡片，不适合做低噪声的 badge 集群。

## 3. 当前页面结构
- 标题：`Badge Group`
- 主区：一个标准 `badge_group`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `SET / Compact`
- 右侧 preview：`read only`，固定显示 `ARCHIVE / Read only`

目录：
- `example/HelloCustomWidgets/display/badge_group/`

## 4. 主区 reference 快照
主区录制轨道保留 4 组程序化快照和最终稳定帧：

1. 默认态
   眉标：`TRIAGE`
   标题：`Release lanes`
   tone：`accent`
   badge：`Review`、`Ready`、`Risk`、`Archive`
2. 快照 2
   眉标：`QUEUE`
   标题：`Ops handoff`
   tone：`success`
   badge：`Online`、`Shadow`、`Sync`、`Alert`
3. 快照 3
   眉标：`RISK`
   标题：`Change review`
   tone：`warning`
   badge：`Queued`、`Hold`、`Owner`、`Done`
4. 快照 4
   眉标：`CALM`
   标题：`Archive sweep`
   tone：`neutral`
   badge：`Pinned`、`Calm`、`Watch`、`Live`
5. 最终稳定帧
   眉标：`TRIAGE`
   标题：`Release lanes`
   tone：`accent`
   badge：`Review`、`Ready`、`Risk`、`Archive`

底部 preview 在整条轨道中始终固定：
1. `compact`
   眉标：`SET`
   标题：`Compact`
   badge：`Ready`、`Muted`
2. `read only`
   眉标：`ARCHIVE`
   标题：`Read only`
   badge：`Pinned`、`Review`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 242`
- 主控件：`196 x 118`
- 底部 preview 行：`216 x 84`
- 单个 preview：`104 x 84`
- 页面结构：标题 -> 主 `badge_group` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音边框、focus tone 只做轻量强化，不回退到 showcase 式说明页

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `TRIAGE` | `SET` | `ARCHIVE` |
| 快照 2 | `QUEUE` | 保持不变 | 保持不变 |
| 快照 3 | `RISK` | 保持不变 | 保持不变 |
| 快照 4 | `CALM` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `TRIAGE` | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `QUEUE`
4. 抓取第二组主区快照
5. 切到 `RISK`
6. 抓取第三组主区快照
7. 切到 `CALM`
8. 抓取第四组主区快照
9. 回到默认 `TRIAGE`
10. 抓取最终稳定帧

说明：
- 录制阶段不再轮换 `compact` preview
- 不再通过点击 preview 收主区焦点
- 底部 preview 统一通过 `egui_view_badge_group_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担页面桥接职责

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `BADGE_GROUP_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_badge_group.c` 当前覆盖两部分：

1. 主控件状态与交互守卫
   覆盖 `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`compact / read_only` 模式切换、same-target release、`ACTION_CANCEL`、键盘 click、禁用与只读 guard，以及内部 helper
2. 静态 preview 不变性断言
   通过 `badge_group_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`snapshots`、`font`、`meta_font`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`neutral_color`、`snapshot_count`、`current_snapshot`、`compact_mode`、`read_only_mode`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/badge_group --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/badge_group
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_badge_group
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：`PASS`，`make all APP=HelloCustomWidgets APP_SUB=display/badge_group PORT=pc`
- `HelloUnitTest`：`PASS`，已通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 与 `X:\output\main.exe`，总计 `845 / 845`，其中 `badge_group` suite `8 / 8`
- `sync_widget_catalog.py`：`PASS`，同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`，`custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：`PASS`，`134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`，`106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`，`11 frames captured -> runtime_check_output/HelloCustomWidgets_display_badge_group/default`
- display 分类 compile/runtime 回归：`PASS`
  compile `21 / 21`，runtime `21 / 21`
- wasm 构建：`PASS`，`web/demos/HelloCustomWidgets_display_badge_group`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1845 colors=195`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_badge_group/default`

复核结果：
- 总帧数：`11`
- 主区 RGB 差分边界：`(46, 87) - (433, 250)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`4`
- 按 `y >= 304` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `4`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `badge_group` reference 主体，主区外页面 chrome 在整条轨道中保持静态。
- `11` 帧里主区保持 `4` 组唯一状态，对应 `TRIAGE / QUEUE / RISK / CALM` 四组主区快照；最终稳定帧已显式回到默认 `TRIAGE`。
- 按 `y >= 304` 裁切底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长 label 和超过 `6` 个 badge 的数据。
- 当前不做真实图标、hover、focus ring 和复杂列表交互细节。
- 当前 badge 宽度估算基于简化字符宽度，不是完整文本测量系统。
- 底部 `compact / read only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `notification_badge`：这里不是单个计数泡，而是一组可混合 tone 的 badge 集群。
- 相比 `chips`：这里不是交互筛选条，不强调选中、取消和筛选结果。
- 相比 `tag_cloud`：这里不是权重词云，不做自由散点布局。
- 相比 `card_panel`：这里更轻、更扁平，重点在 badge 组合和 focus summary。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_badge_group` custom view，不修改 SDK。
- 主区保留 `accent / success / warning / neutral` 四组 reference 快照。
- 底部 preview 通过 `egui_view_badge_group_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区多 tone 快照、底部 preview 全程静态，以及 runtime 录制无多余页面桥接逻辑，再评估是否需要扩展成更复杂的分组与滚动场景。
