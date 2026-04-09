# auto_suggest_box 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`AutoSuggestBox`
- 本次保留状态：`standard`、`compact`、`read only`、`expanded`、`focused`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、标签点击切换、搜索图标、复杂 hover、桌面阴影、Acrylic、异步建议流
- EGUI 适配说明：继续复用仓库里的 `autocomplete -> combobox` 基础实现，本轮重点收口 reference 页面结构、统一 palette，并保留轻量键盘展开/选择闭环

## 1. 为什么需要这个控件

`auto_suggest_box` 适合做轻量命令查找、成员搜索、模板选择和最近项匹配。它比普通下拉框更强调“建议结果”，也比完整文本输入更轻，适合小屏和固定候选集。

## 2. 为什么现有控件不够用

- `combobox` / `autocomplete` 现有示例更偏基础功能验证，缺少统一的 reference 页面
- `textinput` 更偏自由输入，不适合直接展示固定建议集
- `menu_flyout` 更偏命令面板，不是输入语义
- 当前主线需要一版更接近 `Fluent 2 / WPF UI AutoSuggestBox` 的标准建议输入 reference

因此这里继续保留 `auto_suggest_box`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `auto_suggest_box`，用于建议集切换
- 左下 `compact` 预览展示窄宽度建议框
- 右下 `read only` 预览展示只读静态对照态
- 示例页只保留标题、主 `auto_suggest_box` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和标签点击
- 录制动作改为程序化 `expand/collapse`、键盘选择和 snapshot 切换，不再依赖坐标点击页面 chrome

组件目录：

- `example/HelloCustomWidgets/input/auto_suggest_box/`

## 4. 视觉与布局规格

- 页面尺寸：`480 x 480`
- 根布局：`224 x 206`
- 页面结构：标题 -> 主 `auto_suggest_box` -> `compact / read only` 双预览
- 主控件尺寸：`196 x 34`
- 底部双预览行：`216 x 28`
- `compact` 预览：`104 x 28`
- `read only` 预览：`104 x 28`
- 视觉约束：
  - 保持浅色 page panel、白色输入面和轻边框
  - 展开列表只保留单层白底与低饱和高亮，不增加 showcase 装饰
  - focus 反馈维持轻量边框强调，不做 glow
  - `compact` 只收紧尺寸与可见项数量，不改变语义
  - `read only` 保留当前建议文本，但不承担交互职责

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `AutoSuggest Box` | 页面标题 |
| `control_primary` | `egui_view_autocomplete_t` | `196 x 34` | `Alicia Gomez` | 主建议框 |
| `control_compact` | `egui_view_autocomplete_t` | `104 x 28` | `Recent` | 紧凑静态预览 |
| `control_read_only` | `egui_view_autocomplete_t` | `104 x 28` | `Recent` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主建议框 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Alicia Gomez` | `Recent` | `Recent` |
| 展开态 | 列出当前 suggestions | 不响应 | 不响应 |
| 键盘选择 | `Down / End / Space / Enter` 闭环选择 | 不响应 | 不响应 |
| 主 snapshot 轮换 | 人员建议 -> 部署命令 | 保持 | 保持 |
| 紧凑 snapshot 轮换 | 保持 | `Recent` -> `Auto` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 保留静态只读对照 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主 snapshot、`compact` snapshot 和只读对照，并请求主控件焦点
2. 请求第一页默认截图
3. 程序化展开主 suggestions
4. 请求第二页展开态截图
5. 发送 `Down`，切换高亮建议项
6. 请求第三页键盘导航截图
7. 发送 `Space` 提交当前项并收起
8. 请求第四页提交结果截图
9. 程序化切换主 snapshot 到部署命令集
10. 请求第五页 snapshot 切换截图
11. 发送 `Enter` 展开，再发送 `End + Space` 选择最后一项
12. 请求第六页命令集提交结果截图
13. 程序化切换 `compact` 预览到第二个静态快照
14. 请求最终对照截图

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主建议框、展开列表和底部双预览必须完整可见
- 展开列表边框、文字和当前高亮项都要清晰可辨
- 主控件必须像标准 `AutoSuggestBox`，而不是状态展示卡片
- `compact` 与 `read only` 必须是静态对照，不再承担标签切换职责
- 页面中不再出现 guide、状态文案、standard label、section divider 和外部 preview label

## 9. 已知限制与后续方向

- 当前版本是固定 suggestions 数组，不做动态过滤
- 当前不支持输入中高亮匹配字串
- 当前不做多段 icon / category item
- 如后续沉入框架层，可继续补 placeholder、leading icon 和滚动长列表

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `combobox`：这轮重点是标准建议输入语义和 reference 页面收口，而不是基础 API 验证
- 相比 `textinput`：这里不强调自由文本编辑，而是固定建议项选择
- 相比 `menu_flyout`：这里是输入语义，不是命令面板
- 相比 `segmented_control`：这里是可展开建议列表，而不是互斥胶囊切换

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名与保留核心状态

- 对应组件名：`AutoSuggestBox`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `expanded`
  - `focused`

## 13. 相比参考原型删除的效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做标签点击轮换和外部状态桥接
- 不做搜索图标、异步结果刷新和复杂 hover / 阴影
- 不做带 icon / shortcut 的富建议项模板

## 14. EGUI 适配时的简化点与约束

- 直接复用 `autocomplete/combobox` 基础结构，避免新增重复基础控件
- 通过样式 helper 统一 Reference Track 颜色和尺寸
- 用程序化 `expand/collapse` 与键盘事件替代页面标签点击
- 先完成示例级 `AutoSuggestBox`，再决定是否继续补 placeholder / 过滤能力
