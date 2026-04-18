# card_action 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI CardAction`
- 对应组件名：`CardAction`
- 本次保留语义：`workspace entry / identity review / release approval`、`compact`、`read only`
- 本次删除内容：preview 点击清主控件焦点、第二条 `compact` preview 轨道、录制里的 `preview dismiss` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_card_action`，本轮只收口 `reference` 页面结构、录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`card_action` 用来表达“整张卡片直接作为行动入口，同时只保留轻量 chevron affordance”的标准动作卡片语义。它适合流程入口、提醒摘要、审批快捷跳转和轻量导航入口场景。

## 2. 为什么现有控件不够用
- `card_control` 更偏“整卡 + 右侧附加 control”，不适合纯 action card。
- `settings_card` 更偏设置项语义，不适合作为通用动作卡片入口。
- `card_panel` 更偏信息摘要，不强调整卡 action affordance。

## 3. 目标场景与示例概览
- 主控件保留真实 `CardAction` 语义，展示 `Workspace entry`、`Identity review`、`Release approval` 三组 snapshot。
- 底部左侧是 `compact` 静态 preview，只用于对照缩小尺寸下的动作卡片层级。
- 底部右侧是 `read only` 静态 preview，只用于对照冻结后的弱化视觉。
- 页面只保留标题、一个主 `card_action` 和两个静态 preview，不再让 preview 负责清焦点或收尾叙事。
- 两个 preview 统一通过 `egui_view_card_action_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改动 `current_snapshot / current_part`
  - 不触发 `on_action`

目标目录：`example/HelloCustomWidgets/layout/card_action/`

## 4. 视觉与布局规格
- 根布局：`224 x 232`
- 主控件：`196 x 98`
- 底部对照行：`216 x 72`
- `compact` preview：`104 x 72`
- `read only` preview：`104 x 72`
- 页面结构：标题 -> 主 `card_action` -> `compact / read only`
- 样式约束：
  - 维持浅色 Fluent 容器、低噪音边框和轻量 tone 区分。
  - 主区强调整卡点击与可选 chevron affordance 的关系，不再叠加旧 preview 交互桥接。
  - 底部两个 preview 固定为静态 reference 对照，不再承担焦点桥接或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `card_primary` | `egui_view_card_action_t` | `196 x 98` | `Workspace entry` | 主 `CardAction` |
| `card_compact` | `egui_view_card_action_t` | `104 x 72` | `Compact action` | 紧凑静态对照 |
| `card_read_only` | `egui_view_card_action_t` | `104 x 72` | `Read only action` | 只读静态对照 |
| `primary_snapshots` | `egui_view_card_action_snapshot_t[3]` | - | `Workspace / Identity / Release` | 主控件语义轨道 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace entry` | 默认状态，展示 accent + chevron affordance |
| 主控件 | `Identity review` | 第二组 snapshot，展示 success + chevron affordance |
| 主控件 | `Release approval` | 第三组 snapshot，展示 warning + chevron affordance |
| `compact` | `Compact action` | 固定静态对照，验证紧凑尺寸下的动作卡片层级 |
| `read only` | `Read only action` | 固定静态对照，验证只读弱化与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件保留真实 `CardAction` 键盘与触摸语义：
  - `Home / End / Tab`：保持卡片主 part 为当前交互目标
  - `Enter / Space`：激活当前 part 并触发 listener
- 触摸交互保持 same-target release：只有同一 part `DOWN -> UP` 时才提交。
- `set_snapshots()`、`set_current_snapshot()`、`set_current_part()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`。
- 底部 `compact / read only` preview 固定为静态 reference，对输入只做吞吐和状态清理，不再参与主页面叙事。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Workspace entry`。
2. 切到 `Identity review`，输出第二组主状态。
3. 切到 `Release approval`，输出第三组主状态。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部两个 preview 在整条 `reference` 轨道里保持静态一致，不再承担 preview dismiss 或焦点清理职责。
当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 模板：初始化、主状态切换、preview 重放和最终抓帧都走同一条显式布局路径，不再依赖旧的隐式布局时序。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/card_action PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_action --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_action
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_action
```

## 10. 验收重点
- 主控件必须直接看出 `CardAction` 在三组 snapshot 下保持稳定动作卡片结构。
- `same-target release / keyboard activation / read only / !enable / static preview` 全部通过单测。
- 两个 preview 必须完整可见、无黑白屏，并且在全部 runtime 帧里保持静态一致。
- README、demo 录制轨道、单测入口和验收命令链必须保持一致。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_card_action/default`
- 复核目标：
  - 主区裁剪后只出现 `3` 组唯一状态
  - 遮掉主区变化边界后，边界外区域保持单哈希
  - 按底部 preview 区域裁剪后，所有帧保持单哈希

## 12. 与现有控件的边界
- 相比 `card_control`：这里不承载右侧 value / switch control，只保留动作入口与可选 chevron。
- 相比 `settings_card`：这里强调通用动作卡片，而不是设置项入口。
- 相比 `card_panel`：这里强调整卡 action affordance，而不是信息摘要。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `workspace entry`
  - `identity review`
  - `release approval`
  - `compact`
  - `read only`
- 保留的交互：
  - same-target touch release
  - 键盘 `Home / End / Tab / Enter / Space`
- 删除的装饰或桥接：
  - preview 点击清主控件焦点
  - 第二条 `compact` preview 轨道
  - 录制里的 preview dismiss 收尾动作

## 14. 当前验收结果（2026-04-18）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/card_action PORT=pc`
- `HelloUnitTest`：`PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `card_action` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/card_action --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_layout_card_action/default`
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/card_action`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_card_action`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1769 colors=175`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8]`、`[2,3]`、`[4,5]`
  - 主区变化边界保持在 `(52, 101) - (431, 231)`
  - 按 `y >= 232` 裁切底部 preview 后保持单一哈希，确认 `compact / read only` preview 全程静态
  - 结论：主区覆盖 `Workspace entry / Identity review / Release approval` 三组 reference 状态，最终稳定帧已显式回到默认快照
