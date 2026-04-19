# Drawer 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React / Drawer`
- 对应组件：`Drawer`
- 当前保留形态：`Filters`、`Review`、`Archive`、`compact`、`read only`
- 当前保留交互：主区保留 `inline / overlay`、`start / end anchor`、`open / closed`、same-target release 与 `Enter / Space / Escape` 键盘闭环；底部 preview 保留静态 reference 对照
- 当前移除内容：路由壳层、复杂表单插槽、业务化审批流、旧录制末尾“恢复后立即抓帧”的模板化收尾、只验证“消耗输入”但不复核状态保持的旧 preview 口径，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `layout/drawer`，底层仍复用仓库内现有 `drawer` 实现；本轮只收口 reference 页面结构、录制轨道、README 口径和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`drawer` 用来承载与当前页面强相关、但又不适合直接塞进主内容区的附加信息或次级操作。它既可以作为 inline side rail 与页面并排存在，也可以在 overlay 模式下覆盖当前内容，适合过滤器、审查备注、属性面板和上下文补充。

仓库里已有 `split_view`、`dialog_sheet` 和 `flyout`，但仍缺一个能稳定承接 `Drawer` 临时开合、边缘锚点和 overlay / inline 语义、带独立 reference 页面、README、单测与 web 链路的控件。

## 2. 为什么现有控件不够用
- `split_view` 更偏持久双栏布局，不强调抽屉的临时开合语义。
- `dialog_sheet` 更像确认或提交流程的模态容器，交互重心在 action button，而不是边缘侧抽屉。
- `flyout` 关注的是锚点气泡，不承载大尺寸 side rail 的空间占用和 overlay 语义。

## 3. 当前页面结构
- 标题：`Drawer`
- 主区：1 个保留真实抽屉开合语义的 `drawer`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Quick`
- 右侧 preview：`read only`，固定显示 `Read`

目录：
- `example/HelloCustomWidgets/layout/drawer/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，最终稳定帧显式回到默认态；底部 preview 在整条轨道中始终固定，不再参与轮换：

1. 默认态
   `Filters`
2. 快照 2
   `Review`
3. 快照 3
   `Archive`
4. 最终稳定帧
   回到默认 `Filters`

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Quick`
2. `read only`
   `Read`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 216`
- 主控件：`196 x 112`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `drawer` -> 底部 `compact / read only`
- 风格约束：浅灰页面、白色抽屉面和低噪音边框；通过 host body 是否缩窄、是否覆盖 veil，以及 toggle 所在边缘突出 `inline / overlay / anchor` 差异；底部两个 preview 固定为静态 reference 对照，不再承担点击桥接、焦点收尾或额外轨道切换职责

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Filters` | `Quick` | `Read` |
| 快照 2 | `Review` | 保持不变 | 保持不变 |
| 快照 3 | `Archive` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Filters` | 保持不变 | 保持不变 |
| `inline / overlay`、`start / end anchor`、`open / closed` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Review`
4. 抓取第二组主区快照
5. 切到 `Archive`
6. 抓取第三组主区快照
7. 恢复默认主区和底部 preview 固定状态
8. 等待稳定后抓取最终帧

说明：
- 主区仍保留真实 `toggle / close` 开合、same-target release 和 `Enter / Space / Escape` 键盘语义，供运行时手动复核。
- runtime 录制阶段不再真实发送主区点击或底部 preview 输入。
- 底部 preview 统一通过 `egui_view_drawer_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一走 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，主区首轮切换与最终稳定抓帧使用 `DRAWER_RECORD_FINAL_WAIT`，中间状态切换保留 `DRAWER_RECORD_WAIT / DRAWER_RECORD_FRAME_WAIT`。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_drawer.c` 当前覆盖 `5` 条用例，分为两部分：

1. 主控件交互与状态守卫
   覆盖默认态与 setter 清理 `pressed`、`toggle / close` same-target release、`Enter / Space / Escape` 键盘闭环，以及 `read only / !enable` guard。
2. 静态 preview 输入抑制
   通过独立 preview `drawer` 固定校验 `touch / dispatch_key_event()` 输入被吞掉后，`anchor / presentation_mode / open / compact_mode / read_only_mode / toggle region / close region` 保持不变，且 `on_open_changed` 不触发、`pressed_part / is_pressed` 会被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
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
- 主区录制只允许出现 `Filters`、`Review`、`Archive` 3 组可识别状态，最终稳定帧必须回到默认态。
- `closed` 状态下只能保留 toggle，不允许抽屉主体残影、错位或旧布局残留。
- 主区真实交互仍需保留 same-target release、`read only / !enable` guard 和键盘开合语义。
- 底部 `compact / read only` preview 必须在全部 runtime 帧里保持静态一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_drawer/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(50, 111) - (431, 269)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 以 `y >= 269` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，确认没有 warmup 首帧差异

## 12. 与现有控件的边界
- 相比 `split_view`：这里强调临时开合抽屉，而不是持久双栏布局。
- 相比 `dialog_sheet`：这里是边缘侧抽屉，不是动作确认容器。
- 相比 `flyout`：这里承载的是 side rail 面板，而不是锚点气泡。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Filters`
  - `Review`
  - `Archive`
  - `compact`
  - `read only`
  - `inline / overlay`
  - `start / end anchor`
  - `open / closed`
- 本次保留交互：
  - same-target release
  - 键盘 `Enter / Space / Escape`
- 删减的装饰或桥接：
  - 路由壳层和复杂表单插槽
  - 业务化审批流
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾
  - 只验证“消耗输入”但不复核状态保持的旧 preview 口径
  - 旧版 finalize README 章节结构

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `drawer` suite `5 / 5`
  - 无关 warning：`test_split_view.c:186:13: warning: 'get_view_center' defined but not used`
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
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/drawer`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_drawer`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1647 colors=245`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(50, 111) - (431, 269)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 269` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - `frame_0000` 与 `frame_0001` 哈希一致，没有 warmup 首帧差异
  - 结论：主区覆盖默认 `Filters`、`Review` 与 `Archive` 3 组 reference 快照，最终稳定帧显式回到默认 `Filters`，底部 `compact / read only` preview 全程静态
