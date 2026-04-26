# Accordion 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`Fluent UI React / Accordion`
- 对应组件：`Accordion`
- 当前保留形态：`Workspace`、`Identity`、`Release`、`Compact`、`Read only`
- 当前保留交互：主区保留单项展开、same-target release、`Home / End / Tab / Up / Down / Enter / Space / Escape` 键盘闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- EGUI 适配说明：在 custom 层新增轻量 `egui_view_accordion`，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`accordion` 用来表达多组内容之间的分组折叠关系。它让用户在一组并列 section 中快速扫描标题、描述和摘要，并按需展开其中一项查看正文，适合策略配置、身份校验、发布流程等需要逐项展开的界面。

## 2. 为什么现有控件不够用
- `expander` 更偏单个 disclosure 容器，不表达多 section 之间的互斥展开。
- `card_expander` 强调单张卡片内的摘要和正文，不适合承载多个并列条目。
- `settings_expander` 面向设置行和 nested rows，语义比通用 accordion 更窄。
- `data_list_panel` / `list` 只表达列表浏览，不包含 header 到 body 的折叠闭环。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `accordion` -> 底部 `Compact / Read only` 双 preview。
- 主区包含 `3` 个 section：
  - `Workspace`
  - `Identity`
  - `Release`
- 底部左侧是 `Compact` 静态 preview，展示紧凑布局下的 section header 和单项展开。
- 底部右侧是 `Read only` 静态 preview，展示冻结状态下的弱化视觉。
- 两个 preview 统一通过 `egui_view_accordion_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `expanded_index / focused_index / compact_mode / read_only_mode`
  - 不触发 `on_action`

目标目录：
- `example/HelloCustomWidgets/layout/accordion/`

## 4. 主区 reference 快照
录制轨道只保留主区 `3` 组程序化快照，最终稳定帧回到默认态；底部 preview 在整条轨道中保持静态：

1. 默认态：`Workspace` 展开。
2. 快照 2：`Identity` 展开。
3. 快照 3：`Release` 展开。
4. 最终稳定帧：回到 `Workspace` 展开。

底部 preview 在整条轨道中固定为：
1. `Compact`
2. `Read only`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 262`
- 主控件：`196 x 142`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- 页面结构：标题 -> 主 `accordion` -> 底部 `Compact / Read only`
- 风格约束：浅色 Fluent surface、低噪音边框、轻量 tone rail、圆形 meta glyph、右侧 chevron；展开态只通过 body 区域、chevron 方向和局部 accent 差异表达，避免强装饰化。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Workspace` 展开 | `Sync` 展开 | `Managed` 展开 |
| 快照 2 | `Identity` 展开 | 保持不变 | 保持不变 |
| 快照 3 | `Release` 展开 | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Workspace` | 保持不变 | 保持不变 |
| same-target release / 键盘激活 / Escape | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_accordion.c` 当前覆盖 `8` 条用例：

1. 文本 fitting helper、tone 颜色与默认度量。
2. `set_items()` 的 clamp、默认展开项回落和空 items reset。
3. setter 清理 `pressed_index` 并更新 `expanded_index / focused_index / compact / read only`。
4. `activate_focused()`、listener 与单项展开。
5. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 清理。
6. 键盘 `Home / End / Tab / Up / Down / Enter / Space / Escape` 行为。
7. `read_only` 与 `!enable` 守卫，保持状态不变并清理 pressed。
8. static preview 吞掉 `touch / key`，保持状态不变且不触发 `on_action`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 收口为静态 preview 工作流：

1. 应用主区默认 `Workspace`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧。
2. 切到 `Identity`，等待 `ACCORDION_RECORD_WAIT`。
3. 抓取第二组主区快照。
4. 切到 `Release`，等待 `ACCORDION_RECORD_WAIT`。
5. 抓取第三组主区快照。
6. 恢复主区默认 `Workspace`，同时重放底部 preview 固定状态。
7. 通过最终抓帧输出稳定默认态。

说明：
- 主区真实交互仍保留 header same-target release、键盘激活、`Escape` 收起与 listener 语义，供手动复核与单测覆盖。
- runtime 录制阶段不真实发送底部 preview 输入，避免 preview 成为第二条交互轨道。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证主区快照和最终稳定帧的布局口径一致。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/accordion PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe accordion
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/sync_widget_catalog.py --check
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/accordion --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/accordion
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_accordion
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Workspace`、`Identity`、`Release` `3` 组可识别展开状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 header same-target release、键盘激活、`Escape` 收起与 listener 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `expanded_index / focused_index / compact_mode / read_only_mode`，也不能触发 `on_action`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_accordion` 正常加载。

## 11. 与现有控件的边界
- 相比 `expander`：这里是多项 accordion，保留并列 section 与单项展开关系。
- 相比 `card_expander`：这里不是单张卡片的卡内正文，而是多 section 的分组折叠。
- 相比 `settings_expander`：这里不承载设置 rows，只保留通用内容分组。
- 相比 `data_list_panel`：这里的主语义是展开正文，不是列表选择。

## 12. 当前验收结果（2026-04-26）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/accordion PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `output\main.exe accordion`
  - `accordion` suite `8 / 8`
  - 备注：Windows 长链接触发 `Error 87`，response-file fallback 成功生成 `output/main.exe`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - `python scripts/sync_widget_catalog.py --check`
  - 结果：layout 触摸审计 `custom_audited=30 custom_skipped_allowlist=1`，文档编码 `139 files`，catalog `111 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/accordion --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_accordion/default`
  - 共捕获 `9` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `31 / 31` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/accordion`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_accordion`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1998 colors=167`
- 截图复核结论：
  - 主区覆盖 `Workspace / Identity / Release` 三组 reference 快照
  - 最终稳定帧显式回到默认 `Workspace`
  - 底部 `Compact / Read only` preview 在关键帧中保持静态，无文字重叠或裁切
