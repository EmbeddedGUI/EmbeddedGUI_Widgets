# ContentDialog 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义对照：`WinUI ContentDialog`
- 补充参考实现：`ModernWpf`
- 对应组件名：`ContentDialog`
- 当前保留状态：`warning`、`error`、`accent`、`success`、`compact`、`read only`
- 当前移除内容：旧 preview snapshot 切换、preview 点击清主控件焦点的桥接逻辑、与 reference 页面无关的额外录制动作
- EGUI 适配说明：继续使用仓库内 `dialog_sheet` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义、README 和单测，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`dialog_sheet` 对齐的是 Fluent / WinUI 的 `ContentDialog` 语义，用来承载轻量确认、删除提醒、模板应用和发布确认这类短文本、低噪音、动作明确的模态收口场景。它不是常驻反馈条，也不是 toast 通知，而是页面层级更高、动作更聚焦的确认层。

## 2. 为什么现有控件不够用
- `message_bar` 面向页内反馈，不承担阻断式确认收口。
- `toast_stack` 偏短时通知，不适合作为主要确认入口。
- `teaching_tip` 是锚定式上下文提示，不提供居中确认层语义。
- 仓库仍需要一版贴近 Fluent / WPF UI `ContentDialog` 的 `reference` 示例，用于和其它 feedback 控件形成明确边界。

## 3. 当前页面结构
- 标题：`Dialog Sheet`
- 主区：一个主 `dialog_sheet`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`
- 页面结构统一收口为：标题 -> 主 `dialog_sheet` -> `compact / read only`

当前目录：`example/HelloCustomWidgets/feedback/dialog_sheet/`

## 4. 主区 reference 轨道
主控件录制轨道只保留四组主区 snapshot 和最终稳定帧：

1. `Sync issue`
   `warning`，双动作，默认落在 `primary`
2. `Delete draft`
   `error`，双动作，默认落在 `secondary`
3. `Template`
   `accent`，单动作，无 close
4. `Publishing`
   `success`，单动作，保留 close
5. `Sync issue`
   回到默认 `warning`，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Network`
   紧凑布局、单动作、warning tone
2. `read only`
   `Read only`
   只读弱化配色、无动作

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 258`
- 主 `dialog_sheet` 尺寸：`196 x 132`
- 底部容器尺寸：`216 x 86`
- 单个 preview 尺寸：`104 x 86`
- 页面结构：标题 -> 主 `dialog_sheet` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、低对比 overlay、白色 surface、柔和边框和低噪音 Fluent 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `warning` | 是 | 是 | 否 |
| `error` | 是 | 否 | 否 |
| `accent` | 是 | 否 | 否 |
| `success` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `show_close` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义
- 主 `dialog_sheet` 继续保留 snapshot 切换、主次动作焦点和 same-target release 语义。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交；回到 `A` 后 `UP(A)` 才提交。
- `compact / read only` preview 统一通过 `egui_view_dialog_sheet_override_static_preview_api()` 吞掉 `touch / key`，不再承担任何 snapshot 切换或焦点桥接职责。
- `read only` preview 保持 `set_read_only_mode(..., 1)`，只作为静态对照，不参与真实交互。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前轨道为：

1. 应用主控件默认 `warning` 和底部 preview 固定状态
2. 请求首帧
3. 切到 `error`
4. 请求第二帧
5. 切到 `accent`
6. 请求第三帧
7. 切到 `success`
8. 请求第四帧
9. 回到默认 `warning` 并请求最终稳定帧

说明：
- 录制期间通过 `request_page_snapshot()` 统一触发布局、刷新和截图请求。
- 底部 preview 在整条轨道中不发生任何视觉变化。
- 主区变化只来自四组 snapshot 以及最终回落到默认帧。

## 9. 单元测试口径
`example/HelloUnitTest/test/test_dialog_sheet.c` 当前覆盖十一部分：

1. `set_snapshots()` 钳制与状态复位
2. `current_snapshot / current_action` guard 与 listener 通知
3. 字体、模式、listener 和 palette setter 的 `pressed` 清理
4. 触摸命中测试与动作切换
5. same-target release 与 cancel 行为
6. cancel 清理 `pressed` 且不改选择
7. 键盘导航与 guard
8. `read only` 下清理 `pressed` 并忽略输入
9. `!enable` 下忽略输入
10. 静态 preview 不变性断言
11. 内部 helper、tone、glyph、metrics 和 region 覆盖

其中静态 preview 用例通过 `dialog_sheet_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
`region_screen / background / snapshots / font / meta_font / on_action_changed / api / palette / snapshot_count / current_snapshot / current_action / compact_mode / read_only_mode / pressed_action / alpha / enable / is_focused / is_pressed / padding`

补充说明：
- preview 用例已收口为“consumes input and keeps state”。
- 事件分发统一走 `dispatch_touch_event()` / `dispatch_key_event()`。

## 10. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/dialog_sheet
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_dialog_sheet
```

## 11. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=feedback/dialog_sheet PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `dialog_sheet` suite `11 / 11`
- `sync_widget_catalog.py`：PASS，已同步 `widget_catalog.json` 与 `web/catalog-policy.json`
- `touch release semantics`：PASS，结果 `custom_audited=10 custom_skipped_allowlist=0`
- `docs encoding`：PASS，结果 `134 files`
- `widget catalog check`：PASS，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：PASS，输出 `11` 帧截图到 `runtime_check_output/HelloCustomWidgets_feedback_dialog_sheet/default`
- feedback 分类 compile/runtime 回归：PASS，分类内 `10` 个控件全部通过
- wasm 构建：PASS，输出 `web/demos/HelloCustomWidgets_feedback_dialog_sheet`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1967 colors=167`

## 12. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_feedback_dialog_sheet/default`

- 总帧数：`11`
- 主区 RGB 差分边界：`(48, 87) - (431, 266)`
- 遮罩主区后主区外唯一哈希数：`1`
- 主区唯一状态数：`4`
- 按 `y >= 267` 裁切底部 preview 区域后的唯一哈希数：`1`

结论：
- 主区变化只来自 `warning / error / accent / success` 四组 snapshot；最终回到 `warning` 稳定帧，因此主区唯一状态数为 `4`
- 主区外页面 chrome 全程保持单哈希静态，没有黑白屏、错位或 preview 污染
- 底部 `compact / read only` preview 在 `11` 帧录制中保持单哈希静态一致

## 13. 已知限制
- 当前版本继续使用固定 snapshot 数据，不接真实业务状态流。
- 当前不实现真实 modal 动画、遮罩渐变和关闭行为。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不联动外部弹层系统。

## 14. 与现有控件的边界
- 相比 `message_bar`：这里表达阻断式确认，不是页内提示。
- 相比 `toast_stack`：这里承载明确动作，不是短时通知。
- 相比 `teaching_tip`：这里是居中确认层，不是锚定式说明。

## 15. EGUI 适配说明
- 保持 `dialog_sheet` custom 实现继续驻留在 widgets 仓库，不下沉到 SDK。
- preview 统一复用静态 preview API，吞掉输入并清理残留 `pressed`。
- README、demo、单测和 runtime 验收口径已经对齐到当前 reference workflow。
