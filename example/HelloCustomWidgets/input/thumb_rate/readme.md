# ThumbRate 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`ThumbRate`
- 本次保留状态：`none`、`liked`、`disliked`
- 本次保留交互：触摸点击、键盘左右切换焦点、`Enter / Space` 提交、再次点击当前项回到 `none`
- 删减内容：hover 动画、visited 态、额外文案区块、系统图标依赖、场景化 showcase 装饰
- EGUI 适配说明：仓库内没有现成的 thumb 图标资源，本控件在 custom 层自绘抽象化 thumb up/down 图形，不改 SDK

## 1. 为什么需要这个控件？
`ThumbRate` 适合表达“有帮助 / 没帮助”这类二选一反馈，同时保留未投票的中立态。它比星级评分更轻量，也比普通按钮更明确地表达“反馈”而非“执行命令”。

## 2. 为什么现有控件不够用？
- `button` / `hyperlink_button` 更偏向动作触发，不带三态反馈语义。
- `toggle_button` 只有单个开关面，无法自然表达 likes / dislikes 两个互斥选项。
- `rating_control` 面向离散等级，不适合“正负反馈”这种双分支场景。

## 3. 目标场景与示例概览
- 主区域保留一个真实可交互的 `ThumbRate`，用于录制 `none -> liked -> disliked -> none` 的闭环。
- 底部左侧保留一个 `compact` 静态对照。
- 底部右侧保留一个 `read only` 静态对照。
- 页面只保留标题、一个信息面板和底部双 preview，不再承担额外场景切换职责。

目录：
- `example/HelloCustomWidgets/input/thumb_rate/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 176`
- 主面板：`196 x 96`
- 主控件：`172 x 52`
- 底部容器：`200 x 36`
- 单个 preview：`96 x 36`

视觉原则：
- 保持 Fluent 风格的浅灰页面、白色表面和轻阴影。
- `liked` 使用更积极的冷色强调，`disliked` 使用警示红色强调。
- `compact` 隐去标签，只保留两段式 thumb 反馈。
- `read only` 保留状态可见性，但降低对比度并吞掉输入。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 176` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 96` | 主展示面板 |
| `heading_label` | `egui_view_label_t` | `172 x 12` | 场景标题 |
| `primary_rate` | `egui_view_thumb_rate_t` | `172 x 52` | 主交互控件 |
| `note_label` | `egui_view_label_t` | `172 x 14` | 当前反馈说明 |
| `compact_preview` | `egui_view_thumb_rate_t` | `96 x 36` | 紧凑静态对照 |
| `read_only_preview` | `egui_view_thumb_rate_t` | `96 x 36` | 只读静态对照 |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| `none` 初始态 | 是 | 否 | 否 |
| `liked` 已选择 | 是 | 是 | 否 |
| `disliked` 已选择 | 是 | 否 | 是 |
| 再次点击当前项回到 `none` | 是 | 否 | 否 |
| `Left / Right / Home / End / Tab` 切换焦点项 | 是 | 否 | 否 |
| `Enter / Space` 提交当前焦点项 | 是 | 否 | 否 |
| `Escape` 清空当前选择 | 是 | 否 | 否 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- 触摸遵循 same-target release：只有 `DOWN(A) -> UP(A)` 才提交。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交；回到原目标再 `UP(A)` 才提交。
- 再次点击当前已选项时，状态从 `liked` 或 `disliked` 回到 `none`。
- 键盘 `Left / Right / Home / End / Tab` 只移动当前焦点项，不直接改值。
- 键盘 `Enter / Space` 提交当前焦点项；若该项已选中，则切回 `none`。
- `Escape` 用于清空当前选择。

## 8. 本轮收口内容
- 新增 `egui_view_thumb_rate.h/.c`，实现三态 thumb 反馈控件。
- 在 custom 层自绘 thumb up/down 图形，避免依赖 SDK 新图标资源。
- 补齐 `standard / compact / read only` 三套样式 helper。
- 补齐 `set_state()`、`set_current_part()`、`set_labels()`、`set_palette()`、静态 preview API 和键盘导航入口。
- demo 页面保留一个主控件与两个静态 preview，录制路径直接覆盖三态闭环。
- 单测覆盖样式 helper、setter 清理 pressed、same-target release、键盘导航、只读/禁用 guard 与静态 preview。

## 9. 录制动作设计
1. 还原默认 `none` 状态。
2. 抓取初始帧。
3. 触摸点击 `Like`。
4. 抓取 `liked` 帧。
5. 切到第二组文案并通过键盘切到 `Dislike` 后提交。
6. 抓取 `disliked` 帧。
7. 再次按 `Space` 清空当前 `Dislike`。
8. 抓取最终 `none` 收尾帧。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`set_state / set_current_part / key dispatch / touch dispatch / snapshot request` 都先走显式布局路径，再进入录制与抓帧。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=input/thumb_rate PORT=pc

# 在 X:\ 短路径下执行，修改 .inc 后建议先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/thumb_rate --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/thumb_rate
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_thumb_rate
```

验收重点：
- 主控件和底部 preview 必须完整可见，不黑屏、不白屏、不裁切。
- `none / liked / disliked` 三态必须能从截图中稳定区分。
- 触摸跨目标移动时不得错误提交。
- `compact` 与 `read only` preview 必须保持静态 reference，不响应真实输入。

## 11. 已知限制
- 当前只覆盖最小 `ThumbRate` 语义，不扩展 tooltip、hover 动画或外链式反馈入口。
- 仓库没有官方 thumb 图标资源，因此使用 custom 自绘图形近似 Fluent 语义。
- `compact` / `read only` preview 仅用于 reference 对照，不承担真实交互职责。

## 12. 当前验收结果（2026-04-17）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/thumb_rate PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make clean APP=HelloUnitTest PORT=pc_test`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `thumb_rate` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/thumb_rate --track reference --timeout 10 --keep-screenshots`
  - `9` 帧输出到 `runtime_check_output/HelloCustomWidgets_input_thumb_rate/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/thumb_rate`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_thumb_rate`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1342 colors=179`
- 截图复核结论：
  - 主区差分边界收敛到 `(44, 139) - (381, 246)`，覆盖 `none / liked / disliked / none` 四组主状态
  - 主区共 `4` 组唯一状态，录制中的三态切换与最终回到 `none` 都已通过显式布局路径稳定落帧
  - `compact` preview `9` 帧单一哈希，说明整条录制轨道中保持静态一致
  - `read only` preview `9` 帧单一哈希，说明只读对照在录制中持续稳定
