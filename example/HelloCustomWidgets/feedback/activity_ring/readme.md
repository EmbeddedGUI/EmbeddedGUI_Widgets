# activity_ring 设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`WinUI ProgressRing`
- 对应组件名：`ProgressRing`
- 本次保留状态：`standard`、`compact`、`paused`、`indeterminate`
- 本次删除效果：多环 fitness 风格、页面级 guide、与进度环无关的场景装饰
- EGUI 适配说明：继续复用 SDK `activity_ring` 的单环绘制能力，在 custom 层补齐 `ProgressRing` 的单环 determinate / indeterminate 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`ProgressRing` 适合表达圆形进度或持续中的后台任务，常见于同步、加载和后台处理反馈。仓库里原先只有 determinate 的单环 reference 页面，还缺少 Fluent 主线里同样核心的 `indeterminate` 语义，因此需要继续收口。

## 2. 为什么现有控件不够用
- `progress_bar` 面向线性进度，不适合圆形视觉重心。
- `spinner` 更偏持续旋转反馈，不承担明确进度值语义。
- SDK `activity_ring` 原始能力偏多环展示，这里需要的是收窄成单环 `ProgressRing` 的 reference 语义。

## 3. 目标场景与示例概览
- 主控件默认展示 `indeterminate` 主环，用来表达“正在同步”。
- 录制轨道里保留一段旋转中的 loading，再切到 `86% active` 的 determinate 完成态。
- 底部保留两个静态 preview：
  - `compact`
  - `paused`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 188`
- 主控件尺寸：`88 x 88`
- 状态文案尺寸：`196 x 14`
- 底部对照行尺寸：`104 x 48`
- `compact` 预览：`48 x 48`
- `paused` 预览：`48 x 48`

视觉约束：
- 页面继续保持浅灰背景与低噪音 Fluent 卡片感。
- 主环只保留单环 `ProgressRing` 语义，不回退到多环 showcase 风格。
- `paused` 使用暖色强调暂停态，`indeterminate` 继续沿用主蓝色体系。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 188` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `ProgressRing` | 页面标题 |
| `activity_ring_primary` | `egui_view_activity_ring_t` | `88 x 88` | `indeterminate` | 主环 |
| `activity_ring_status` | `egui_view_label_t` | `196 x 14` | `Syncing...` | 当前状态文案 |
| `activity_ring_compact` | `egui_view_activity_ring_t` | `48 x 48` | `38%` | `compact` 静态预览 |
| `activity_ring_paused` | `egui_view_activity_ring_t` | `48 x 48` | `56%` | `paused` 静态预览 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `indeterminate` | attach 后开始轻量旋转动画 |
| 主控件 | `86% active` | 录制收尾时回落到 determinate 完成态 |
| `compact` | `38%` | 小尺寸单环对照 |
| `paused` | `56%` | 暖色暂停态对照 |

## 7. 交互与状态语义
- `ProgressRing` 仍然是 display-only 控件，不承接真实点击或导航职责。
- `indeterminate` 通过 custom 层的轻量 timer 驱动：周期性更新 `start_angle` 和弧长，形成单环旋转效果。
- `set_value()` 会退出 `indeterminate` 并恢复 determinate 语义。
- `compact` / `paused` 预览继续通过 `hcw_activity_ring_override_static_preview_api()` 吞掉 `touch / key`，只做静态 reference 对照。
- attach 到窗口后才启动动画 timer；detach 时必须停止 timer，避免残留状态。

## 8. 本轮收口内容
- 继续维护 `egui_view_activity_ring.h/.c`
- 新增 / 补齐：
  - `hcw_activity_ring_init()`
  - `hcw_activity_ring_apply_indeterminate_style()`
  - `hcw_activity_ring_set_indeterminate_mode()`
  - `hcw_activity_ring_get_indeterminate_mode()`
  - attach / detach 的 timer 生命周期管理
- demo 页面改为主环默认 `indeterminate`，录制里再切回 determinate 完成态
- 单测补齐动画状态、attach / detach 与 `set_value()` 从动画态回落的覆盖

## 9. 录制动作设计
1. 还原主环 `indeterminate` 初始相位
2. 请求第一张截图
3. 等待动画推进
4. 请求第二张截图
5. 切到 `86% active`
6. 请求第三张截图
7. 再请求一张最终稳定帧，确认状态文案和底部 preview 没有脏态

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/activity_ring PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/activity_ring --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/activity_ring
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_activity_ring
```

验收重点：
- 主环默认帧、动画推进帧和回落到 `86%` 的完成帧都必须完整可见。
- 底部 `compact / paused` 预览始终保持静态 reference，不响应输入。
- attach / detach 后不能留下残留 timer、黑白屏或裁切问题。

## 11. 已知限制
- 当前 `indeterminate` 仍是轻量 reference 动画，不追求完整 WinUI easing。
- 当前仍然只维护单环 `ProgressRing`，不恢复多环 activity demo 风格。
- 当前不叠加图标、剩余时间或业务步骤说明。
