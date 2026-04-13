# Spinner 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Spinner`
- 本次保留语义：单个 indeterminate loading 指示器、标准/紧凑/静默外观、静态 preview 输入抑制
- 本次删除内容：阻塞式遮罩、整页 loading 容器、百分比进度、额外交互
- EGUI 适配说明：复用 SDK `egui_view_spinner` 作为底层绘制和动画实现，custom 层只补 style helper、palette/stroke/arc setter 与 static preview API

## 1. 为什么需要这个控件？
`Spinner` 用来表达“后台工作正在进行，但当前没有明确百分比”的等待态。现有 `activity_ring` 对齐的是 `ProgressRing`，偏有值进度；`progress_bar` 也承担线性进度显示。仓库里还缺少 Fluent 2 `Spinner` 这种轻量 indeterminate loading 语义，因此需要补齐。

## 2. 为什么现有控件不够用？
- `activity_ring` 以数值进度为主，不是纯等待态。
- `progress_bar` 强调线性推进，不适合附着式小型 loading 指示。
- `message_bar`、`toast_stack` 等反馈控件层级更高，不适合替代 spinner。

## 3. 目标场景与示例概览
- 主面板：一个标准 spinner，在录制轨道中轮播三种等待场景配色。
- 底部两个静态 preview：
  - `compact`
  - `muted`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 196`
- 主面板：`196 x 92`
- 主 spinner：`44 x 44`
- 底部容器：`216 x 70`
- 单个 preview 面板：`104 x 70`
- preview spinner：`28 x 28`

## 5. 控件清单与状态矩阵
- 主控件：标准 spinner，持续旋转。
- `compact` preview：缩小尺寸和笔画的静态对照。
- `muted` preview：低噪音配色的静态对照。

| 状态 / 能力 | 主控件 | Compact preview | Muted preview |
| --- | --- | --- | --- |
| 旋转动画 | 是 | 是 | 是 |
| 标准品牌色 | 是 | 是 | 否 |
| 紧凑尺寸 | 否 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 6. 录制动作设计
1. 初始化 `Syncing files`
2. 抓取第一帧
3. 切到 `Publishing docs`
4. 抓取第二帧
5. 切到 `Refreshing cache`
6. 抓取第三帧
7. 恢复默认场景并收尾

## 7. 编译 / runtime / 截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/spinner PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category feedback
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/spinner --track reference --timeout 10 --keep-screenshots
```

验收重点：
- 主 spinner 和两个 preview 都必须完整可见。
- 旋转弧段不能裁切、断裂或消失。
- `compact` 和 `muted` 必须保持低噪音，不压过主面板。

## 8. 已知限制
- 当前只覆盖单个 spinner，不实现遮罩或页面级 loading 容器。
- 不显示百分比文本，也不承担 determinate progress 语义。
- 不修改 SDK 动画逻辑，只在 custom 层补样式和 reference 页面。
