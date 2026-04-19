# tab_view 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件：`TabView`
- 当前保留形态：`Docs workspace / Home`、`Docs workspace / Publish`、`Docs workspace / close current`、`Docs workspace / restore`、`Ops workspace`、`compact`、`read only`
- 当前保留交互：主区保留页签切换、关闭当前页签、恢复隐藏页签、`same-target release` 与 `Left / Right / Home / End / Tab / Enter / Space / Escape` 键盘闭环；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、状态栏、section label、额外 workspace/helper 文案、preview 点击桥接、第二条 `compact` 预览轨道、重阴影、强按钮化 tab shell，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用当前目录下的 `tab_view` 包装层与本地 `snapshot + closed_mask` 工作区语义，在不修改 `sdk/EmbeddedGUI` 的前提下，只收口 `reference` 页面结构、录制轨道、静态 preview、README 和单测口径

## 1. 为什么需要这个控件
`tab_view` 用来表达“页签头和内容面板是一体化工作区”的标准多标签容器语义，适合文档页、后台工作台、设置页和运维面板这类需要在同一个内容壳内切换多个任务页签的场景。

## 2. 为什么现有控件不够用
- `tab_strip` 只表达页签头，不承载 body 内容面板。
- `flip_view` 是顺序翻页，不是并列工作区页签。
- `nav_panel` 负责导航结构，不负责标签式内容容器。
- `menu_bar` 和 `command_bar` 负责命令组织，不承担工作区内容壳。

因此这里继续保留 `tab_view`，但示例页只保留统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 当前页面结构
- 标题：`Tab View`
- 主区：一个标准 `tab_view`，默认显示 `Docs workspace / Home`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Compact docs`
- 右侧 preview：`read only`，固定显示 `Read only`

目录：
- `example/HelloCustomWidgets/navigation/tab_view/`

## 4. 主区 reference 快照
主区录制轨道保留 `5` 组 reference 快照，最终稳定帧回到默认 `Docs workspace / Home`；底部 preview 在整条轨道中始终保持不变：

1. 默认态
   `Docs workspace / Home`
2. 快照 2
   `Docs workspace / Publish`
3. 快照 3
   `Docs workspace / close current`
4. 快照 4
   `Docs workspace / restore`
5. 快照 5
   `Ops workspace`
6. 最终稳定帧
   `Docs workspace / Home`

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Compact docs`
2. `read only`
   `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主控件：`198 x 112`
- 底部 preview 行：`216 x 72`
- 单个 preview：`104 x 72`
- 页面结构：标题 -> 主 `tab_view` -> 底部 `compact / read only`
- 风格约束：浅底 `page panel`、轻边框 tab shell、低噪音 body card 和 footer badge；active tab 只保留轻量 fill、细 underline 与 close 入口，不回退到厚重按钮化风格

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Docs workspace / Home` | `Compact docs` | `Read only` |
| 快照 2 | `Docs workspace / Publish` | 保持不变 | 保持不变 |
| 快照 3 | `Docs workspace / close current` | 保持不变 | 保持不变 |
| 快照 4 | `Docs workspace / restore` | 保持不变 | 保持不变 |
| 快照 5 | `Ops workspace` | 保持不变 | 保持不变 |
| 最终稳定帧 | 回到默认 `Docs workspace / Home` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_tab_view.c` 当前覆盖 `16` 条用例：

1. `set_snapshots()` clamp 并清理 `pressed`。
   覆盖快照数量、当前快照与关闭状态在换轨时的重置。
2. snapshot / tab / part guard 清理 `pressed`。
   覆盖非法 `current_snapshot / current_tab / current_part` 输入时的保护路径。
3. setter 清理 `pressed`。
   覆盖 `set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 等入口。
4. 触摸切换页签。
   覆盖点击 `tab` 后 `current_tab` 与 `on_changed` 回调更新。
5. `ACTION_CANCEL` 清理 `pressed` 且不产生副作用。
   覆盖触摸取消后的 `pressed_tab / pressed_part / is_pressed` 复位。
6. 关闭和恢复页签。
   覆盖 `close current` 与 `restore tabs` 的可见 tab 数量变化。
7. snapshot 切换重置 `closed_mask`。
   覆盖主区在 `Docs workspace` 与 `Ops workspace` 间切换时关闭状态清零。
8. 关闭到只剩最后一个可见页签后恢复首项。
   覆盖极限关闭路径与恢复后的首个可见 tab 选择。
9. 键盘导航与动作提交。
   覆盖 `Left / Right / Home / End / Tab / Enter / Space` 的页签与 action 闭环。
10. `Escape` 回到 `TAB` part。
    覆盖从 `CLOSE / ADD` part 返回 tab 焦点语义。
11. `read_only` 忽略输入。
    覆盖只读模式下的触摸与键盘抑制。
