# flip_view 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI FlipView`
- 补充对照控件：`pips_pager`、`tab_view`、`coverflow_strip`
- 对应组件名：`FlipView`
- 当前保留形态：主区 `Highlights / Shipping board`、`Release note`、`Archive handoff`、`Operations / Planning board`，以及底部 `compact / read only` 静态 preview
- 当前保留交互：主区 `previous / surface / next` 命中、`same-target release`、`Tab / Left / Right / Home / End / Enter / Space / Plus / Minus / Escape` 键盘入口，以及 setter / guard / static preview 的残留 `pressed` 清理闭环
- 当前移除内容：页面级 `guide`、状态说明条、`section divider`、`preview label`、preview 点击桥接、preview 收尾轨道、故事化 hero 文案、过重阴影与高对比强调按钮
- EGUI 适配说明：继续复用仓库内 `flip_view` 基础实现，本轮只收口 `reference` README 到当前 finalize 模板，并沿用 `2026-04-19` 已归档 acceptance 数据，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件

`flip_view` 用来表达“当前只保留一张主卡片，但允许用户按顺序浏览前后内容”的标准单卡翻页语义，适合精选摘要、运营看板、记录回看、步骤卡片这类一次只希望用户聚焦当前页的场景。

## 2. 为什么现有控件不够用

- `coverflow_strip` 强调中心卡和侧卡透视层级，不是标准 `FlipView` 的单卡浏览语义。
- `pips_pager` 侧重页码位置反馈，本身不承载主内容。
- `tab_view` 和 `tab_strip` 强调多页签切换，不适合“保留单页内容并按顺序前后翻页”的场景。

因此这里继续保留 `flip_view`，但示例页只保留统一的 `Fluent / WPF UI` reference 结构。

## 3. 当前页面结构

- 标题：`Flip View`
- 主区：一个真实可交互的 `flip_view_primary`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Compact / Snapshot`
- 右侧 preview：`read only`，固定显示 `Read only / Snapshot`
- 页面结构统一收口为：标题 -> 主 `flip_view` -> 底部 `compact / read only`

目录：`example/HelloCustomWidgets/navigation/flip_view/`

## 4. 主区 reference 快照

主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Highlights / Shipping board`
   默认主状态，停在 `Highlights` 轨道第 2 项
2. `Highlights / Release note`
   发送 `Right`，验证顺序翻页后的主卡内容和 footer 变化
3. `Highlights / Archive handoff`
   发送 `End`，验证尾页边界与 `next` 禁用态
4. `Operations / Planning board`
   程序化切换主轨道，验证标题、文案和配色同步变化
5. `Highlights / Shipping board`
   回到默认主状态，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Compact / Snapshot`
   静态紧凑对照，固定 `compact_mode`
2. `read only`
   `Read only / Snapshot`
   静态只读对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格

- 画布：`480 x 480`
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 64`
- 单个 preview 尺寸：`104 x 64`
- 页面结构：标题 -> 主 `flip_view` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色主卡、低噪音边框和轻量 overlay 箭头；底部两个 preview 继续复用同一套控件语义，只压缩内容密度和交互入口

## 6. 状态矩阵

| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Shipping board` | 是 | 否 | 否 |
| `Release note` | 是 | 否 | 否 |
| `Archive handoff` | 是 | 否 | 否 |
| `Planning board` | 是 | 否 | 否 |
| `Compact / Snapshot` | 否 | 是 | 否 |
| `Read only / Snapshot` | 否 | 否 | 是 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `previous / next` same-target release | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径

`example/HelloUnitTest/test/test_flip_view.c` 当前覆盖 `11` 条用例：

1. `set_items()` 会钳制 `item_count / current_index`，并保持 `current_part = surface`
2. `set_font / set_meta_font / set_title / set_helper / set_palette / set_items / set_current_index / set_current_part / set_compact_mode / set_read_only_mode` 全部会清理残留 `pressed_part / is_pressed`
3. `Tab` 在 `surface -> next -> previous -> surface` 之间轮转
4. 键盘导航覆盖 `Right / Home / End`
5. `Plus / Minus` 逐项切换时不会在边界重复通知
6. `previous / next / surface` 触摸命中都能得到正确 `on_changed` 结果
7. 主区触摸继续遵守 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到 `A` 后 `UP(A)` 才提交；`ACTION_CANCEL` 只清理 `pressed`
8. `surface` 的 hit region 必须可解析，确保主区布局和命中区域稳定
9. `compact_mode` 会先清理残留 `pressed`，再忽略后续 `touch / key` 输入；恢复后重新允许交互
10. `read_only_mode / !enable` 会先清理残留 `pressed`，再忽略后续 `touch / key` 输入；恢复后重新允许交互
11. 静态 preview 用例验证 “consumes input and keeps state”：固定校验 `current_index / current_part / compact_mode` 不变，且不会触发 `on_changed`

补充说明：

- 主区 `previous / next` 属于非拖拽点击目标，统一遵守 same-target release
- `compact / read_only / !enable` guard 在拒绝新输入前必须先清理残留 `pressed`
- 底部 `compact / read only` preview 统一通过 `egui_view_flip_view_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责

