# ScrollBar 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI ScrollBar`
- 对应组件名：`ScrollBar`
- 当前保留状态：`standard`、`compact`、`read only`
- 当前移除内容：旧版 preview 焦点桥接、preview 点击驱动录制、第二组 compact preview 切换、额外 guide / section divider / preview 标签
- EGUI 适配说明：保留独立滚动条的 `decrease / track / thumb / increase` 语义、viewport 比例和键盘闭环；页面结构收口为标题、主控件和底部双静态 preview

## 1. 为什么需要这个控件
`scroll_bar` 用来表达长内容窗口在整段内容中的位置和可视比例，适合文档轨道、日志浏览、时间轴和属性面板等场景。它不是抽象数值选择器，而是围绕 `content_length / viewport_length / offset` 表达“当前位置 + 可视窗口比例”。

## 2. 为什么现有控件不够用
- `slider` 更偏数值选择，thumb 大小固定，不能表达 viewport 比例。
- `progress_bar` 只有展示语义，没有按钮、track 分页和 thumb drag。
- `scroll` 是容器行为，不是独立的标准控件。
- `number_box` / `stepper` 偏离散值编辑，不适合连续内容定位。

因此这里继续保留 `scroll_bar`，但页面和验收口径统一收口到当前 `reference` workflow。

## 3. 当前页面结构
- 标题：`Scroll Bar`
- 主区：一个标准 `scroll_bar`
- 底部：两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`

页面目录：
`example/HelloCustomWidgets/input/scroll_bar/`

主区数据集中保留三组 reference 数据：
- `Document rail`
- `Timeline rail`
- `Audit rail`

当前 runtime 录制轨道只使用：
- 默认 `Document rail`
- `Document rail` 上的 `Down / + / End` 键盘语义
- `Timeline rail`
- 最终回到默认 `Document rail`

底部 `compact / read only` preview 全程固定为静态 reference，对录制轨道不再承担状态切换职责。

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 238`
- 主控件：`196 x 146`
- 底部容器：`216 x 52`
- 单个 preview：`104 x 52`
- 页面结构：标题 -> 主 `scroll_bar` -> 底部 `compact / read only`

视觉规则：
- 使用浅灰 `page panel` 和低噪音白色 surface，避免旧版 demo chrome。
- 主控件保留 `label + helper + viewport preview + rail` 四段层级。
- `compact` preview 仅保留静态比例摘要，不再承担额外交互。
- `read only` preview 用只读调色板表达锁定态，不再引入额外场景化装饰。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 238` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Scroll Bar` | 页面标题 |
| `scroll_bar_primary` | `egui_view_scroll_bar_t` | `196 x 146` | `Document rail` | 标准主控件 |
| `scroll_bar_compact` | `egui_view_scroll_bar_t` | `104 x 52` | compact | 静态 compact preview |
| `scroll_bar_read_only` | `egui_view_scroll_bar_t` | `104 x 52` | compact + read only | 静态只读 preview |

## 6. 状态矩阵

| 状态 / 区域 | 主 `scroll_bar` | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 `Document rail` | 是 | 是 | 是 |
| `Timeline rail` | 是 | 否 | 否 |
| `Audit rail` 数据保留 | 是 | 否 | 否 |
| 键盘 `Down / + / End` | 是 | 否 | 否 |
| 触摸 `decrease / track / thumb` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `scroll_bar` 保留真实键盘闭环：`Tab / Up / Down / Home / End / +/- / Enter / Space / Escape`。
- 主 `scroll_bar` 保留真实触摸语义：`decrease` 按钮、`track` 分页和 `thumb` 拖拽。
- setter、模式切换、禁用、`touch cancel` 和失焦后都必须清理残留 pressed 状态。
- `compact / read only` preview 统一通过 `egui_view_scroll_bar_override_static_preview_api()` 吞掉 `touch / key`。
- preview 不再清主控件 focus，不再承担 preview 点击驱动状态切换，不再参与录制语义。
- 静态 preview 的验收口径升级为“输入被消费，但完整状态保持不变”。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道固定为六个主区语义帧：

1. 还原默认 `Document rail`，同步底部双静态 preview，并抓取首帧。
2. 派发 `Down`，抓取 line step 结果。
3. 派发 `+`，抓取 page step 结果。
4. 派发 `End`，抓取跳到末尾后的结果。
5. 程序化切到 `Timeline rail`，抓取第二组 reference。
6. 回到默认 `Document rail`，抓取最终稳定帧。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先重排，再失效，再请求录制。
- 底部 `compact / read only` preview 在整条轨道中保持静态不变。
- 主区变化只来自主控件 reference 数据切换和键盘语义。