12. `read_only` 与 view disabled guard 清理 `pressed`。
    覆盖收到新输入时先清 `pressed` 再拒绝提交。
13. `compact_mode` 隐藏 close part region。
    覆盖紧凑态下 close 入口不可点击。
14. `set_current_tab()` 忽略已关闭目标。
    覆盖 closed target 不允许重新命中。
15. static preview 吞输入且保持状态不变。
    固定校验 `current_snapshot`、`current_tab`、`current_part`、`closed_mask`、`compact_mode` 不变，并要求 `changed_count == 0`、`action_count == 0`。
16. view disabled 忽略输入且清 `pressed`。
    覆盖整控件禁用后的触摸保护与恢复路径。

说明：
- 主控件继续遵循 non-dragging 控件的 `same-target release`：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到原命中目标释放才提交。
- `close current`、`restore tabs` 与 snapshot 切换都要求把 `pressed_tab / pressed_part / is_pressed` 收口到统一清理路径。
- 底部 `compact / read only` preview 统一通过 `egui_view_tab_view_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的 `reference` 轨道顺序如下：

1. 恢复主区默认 `Docs workspace / Home` 与底部双静态 preview，请求首帧截图，等待 `TAB_VIEW_RECORD_FRAME_WAIT`。
2. 程序化切到 `Publish` 页签，等待 `TAB_VIEW_RECORD_WAIT`。
3. 请求第二组主区快照，等待 `TAB_VIEW_RECORD_FRAME_WAIT`。
4. 执行 `close current`，等待 `TAB_VIEW_RECORD_WAIT`。
5. 请求第三组主区快照，等待 `TAB_VIEW_RECORD_FRAME_WAIT`。
6. 执行 `restore tabs`，等待 `TAB_VIEW_RECORD_WAIT`。
7. 请求第四组主区快照，等待 `TAB_VIEW_RECORD_FRAME_WAIT`。
8. 程序化切到 `Ops workspace`，等待 `TAB_VIEW_RECORD_WAIT`。
9. 请求第五组主区快照，等待 `TAB_VIEW_RECORD_FRAME_WAIT`。
10. 程序化回到默认 `Docs workspace / Home` 并同步恢复底部 preview，等待 `TAB_VIEW_RECORD_WAIT`。
11. 请求最终稳定帧，并继续等待 `TAB_VIEW_RECORD_FINAL_WAIT`。

说明：
- 录制阶段只导出主区状态变化，底部 `compact / read only` preview 全程保持单一静态对照。
- 录制阶段不再点击 preview，也不再保留旧版 `compact` 第二轨道切换。
- `apply_primary_default_state()`、`apply_preview_states()` 与 `request_page_snapshot()` 共同负责统一页面恢复路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tab_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tab_view
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- header、body、footer 与底部 `compact / read only` preview 必须完整可辨。
- 主区 `Home -> Publish -> close current -> restore -> Ops workspace -> Home` 轨道必须在截图中稳定复核。
- `same-target release`、`close / restore`、`read_only / disabled` guard 和 static preview 语义必须全部与单测一致。
- 底部 `compact / read only` preview 在所有 runtime 帧里都必须保持静态一致。
- WASM demo 必须能以 `HelloCustomWidgets_navigation_tab_view` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_tab_view/default`
- 本轮复核结果：
  - 共捕获 `13` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1,6,7,10,11,12] / [2,3] / [4,5] / [8,9]`
  - 主区 RGB 差分边界收敛到 `(58, 106) - (342, 254)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 255` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `tab_strip`：这里把 body 内容视为控件语义的一部分。
- 相比 `flip_view`：这里是并列页签，不是顺序翻页。
- 相比 `nav_panel`：这里表达工作区页签，不是导航结构。
- 相比 `menu_bar`：这里是内容容器，不是命令入口。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Docs workspace / Home`
  - `Docs workspace / Publish`
  - `Docs workspace / close current`
  - `Docs workspace / restore`
  - `Ops workspace`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸 `tab / close / add`
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space / Escape`
  - `snapshot + closed_mask` 工作区收口
  - static preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide、状态栏、section label 和额外 workspace/helper 文案
  - preview 点击桥接与第二条 `compact` 预览轨道
  - 重阴影、厚描边和过度按钮化 tab shell
  - 旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/tab_view PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，其中 `tab_view` suite `16 / 16`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tab_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_tab_view/default`
  - 共捕获 `13` 帧
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tab_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tab_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=152`
- 截图复核结论：
  - 主区覆盖默认 `Docs workspace / Home`、`Docs workspace / Publish`、`Docs workspace / close current` 与 `Ops workspace` 四组唯一 reference 状态，其中 `restore` 与最终稳定帧回到默认 `Home` 同组
  - 主区 RGB 差分边界收敛到 `(58, 106) - (342, 254)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 255` 裁切后全程保持单哈希静态
