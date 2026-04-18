# color_picker 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ColorPicker`
- 开源母本：`WPF UI`
- 对应组件名：`ColorPicker`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留真实 `tone palette / hue rail` 的 `touch` 命中与 `Tab / Left / Right / Up / Down / Home / End / Escape` 键盘闭环；底部 `compact / read only` preview 统一收口为静态 reference 对照
- 当前移除内容：页面级 guide、状态说明文案、preview 快照轮换、额外收尾态、弹出式高级编辑器、透明度通道、eyedropper

## 1. 为什么需要这个控件?
`color_picker` 用来表达标准颜色选择语义，适合主题色、状态色、卡片强调色、图标前景色等离散但可导出 tone 的场景。它补上了当前 `HelloCustomWidgets` 里“同一 hue 下继续选择明暗与饱和度”的标准输入能力。

## 2. 为什么现有控件不够用
- `swatch_picker` 只覆盖离散色板切换，不支持 `tone palette + hue rail` 的组合语义。
- `slider` / `xy_pad` 虽然能做连续输入，但不直接表达颜色选择，也没有当前色预览和 `hex` 摘要。
- `number_box` / `text_box` 可以承载数值或文本，却不适合作为主颜色选择入口。
- 仓库里当前 `input/color_picker` 的 README 仍停留在旧版 finalize 章节结构，没有完整收口到当前 static preview 模板。

## 3. 当前页面结构
- 标题：`Color Picker`
- 主区：一个保留真实 `tone palette + hue rail` 键盘闭环的 `color_picker`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Mint`
- 右侧 preview：`read only`，固定显示 `Locked`

目录：
- `example/HelloCustomWidgets/input/color_picker/`

## 4. 主区 reference 快照
主区录制轨道保留 `4` 组 reference 状态，底部 preview 在整条轨道中保持不变：
1. 默认态
   `Accent color` / palette focused
2. `Right` 后
   `Accent color` / tone adjusted
3. `Tab + Down` 后
   `Accent color` / hue adjusted
4. 第二组主快照
   `Signal color` / palette focused

底部 preview 在整条轨道中始终固定：
1. `compact`
   `Mint`
2. `read only`
   `Locked`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 204`
- 主控件：`196 x 112`
- 底部 preview 行：`216 x 52`
- 单个 preview：`104 x 52`
- 页面结构：标题 -> 主 `color_picker` -> 底部 `compact / read only`
- 风格约束：使用浅色 page panel、低噪音边框和单层白底主卡；主区只保留 `tone palette + hue rail + swatch + hex` 的最小完整语义；`compact / read only` 只做静态 reference 对照；focus 仅在主控件内部 `palette / hue rail` 间切换，不叠加夸张 glow 或复杂阴影

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Accent color` / palette focused | `Mint` / compact | `Locked` / read only |
| 快照 2 | `Accent color` / tone adjusted | 保持不变 | 保持不变 |
| 快照 3 | `Accent color` / hue adjusted | 保持不变 | 保持不变 |
| 快照 4 / 最终稳定帧 | `Signal color` / palette focused | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 保留真实主控件键盘闭环，但底部 preview 已收口为静态 reference 工作流：

1. 恢复主控件默认 `Accent color` / palette focused，同时恢复底部 `compact / read only` 固定状态
2. 抓取首帧
3. 发送一次 `Right`，把 tone 向高饱和方向推进一格
4. 抓取第二组主区快照
5. 发送 `Tab + Down`，切到 hue rail 并切换下一条色相
6. 抓取第三组主区快照
7. 程序化切换到第二组主快照 `Signal color`
8. 抓取第四组主区快照
9. 保持 `Signal color` 不变并抓取最终稳定帧

说明：
- 录制阶段只有主区状态会变化
- 底部 preview 统一通过 `egui_view_color_picker_override_static_preview_api()` 吞掉 `touch / dispatch_key_event()`
- 静态 preview 收到输入时立刻清理残留 `pressed`
- preview 不改 `label / helper / selection / hex / region_screen / palette`

当前 `test.c` 已保持统一 finalize 模板：保留 `COLOR_PICKER_RECORD_FINAL_WAIT`、`COLOR_PICKER_DEFAULT_SNAPSHOT`、`PRIMARY_SNAPSHOT_COUNT`、`apply_primary_snapshot()`、`apply_preview_states()`、`focus_primary_picker()`、`layout_page()` 与 `request_page_snapshot()`，初始化阶段在 root view 挂载前后统一回放默认态与 preview，确保主区四组快照与最终稳定帧都走同一条布局重放路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_color_picker.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `set_selection()`、`set_current_part()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`touch` 命中选择、`Tab / Left / Right / Up / Down / Home / End / Escape` 键盘闭环，以及 `pressed_part / is_pressed` 清理。
2. 静态 preview 不变性断言
   通过统一的 `dispatch_key_event()` 入口把 preview 用例收口为 “consumes input and keeps state”，固定校验 `font`、`meta_font`、`label`、`helper`、`region_screen`、`alpha`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`selected_color`、`hue`、`saturation`、`value`、`current_part`、`compact_mode`、`read_only_mode`、`hex_text` 不变，并要求 `g_changed_count == 0`、`g_changed_hue / saturation / value == 0xFF`、`g_changed_part == EGUI_VIEW_COLOR_PICKER_PART_NONE`，且 `is_pressed / pressed_part` 被清理。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/color_picker PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/color_picker --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/color_picker
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_color_picker
```

## 10. 验收重点
- 主区和底部 `compact / read only` preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Accent color` palette focused、tone adjusted、hue adjusted 与 `Signal color` 四组 reference 状态必须能从截图中稳定区分。
- 主控件 `touch`、键盘导航与 setter 状态清理链路收口后不能残留 `pressed_part / is_pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_color_picker/default`
- 本轮复核结果：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 变化边界位于 `(52, 123) - (427, 273)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 按 `y >= 274` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `swatch_picker`：这里保留 `tone palette + hue rail`，不是离散色板列表。
- 相比 `xy_pad`：这里强调颜色语义、当前色预览与 `hex` 摘要，不是二维参数输入。
- 相比 `slider` / `number_box`：这里直接表达颜色选择，不是单轴数值调节或文本输入。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Accent color` / palette focused
  - `Accent color` / tone adjusted
  - `Accent color` / hue adjusted
  - `Signal color` / palette focused
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 页面级 guide 与状态说明
  - preview 快照轮换
  - 旧录制轨道中的额外收尾态
  - 弹出式高级编辑器、透明度通道与 eyedropper

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/color_picker PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `color_picker` suite `12 / 12`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/color_picker --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_color_picker/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/color_picker`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_color_picker`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1556 colors=260`
- 截图复核结论：
  - 共捕获 `10` 帧
  - 全帧共出现 `4` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5] / [6,7,8,9]`
  - 主区 RGB 变化边界位于 `(52, 123) - (427, 273)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 按 `y >= 274` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Accent color`、tone 调整、hue 调整与 `Signal color` 四组 reference 状态，最终稳定帧保持 `Signal color`，底部 `compact / read only` preview 全程静态
