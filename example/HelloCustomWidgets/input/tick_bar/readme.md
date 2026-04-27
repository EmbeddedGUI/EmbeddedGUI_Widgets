# TickBar 控件

## 1. 为什么需要这个控件

`TickBar` 对齐 WPF `System.Windows.Controls.Primitives.TickBar`，用于在 `Slider` / range 输入体系中表达离散刻度、刻度频率、选中范围和方向。它本身不负责拖拽和值提交，只提供范围刻度的视觉参照。

## 2. 为什么现有控件不够用

`slider` 负责值选择和 thumb 交互，`scroll_bar` 负责滚动范围，`divider` 只是分隔线。`TickBar` 的重点是 `Minimum` / `Maximum`、`TickFrequency`、`Placement`、`IsDirectionReversed` 与选中范围刻度，这些语义不能靠普通线条或 slider 轨道准确表达。

## 3. 目标场景与示例概览

- `Bottom / frequency`：底部刻度，展示 0-100 范围和 10 的刻度频率。
- `Top / selected range`：顶部刻度，展示选中范围高亮。
- `Left / reversed`：左侧垂直刻度，展示反向映射。
- `Right / read only`：右侧只读刻度，展示 muted reference 状态。

## 4. 视觉与布局规格

- 页面使用标题、主 `TickBar`、状态 caption 和底部 `compact / read only` 静态 preview。
- 主控件尺寸为 `170 x 64`，底部 preview 尺寸为 `72 x 34`。
- 刻度条保留浅色 rail、细 tick、当前值 marker 和 selected range 高亮。
- 颜色保持 Fluent / WPF UI 的低噪声浅色语言，不引入场景化背景或仪表盘装饰。

## 5. 控件清单与状态矩阵

| 状态 | Placement | Range | TickFrequency | Value | Selection | Reversed |
| --- | --- | --- | --- | --- | --- | --- |
| Standard | Bottom | 0-100 | 10 | 40 | 20-70 | 否 |
| Accent | Top | 0-120 | 15 | 75 | 30-90 | 否 |
| Vertical | Left | 0-10 | 1 | 6 | 3-7 | 是 |
| Compact | Bottom | 0-6 | 1 | 2 | 1-4 | 否 |
| Read only | Right | 0-100 | 25 | 65 | 25-75 | 否 |

## 6. 录制动作设计

录制只切换主区 reference 状态，不模拟拖拽；底部 `compact / read only` preview 全程保持静态。动作顺序为 default、accent、vertical、read only、回到 default，最后一帧必须稳定。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=input/tick_bar PORT=pc`
- `make all APP=HelloUnitTest PORT=pc_test`
- `output\main.exe tick_bar`
- `python scripts\checks\check_touch_release_semantics.py --scope custom --category input`
- `python scripts\code_runtime_check.py --app HelloCustomWidgets --app-sub input/tick_bar --track reference --timeout 10 --keep-screenshots`

验收时需要确认横向和纵向刻度完整可见，selected range 和 value marker 可辨认，底部 preview 静态一致，无黑屏、白屏、裁切或重叠。

## 8. 参考设计体系与开源母本

- WPF `TickBar`
- WPF `TickBar.Minimum`
- WPF `TickBar.Maximum`
- WPF `TickBar.TickFrequency`
- WPF `TickBar.Placement`
- WPF `TickBar.IsDirectionReversed`

## 9. 对应的 Fluent / WPF UI 组件名

- Reference system: `Fluent 2 / WPF UI`
- Reference library: `WPF`
- Reference component: `TickBar`

## 10. 保留的核心状态与删除的装饰效果

保留 range、frequency、placement、selected range、reversed、compact 和 read only。删除复杂 slider 交互、任意 `Ticks` 集合、tooltip、thumb 拖拽、动画和业务仪表盘式装饰。

## 11. EGUI 适配时的简化点与限制

- `minimum` / `maximum` / `value` 使用 `int16_t`。
- `tick_frequency` 使用 `uint8_t`，为 0 时自动归一到 1。
- `placement` 覆盖 top / bottom / left / right。
- 当前实现不支持 WPF 的任意 `Ticks` 集合，只支持等距频率刻度。
- 当前实现是 custom reference 控件，不修改 `sdk/EmbeddedGUI`。
