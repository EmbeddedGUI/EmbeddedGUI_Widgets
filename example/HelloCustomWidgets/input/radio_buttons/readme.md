# RadioButtons

## 1. 为什么需要这个控件

`RadioButtons` 用于在一组互斥选项中只保留一个当前值，适合通知渠道、主题模式、同步频率这类“单选但需要整体呈现”的场景。

## 2. 为什么现有控件不够用

- `radio_button` 目前只覆盖单个按钮语义，reference demo 里需要用多个控件手工拼组。
- 手工拼组时，same-target release、键盘切换、整体布局和静态 preview 抑制都要在页面层重复处理。
- `segmented_control` 更偏横向分段，不适合长标签或垂直选项列表。

## 3. 目标场景与示例概览

- 主示例：三项单选列表，展示标准密度、焦点和键盘切换。
- 紧凑预览：两项 compact 版本，验证小尺寸下的垂直排布。
- 只读预览：两项 read-only 版本，验证弱化颜色与禁交互语义。

## 4. 视觉与布局规格

- 外层是轻量圆角容器，保持 Fluent 2 / WinUI 的低噪音白底卡片观感。
- 每个 item 由选中圆点、标签文本和轻分隔线组成。
- 选中项使用弱化 accent fill，未选中项保持白底文本对比。
- compact 模式缩小 padding、row height 和 indicator 尺寸。

## 5. 控件清单与状态矩阵

| 控件 | 类型 | 尺寸 | 说明 |
| --- | --- | --- | --- |
| `primary_widget` | `egui_view_radio_buttons_t` | `176 x 78` | 主交互控件 |
| `compact_widget` | `egui_view_radio_buttons_t` | `84 x 50` | compact 静态 preview |
| `read_only_widget` | `egui_view_radio_buttons_t` | `84 x 50` | read-only 静态 preview |

状态覆盖：

- 默认态
- 选中态
- pressed 视觉反馈
- focus ring
- compact mode
- read-only mode
- static preview 输入抑制

## 6. 录制动作设计

- 初始截图：主控件显示 `Email / Push / SMS`，默认选中 `Email`。
- 键盘 `Down`：切到第二项，验证组内切换。
- 切换数据集：替换为 `Daily / Weekly / Monthly`，验证 API 更新。
- 键盘 `End`：切到最后一项。
- 最终截图：切换为 `Auto / Light / Dark`，收口主状态。

## 7. 编译 / runtime / 截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/radio_buttons PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/radio_buttons --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/radio_buttons
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_radio_buttons
```

## 8. 参考设计体系与开源母本

- Fluent 2
- WinUI `RadioButtons`
- WPF UI `RadioButtons`

## 9. 对应的 Fluent / WPF UI 组件名

- `RadioButtons`

## 10. 保留的核心状态与删掉的装饰效果

保留：

- 单组选中项
- 键盘方向键 / Home / End / Tab 切换
- same-target release
- compact / read-only 语义

删掉：

- 页面级说明文字与外部组装逻辑
- hover 专属动画
- 复杂模板化 item 内容

## 11. EGUI 适配时的简化点与限制

- 当前只支持纯文本 item，不支持图标或自定义模板。
- item 数量上限固定为 `6`，便于 reference runtime 和单测覆盖。
- 组内焦点直接绑定当前选中项，不单独维护“未选中但有焦点”的游标状态。
- static preview 通过 override API 直接吞掉 `touch / key`，不改 SDK。
