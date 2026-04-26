# TwoPaneView

## 1. 为什么需要这个控件

`TwoPaneView` 用于在同一任务里并排、上下或单窗格展示两个相关内容区域。它适合消息列表与阅读区、导航目录与详情区、摘要与操作面板等需要保持上下文的嵌入式界面。

## 2. 为什么现有控件不够用

`split_view` 解决导航窗格的展开 / 折叠问题，`master_detail` 解决列表选择与详情同步问题，但它们都不是纯粹的双窗格布局容器。`two_pane_view` 保留两个对等 pane 的布局语义，重点覆盖 `Wide / Tall / SinglePane` 三种自适应形态和单窗格优先级。

## 3. 目标场景与示例概览

- 主示例：左侧 inbox timeline 与右侧 reading surface 在 `Wide`、`Tall`、`Single` 之间切换。
- compact preview：展示小尺寸下只保留单窗格内容的状态。
- read only preview：展示禁用交互后的浅色静态双窗格状态。

## 4. 视觉与布局规格

- 外层采用浅色 Fluent surface、细边框和低半径圆角。
- 顶部为布局选择 chips：`Wide`、`Tall`、`Single`。
- 顶部右侧为单窗格优先级 chips：`P1`、`P2`。
- 内容区根据模式切换：
  - `Wide`：两个 pane 左右并排。
  - `Tall`：两个 pane 上下堆叠。
  - `SinglePane`：只显示当前优先 pane。
- 每个 pane 保留 eyebrow、title、meta、body 和 action pill，避免场景化大插画或 HMI 装饰。

## 5. 控件清单与状态矩阵

- 标准态：两个 pane 均可见，默认 `Wide + P1`。
- Tall 态：两个 pane 上下堆叠。
- SinglePane 态：仅显示 P1 或 P2。
- Focus 态：主控件获得焦点时显示细 accent focus ring。
- Pressed 态：按下 layout / pane chip 时改变填充。
- Read only 态：忽略触摸和键盘输入，颜色降噪。
- Static preview 态：消费输入但保持状态不变。

## 6. 录制动作设计

录制轨道只切换控件状态并截图，不依赖鼠标轨迹：

1. 默认 `Wide + P1`。
2. 切换到 `Tall + P1`。
3. 切换到 `Single + P2`。
4. 切换到 `Wide + P2`。
5. 回到默认 `Wide + P1`。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/two_pane_view PORT=pc` 必须通过。
- `make all APP=HelloUnitTest PORT=pc_test` 和 `output\main.exe two_pane_view` 必须通过。
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout` 必须通过。
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/two_pane_view --track reference --timeout 10 --keep-screenshots` 必须通过。
- 截图中必须能看出三种 layout 变化，底部 preview 不得抖动、裁切或变黑。

## 8. 参考设计体系与开源母本

- Microsoft Learn / WinUI `TwoPaneView`：双窗格布局容器，覆盖 `Wide`、`Tall`、`SinglePane` 等自适应语义。
- Fluent 2：浅色 surface、低噪声边框、明确 selected / pressed / disabled 状态。

## 9. 对应 Fluent / WPF UI 组件名

- `TwoPaneView`
- 本仓库目录：`example/HelloCustomWidgets/layout/two_pane_view/`

## 10. 保留的核心状态与删除的装饰效果

保留：

- 两个相关 pane 的对等布局。
- `Wide / Tall / SinglePane` 模式。
- 单窗格优先级 `P1 / P2`。
- focus、pressed、read only、static preview。

删除：

- 设备折页、铰链、硬件屏幕等强设备叙事。
- 大面积渐变、阴影卡片和场景化仪表盘装饰。
- 与双窗格语义无关的复杂列表和业务动作。

## 11. EGUI 适配时的简化点与限制

- 当前版本是 `HelloCustomWidgets` reference 控件，不下沉到 SDK。
- 只绘制两个 pane 的参考结构，不承载任意子控件树。
- 通过 `set_panes()` 提供静态内容，后续如需容器化可再讨论框架层 API。
- 默认不做手动多分辨率验收，按仓库 workflow 使用配置分辨率 runtime。
