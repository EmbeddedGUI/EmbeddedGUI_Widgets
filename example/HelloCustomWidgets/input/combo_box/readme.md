# combo_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WinUI 3 ComboBox`
- 开源母本：`WPF UI`
- 对应组件：`ComboBox`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留真实 `touch` 展开与 `Down / End / Space / Enter` 提交闭环；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 快照切换、旧录制轨道里的额外收尾态

## 1. 为什么需要这个控件
`combo_box` 用来表达在固定候选项里选择一个当前值的标准表单语义，适合工作区切换、模式选择、时间范围、布局密度和默认模板等离散单选场景。它比命令按钮更像输入控件，也比建议输入更强调“当前已选值”。

## 2. 为什么现有控件不够用
- `auto_suggest_box` 更强调建议列表与搜索/匹配语义，不适合作为纯粹的当前值选择器。
- `drop_down_button` 与 `split_button` 更接近命令入口，不承担表单字段语义。
- 仓库里当前 `input/combo_box` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Combo Box`
- 主区：一个保留真实展开与提交闭环的主 `combo_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Auto`
- 右侧 preview：`read only`，固定显示 `Tablet`

目录：
- `example/HelloCustomWidgets/input/combo_box/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：

1. 默认态
   `Work` / collapsed
2. 第一次 `Down`
   `Work` / expanded
3. 第二次 `Down`
   `Travel` / expanded
4. 第二组主快照提交后
   `Compact` / collapsed

底部 preview 在整条轨道中始终固定：

1. `compact`
   `Auto`
2. `read only`
   `Tablet`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主控件：`196 x 34`
- 底部对照行：`216 x 28`
- 单个 preview：`104 x 28`
- 页面结构：标题 -> 主 `combo_box` -> 底部 `compact / read only`
- 风格约束：保持浅色 page panel、白色字段面和轻边框；展开列表只保留单层白底与低噪音强调，不叠加厚阴影或说明卡片；focus 只保留轻量边框强调；`compact` 只压缩尺寸和可见项数量；`read only` 保留当前值展示但不承担真实输入职责

## 6. 状态矩阵
| 状态 | 主 `combo_box` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Work` / collapsed | `Auto` / collapsed | `Tablet` / collapsed |
| 快照 2 | `Work` / expanded | 保持不变 | 保持不变 |
| 快照 3 | `Travel` / expanded | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Compact` / collapsed | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件交互，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Work` 折叠态，同时恢复底部 `compact / read only` 固定状态
2. 抓取首帧
3. 发送 `Down`，展开主控件
4. 抓取第二组主区快照
5. 再发送一次 `Down`，把当前项导航到 `Travel`
6. 抓取第三组主区快照
7. 切换到第二组主快照 `Balanced`，再发送 `Enter -> End -> Space`，提交 `Compact`
8. 抓取第四组主区快照
9. 保持 `Compact` 折叠态不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `hcw_combo_box_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立即清理残留 `pressed / expanded`
- preview 不改 `items / current_index / current_text / region_screen / palette / metrics`

当前 `test.c` 已保持统一 finalize 模板：保留 `COMBO_BOX_RECORD_FINAL_WAIT`、`COMBO_BOX_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_snapshot()`、`apply_preview_states()`、`focus_primary_box()`、`layout_page()` 与 `request_page_snapshot()`，初始化阶段在 root view 挂载前后重放默认态与 preview，确保主区 `Work / Travel / Compact` 轨道与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_combo_box.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `set_items()`、`set_current_index()`、样式 helper、wrapper setters、`touch` 展开与选择、`Down / Home / End / Space / Enter / Escape` 键盘闭环，以及 disabled / empty guard 的 `pressed / expanded / pressed_index` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `items`、`current_index`、`current_text`、`font`、`icon_font`、`expand_icon`、`collapse_icon`、`region_screen`、`palette`、`collapsed_height`、`item_height`、`icon_text_gap`、`max_visible_items` 不变，并要求 `g_selected_count == 0`、`g_last_selected == 0xFF`，且 `is_pressed`、`pressed_index` 与 `pressed_is_header` 被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/combo_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/combo_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/combo_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_combo_box
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Work` collapsed、`Work` expanded、`Travel` expanded 与 `Compact` collapsed 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、键盘提交与 setter / 样式 helper 的状态清理链路收口后不能残留 `pressed / expanded / pressed_index` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_combo_box/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(22, 124) - (457, 283)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 284` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `auto_suggest_box`：这里强调当前值与固定候选，不强调搜索建议。
- 相比 `drop_down_button` / `split_button`：这里是表单字段，不是命令入口。
- 相比 SDK `combobox` 示例：这里强调 Fluent reference 页面和静态 preview 收口，而不是基础 API 演示。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Work` / collapsed
  - `Work` / expanded
  - `Travel` / expanded
  - `Compact` / collapsed
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 快照切换
  - 旧录制轨道中的额外收尾态
  - placeholder、leading icon、搜索联想与非必要页面 chrome

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/combo_box PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `combo_box` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/combo_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_combo_box/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/combo_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_combo_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1571 colors=104`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 差分边界为 `(22, 124) - (457, 283)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 284` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Work` 折叠、展开 `Work`、导航到 `Travel` 与提交 `Compact` 四组 reference 状态，最终稳定帧保持 `Compact` 折叠态，底部 `compact / read only` preview 全程静态
