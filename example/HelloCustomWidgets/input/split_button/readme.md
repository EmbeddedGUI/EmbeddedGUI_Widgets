# split_button 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`SplitButton`
- 本次保留状态：`standard`、`compact`、`disabled`、`primary part`、`menu part`
- 删除效果：页面级 guide、状态回显、外部 preview 标签、section divider、真实下拉菜单弹层、桌面级 hover / pressed 动画、Acrylic 与复杂命令栏包装
- EGUI 适配说明：保留“主动作 + 菜单入口”的复合按钮语义与主段 / 菜单段键盘切换，在 `480 x 480` 页面里优先保证双段边界、标题层级和底部双预览对照稳定

## 1. 为什么需要这个控件

`split_button` 用来表达“存在一个默认主动作，同时还有更多动作挂在菜单入口”的标准复合按钮。它适合保存、分享、导出、归档这类高频命令：默认点击直接执行主动作，右侧菜单段保留更多选择。

## 2. 为什么现有控件不够用

- 普通 `button` 只有单一动作，不表达“默认动作 + 更多动作”的双入口
- `menu_flyout` 是独立弹出面板，不是页内复合按钮
- `command_bar` 承担的是工具栏语义，不是单个复合按钮语义
- `drop_down_button` 只有菜单入口，没有主段 / 菜单段并存的 split 结构

因此这里继续保留 `split_button`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `split_button`，保留 `save / share / export / archive` 一组主线 snapshot
- 左下 `compact` 预览展示紧凑尺寸下的轻量 split button
- 右下 `disabled` 预览展示禁用态 split button
- 主控件保留 `Left / Right / Tab / Home / End` 键盘切换主段 / 菜单段
- 示例页只保留标题、主 `split_button` 和底部 `compact / disabled` 双预览，不再保留 guide、状态回显和外部标签

目录：

- `example/HelloCustomWidgets/input/split_button/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 154`
- 页面结构：标题 -> 主 `split_button` -> `compact / disabled` 双预览
- 主控件：`196 x 74`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`104 x 44`
- `disabled` 预览：`104 x 44`
- 视觉规则：
  - 使用浅灰白 page panel + 白底低噪音表面
  - 主控件保留 SplitButton 的左右双段结构、分隔线和独立菜单入口
  - `compact` 预览保留相同双段语义，但减少信息密度
  - `disabled` 预览弱化对比度，但仍保留完整 split 结构

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 154` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Split Button` | 页面标题 |
| `button_primary` | `egui_view_split_button_t` | `196 x 74` | `Save draft` | 标准主控件 |
| `button_compact` | `egui_view_split_button_t` | `104 x 44` | `Quick / Save` | 紧凑静态预览 |
| `button_disabled` | `egui_view_split_button_t` | `104 x 44` | `Locked / Publish` | 禁用静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Disabled |
| --- | --- | --- | --- |
| `primary part` | 是 | 是 | 是 |
| `menu part` | 是 | 是 | 是 |
| 主线 snapshot 轮换 | 是 | 否 | 否 |
| 触摸切换分段 | 是 | 否 | 否 |
| 键盘 `Left / Right / Tab / Home / End` | 是 | 否 | 否 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主控件与 `compact` 预览状态
2. 请求第一页截图
3. 通过 `Right` 把主控件焦点切到菜单段
4. 请求第二页截图
5. 程序化切换主控件到 `Share handoff`
6. 请求第三页截图
7. 通过 `Left` 把主控件焦点切回主段
8. 请求第四页截图
9. 程序化切换主控件到 `Archive page`，并把 `compact` 预览切换到第二组静态对照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/split_button PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/split_button --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件、`compact` 和 `disabled` 三个 split button 都必须完整可见，不能被裁切
- 主段与菜单段边界必须清晰，不能糊成单按钮
- 标题、标签和辅助文案要保持 Fluent / WPF UI 的低噪音层级
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `disabled` 预览必须一眼可辨，但仍保留 split button 结构
- `snapshots / current snapshot / current part / compact / disabled` 切换链路需要共用同一套 `pressed` 清理语义
- setter 更新、模式切换和 `touch cancel` 后，不能残留 `primary / menu` 的 `pressed` 高亮
- `compact / disabled_mode / !enable` 收到新的 touch 或 key 输入时，必须先清理残留 pressed，再拒绝后续交互
- 触摸释放语义必须继续满足“按下与抬起命中同一分段才提交”
- unit test 中已有的主段 / 菜单段切换、`compact / disabled` 输入抑制和 `touch cancel` 语义不能回归

## 9. 已知限制与后续方向

- 当前只保留菜单入口语义，不弹出真实 flyout
- 当前 glyph 使用双字母占位，不接真实图标资源
- 当前 `compact` 与 `disabled` 仅作为静态对照，不承担交互职责
- 若后续要沉入框架层，再单独评估与 `button`、`menu_flyout` 的复用边界

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `button`：本控件是复合双入口，不是单动作按钮
- 相比 `drop_down_button`：本控件同时保留主动作段和菜单入口
- 相比 `menu_flyout`：本控件是页内复合按钮，不是独立弹出命令面板
- 相比 `command_bar`：本控件是单个复合命令，不承担整条工具栏语义

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`SplitButton`
- 本次保留状态：
  - `standard`
  - `compact`
  - `disabled`
  - `primary part`
  - `menu part`

## 13. 相比参考原型删除了哪些效果或装饰

- 不做页面级 guide、状态回显、外部 preview 标签和 section divider
- 不做真实菜单弹层和二级命令列表
- 不做桌面级 hover、pressed、阴影扩散和 Acrylic
- 不做系统级图标资源、快捷键标签和复杂主题联动

## 14. EGUI 适配时的简化点与约束

- 用固定 snapshot 驱动，优先保证 `480 x 480` 页面里的稳定 reference
- 主控件保留 `title + split row + helper` 三段结构
- 底部 `compact` 与 `disabled` 固定为静态对照，不再承担额外交互
- 交互收口阶段统一要求 setter、模式切换、禁用 guard 和 `touch cancel` 都能清理残留 `pressed`，确保交互后的渲染稳定
- 先完成示例级审阅稳定性，再决定是否抽象到框架公共层
