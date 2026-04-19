# Viewbox 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 官方语义参考：`WPF / Viewbox`
- 对应组件：`Viewbox`
- 当前保留形态：`Device preview`、`Cover preview`、`Inspector thumb`、`Compact fit`、`Read only viewbox`
- 当前保留交互：主区保留真实 preset same-target release 与键盘 `Left / Right / Home / End / Tab / Enter / Space` 闭环；底部 `Compact / Read only` preview 保持静态 reference 对照
- 当前移除内容：preview focus bridge、第二条 `compact` preview 轨道、录制里的 `preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_viewbox`；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`viewbox` 用来表达“同一块内容在受限视口内按统一缩放规则适配”的语义，适合设备预览、封面容器、缩略卡片、检查面板这类需要同时保留内容整体比例和缩放结果可读性的场景。

## 2. 为什么现有控件不够用
- `card_control`、`card_action` 更偏卡片入口，不负责统一缩放规则本身。
- `uniform_grid`、`wrap_panel`、`items_repeater` 关注多项排布，不是单块内容的缩放容器。
- 直接使用基础布局只能控制尺寸，不能提供当前仓库需要的 `Viewbox` reference 页面、preset 切换与静态 preview 验收闭环。

## 3. 当前页面结构
- 页面结构固定为：标题 -> 主 `viewbox` -> 底部 `Compact / Read only` 双 preview。
- 主区保留 `3` 组录制快照和 `1` 组 preset 切换态：
  - `Device preview`
  - `Device preview` 下的 `Right` preset 切换态
  - `Cover preview`
  - `Inspector thumb`
- 录制最终稳定帧显式回到默认 `Device preview`。
- 底部左侧是 `Compact` 静态 preview，只负责对照紧凑尺寸下的缩放结果。
- 底部右侧是 `Read only` 静态 preview，只负责对照只读弱化效果与输入屏蔽语义。
- 两个 preview 统一通过 `egui_view_viewbox_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清理残留 `pressed`
  - 不改 `current_snapshot / current_preset / stretch_mode / compact_mode / read_only_mode`
  - 不触发 `on_action_listener`

目标目录：
- `example/HelloCustomWidgets/layout/viewbox/`

## 4. 主区 reference 快照
主区录制轨道保留 `3` 组程序化快照、`1` 组 preset 切换态，以及最终稳定帧；底部 preview 在整条轨道中保持静态：

1. 默认态
   `Device preview`
2. preset 切换态
   `Right` 后的 `Device preview`
3. 快照 2
   `Cover preview`
4. 快照 3
   `Inspector thumb`
5. 最终稳定帧
   回到默认 `Device preview`

底部 preview 在整条轨道中固定为：
1. `Compact fit`
2. `Read only viewbox`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 240`
- 主控件：`196 x 120`
- 底部 preview 行：`216 x 78`
- 单个 preview：`104 x 78`
- 页面结构：标题 -> 主 `viewbox` -> 底部 `Compact / Read only`
- 风格约束：保持浅色 Fluent 容器、低噪音边框和轻量强调色；主区 preset pills 是唯一交互焦点；底部 preview 固定为静态 reference 对照，不再承担额外交互职责。

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Device preview` | `Compact fit` | `Read only viewbox` |
| preset 切换态 | `Right` 后的 `Device preview` | 保持不变 | 保持不变 |
| 快照 2 | `Cover preview` | 保持不变 | 保持不变 |
| 快照 3 | `Inspector thumb` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到 `Device preview` | 保持不变 | 保持不变 |
| 主区 preset touch / key / activate | 是 | 否 | 否 |
| static preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_viewbox.inc` 当前覆盖 `7` 条用例：

