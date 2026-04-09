# toggle_split_button 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI ToggleSplitButton`
- 对应组件名：`ToggleSplitButton`
- 本次保留状态：`standard`、`compact`、`read only`、`checked`、`unchecked`、`primary part`、`menu part`
- 删除效果：页面级 guide、状态回显、外部 preview 标签、section divider、场景化轮播入口、复杂 hover / pressed 动画、Acrylic 和完整菜单弹层
- EGUI 适配说明：保留 checked 主动作与菜单入口并存的复合语义，在 `480 x 480` 页面里优先保证双段边界、`ON/OFF` 徽标和底部双预览对照稳定

## 1. 为什么需要这个控件

`toggle_split_button` 用来表达“主按钮本身是持续状态，同时右侧还保留更多动作入口”的标准复合控件。它适合告警路由、同步监控、关注订阅、录制开关这类既要保留 on/off 状态，又要切换附加模式的页面。

## 2. 为什么现有控件不够用

- `toggle_button` 只有 checked 主动作，没有菜单入口
- `split_button` 有主动作和菜单入口，但不保留 checked 语义
- `menu_flyout` 是独立弹出面板，不是页内二段式按钮
- 当前 reference 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleSplitButton`

因此这里继续保留 `toggle_split_button`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `toggle_split_button`，保留 checked/unchecked 与主段/菜单段双重语义
- 左下 `compact` 预览展示紧凑尺寸下的轻量 reference
- 右下 `read only` 预览展示静态只读对照
- 主控件保留 `Left / Right / Tab / Enter / Space / Plus / Minus / Escape` 键盘闭环
- 示例页只保留标题、主 `toggle_split_button` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和外部标签

目录：

- `example/HelloCustomWidgets/input/toggle_split_button/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 160`
- 页面结构：标题 -> 主 `toggle_split_button` -> `compact / read only` 双预览
- 主控件：`196 x 80`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`104 x 44`
- `read only` 预览：`104 x 44`
- 视觉规则：
  - 使用浅灰白 page panel + 白底低噪音表面
  - 主控件保留 `ON/OFF` badge、主段 / 菜单段边界和右侧菜单入口
  - `compact` 预览保留同样的双段 checked 语义，但减少信息密度
  - `read only` 预览只保留静态状态表达，不承担真实交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 160` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toggle Split Button` | 页面标题 |
| `button_primary` | `egui_view_toggle_split_button_t` | `196 x 80` | `Alert routing / On` | 标准主控件 |
| `button_compact` | `egui_view_toggle_split_button_t` | `104 x 44` | `Quick / On` | 紧凑静态预览 |
| `button_read_only` | `egui_view_toggle_split_button_t` | `104 x 44` | `Locked / On` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| `checked / unchecked` | 是 | 是 | 是 |
| `primary part` | 是 | 是 | 是 |
| `menu part` | 是 | 是 | 是 |
| 主线 snapshot 轮换 | 是 | 否 | 否 |
| 触摸激活 | 是 | 否 | 否 |
| 键盘切换 | 是 | 否 | 否 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主控件与 `compact` 预览状态
2. 请求第一页截图
3. 通过 `Space` 切换主控件 checked 状态
4. 请求第二页截图
5. 通过 `Right + Enter` 把主控件切到菜单段并切换到下一组 snapshot
6. 请求第三页截图
7. 程序化切换主控件到 `Follow thread`，并通过 `Left` 回到主段
8. 请求第四页截图
9. 程序化切换主控件到 `Record scene`，并把 `compact` 预览切换到第二组静态对照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_split_button PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_split_button --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部双预览必须完整可见，不能被裁切
- `ON/OFF` 徽标、主段 / 菜单段边界和 chevron 必须清晰可辨
- checked 与 unchecked 两态必须一眼可辨，不能退回普通 split button
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 的低噪音浅色 reference

## 9. 已知限制与后续方向

- 当前只保留菜单入口语义，不弹出真实 flyout
- 当前 glyph 使用双字母占位，不接真实图标资源
- 当前 `compact` 与 `read only` 仅作为静态对照，不承担交互职责
- 若后续要沉入框架层，再单独评估与 `toggle_button`、`split_button`、`menu_flyout` 的复用边界

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `toggle_button`：本控件额外保留菜单入口
- 相比 `split_button`：本控件额外保留 checked 语义
- 相比 `menu_flyout`：本控件是页内复合按钮，不是独立弹出面板
- 相比 `command_bar`：本控件是单个复合命令，不承担整条工具栏语义

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`WinUI ToggleSplitButton`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`ToggleSplitButton`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `checked`
  - `unchecked`
  - `primary part`
  - `menu part`

## 13. 相比参考原型删除了哪些效果或装饰

- 不做页面级 guide、状态回显、外部 preview 标签和 section divider
- 不做真实菜单弹层和二级命令列表
- 不做桌面级 hover、pressed、阴影扩散和 Acrylic
- 不做系统级图标资源、快捷键标签和复杂主题联动

## 14. EGUI 适配时的简化点与约束

- 用固定 snapshot 驱动，优先保证 `480 x 480` 页面里的稳定 reference
- 主控件保留 `title + checked badge + split row + helper` 四段结构
- 底部 `compact` 与 `read only` 固定为静态对照，不再承担额外交互
- 先完成示例级审阅稳定性，再决定是否抽象到框架公共层