## 8. 录制动作设计

`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Highlights / Shipping board` 和底部两个静态 preview，请求首帧并等待 `FLIP_VIEW_RECORD_FRAME_WAIT = 150`
2. 发送 `Right`，等待 `FLIP_VIEW_RECORD_WAIT = 110`
3. 请求第二帧并继续等待 `150`
4. 发送 `End`，等待 `110`
5. 请求第三帧并继续等待 `150`
6. 程序化切到 `Operations / Planning board`，等待 `110`
7. 请求第四帧并继续等待 `150`
8. 回到默认 `Highlights / Shipping board`，同时重新应用底部两个静态 preview，等待 `110`
9. 请求最终稳定帧，并继续等待 `FLIP_VIEW_RECORD_FINAL_WAIT = 520`

说明：

- 全部截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制
- `apply_primary_track()` 与 `apply_preview_states()` 在 `ui_ready` 后都会显式触发 `layout_page()`
- 底部 `compact / read only` preview 在整条轨道中不承担切换、桥接或收尾职责

## 9. 验收命令

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/flip_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_flip_view
```

## 10. 验收重点

- 主控件与底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切
- 主卡、overlay 箭头和底部两个 preview 必须继续保持低噪音 Fluent / WPF UI reference 风格
- 主区需要真实出现 `Shipping board -> Release note -> Archive handoff -> Planning board -> Shipping board` 的状态变化
- `same-target release`、`ACTION_CANCEL`、`compact / read_only / !enable guard` 与 `static preview` 必须全部通过单测
- 最终稳定帧必须显式回到默认 `Shipping board`，底部 preview 全程保持静态一致

## 11. 截图复核口径

- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_flip_view/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Shipping board`、`Release note`、`Archive handoff` 与 `Planning board`
  - 主区 RGB 差分边界为 `(53, 109) - (427, 274)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 按 `y >= 274` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Shipping board`

## 12. 与现有控件的边界

- 相比 `coverflow_strip`：这里不展示左右透视侧卡，只保留单卡 `surface`
- 相比 `pips_pager`：这里承载当前页内容本体，页码只是次级信息
- 相比 `tab_view`：这里是连续翻页浏览，不是多页签切换
- 相比旧版 showcase 页面：这里不再保留 preview 桥接、guide 和额外叙事型壳层

## 13. 本轮保留与删减

- 保留的主区状态：`Highlights / Shipping board`、`Release note`、`Archive handoff`、`Operations / Planning board`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：`previous / surface / next` 命中、same-target release、`Tab / Left / Right / Home / End / Enter / Space / Plus / Minus / Escape` 键盘入口、setter 清理 `pressed`、guard 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：preview 点击桥接、preview 收尾录制、guide、状态条、`section divider`、`preview label`、故事化 hero 文案、过重阴影和高对比强调按钮

## 14. 当前验收结果（2026-04-19）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc`
  - 本轮沿用 `2026-04-19` 已归档 acceptance 结果
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `flip_view` suite `11 / 11`
- 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_flip_view/default`
  - 本轮沿用已归档 runtime 结果，并重新复核截图边界与哈希
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - 沿用 `2026-04-19` 已归档分类回归结果
- web 链路：`PASS`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_flip_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1708 colors=171`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Shipping board`、`Release note`、`Archive handoff` 与 `Planning board`
  - 主区 RGB 差分边界为 `(53, 109) - (427, 274)`
  - 按 `y >= 274` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Shipping board`，底部 `compact / read only` preview 全程保持静态
