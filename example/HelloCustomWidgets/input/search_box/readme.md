# search_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 次级补充参考：`WPF UI`
- 对应组件：`Searchbox`
- 当前保留形态：`filled`、`cleared`、`compact`、`read only`
- 当前保留交互：主区保留搜索图标、文本输入与清空按钮语义；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、preview 清焦桥接、录制阶段真实键盘编辑、`Enter` 同步 compact preview、清空按钮实录与场景化说明

## 1. 为什么需要这个控件
`search_box` 用于承载页内搜索、资源筛选和轻量检索入口。它和普通 `text_box` 的差异不在于更复杂的输入能力，而在于固定搜索图标、可见的清空 affordance，以及更明确的“搜索入口”语义。

## 2. 为什么现有控件不够用
- `text_box` 只覆盖最小文本输入，不带搜索图标和清空入口。
- `auto_suggest_box` 更偏建议输入，不要求当前这种静态搜索入口。
- `password_box`、`number_box`、`rich_edit_box` 都不是搜索语义。
- 当前主线仍需要一版与 `Fluent 2 / WPF UI Searchbox` 语义对齐的 `Searchbox` reference。

## 3. 当前页面结构
- 标题：`Search Box`
- 主区：一个标准 `search_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Assets`
- 右侧 preview：`read only`，固定显示 `Archive`

目录：
- `example/HelloCustomWidgets/input/search_box/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段发送真实键盘事件、点击清空按钮，也不再依赖 preview 帮主区清焦：

1. `filled`
   placeholder：`Search templates`
   文本：`Roadmap`
2. `filled`
   placeholder：`Search templates`
   文本：`Asset audit`
3. `cleared`
   placeholder：`Search templates`
   文本：空

底部 preview 在整条轨道中始终保持不变：

1. `compact`
   placeholder：`Recent`
   文本：`Assets`
2. `read only`
   placeholder：`Pinned`
   文本：`Archive`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 136`
- 主控件：`196 x 40`
- 底部 preview 行：`216 x 32`
- 单个 preview：`104 x 32`
- 页面结构：标题 -> 主 `search_box` -> 底部 `compact / read only`
- 风格约束：浅灰 page panel、白色主表面、常驻搜索图标、低噪音边框与轻量 clear affordance，不回退到建议列表或命令面板语义

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Roadmap` | `Assets` | `Archive` |
| 快照 2 | `Asset audit` | 保持不变 | 保持不变 |
| 快照 3 | 清空文本，仅显示 placeholder | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `Roadmap` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Asset audit`
4. 抓取第二组主区快照
5. 切到清空态
6. 抓取第三组主区快照
7. 回到默认 `Roadmap`
8. 抓取最终稳定帧

说明：
- 录制阶段不再发送 `Backspace`、字母键、`Enter`
- 录制阶段不再点击主区 clear 按钮
- 底部 preview 统一通过 `egui_view_search_box_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担清焦或页面状态桥接职责
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `SEARCH_BOX_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_search_box.c` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖样式 helper、键盘编辑与提交、clear 按钮 same-target release，以及只读态输入拦截。
2. 静态 preview 不变性断言
   通过 `search_box_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`on_submit`、`font`、`icon_font`、`placeholder`、`text_color`、`text_alpha`、`placeholder_color`、`placeholder_alpha`、`cursor_color`、`icon_color`、`clear_fill_color`、`clear_fill_pressed_color`、`clear_icon_color`、`text`、`text_len`、`cursor_pos`、`max_length`、`cursor_visible`、`align_type`、`scroll_offset_x`、`alpha`、`enable`、`clear_pressed`、`is_focused`、`padding`

同时要求：
- `is_pressed == false`
- `submit_count == 0`
- `submitted_text == ""`

验收命令：
```bash
make all APP=HelloCustomWidgets APP_SUB=input/search_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/search_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/search_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_search_box
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Roadmap`、`Asset audit` 与 `cleared` 三组 reference 快照必须能从截图中稳定区分。
- 搜索图标、clear 按钮和文本输入链路收口后不能残留 `pressed` 污染或旧录制桥接。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_search_box/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(44, 190) - (434, 200)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 201` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `text_box`：这里强调搜索图标、clear 按钮和搜索入口语义。
- 相比 `auto_suggest_box`：这里不做展开候选列表和建议项选择。
- 相比 `command_bar`：这里是轻量搜索输入，不是命令触发入口。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Roadmap`
  - `Asset audit`
  - `cleared`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与搜索结果面板
  - 候选建议列表与异步数据源
  - preview 清焦桥接
  - 录制阶段真实键盘编辑、`Enter` 同步 preview 与 clear 按钮实录

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/search_box PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `search_box` suite `5 / 5`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/search_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_search_box/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/search_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_search_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1038 colors=99`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(44, 190) - (434, 200)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 201` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Roadmap`、`Asset audit` 与 `cleared` 三组 reference 快照，最终稳定帧已显式回到默认 `Roadmap`，底部 `compact / read only` preview 全程静态
