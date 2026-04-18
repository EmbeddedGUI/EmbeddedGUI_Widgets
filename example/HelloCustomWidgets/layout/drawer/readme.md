# drawer 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`Fluent UI React / Drawer`
- 对应组件语义：`Drawer`
- 本次保留语义：`Filters / Review / Archive / compact / read only / inline / overlay / start / end anchor / open / closed`
- 本次删除内容：路由壳层、复杂表单插槽、业务化审批流、旧录制末尾“恢复后立即抓帧”的模板化收尾，以及只验证“消耗输入”但不复核状态保持的旧 preview 口径
- EGUI 适配说明：目录和 demo 继续使用 `layout/drawer`，底层仍复用仓库内现有 `drawer` 实现；本轮只收口 `reference` 页面结构、录制轨道、README 口径和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`drawer` 用来承载与当前页面强相关、但又不适合直接塞进主内容区的附加信息或次级操作。它既可以作为 inline side rail 与页面并排存在，也可以在 overlay 模式下覆盖当前内容，适合过滤器、审查备注、属性面板和上下文补充。

## 2. 为什么现有控件不够用
- `split_view` 更偏持久双栏布局，不强调抽屉的临时开合语义。
- `dialog_sheet` 更像确认或提交流程的模态容器，交互重心在 action button，而不是边缘侧抽屉。
- `flyout` 关注的是锚点气泡，不承载大尺寸 side rail 的空间占用和 overlay 语义。

## 3. 目标场景与页面结构
- 页面结构统一为：标题 -> 主 `drawer` -> 底部 `compact / read only` 双静态 preview。
- 主控件负责导出三组主区状态：
  - `Filters`
  - `Review`
  - `Archive`
- 底部左侧是 `compact` 静态 preview，固定展示紧凑 overlay 抽屉。
- 底部右侧是 `read only` 静态 preview，固定展示冻结的 inline 抽屉。
- 两个 preview 统一通过 `egui_view_drawer_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不修改 `anchor / presentation_mode / open / compact_mode / read_only_mode`
  - 不触发 `on_open_changed`

目标目录：`example/HelloCustomWidgets/layout/drawer/`

## 4. 视觉与布局规格
- 根布局：`224 x 216`
- 主控件：`196 x 112`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 页面结构：标题 -> 主 `drawer` -> `compact / read only`
- 风格约束：
  - 保持浅灰页面、白色抽屉面和低噪音边框。
  - 通过 host body 是否缩窄、是否覆盖 veil，以及 toggle 所在边缘来突出 `inline / overlay / anchor` 差异。
  - 底部两个 preview 固定为静态 reference 对照，不再承担点击桥接、焦点收尾或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_drawer` | `egui_view_drawer_t` | `196 x 112` | `Filters` | 主 `Drawer` |
| `compact_drawer` | `egui_view_drawer_t` | `104 x 72` | `Quick` | 紧凑静态 preview |
| `read_only_drawer` | `egui_view_drawer_t` | `104 x 72` | `Read` | 只读静态 preview |
| `primary_snapshots` | `drawer_demo_snapshot_t[3]` | - | `Filters / Review / Archive` | 主状态轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Filters` | `inline + start + open`，默认状态 |
| 主控件 | `Review` | `overlay + end + open`，验证 veil 与 end anchor |
| 主控件 | `Archive` | `inline + end + closed`，验证收口后只保留 toggle |
| `compact` preview | `Quick` | 固定静态对照，验证紧凑 overlay 抽屉 |
| `read only` preview | `Read` | 固定静态对照，验证只读 inline 抽屉 |

## 7. 交互语义与单测要求
- 主控件通过 edge `toggle` 负责开关 drawer。
- 打开状态下保留 header `close` 按钮，并继续遵循 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- 键盘语义：
  - `Enter / Space`：切换开合
  - `Escape`：关闭抽屉
- `set_title()`、`set_anchor()`、`set_presentation_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`set_open()`、`read only`、`!enable` 都必须先清理残留 `pressed`。
- static preview 用例必须验证：
  - `touch / dispatch_key_event()` 输入会被消耗
  - `anchor / presentation_mode / open / compact_mode / read_only_mode` 保持不变
  - `toggle / close` 的部件区域保持不变
  - `on_open_changed` 不触发
  - `pressed_part / is_pressed` 被清理
- 预览态键盘入口统一走 `dispatch_key_event()`，不再退回旧的直接 key handler 路径。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，直接输出默认 `Filters`
2. 切到 `Review`
3. 输出第二帧
4. 切到 `Archive`
5. 输出第三帧
6. 恢复默认主状态
7. 输出最终稳定帧

录制只导出主控件状态变化。底部 `compact / read only` preview 在整条 reference 轨道里保持静态一致，不再包含旧模板里的“恢复后立即抓帧”收尾。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 布局重放路径，主区首轮切换与最终稳定抓帧使用 `DRAWER_RECORD_FINAL_WAIT`，中间状态切换仍保留 `DRAWER_RECORD_WAIT / DRAWER_RECORD_FRAME_WAIT`。

## 9. 编译、单测、运行时与文档检查
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
- `inline / overlay`、`start / end anchor`、`open / closed` 三组主状态必须能从主区直接看出差异。
- `closed` 状态下只能保留 toggle，不允许抽屉主体残影、错位或旧布局残留。
- `same-target release / read_only / !enable / static preview` 必须全部通过单测。
- 两个 preview 必须完整可见，不黑白屏、不抖动，并且在所有 runtime 帧里保持静态一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_drawer/default`
- 复核目标：
  - 主区存在 `3` 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 差分变化边界只出现在主区，不扩散到 preview 区

## 12. 与现有控件的边界
- 相比 `split_view`：这里强调临时开合抽屉，而不是持久双栏布局。
- 相比 `dialog_sheet`：这里是边缘侧抽屉，不是动作确认容器。
- 相比 `flyout`：这里承载的是 side rail 面板，而不是锚点气泡。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `filters`
  - `review`
  - `archive`
  - `compact`
  - `read only`
  - `inline / overlay`
  - `start / end anchor`
  - `open / closed`
- 保留的交互：
  - same-target release
  - 键盘 `Enter / Space / Escape`
- 删减的旧桥接与旧口径：
  - 路由壳层和复杂表单插槽
  - 业务化审批流
  - 旧录制末尾“恢复后立即抓帧”的模板化收尾
  - 只验证“消耗输入”但不复核状态保持的旧 preview 口径

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `drawer` suite `5 / 5`
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
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_drawer/default`
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
  - 主区变化边界保持在 `(50, 111) - (431, 269)`
  - 按 `y >= 269` 裁切底部 preview 区域后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Filters`、`Review` 与 `Archive` 三组 reference 状态，最终稳定帧已显式回到默认快照