1. `set_snapshots()` 的 clamp、默认 preset 回落与空快照 reset。
2. `set_font()`、`set_meta_font()`、`set_compact_mode()`、`set_read_only_mode()`、`set_palette()`、`set_current_snapshot()`、`set_current_preset()`、`set_stretch_mode()` 的 `pressed` 清理与状态更新。
3. `get_preset_region()`、`activate_current_preset()` 与 `on_action_listener` 行为。
4. 触摸 same-target release、移出取消与 `ACTION_CANCEL` 行为，验证 `pressed` 清理和提交条件。
5. 键盘 `Left / Right / Home / End / Tab / Enter` 导航，覆盖 preset 切换、跨 snapshot 跳转与提交。
6. `read_only` 与 `!enable` 守卫，保持 `current_snapshot / current_preset / stretch_mode` 不变；恢复后继续验证 `Right / Enter / Space`。
7. static preview 吞掉 `touch / key`，并保持 `current_snapshot / current_preset / stretch_mode / compact_mode / read_only_mode` 不变，同时不触发 `on_action_listener`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认 `Device preview`，同时重放底部 `Compact / Read only` preview 固定状态并抓取首帧，等待 `VIEWBOX_RECORD_FRAME_WAIT`。
2. 对主区发送一次 `Right`，等待 `VIEWBOX_RECORD_WAIT`。
3. 抓取 preset 切换后的主区帧，等待 `VIEWBOX_RECORD_FRAME_WAIT`。
4. 切到 `Cover preview`，等待 `VIEWBOX_RECORD_WAIT`。
5. 抓取第二组主区快照，等待 `VIEWBOX_RECORD_FRAME_WAIT`。
6. 切到 `Inspector thumb`，等待 `VIEWBOX_RECORD_WAIT`。
7. 抓取第三组主区快照，等待 `VIEWBOX_RECORD_FRAME_WAIT`。
8. 恢复主区默认 `Device preview`，同时重放底部 preview 固定状态，等待 `VIEWBOX_RECORD_FINAL_WAIT`。
9. 通过最终抓帧输出稳定的默认态，并继续等待 `VIEWBOX_RECORD_FINAL_WAIT`。

说明：
- 录制只导出主区状态变化，底部 `Compact / Read only` preview 在整条 reference 轨道里保持静态一致。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 preset 切换态、三组主区快照和最终稳定帧的布局口径一致。
- README 这里按当前 `test.c` 如实保留中间状态切换使用 `VIEWBOX_RECORD_WAIT`、抓帧使用 `VIEWBOX_RECORD_FRAME_WAIT`、最终回落与最终抓帧使用 `VIEWBOX_RECORD_FINAL_WAIT` 的等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/viewbox PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/viewbox --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/viewbox
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_viewbox
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现默认 `Device preview`、一次 `Right` preset 切换态、`Cover preview`、`Inspector thumb` 四组可识别状态，最终稳定帧必须回到默认态。
- 主区真实交互仍需保留 preset same-target release、键盘导航与 `on_action_listener` 语义。
- 底部 `Compact / Read only` preview 必须在全部 runtime 帧里保持静态一致。
- static preview 收到输入后，不能改写 `current_snapshot / current_preset / stretch_mode / compact_mode / read_only_mode`，也不能触发 `on_action_listener`。
- WASM demo 必须能够以 `HelloCustomWidgets_layout_viewbox` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_viewbox/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区唯一状态分组：`[0,1,8,9,10] / [2,3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界：`(57, 100) - (436, 255)`
  - 遮罩主区变化边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 255` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Device preview`

## 12. 与现有控件的边界
- 相比 `card_control`：这里强调缩放语义，不是卡片入口容器。
- 相比 `uniform_grid`、`wrap_panel`：这里处理单块内容适配，不负责多项排布。
- 相比 `items_repeater`：这里不处理模板化列表，只保留单内容块缩放与 preset 切换。
- 相比基础布局容器：这里提供可直接审阅的 `Viewbox` 标准语义和静态 preview 对照。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Device preview`
  - `Cover preview`
  - `Inspector thumb`
- 保留的底部对照：
  - `Compact fit`
  - `Read only viewbox`
- 保留的交互：
  - preset same-target release
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space`
- 删减的旧桥接与旧轨道：
  - preview focus bridge
  - 第二条 `compact` preview 轨道
  - 录制里的 `preview click` 收尾动作

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=layout/viewbox PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `viewbox` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/viewbox --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_layout_viewbox/default`
  - 共捕获 `11` 帧
- layout 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
  - layout `29 / 29` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/viewbox`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_viewbox`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.183 colors=237`
- 截图复核结论：
  - 主区覆盖默认 `Device preview`、一次 `Right` preset 切换态、`Cover preview` 与 `Inspector thumb` 四组 reference 状态
  - 最终稳定帧显式回到默认 `Device preview`
  - 主区 RGB 差分边界收敛到 `(57, 100) - (436, 255)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `Compact / Read only` preview 以 `y >= 255` 裁切后全程保持单哈希静态
