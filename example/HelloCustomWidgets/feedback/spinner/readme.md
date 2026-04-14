# spinner 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 补充对照实现：`WinUI Spinner`
- 对应组件名：`Spinner`
- 本次保留状态：`standard`、`compact`、`muted`、`indeterminate`
- 本次删除效果：场景卡片、说明 note、阻塞式 loading 容器和与等待指示无关的页面装饰
- EGUI 适配说明：继续复用 SDK `egui_view_spinner` 的圆弧绘制和旋转能力，在 custom 层补齐样式 helper、固定相位静态 preview API 与 `rotation_angle` setter，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Spinner` 用来表达“后台任务正在进行，但当前没有明确百分比”的等待态，适合附着在局部区域或轻量状态文案旁边。仓库里已经有偏进度语义的 `activity_ring` 和 `progress_bar`，但还缺少 Fluent 主线里更纯粹的 indeterminate loading 指示器，因此需要保留并继续收口。

## 2. 为什么现有控件不够用
- `activity_ring` 对齐的是 `ProgressRing`，仍然保留 determinate 进度值语义。
- `progress_bar` 强调线性推进，不适合替代小尺寸等待指示。
- `skeleton` 更偏内容占位，不适合表达正在处理中的独立动作。
- SDK 自带 `spinner` 只有基础绘制和旋转能力，缺少当前仓库要求的统一 reference 页面结构与静态 preview 语义。

## 3. 目标场景与示例概览
- 主控件保留真实旋转的标准 spinner，用来表达 `Syncing files` 一类后台工作。
- 主控件下方只保留一行轻量状态文案，不再使用场景卡片或说明 note。
- 底部保留两个真正静态的 preview：
  - `compact`
  - `muted`
- 页面结构统一收口为：标题 -> 主 `spinner` -> 状态文案 -> `compact / muted` 双 preview。
- 录制轨道抓取三张旋转中的主控件快照，底部 preview 始终保持固定弧段相位，不参与动画。

目标目录：`example/HelloCustomWidgets/feedback/spinner/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 156`
- 主控件尺寸：`44 x 44`
- 状态文案尺寸：`196 x 14`
- 底部对照行尺寸：`200 x 56`
- 单个 preview 面板尺寸：`96 x 56`
- preview spinner 尺寸：`24 x 24`

视觉约束：
- 页面继续保持浅灰背景与低噪音 reference 语义，不再套额外大卡片。
- 主控件使用品牌蓝为基础色，并允许录制轨道中切换暖色与青绿色以表达不同后台任务阶段。
- `compact` 保持更轻的笔画和更短弧段。
- `muted` 使用低饱和灰蓝色，弱化视觉存在感。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 156` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Spinner` | 页面标题 |
| `primary_spinner` | `egui_view_spinner_t` | `44 x 44` | `standard / spinning` | 主等待指示器 |
| `primary_status_label` | `egui_view_label_t` | `196 x 14` | `Syncing files` | 当前状态文案 |
| `compact_spinner` | `egui_view_spinner_t` | `24 x 24` | `compact / static` | 紧凑静态对照 |
| `muted_spinner` | `egui_view_spinner_t` | `24 x 24` | `muted / static` | 低噪音静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Syncing files` | 蓝色标准 spinner，持续旋转 |
| 主控件 | `Publishing docs` | 暖色 spinner，持续旋转 |
| 主控件 | `Refreshing cache` | 青绿色 spinner，持续旋转 |
| `compact` | 固定相位 `300°` | 静态、小尺寸 reference 对照 |
| `muted` | 固定相位 `24°` | 静态、低噪音 reference 对照 |

## 7. 交互与状态语义
- 主 `spinner` 仍然是 display-only 控件，不承接真实点击或导航职责。
- 主控件保持真实 indeterminate 旋转，不回退到“假静态”截图式展示。
- `compact / muted` preview 通过 `hcw_spinner_set_spinning(..., 0)` 停止旋转，并用 `hcw_spinner_set_rotation_angle()` 固定弧段相位。
- `compact / muted` preview 继续通过 `hcw_spinner_override_static_preview_api()` 吞掉 `touch / key`，只做静态 reference 对照。
- 本轮不修改 SDK 动画逻辑，只在 custom 层补齐更明确的 preview 控制能力。

## 8. 本轮收口内容
- 继续维护 `egui_view_spinner.h/.c`
- 新增 / 补齐：
  - `hcw_spinner_set_rotation_angle()`
  - `compact / muted` 静态 preview 的固定相位设置
- demo 页面从“主卡片 + 多段说明文案”收口为更直接的 reference 布局
- 单测补齐固定角度归一化、attach 后静态 preview 不启动 timer、preview 输入抑制覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `Syncing files`，并同步 `compact / muted` 的静态 preview 状态。
2. 请求 `snapshot 0`。
3. 切到 `Publishing docs`。
4. 请求 `snapshot 1`。
5. 切到 `Refreshing cache`。
6. 请求 `snapshot 2`。
7. 恢复默认 `Syncing files` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/spinner --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/spinner
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_spinner
```

验收重点：
- 主 spinner 的三张快照必须能直接看出旋转相位变化。
- `compact / muted` preview 必须始终保持静态，不得在 runtime 截图里继续转动。
- 页面不能出现黑白屏、裁切、弧段断裂或底部 preview 脏态。
- preview 不响应触摸或键盘输入。

## 11. 已知限制
- 当前只维护单个 `Spinner` 指示器，不实现遮罩、页面级 loading 或批量任务容器。
- 当前不显示百分比，不承担 determinate progress 语义。
- 当前固定相位 preview 只服务于 reference 截图稳定性，不追求完整 WinUI easing 或更复杂动画节奏。
