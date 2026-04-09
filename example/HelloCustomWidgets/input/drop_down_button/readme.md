# drop_down_button 自定义控件设计说明
## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`DropDownButton`
- 本次保留状态：`standard`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态回显、`section divider`、外部 `preview` 标签、标签点击切换、场景化 demo 壳、复杂 hover / pressed 动画、真实 flyout 定位与多级菜单
- EGUI 适配说明：保留“整个按钮都是下拉入口”的核心语义，在 `480 x 480` 画布中优先保证标题层级、主按钮信息密度、底部 `compact / read only` 双预览和键盘闭环稳定

## 1. 为什么需要这个控件？
`drop_down_button` 用来表达“单一入口打开一组选项”的标准命令按钮语义。它适合排序、布局切换、主题切换、过滤切换这类场景：用户点击整个按钮即可进入下拉动作，而不是像 `split_button` 那样区分主动作段和菜单段。

## 2. 为什么现有控件不够用
- `split_button` 强调“主动作 + 菜单段”双入口，不等于整个按钮统一展开
- `toggle_split_button` 还带 `checked` 语义，不适合纯下拉命令入口
- `menu_flyout` 是独立弹出菜单容器，不是页内按钮控件
- `command_bar` 承担的是工具栏语义，不是单个下拉按钮语义

因此这里继续保留 `drop_down_button`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `drop_down_button`，覆盖 `Sort`、`Layout`、`Theme` 三组主线 snapshot
- 左下 `compact` 预览展示紧凑尺寸下的静态对照
- 右下 `read only` 预览展示可见但不可交互的锁定对照
- 主控件保留真实触摸点击和 `Enter` 键触发闭环
- 页面只保留标题、主 `drop_down_button` 和底部 `compact / read only` 双预览，不再保留旧版 `guide`、状态文本、分隔线和 label-click 场景切换

目录：
- `example/HelloCustomWidgets/input/drop_down_button/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 160`
- 页面结构：标题 -> 主 `drop_down_button` -> `compact / read only` 双预览
- 主控件：`196 x 76`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`104 x 44`
- `read only` 预览：`104 x 44`
- 视觉规则：
  - 使用浅灰白 `page panel` 和低噪音白色按钮表面
  - 主控件保留 `title + row + helper` 三段式信息层级，明确“整块点击打开选项”
  - `compact` 预览收敛为静态紧凑摘要，不再承担演示切换职责
  - `read only` 预览通过只读调色板和输入屏蔽表达锁定态，而不是额外加重装饰

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 160` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Drop Down Button` | 页面标题 |
| `button_primary` | `egui_view_drop_down_button_t` | `196 x 76` | `Sort` | 标准主控件 |
| `button_compact` | `egui_view_drop_down_button_t` | `104 x 44` | `Sort` | 紧凑静态预览 |
| `button_read_only` | `egui_view_drop_down_button_t` | `104 x 44` | `Locked` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 `Sort` | 是 | 是 | 否 |
| 切换到 `Layout` | 是 | 否 | 否 |
| 切换到 `Theme` | 是 | 否 | 否 |
| 切换到 `Filter` | 否 | 是 | 否 |
| 触摸点击 | 是 | 否 | 否 |
| 键盘 `Enter` | 是 | 否 | 否 |
| 只读锁定 | 否 | 否 | 是 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主控件和 `compact` 预览状态
2. 抓取首帧 `Sort` reference
3. 通过主控件真实触摸点击切换到 `Layout`
4. 抓取第二帧触摸切换结果
5. 通过 `Enter` 键再切换到 `Theme`
6. 抓取第三帧键盘切换结果
7. 程序化把 `compact` 预览切到 `Filter`
8. 抓取最终收尾截图并保留静态 `read only` 对照

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/drop_down_button PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/drop_down_button --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部双预览必须完整可见，不能被裁切
- 主按钮必须被识别为单一点击面，而不是误读成 `split button`
- 主控件触摸点击和 `Enter` 键切换都必须生效
- 页面中不再出现旧版 `guide`、状态回显、分隔线和外部 `preview` 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 风格的低噪音浅色 reference

## 9. 已知限制与后续方向
- 当前先做示例级 `DropDownButton`，不实现真实 flyout 布局系统
- 当前 glyph 仍使用轻量字母占位，不接入真实图标资源
- `compact` 与 `read only` 预览只做静态对照，不承担交互职责
- 若后续下沉到框架层，再评估与 `button`、`menu_flyout` 的复用边界

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `split_button`：这里是整个按钮统一展开，没有主段 / 菜单段双入口
- 相比 `toggle_split_button`：这里没有 `checked` 复合语义
- 相比 `menu_flyout`：这里是按钮控件，不是独立弹出菜单容器
- 相比 `command_bar`：这里是单控件命令入口，不承载整条工具栏布局职责

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`DropDownButton`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `sort`
  - `layout`
  - `theme`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做页面级 `guide`、状态文本、分隔线和外部 `preview` 标签
- 不做真实 flyout 弹层、滚动菜单和多级 submenu
- 不做桌面级 hover / pressed 动画、复杂阴影和 Acrylic
- 不做真实图标资源、快捷键标记和额外徽标

## 14. EGUI 适配时的简化点与约束
- 用固定 snapshot 驱动 reference，优先保证 `480 x 480` 下的稳定审阅
- 主控件保留 `title + row + helper` 三段式信息层级
- `compact` 预览通过 touch/key override 固定为静态对照
- `read only` 预览通过 `read_only_mode + compact_mode` 固定为静态锁定态
