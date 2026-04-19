# Drawer 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React / Drawer`
- 对应组件：`Drawer`
- 当前保留形态：`Filters`、`Review`、`Archive`、`Quick`、`Read`
- 当前保留交互：主区保留 `inline / overlay`、`start / end anchor`、`open / closed`、same-target release 与键盘 `Enter / Space / Escape` 闭环；底部 `Quick / Read` preview 保持静态 reference 对照
- 当前移除内容：路由壳层、复杂表单插槽、业务化审批流、旧录制末尾“恢复后立即抓帧”的模板化收尾、只验证“消耗输入”但不复核状态保持的旧 preview 口径，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/drawer`，底层仍复用仓库内现有 `egui_view_drawer` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`drawer` 用来承载与当前页面强相关、但又不适合直接塞进主内容区的附加信息或次级操作。它不是持久双栏布局，而是 Fluent 页面里承接临时 side rail、边缘锚点和 overlay / inline 切换的 reference 控件。

## 2. 为什么现有控件不够用
- `split_view` 更偏持久双栏布局，不强调抽屉的临时开合语义。
- `dialog_sheet` 更像确认或提交流程的模态容器，交互重心在 action button，而不是边缘侧抽屉。
- `flyout` 关注的是锚点气泡，不承载大尺寸 side rail 的空间占用和 overlay 语义。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `drawer` -> 底部 `Quick / Read` 双 preview。
- 主区保留 `3` 组录制快照：
  - `Filters`
  - `Review`
  - `Archive`
- 录制最终稳定帧显式回到默认 `Filters`。
- `Archive` 是 `open = 0` 的 `closed` 快照，用来验证只保留 toggle 的抽屉收起状态。
- 底部左侧是 `Quick` 静态 preview，固定对照 `Quick`，保持 `anchor = start`、`presentation_mode = overlay`、`open = 1`、`compact_mode = 1`。
- 底部右侧是 `Read` 静态 preview，固定对照 `Read`，保持 `anchor = end`、`presentation_mode = inline`、`open = 1`、`compact_mode = 1`、`read_only_mode = 1`。
- 两个 preview 统一通过 `egui_view_drawer_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `anchor / presentation_mode / open / compact_mode / read_only_mode`
  - 不改 `toggle / close` 区域
  - 不触发 `on_open_changed`

目标目录：
- `example/HelloCustomWidgets/layout/drawer/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照与最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Filters`
2. 快照 2
   `Review`
3. 快照 3
   `Archive`
   该快照为 `closed`
4. 最终稳定帧
   回到默认 `Filters`

底部 preview 在整条轨道中固定为：
1. `Quick`
2. `Read`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 216`
- 主控件：`196 x 112`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `drawer` -> 底部 `Quick / Read`
- 风格约束：保持浅灰页面、白色抽屉面和低噪音边框；通过 host body 是否缩窄、是否覆盖 veil，以及 toggle 所在边缘突出 `inline / overlay / anchor` 差异；底部 preview 固定为静态 reference 对照，不再承担点击桥接、焦点收尾或额外轨道切换职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Filters` | `Quick` | `Read` |
| 快照 2 | `Review` | 保持不变 | 保持不变 |
| 快照 3 | `Archive`（closed） | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Filters` | 保持不变 | 保持不变 |
| `inline / overlay`、`start / end anchor`、`open / closed` | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_drawer.c` 当前覆盖 `5` 条用例：

1. 默认态、字体 / 配色 / `anchor / presentation_mode / compact / read_only` setter 的 `pressed` 清理与状态更新。
2. `toggle / close` 的触摸 same-target release 语义。
3. 键盘 `Enter / Space / Escape` 开合闭环。
4. static preview 吞掉 `touch / key`，并保持 `anchor / presentation_mode / open / compact_mode / read_only_mode / toggle region / close region` 不变，同时不触发 `on_open_changed`。
5. `read_only / !enable` 守卫清理残留 `pressed` 并屏蔽输入；恢复后重新允许 `SPACE` 切换开合。

