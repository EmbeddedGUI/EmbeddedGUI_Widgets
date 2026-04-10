# rating_control 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`RatingControl`
- 当前保留状态：`standard`、`compact`、`read only`
- 当前保留交互：点击评分、拖拽跨星更新、`Clear` 清空、键盘步进评分
- 已删减效果：页面级 guide、状态回显条、额外 preview 标签、showcase/HMI 化装饰、半星评分、tooltip、复杂 hover 动画

## 1. 为什么需要这个控件
`rating_control` 用于表达离散等级评分，适合服务评价、速度反馈、安装体验、满意度选择等场景。相比普通单选或数字输入，星级评分更符合用户对“1..5 档位评价”的直觉认知。

## 2. 为什么现有控件不够用
- `radio_button` 只表达互斥选项，不具备星级评分语义。
- `slider` 偏连续拖动，不适合标准离散评分。
- `segmented_control` 更适合页内切换，不适合满意度表达。
- `number_box` 偏数值编辑，不是评价型控件。

因此保留 `rating_control`，但示例页必须收口到 Fluent / WPF UI 风格的低噪音 reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `rating_control`，覆盖 `Service quality`、`Delivery speed`、`Setup experience` 三组快照。
- 底部左侧保留一个 `compact` 静态对照。
- 底部右侧保留一个 `read only` 静态对照。
- 主控件保留真实 touch 和 keyboard 交互。
- 底部 preview 只做静态 reference，对外统一吞掉 touch / key，不再承担交互职责。

目录：
`example/HelloCustomWidgets/input/rating_control/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 174`
- 页面结构：标题 -> 主 `rating_control` -> 底部 `compact / read only`
- 主控件尺寸：`196 x 92`
- 底部容器尺寸：`216 x 42`
- 两个 preview 尺寸：`104 x 42`

视觉原则：
- 使用浅灰页面底板和白色评分卡，避免回到 showcase 页面风格。
- 保留暖金色 accent，维持评分语义。
- `Clear` 保持轻量页内入口，不再依赖额外说明文案。
- preview 维持低噪音静态对照，不显示交互焦点残影。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 174` | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `control_primary` | `egui_view_rating_control_t` | `196 x 92` | 主评分卡 |
| `control_compact` | `egui_view_rating_control_t` | `104 x 42` | 紧凑静态对照 |
| `control_read_only` | `egui_view_rating_control_t` | `104 x 42` | 只读静态对照 |

## 6. 状态矩阵
| 状态 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认值展示 | 是 | 是 | 是 |
| 点击星级提交 | 是 | 否 | 否 |
| 拖拽到新星后提交最新 part | 是 | 否 | 否 |
| `Clear` 清空 | 是 | 否 | 否 |
| 键盘 `Left/Right/Up/Down/Home/End/Tab/Enter/Space/Esc` | 是 | 否 | 否 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
1. 应用默认主快照和 compact 快照。
2. 抓首帧 reference。
3. 点击第 5 颗星，展示主控件评分提交。
4. 抓交互结果帧。
5. 点击 `Clear`，展示清空路径。
6. 抓清空结果帧。
7. 切到第二组主快照并执行 `End`。
8. 抓键盘步进后的结果帧。
9. 切换 compact 静态对照快照。
10. 抓最终收尾帧，同时保留 read only 对照。

## 8. 本轮交互收口
- 新增统一的 `rating_control_clear_pressed_state()`，所有会切状态的 setter 和 guard 都先清残留 pressed。
- `ACTION_UP / ACTION_CANCEL` 改为统一走 pressed 清理，不再分散手写。
- 保留 `ACTION_MOVE` 的拖拽语义：
  `DOWN(2) -> MOVE(5) -> UP(5)` 仍然提交最新星位。
- `read only / disabled` touch-key guard 现在都会先清残留 pressed，再保持既有返回语义。
- 新增 `egui_view_rating_control_override_static_preview_api()`，让底部 preview 统一吞掉 touch / key，并立即清 pressed。
- 绘制层把焦点强化收口到真正获得 focus 的主控件，避免 compact preview 出现误导性的焦点 ring。

## 9. 编译、测试与 runtime 验收
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc
make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rating_control --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主卡和底部两个 preview 必须完整可见，不被裁切。
- 交互后不能残留 `pressed_part` 或假焦点高亮。
- 主控件拖拽跨星时只更新当前评分，不出现黑屏、白屏或大面积错位。
- preview 在 touch / key 后仍保持静态 reference，不误改值、不误触发清空。

## 10. 已知限制
- 当前只支持整星评分，不支持半星或浮点评分。
- `Clear` 是轻量页内入口，不做二次确认。
- `compact` / `read only` preview 仅做 reference 对照，不承担交互职责。

## 11. 与现有控件的边界
- 相比 `radio_button`：这里是标准评分语义，不是普通互斥项。
- 相比 `slider`：这里是离散等级，不是连续调节。
- 相比 `segmented_control`：这里表达评价，不表达页内切换。
- 相比 `number_box`：这里表达满意度档位，不是数值录入。
