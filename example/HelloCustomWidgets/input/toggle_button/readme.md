# toggle_button 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI ToggleButton`
- 对应组件名：`ToggleButton`
- 本次保留状态：`standard`、`compact`、`read only`、`on`、`off`
- 删除效果：页面级 guide、状态回显、`Standard` / `Compact` / `Read only` 外部标签、section divider、场景化轮播入口、复杂 hover / shadow / Acrylic 表达
- EGUI 适配说明：保留单按钮 checked 语义、图标 + 文本组合和键盘 `Enter / Space` 闭环，在 `480 x 480` 页面里优先保证主按钮识别度与底部双预览对照稳定

## 1. 为什么需要这个控件

`toggle_button` 适合表达“按钮自身就是持续状态”的页内命令，例如提醒开关、可见性切换、收藏态和固定态。它不是设置页拨杆，也不是带菜单的复合按钮，而是最小可用的 checked command。

## 2. 为什么现有控件不够用

- `switch` 更像设置项中的开关拨杆，不强调命令按钮语义
- `button` 只有瞬时动作，不保留 checked 状态
- `split_button` 与 `toggle_split_button` 都引入了额外分段或菜单入口，信息密度更高
- 当前 reference 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `ToggleButton`

因此这里继续保留 `toggle_button`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `toggle_button`，保留图标 + 文本 + on/off 切换
- 左下 `compact` 预览展示紧凑尺寸下的轻量 reference
- 右下 `read only` 预览展示静态只读对照
- 示例页只保留标题、主按钮和底部 `compact / read only` 双预览，不再保留 guide、状态回显和标签点击入口

目录：

- `example/HelloCustomWidgets/input/toggle_button/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 142`
- 页面结构：标题 -> 主 `toggle_button` -> `compact / read only` 双预览
- 主按钮：`196 x 52`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`104 x 44`
- `read only` 预览：`104 x 44`
- 视觉规则：
  - 使用浅灰白 page panel + 白底低噪音表面
  - 主按钮保留 Fluent 风格强调色，off 态退回浅色 surface
  - `compact` 预览保留相同图标 + 文本语义，但尺寸更轻
  - `read only` 预览只保留静态状态表达，不承担真实交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 142` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Toggle Button` | 页面标题 |
| `button_primary` | `egui_view_toggle_button_t` | `196 x 52` | `Alerts / On` | 标准主按钮 |
| `button_compact` | `egui_view_toggle_button_t` | `104 x 44` | `Visible / On` | 紧凑静态预览 |
| `button_read_only` | `egui_view_toggle_button_t` | `104 x 44` | `Pinned / On` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主按钮 | Compact | Read only |
| --- | --- | --- | --- |
| `on / off` | 是 | 是 | 是 |
| 图标 + 文本 | 是 | 是 | 是 |
| 主线 snapshot 轮换 | 是 | 否 | 否 |
| 触摸切换 | 是 | 否 | 否 |
| 键盘 `Enter / Space` | 是 | 否 | 否 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主按钮与 `compact` 预览状态
2. 请求第一页截图
3. 通过 `Space` 切换主按钮到 `Alerts / Off`
4. 请求第二页截图
5. 程序化切换主按钮到 `Visible / Off`
6. 请求第三页截图
7. 通过 `Enter` 切换主按钮到 `Visible / On`
8. 请求第四页截图
9. 程序化切换主按钮到 `Favorite / On`，并把 `compact` 预览切换到第二组静态对照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_button PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_button --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主按钮 on/off 两态必须一眼可辨，不能退回普通按钮视觉
- 主按钮和底部双预览必须完整可见，不能被裁切
- 图标、文本和左右留白必须均衡，不能出现图标压字或文本贴边
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 的低噪音浅色 reference

## 9. 已知限制与后续方向

- 当前版本只保留最小 checked command，不覆盖 toolbar group、menu button 组合关系
- 当前 `read only` 通过吞掉输入事件实现静态对照，不额外引入独立 API
- 当前不做 hover、focus ring、Acrylic 和系统级主题动画
- 若后续要沉入框架层，再单独评估与命令栏、导航栏和表单系统的联动

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `switch`：本控件是页内命令按钮，不是设置页拨杆
- 相比 `button`：本控件保留 checked 状态，而不是一次性触发
- 相比 `split_button`：本控件没有下拉入口
- 相比 `toggle_split_button`：本控件只保留单入口切换，不保留复合分段

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`WinUI ToggleButton`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`ToggleButton`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `on`
  - `off`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态回显、外部 preview 标签和 section divider
- 不做 hover 阴影、Acrylic、系统主题切换过渡和复杂焦点动画
- 不做 toolbar / command bar 级组合布局
- 不做场景化轮播入口，只保留程序化 reference snapshot

## 14. EGUI 适配时的简化点与约束

- 基于现有 `egui_view_toggle_button` 核心控件做示例级 reference 收口，不下沉到框架层
- 颜色与圆角由当前目录的样式包装统一收敛
- 主按钮保留最小键盘闭环，底部 `compact` 与 `read only` 固定为静态对照
- 先完成示例级审阅稳定性，再评估是否抽象出更完整的 read-only / focus API
