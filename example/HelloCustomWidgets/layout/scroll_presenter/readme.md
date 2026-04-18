# scroll_presenter 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WinUI 3`
- 对应组件名：`ScrollPresenter`
- 本次保留语义：`surface / viewport / offset`、`both-axis pan`、`compact`、`read only`
- 本次删减内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_scroll_presenter`，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`scroll_presenter` 用来表达“内容 surface 自身可以平移浏览，但默认不暴露完整滚动条 chrome”的标准语义。它适合大画布摘要、缩略时间线、自由排布面板、图像检查区和轻量地图这类既要保留 `viewport / extent / offset`，又不想让滚动条抢占视觉层级的场景。

## 2. 为什么现有控件不够用
- `scroll_viewer` 会显式呈现 `scrollbar / thumb`，更适合带完整 chrome 的通用滚动容器。
- `scroll_bar` 只覆盖滚动条本体，不负责内容裁切、双轴 `offset` 和 surface 拖拽。
- `data_list_panel` 偏向列表数据容器，不适合自由内容 surface 的平移语义。
- 当前仓库需要一个能完整走通 `reference`、单测、runtime 和 web 发布链路的 `ScrollPresenter` 页面。

## 3. 目标场景与示例概览
- 主控件保留真实 `ScrollPresenter` 语义，展示 `Canvas overview`、`Timeline branch`、`Far corner` 三组 surface 状态。
- 底部左侧是 `compact` 静态 preview，只用于对照窄尺寸 viewport 密度。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结 surface 的弱化表现。
- 页面只保留标题、一个主 `scroll_presenter` 和底部两个静态 preview，不再承担 preview 清焦点或收尾交互。
- 两个 preview 统一通过 `egui_view_scroll_presenter_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed / drag`
  - 不改写 `current_snapshot / vertical_offset / horizontal_offset`
  - 不触发 `on_view_changed`

目标目录：`example/HelloCustomWidgets/layout/scroll_presenter/`

## 4. 视觉与布局规格
- 根布局：`224 x 284`
- 主控件：`196 x 160`
- 底部对照行：`216 x 88`
- `compact` preview：`104 x 88`
- `read only` preview：`104 x 88`
- 页面结构：标题 -> 主 `scroll_presenter` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和清晰的 viewport 裁切。
  - 主控件继续显示 snapshot 标题、helper、offset 与 extent 指标。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `scroll_presenter_primary` | `egui_view_scroll_presenter_t` | `196 x 160` | `Canvas overview` | 主 `ScrollPresenter` |
| `scroll_presenter_compact` | `egui_view_scroll_presenter_t` | `104 x 88` | `Compact pan` | 紧凑静态对照 |
| `scroll_presenter_read_only` | `egui_view_scroll_presenter_t` | `104 x 88` | `Read only pan` | 只读静态对照 |
| `primary_snapshots` | `egui_view_scroll_presenter_snapshot_t[3]` | - | `Canvas / Timeline / Corner` | 主控件录制轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Canvas overview` | 默认状态，显示原点附近画布与基础 offset |
| 主控件 | `Timeline branch` | 展示第二组内容与双轴偏移 |
| 主控件 | `Far corner` | 直接切到右下角内容簇 |
| `compact` | `Compact pan` | 固定静态对照，验证窄尺寸 viewport |
| `read only` | `Read only pan` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件保留真实 `ScrollPresenter` 键盘与触摸语义：
  - `Up / Down`：按行调整垂直 `offset`
  - `Left / Right`：按列调整水平 `offset`
  - `Home / End`：跳到左上原点 / 右下末端
  - `+ / -`：按页滚动垂直 `offset`
  - `Enter / Space`：切到下一组预设 snapshot
- 触摸交互只保留 surface drag，不引入 preview 反向影响主控件焦点的桥接逻辑。
- `set_snapshots()`、`set_current_snapshot()`、`set_vertical_offset()`、`set_horizontal_offset()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed / drag` 状态。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Canvas overview`。
2. 切到 `Timeline branch`，输出第二组主状态。
3. 切到 `Far corner`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 reference 轨道中保持静态一致，不再承担 preview dismiss 或焦点清理职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/scroll_presenter PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_presenter --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_presenter
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_presenter
```

验收重点：
- 主控件三组 snapshot 必须能直接看出双轴 `offset` 与内容位置变化。
- `surface drag / key navigation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向
- 当前只收口单容器 `ScrollPresenter` reference，不接入缩放、嵌套滚动链、惯性或系统级输入桥接。
- 继续使用 snapshot 数组描述大画布内容、viewport 和 offset，不下沉到 SDK 通用容器层。
- 若后续复用价值稳定，再评估是否与 `scroll_viewer`、`scroll_bar` 抽共享的 offset metrics 或 surface drag helper。

## 11. 与现有控件的边界
- 相比 `scroll_viewer`：这里强调内容 surface 自身的平移，而不是显式 `scrollbar` chrome。
- 相比 `scroll_bar`：这里负责完整 `viewport / surface / offset`，而不只是滚动条本体。
- 相比 `data_list_panel`：这里承载任意内容块，不绑定列表数据结构。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `surface / viewport / offset`
  - `both-axis pan`
  - `compact`
  - `read only`
- 保留的交互：
  - surface 直接拖拽
  - 键盘 `Up / Down / Left / Right / Home / End / + / - / Enter / Space`
- 删减的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview dismiss` 收尾动作

## 13. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/scroll_presenter PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `scroll_presenter` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/scroll_presenter --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_scroll_presenter/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/scroll_presenter`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_scroll_presenter`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.2165 colors=396`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区变化边界保持在 `(44, 54) - (435, 293)`
  - 按 `y >= 295` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖默认 `Canvas overview`、`Timeline branch` 与 `Far corner` 三组 reference 状态，最终稳定帧已显式回到默认快照
