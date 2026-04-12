# arc 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`Arc`
- 补充对照控件：`activity_ring`、`progress_bar`
- 本次保留状态：`standard`、`subtle`、`attention`
- 本次删除效果：拖拽 thumb、输入型 slider 语义、页面级说明条和额外装饰壳
- EGUI 适配说明：新增只读 `egui_view_arc`，在 custom 层完成弧线绘制、样式 helper 和 static preview 输入收口，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`arc` 用于表达轻量、只读、非阻塞的环形进度或状态占比。它适合放在设置页、概览卡片、同步流程和发布摘要里，用比 `progress_bar` 更紧凑的方式承载百分比变化。

## 2. 为什么现有控件不够用
- `activity_ring` 偏向持续运行中的不确定进度，不适合表达稳定百分比。
- `progress_bar` 是线性信息条，不适合卡片级的环形占位。
- SDK 里的 `arc_slider` 带有输入控件语义和拖拽 thumb，不符合 Fluent / WPF UI `Arc` 的只读展示语义。

## 3. 目标场景与示例概览
- 主控件展示 `standard arc`，通过录制轨道依次切换 `32% -> 58% -> 86%`。
- 底部左侧展示 `subtle` 静态预览，验证低噪音状态下的几何和留白。
- 底部右侧展示 `attention` 静态预览，验证提醒态下的高对比配色。
- 页面结构统一为：标题 -> 主 `arc` -> `subtle / attention` 双预览。

目标目录：`example/HelloCustomWidgets/display/arc/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 228`
- 主面板尺寸：`196 x 124`
- 主弧线尺寸：`76 x 76`
- 底部对照行：`216 x 84`
- 单个预览面板：`104 x 84`
- 单个预览弧线：`42 x 42`
- 视觉约束：
  - 使用浅灰页面底、白色主面板和更轻的 muted preview panel。
  - `Arc` 只保留 track 与 active arc，不引入 thumb、刻度和输入提示。
  - 百分比文本与说明文案放在弧线外部，保持 reference 页面稳定且低噪音。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 228` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Arc` | 页面标题 |
| `primary_arc` | `egui_view_arc_t` | `76 x 76` | `32%` | 主展示弧线 |
| `subtle_arc` | `egui_view_arc_t` | `42 x 42` | `24%` | `subtle` 静态预览 |
| `attention_arc` | `egui_view_arc_t` | `42 x 42` | `72%` | `attention` 静态预览 |

## 6. 状态覆盖矩阵

| 区域 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Sync baseline` | 默认蓝色进度 |
| 主控件 | `Review queue` | 提醒态高对比进度 |
| 主控件 | `Ready to publish` | 成功态绿色进度 |
| 预览 | `subtle` | 低噪音次级状态 |
| 预览 | `attention` | 红色提醒状态 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件与底部双预览到默认状态。
2. 请求第一张截图。
3. 程序化切换主控件到 `58%`。
4. 请求第二张截图。
5. 程序化切换主控件到 `86%`。
6. 请求第三张截图。
7. 回到默认 `32%` 并请求最终稳定帧。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=display/arc PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/arc --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主面板和底部双预览必须完整可见，不能裁切。
- 三段主控件状态需要一眼可辨，但不能退回 showcase 风格。
- `Arc` 必须保持只读展示语义，不响应真实输入。
- static preview 必须吞掉 `touch / key`，不能产生点击副作用。

## 9. 已知限制与后续方向
- 当前只验证固定百分比轨道，不接业务数据流。
- 当前不实现圆心文本内嵌布局，保持页面和录制结果更稳定。
- 当前优先服务 `reference` 页面，后续是否下沉到 SDK 再单独评估。

## 10. 与现有控件的边界
- 相比 `activity_ring`：这里表达确定性百分比，不做持续旋转。
- 相比 `progress_bar`：这里强调环形占位和卡片级展示，不做线性条形语义。
- 相比 SDK `arc_slider`：这里是只读展示控件，不承担拖拽输入职责。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`Arc`
- 本次保留核心状态：
  - `standard`
  - `subtle`
  - `attention`

## 13. 相比参考原型删除的效果或装饰
- 删除 slider thumb 和拖拽反馈。
- 删除页面级说明条、旧 preview 标签和额外卡片壳。
- 删除输入型进度调节语义，只保留只读展示。

## 14. EGUI 适配时的简化点与约束
- 使用自定义 `egui_view_arc` 直接绘制 round-cap arc，避免修改 SDK。
- 通过 helper 统一 `stroke / angles / palette`，保证 reference 页面风格稳定。
- 通过 static preview API 吞掉 `touch / key`，避免底部对照预览干扰页面录制。
