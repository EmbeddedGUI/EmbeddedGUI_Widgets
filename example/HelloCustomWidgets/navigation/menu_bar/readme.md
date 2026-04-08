# menu_bar 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源母本：`WPF UI`、`ModernWpf`
- 对应组件名：`MenuBar`
- 本次保留状态：`standard`、`compact`、`read only`、`active menu`、`dropdown panel`
- 删减效果：不做系统级 menubar 接管，不做复杂多级子菜单，不做原生快捷键分发，不做窗口级菜单栏
- EGUI 适配说明：保留顶层菜单分类、当前菜单高亮和锚定下拉面板，用静态 snapshot 替代复杂桌面菜单状态机

## 1. 为什么需要这个控件
`menu_bar` 用来表达桌面应用或复杂信息页里的“顶层命令分类 + 下拉命令面板”语义，适合 `File / Edit / View / Tools` 这类分组明确、动作较多的命令结构。

## 2. 为什么现有控件不够用
- `menu_flyout` 是局部弹出命令面板，不承担顶层常驻菜单栏语义
- `command_bar` 更像工具栏，强调主命令和 overflow，不强调顶层分类菜单
- `nav_panel` 是页面导航，不是命令型菜单
- `tab_strip` 负责页面切换，不是顶层菜单与下拉面板组合

## 3. 目标场景与示例概览
- 主卡展示标准桌面菜单栏：常驻顶部菜单、当前菜单高亮、下方命令面板
- 左下 `Compact` 预览保留 3 个顶层菜单与轻量下拉面板
- 右下 `Read only` 预览弱化为不可交互的菜单摘要
- 示例页结构收敛为标题、主 `menu_bar` 和 compact / read-only 双预览，不再保留页面级 guide、状态文案和 section label

目标目录：
- `example/HelloCustomWidgets/navigation/menu_bar/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主卡尺寸：`196 x 108`
- 底部预览区：`216 x 74`
- 单列预览卡：`104 x 74`
- 视觉规则：
  - 页面维持浅灰 page panel + 白色菜单卡的 Fluent 风格
  - 顶层菜单项使用轻量 underline 和弱填充，不做厚重桌面阴影
  - 下拉面板强调层级，但保持边框克制、留白稳定
  - `read only` 通过统一弱化 palette 表达锁定，不额外堆砌说明 chrome
  - 主页面不再保留额外说明控件，只保留菜单栏本体与双预览结构

## 5. 控件清单
| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Menu Bar` | 页面标题 |
| `menu_bar_primary` | `egui_view_menu_bar_t` | `196 x 108` | `File` 菜单展开 | 主菜单栏控件 |
| `menu_bar_compact` | `egui_view_menu_bar_t` | `104 x 74` | compact | 底部紧凑对照 |
| `menu_bar_locked` | `egui_view_menu_bar_t` | `104 x 74` | read only | 底部只读对照 |

## 6. 状态覆盖矩阵
| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 顶层菜单 + 下拉面板 | 3 个菜单 + 小面板 | 顶层菜单摘要 |
| current menu | 有 | 有 | 有 |
| panel focus row | 有 | 有 | 仅摘要 |
| disabled row | 有 | 可省略 | 仅弱化 |
| submenu arrow | 有 | 有（轻量） | 无需展开 |
| 锁定弱化 | 无 | 无 | 有 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 首帧记录 `File` 菜单展开态
2. 记录 `View` 顶层菜单按压态
3. 记录 `Open recent` row 按压态
4. 切到 `Compact / View`
5. 切到 `Compact / Tools`
6. 最后切到 `Tools` 菜单并激活当前 row

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/menu_bar PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/menu_bar --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
```

验收重点：
- 顶层菜单项居中、留白均匀，当前菜单的 underline 不偏心
- 下拉面板与顶部菜单的锚定关系清晰，不出现漂移
- panel row 的标题、meta、箭头三列对齐稳定
- compact / read only 区域仍然可读，不退化成拥挤小块
- 主页面不再出现大段空白页板，主卡与双预览比例稳定

## 9. 已知限制与下一轮迭代计划
- 当前仍以静态 snapshot 驱动，不实现真实多级子菜单状态机
- 命令激活只联动 snapshot / 状态变化，不做真实业务回调
- panel 宽度仍使用轻量估算，不做精确文本测量

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `menu_flyout`：这里是常驻顶层菜单栏，不是局部弹出菜单
- 相比 `command_bar`：这里强调菜单分类和下拉层级，不是工具栏按钮集
- 相比 `nav_panel`：这里是命令结构，不是页面导航结构
- 相比 `tab_strip`：这里不是页面切换，而是命令分组入口

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源母本：`WPF UI`、`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`MenuBar`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `active menu`
  - `dropdown panel`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做窗口级系统菜单融合
- 不做复杂 hover、pressed、nested submenu 过渡
- 不做多级阴影和 acrylic 材质
- 不做原生快捷键事件转发，只展示 meta 文本
- 不做页面级 guide、状态栏、section label 与额外说明 chrome

## 14. EGUI 适配时的简化点与约束
- 用 snapshot 数据驱动菜单与下拉内容，降低状态机复杂度
- 文本宽度先用近似规则控制，不引入复杂布局求解
- read only 版本保留摘要语义，避免小尺寸下重复绘制完整菜单
- 当前以 `HelloCustomWidgets` 示例优先，先验证结构、视觉与交互方向
