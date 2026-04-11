# slider 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI Slider`
- 对应组件名：`Slider`
- 本次保留状态：`standard`、`compact`、`read only`、`keyboard step`、`drag`
- 删除效果：页面级 `guide`、状态说明文案、额外装饰卡片、场景化 demo 壳
- EGUI 适配说明：复用 SDK `egui_view_slider` 的连续拖动基础语义，在 custom 层补齐 Fluent 风格 draw、键盘步进、静态 preview 吞输入和 setter 状态清理

## 1. 为什么需要这个控件
`slider` 用来表达“在连续数值范围中拖动选择当前值”的标准输入语义，适合音量、亮度、透明度、缩放比例、敏感度和阈值调节。它应该明显区别于离散输入的 `number_box`、评分语义的 `rating_control`，以及表达 viewport 比例的 `scroll_bar`。

## 2. 为什么现有控件不够用
- `number_box` 更偏精确步进输入，不是连续拖动。
- `rating_control` 是离散等级，不适合一般数值范围。
- `scroll_bar` 表达内容窗口位置，不表达抽象数值。
- SDK 自带 `slider` 更偏基础控件验证，缺少统一的 Fluent / WPF UI reference 页面和静态 preview 收口。

因此这里补上一套 `input/slider` reference，把标准 `Slider` 接入 `HelloCustomWidgets` 主线。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `slider`，用于连续数值调节。
- 左下 `compact` preview 展示紧凑版静态对照。
- 右下 `read only` preview 展示只读版静态对照。
- 页面只保留标题、主 `slider` 与底部双 preview，不保留额外说明 chrome。

目录：
- `example/HelloCustomWidgets/input/slider/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 174`
- 页面结构：标题 -> 主 `slider` -> `compact / read only` 双 preview
- 主控件尺寸：`196 x 38`
- 底部 preview 行：`216 x 28`
- 单个 preview：`104 x 28`

视觉约束：
- 使用浅色 page panel、低噪音轨道与白色 thumb。
- 主控件保留轻量 focus ring，不做重阴影和 showcase 风格装饰。
- `compact` 只压缩尺寸和节奏，不改变语义。
- `read only` 保留当前位置展示，但不再承担输入职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 174` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Slider` | 页面标题 |
| `slider_primary` | `egui_view_slider_t` | `196 x 38` | `52%` | 主控件 |
| `slider_compact` | `egui_view_slider_t` | `104 x 28` | `28%` | 紧凑静态预览 |
| `slider_read_only` | `egui_view_slider_t` | `104 x 28` | `42%` | 只读静态预览 |

## 6. 状态覆盖矩阵
| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 键盘步进 | 是 | 否 | 否 |
| 拖动更新 | 是 | 否 | 否 |
| `Home / End / +/-` | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 交互语义
- `Left / Down`：小步减小当前值。
- `Right / Up`：小步增加当前值。
- `Minus / Plus`：大步减小 / 增加当前值。
- `Home / End`：跳到 `0` / `100`。
- 主控件保留真实 touch drag。
- 底部 `compact / read only` preview 通过 `hcw_slider_override_static_preview_api()` 统一吞掉 `touch / key`，收到输入时先清理残留 `pressed / is_dragging`，再拒绝改变 value。

## 8. 本轮收口内容
- 新增 `egui_view_slider.h/.c`，作为 SDK `slider` 的 Fluent reference 包装层。
- 在包装层补齐：
  - `standard / compact / read only` 样式 helper
  - `set_value()` 统一清理交互态
  - 键盘导航 `Left / Right / Up / Down / Home / End / +/-`
  - `static preview` 输入吞掉与状态清理
  - custom draw，避免 SDK theme 回退掉本轮 reference 调色
- 新增 `test.c` reference 页面，只保留主控件与底部双 preview。
- 新增 `HelloUnitTest` 对应单测，覆盖 style helper、setter 清理、键盘步进、拖动、禁用态和静态 preview。

## 9. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 还原主控件、`compact` 和 `read only` 默认值，并给主控件 request focus
2. 截默认态
3. 发送 `Right`
4. 截小步增加结果
5. 发送 `Plus`
6. 截大步增加结果
7. 发送 `End`
8. 截最大值结果
9. 程序化切回第二组主值并重新 request focus
10. 再发送 `Right`
11. 截最终主控件结果
12. 程序化切换 `compact` preview 到第二组值并截最终对照

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/slider PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/slider --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主 `slider` 与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主控件的轨道、active fill 和 thumb 必须清楚可辨。
- `Left / Right / Home / End / +/-` 的键盘步进结果必须明显。
- `compact / read only` preview 必须吞掉输入并保持静态对照。

## 11. 已知限制
- 当前只做单轴水平 `Slider` reference，不扩展垂直方向。
- 不做刻度、标签提示或区间选择。
- 不做多 thumb、range slider 或 tooltip。

## 12. 与现有控件的差异边界
- 相比 `number_box`：这里强调连续数值调节，不强调精确步进输入。
- 相比 `rating_control`：这里不是离散等级评分。
- 相比 `scroll_bar`：这里的 thumb 尺寸固定，目标是选择值，而不是表达 viewport 比例。

## 13. EGUI 适配时的简化点与约束
- 继续复用 SDK `slider` 的 value 计算与 touch drag。
- 在 custom 层覆盖 draw / key / static preview 语义，避免改动 `sdk/EmbeddedGUI`。
- 先完成 reference 级 `Slider`，后续再考虑是否需要更复杂的带刻度或带标签版本。
