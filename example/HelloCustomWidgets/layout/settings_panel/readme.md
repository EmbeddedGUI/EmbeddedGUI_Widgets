# settings_panel 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WPF UI / WinUI SettingsCard`
- 对应组件语义：`SettingCardGroup`
- 本次保留语义：`Workspace settings`、`Backup and alerts`、`Release controls`、`Account review`、`compact`、`read only`
- 本次删除内容：旧 preview focus bridge、第二条 `compact` preview 轨道、录制里的 `preview dismiss / preview click` 收尾
- EGUI 适配说明：继续在 custom 层维护轻量 `egui_view_settings_panel` reference 实现，本轮只收口参考页结构、录制轨道、静态 preview 语义和单测入口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`settings_panel` 用来表达“多行设置项分组”的标准语义。它不是普通列表，也不是单张设置卡，而是把一组带 `value / switch / chevron` 尾部 affordance 的设置行稳定地组织在同一块 Fluent 风格容器里。

## 2. 为什么现有控件不够用
- `settings_card` 只覆盖单个设置入口，不承载多行分组。
- `settings_expander` 面向“设置头部 + 可展开嵌套项”，不是平铺的 setting group。
- `data_list_panel` 更接近通用列表，不强调 setting row 的层级、tone 和尾部控件节奏。

## 3. 目标场景与页面结构
- 页面只保留标题、一个主 `settings_panel`，以及底部两个静态 preview。
- 主控件展示四组 snapshot：
  - `Workspace settings`
  - `Backup and alerts`
  - `Release controls`
  - `Account review`
- 左下角是 `compact` 静态 preview，只负责对照紧凑布局密度。
- 右下角是 `read only` 静态 preview，只负责对照只读弱化状态。
- 两个 preview 统一通过 `egui_view_settings_panel_override_static_preview_api()` 收口：
  - 吞掉新的 `touch / key`
  - 只清理残留 `pressed`
  - 不改变 `current_snapshot / compact_mode / read_only_mode`
  - 不触发点击行为

目标目录：`example/HelloCustomWidgets/layout/settings_panel/`

## 4. 视觉与布局规格
- 根布局：`224 x 258`
- 主控件：`196 x 132`
- 底部对照行：`216 x 84`
- `compact` preview：`104 x 84`
- `read only` preview：`104 x 84`
- 页面结构：标题 -> 主 `settings_panel` -> `compact / read only`
- 样式约束：
  - 保持浅色 Fluent 容器、低噪音边框和轻量 tone 提示。
  - tone 只保留在 eyebrow、focus row、尾部 affordance 和 footer 的轻量差异上。
  - 底部 preview 固定为静态 reference 对照，不再承担清焦点、切换轨道或页面桥接职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `panel_primary` | `egui_view_settings_panel_t` | `196 x 132` | `Workspace settings` | 主 `SettingCardGroup` |
| `panel_compact` | `egui_view_settings_panel_t` | `104 x 84` | `Compact` | 紧凑静态 preview |
| `panel_read_only` | `egui_view_settings_panel_t` | `104 x 84` | `Read only` | 只读静态 preview |
| `primary_snapshots` | `egui_view_settings_panel_snapshot_t[4]` | - | `Workspace / Backup / Release / Account` | 主状态轨道 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Workspace settings` | 默认状态，accent value row |
| 主控件 | `Backup and alerts` | 第二组 snapshot，switch rows 为主 |
| 主控件 | `Release controls` | 第三组 snapshot，warning focus row |
| 主控件 | `Account review` | 第四组 snapshot，muted review rows |
| `compact` | `Compact` | 固定静态对照，只验证紧凑布局 |
| `read only` | `Read only` | 固定静态对照，只验证只读弱化与输入屏蔽 |

## 7. 交互语义与单测要求
- 主控件保留真实点击闭环：
  - `touch down/up` 触发面板点击
  - `dispatch_key_event + Enter / Space` 触发面板点击
- `set_snapshots()`、`set_current_snapshot()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 都必须先清理残留 `pressed`
- `read_only` 与 `!enable` 期间：
  - `touch / dispatch_key_event` 都不能改状态
  - `current_snapshot / compact_mode / read_only_mode` 保持符合预期
  - 不触发点击
- static preview 期间：
  - 只清理残留 `pressed`
  - 保持 `current_snapshot / compact_mode / read_only_mode` 不变
  - 不触发点击

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件和底部 `compact / read only` preview，输出默认 `Workspace settings`
2. 切到 `Backup and alerts`，输出第二组主状态
3. 切到 `Release controls`，输出第三组主状态
4. 切到 `Account review`，输出第四组主状态
5. 恢复主控件默认状态并输出最终稳定帧

录制只导出主控件状态变化。底部两个 preview 在整条 reference 轨道里保持静态一致，不再包含第二条 `compact` 轨道，也不再包含 `preview dismiss / preview click` 收尾。

## 9. 编译、单测、运行时与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc

# 在 X:\ 短路径下执行；修改 HelloUnitTest 后先 clean 再重建
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/settings_panel
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_settings_panel
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 必须完整可见，不能黑白屏、裁切或重叠。
- 主区四组 `SettingCardGroup` 状态变化要清晰可辨，底部 preview 全程保持静态。
- `dispatch_key_event` 路径下的 `Enter / Space`、`read only`、`!enable`、`static preview keeps state` 要全部通过单测。
- `snapshot / compact / read only / disabled` 切换后不能残留旧的 `pressed` 高亮。
- WASM demo 必须正常加载，文档面板能渲染本 README。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_layout_settings_panel/default`
- 复核目标：
  - 主区存在 4 组可辨识唯一状态
  - 底部 preview 区域在全程保持单一静态哈希
  - 变化边界只出现在主区，不扩散到底部 preview

## 12. 与现有控件的边界
- 相比 `settings_card`：这里是多行设置分组，不是单个入口卡。
- 相比 `settings_expander`：这里不承接 `expand / collapse`，只保留平铺 rows。
- 相比 `data_list_panel`：这里强调 setting row 语义和尾部 affordance，不是通用列表。

## 13. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `Workspace settings`
  - `Backup and alerts`
  - `Release controls`
  - `Account review`
  - `compact`
  - `read only`
- 保留的交互：
  - 面板点击
  - 键盘 `Enter / Space`
- 删减的旧桥接与轨道：
  - preview 点击清主控件 focus
  - 第二条 `compact` preview 轨道
  - 录制中的 `preview dismiss / preview click` 收尾
