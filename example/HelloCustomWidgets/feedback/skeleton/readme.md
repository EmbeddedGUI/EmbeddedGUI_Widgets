# skeleton 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`Skeleton`
- 本次保留状态：`wave`、`compact`、`read only`
- 本次删除效果：preview 动画、preview snapshot 切换、preview 点击清 focus 的桥接动作，以及与骨架占位无关的额外录制交互
- EGUI 适配说明：沿用仓库内 `skeleton` custom 实现，本轮只收口 `reference` 页面结构、静态 preview 语义和录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Skeleton` 用于在真实内容尚未到达时先表达页面结构、信息密度和重点区域，比单纯的 `spinner` 更能说明“内容将会如何排布”，适合文章、列表、设置页和轻量卡片的加载占位。

## 2. 为什么现有控件不够用
- `spinner` 只能表达“正在加载”，不能表达内容骨架。
- `progress_bar` 更适合数值进度，不适合页面结构占位。
- 旧的 showcase 风格 skeleton 视觉更重，不符合当前 `reference` 主线。
- 当前仓库仍需要一版贴近 Fluent / WPF UI 的浅色 `Skeleton` 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `wave` skeleton，通过录制轨道覆盖 `Article / Feed / Settings` 三组骨架。
- 底部左侧展示 `compact` 静态对照，保留紧凑尺寸和轻量块布局，但不再播放 pulse 动画。
- 底部右侧展示 `read only` 静态对照，保留弱化 chrome 与只读语义。
- 页面结构统一收口为：标题 -> 主 `skeleton` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_skeleton_override_static_preview_api()` 吞掉 `touch / key`，不再切换 snapshot，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/feedback/skeleton/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 124`
- 底部对照行尺寸：`216 x 60`
- `compact` 预览：`104 x 60`
- `read only` 预览：`104 x 60`

视觉约束：
- 页面保持浅灰 page panel、白底 skeleton card 和低噪音浅边框。
- 主控件保留轻量 `wave shimmer`，用于表达真实加载中的骨架态。
- `compact` 与 `read only` 都必须是静态 preview，只负责 reference 对照，不得继续转为 pulse 或交互桥接。
- `read only` 继续通过更弱的边框和更淡的占位块表达只读语义。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Skeleton` | 页面标题 |
| `skeleton_primary` | `egui_view_skeleton_t` | `196 x 124` | `Article / wave` | 主骨架 |
| `skeleton_compact` | `egui_view_skeleton_t` | `104 x 60` | `Compact row / static` | 紧凑静态对照 |
| `skeleton_read_only` | `egui_view_skeleton_t` | `104 x 60` | `Read only / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Article` | 默认 `wave` 骨架 |
| 主控件 | `Feed` | 第二组加载骨架 |
| 主控件 | `Settings` | 第三组加载骨架 |
| `compact` | `Compact row` | 紧凑静态 preview |
| `read only` | `Read only` | 只读静态 preview |

## 7. 交互与状态语义
- 主 `skeleton` 仍然是 display-only 占位控件，但保留真实 `wave` 动画。
- `compact / read only` preview 都通过 `set_animation_mode(..., NONE)` 停止动画，作为真正静态的 reference 对照。
- `read only` preview 继续使用 `set_read_only_mode(..., 1)`，保持弱化语义并确保 timer 不启动。
- 两个 preview 继续通过 `override_static_preview_api()` 吞掉 `touch / key`，但不再承担任何清焦点、切换 snapshot 或收尾桥接职责。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/feedback/skeleton/test.c`
- 调整底部 `compact` preview 为单一静态 snapshot，删除 preview pulse 动画
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件 `Article / Feed / Settings` 的状态切换与最终稳定帧
- 单测同步补齐“静态 preview 不启动 timer、输入抑制后仍保持固定 snapshot / emphasis”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `Article`，并同步两个静态 preview 状态。
2. 请求 `snapshot 0`。
3. 切到 `Feed`。
4. 请求 `snapshot 1`。
5. 切到 `Settings`。
6. 请求 `snapshot 2`。
7. 恢复默认 `Article` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/skeleton PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/skeleton --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category feedback --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category feedback --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub feedback/skeleton
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_feedback_skeleton
```

验收重点：
- 主控件三张关键截图必须能看出不同骨架布局与主区域波带变化。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得继续播放 pulse 或因点击产生额外状态变化。
- 页面不能出现黑白屏、裁切、波带断裂或底部 preview 脏态。
- preview 不响应触摸或键盘输入。

## 11. 已知限制
- 当前版本仍使用固定 snapshot 数据，不接真实业务骨架配置。
- 当前 `wave` 是轻量近似动画，不追求更复杂的渐变带。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不扩展成长列表骨架系统。