## 9. 本轮收口内容
- `example/HelloCustomWidgets/input/scroll_bar/test.c`
  新增 `ui_ready`、`layout_local_views()`、`layout_page()` 和统一的 `request_page_snapshot()`；移除 preview 焦点桥接、preview 点击录制和第二组 compact preview 切换，把页面收口为主区 reference + 底部双静态 preview。
- `example/HelloUnitTest/test/test_scroll_bar.c`
  新增 `scroll_bar_preview_snapshot_t`、`assert_region_equal()`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()`；统一事件分发到 `dispatch_touch_event()` / `dispatch_key_event()`；把静态 preview 用例收口为 `consumes input and keeps state`。
- `example/HelloCustomWidgets/input/scroll_bar/readme.md`
  按当前 workflow 重写，与 demo、单测、runtime 和 web 口径保持一致。

## 10. 单元测试口径
`example/HelloUnitTest/test/test_scroll_bar.c` 当前覆盖：

1. `content_metrics / step_size / offset / current_part` setter 的 clamp 与 pressed 清理。
2. `Tab` 循环、`Up / Down / Home / End / +/-` 键盘闭环。
3. `decrease` 点击、`track` 分页、`thumb` 拖拽。
4. `touch cancel` 清理 pressed。
5. `compact_mode / read_only_mode / !enable` 的输入抑制和恢复。
6. 静态 preview 吞掉输入后保持完整状态不变。

静态 preview 快照当前固定校验：
`region_screen / background / on_changed / font / meta_font / label / helper / api / palette / content_length / viewport_length / offset / line_step / page_step / compact_mode / read_only_mode / current_part / pressed_part / pressed_track_direction / thumb_dragging / alpha / enable / is_focused / is_pressed / padding`

## 11. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/scroll_bar --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/scroll_bar
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_scroll_bar
```

## 12. 当前结果
- `HelloCustomWidgets` 单控件编译：`PASS`
  `make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc`
- `HelloUnitTest`：`PASS`
  在 `X:\` 执行 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `scroll_bar` suite `14 / 14`
- `sync_widget_catalog.py`：`PASS`
  同步后保持 `106` 个 widgets
- `touch release semantics`：`PASS`
  `custom_audited=28 custom_skipped_allowlist=5`
- `docs encoding`：`PASS`
  `134` 个文档文件编码检查通过
- `widget catalog check`：`PASS`
  `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：`PASS`
  `13 frames captured -> runtime_check_output/HelloCustomWidgets_input_scroll_bar/default`
- input 分类 compile/runtime 回归：`PASS`
  compile `33` 个 input widgets 全部通过；runtime `33 / 33`
- wasm 构建：`PASS`
  `web/demos/HelloCustomWidgets_input_scroll_bar`，并刷新 `web/demos/demos.json` 到 `106` demos
- web smoke：`PASS`
  `status=Running canvas=480x480 ratio=0.1815 colors=157`

## 13. Runtime 复核结论
复核目录：
`runtime_check_output/HelloCustomWidgets_input_scroll_bar/default`

- 总帧数：`13`
- 主区 RGB 差分边界：`(54, 98) - (289, 298)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`5`
- 底部 preview 区唯一哈希数：`1`

结论：
- 主区变化严格收敛在 `scroll_bar` 主控件区域；遮罩 `(54, 98) - (289, 298)` 后，主区外页面 chrome 在整条录制轨道中保持静态。
- `13` 帧中主区共出现 `5` 组唯一状态，对应默认 `Document rail`、`Down`、`+`、`End` 和 `Timeline rail`；最终稳定帧回到默认 `Document rail`。
- 按 `y >= 299` 裁切底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 14. 已知限制
- 当前先做纵向 `ScrollBar` reference，不覆盖横向版本。
- 当前不实现 auto-hide、overlay scrollbar 和 hover-only 扩宽。
- 当前不直接驱动真实容器内容滚动，只表达标准滚动条语义。
- 若后续下沉到框架层，再评估横向支持、容器绑定接口和更细的 accessibility 语义。

## 15. 与现有控件的边界
- 相比 `slider`：这里的 thumb 尺寸由 viewport 决定，目标是定位内容窗口，不是选择抽象数值。
- 相比 `scroll`：这里是独立标准控件，不负责 child layout、惯性或内容实际滚动。
- 相比 `progress_bar`：这里有按钮、track page、thumb drag 和 focus part 语义。
- 相比 `number_box` / `stepper`：这里强调连续内容定位，不是离散数值编辑。

## 16. EGUI 适配说明
- `scroll_bar` custom 实现继续保留在 widgets 仓库，不下沉到 SDK。
- 主区通过固定 snapshot 数据保证 `reference` 录制稳定。
- 底部 preview 统一复用 static preview API，只承担静态对照，不承担额外交互职责。
- README、demo、单测、runtime 验收和 web 发布口径已经对齐到当前 `reference` workflow。