说明：
- 主控件键盘入口统一走 `dispatch_key_event()`，不再依赖旧的 `on_key_event()` 直连路径。
- setter、guard 与 static preview 路径都统一要求先清理残留 `pressed_part / is_pressed`，再处理后续状态。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Filters`，同时重放底部 `Quick / Read` preview 固定状态并抓取首帧，等待 `DRAWER_RECORD_FRAME_WAIT`。
2. 切到 `Review`，等待 `DRAWER_RECORD_FINAL_WAIT`。
3. 抓取第二组主区快照，等待 `DRAWER_RECORD_FRAME_WAIT`。
4. 切到 `Archive`，等待 `DRAWER_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `DRAWER_RECORD_FRAME_WAIT`。
6. 恢复主区默认 `Filters`，同时重放底部 preview 固定状态，等待 `DRAWER_RECORD_WAIT`。
7. 通过最终抓帧输出稳定的默认态，并继续等待 `DRAWER_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Quick / Read` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区 `3` 组快照和最终稳定帧的布局口径一致。
- `apply_primary_state()` 与 `apply_preview_states()` 只在 `ui_ready` 后触发布局，避免挂载前后的布局口径分叉。
- README 这里按当前 `test.c` 如实保留首轮切换使用 `DRAWER_RECORD_FINAL_WAIT`、第二轮切换与默认回落使用 `DRAWER_RECORD_WAIT`、抓帧使用 `DRAWER_RECORD_FRAME_WAIT`、最终抓帧使用 `DRAWER_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/drawer --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/drawer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_drawer
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Filters`、`Review`、`Archive` 三组可识别状态，最终稳定帧必须回到默认 `Filters`。
- `closed` 状态下只能保留 toggle，不允许抽屉主体残影、错位或旧布局残留。
- 主区真实交互仍需保留 same-target release、`read only / !enable` guard 和键盘开合语义。
- 底部 `Quick / Read` preview 必须在全部 runtime 帧里保持静态一致，并持续吞掉 `touch / key` 且不改 `anchor / presentation_mode / open / compact_mode / read_only_mode`。
- static preview、setter 和 guard 路径都必须先清理残留 `pressed_part / is_pressed`，不能留下旧高亮污点。
- WASM demo 必须能以 `HelloCustomWidgets_layout_drawer` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_drawer/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 主区唯一状态分组：`[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界：`(50, 111) - (431, 269)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 269` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000`、`frame_0001` 与最终稳定帧同组，确认最终稳定帧显式回到默认 `Filters`

## 12. 与现有控件的边界
- 相比 `split_view`：这里强调临时开合抽屉，而不是持久双栏布局。
- 相比 `dialog_sheet`：这里是边缘侧抽屉，不是动作确认容器。
- 相比 `flyout`：这里承载的是 side rail 面板，而不是锚点气泡。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Filters`
  - `Review`
  - `Archive`
- 保留的底部对照：
  - `Quick`
  - `Read`
- 保留的结构状态：
  - `inline / overlay`
  - `start / end anchor`
  - `open / closed`
- 保留的交互：
  - same-target release
  - 键盘 `Enter / Space / Escape`
- 删减的旧桥接与旧轨道：
  - 路由壳层和复杂表单插槽
  - 业务化审批流
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾
  - 只验证“消耗输入”但不复核状态保持的旧 preview 口径
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `drawer` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/drawer --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_drawer/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/drawer`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_drawer`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1647 colors=245`
- 截图复核结论：
  - 主区覆盖 `Filters / Review / Archive` 三组 reference 状态
  - 最终稳定帧显式回到默认 `Filters`
  - 主区 RGB 差分边界收敛到 `(50, 111) - (431, 269)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Quick / Read` preview 以 `y >= 269` 裁切后全程保持单哈希静态
