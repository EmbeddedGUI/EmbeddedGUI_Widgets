# pips_pager 自定义控件设计说明

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件语义：`PipsPager`
- 本次保留状态：`standard`、`compact`、`read only`、`current page`
- 删减效果：页面级 guide / 状态文案 / section label、额外预览标签、重阴影和 showcase 式说明区
- EGUI 适配说明：保留标题、helper、previous/next、pips rail 和当前页强调，用固定尺寸容器优先保证 `480 x 480` 下的分页导航可读性

## 1. 为什么需要这个控件

`pips_pager` 用来表达离散页码切换、短轨道预览和 onboarding 式分页导航。它比 `tab_strip` 更适合少量离散页面，也比 `flip_view` 更强调页码位置反馈本身。

## 2. 为什么现有控件不够用

- `tab_strip` 更偏平级 section 标签切换，不适合离散页码表达
- `flip_view` 强调 hero content 与翻页卡片，不以 pips rail 为主语义
- `scroll_bar` 连续值语义更强，不适合离散页分页
- 当前主线需要一版克制、浅色、低噪音的标准 `PipsPager`

## 3. 目标场景与示例概览

- 主卡展示标准 `pips_pager`，覆盖 `Onboarding`、`Gallery`、`Report deck` 三组分页快照
- 左下 `Compact` 预览展示小尺寸下的短轨道分页
- 右下 `Read only` 预览展示冻结后的只读分页状态
- 示例页结构收敛为标题、主 `pips_pager` 和 compact / read-only 双预览，不再保留页面级 guide、状态文案和 preview label

目标目录：

- `example/HelloCustomWidgets/navigation/pips_pager/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 198`
- 主分页卡：`196 x 92`
- 底部双预览容器：`216 x 58`
- `Compact` / `Read only` 预览：`104 x 58`
- 视觉规则：
  - 使用浅灰 page panel + 白底轻边框分页卡
  - 主卡保留 title、helper、previous/next 与 pips rail 的完整层级
  - 当前页使用低饱和蓝色强调，不做厚重高亮
  - `Compact` 与 `Read only` 直接通过控件自身模式表达，不再依赖外部标签说明
  - 页面只保留控件本体和双预览，不再堆叠说明性 chrome

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 198` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Pips Pager` | 页面标题 |
| `pager_primary` | `egui_view_pips_pager_t` | `196 x 92` | `Onboarding / 2 of 7` | 标准分页卡 |
| `pager_compact` | `egui_view_pips_pager_t` | `104 x 58` | compact | 紧凑分页预览 |
| `pager_locked` | `egui_view_pips_pager_t` | `104 x 58` | read only | 只读弱化预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Onboarding` | `Compact / 3 of 6` | `Read only / 4 of 7` |
| current page | `Right` / `End` 切换页码 | 录制态切换另一组 compact snapshot | 固定 |
| previous / next | 有 | 有 | 冻结 |
| pips rail | 有 | 有 | 有 |
| read only | 无 | 无 | 有 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 首帧等待并截图，确认默认 `Onboarding`
2. 发送 `Right` 键，验证主卡切到下一页
3. 发送 `End` 键，验证尾页边界
4. 程序化切换主卡 snapshot 到下一组分页数据
5. 程序化切换 compact 预览 snapshot
6. 每次切换后请求精确截图，确保 runtime 直接抓到稳定状态

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
```

验收重点：

- 主分页卡和底部双预览都必须完整可见，不能再出现大段空白页板
- title、helper、previous/next 和 pips rail 需要同时可辨识
- 当前页强调要清楚，但整体不能回到高噪音 showcase 风格
- `Compact` 在小尺寸下仍要清楚看出当前页位置
- `Read only` 弱化后仍能识别页码和边界状态

## 9. 已知限制与后续方向

- 当前版本采用固定分页快照，不接真实业务内容
- 当前录制主要覆盖分页切换与 snapshot 切换，不覆盖更复杂动画
- 当前主页面只验证分页卡本体，不联动外部内容区域

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `tab_strip`：这里强调离散页码反馈，不是 section 标签条
- 相比 `flip_view`：这里以 pips rail 为中心，不以 hero card 为中心
- 相比 `scroll_bar`：这里是离散分页，不是连续范围值
- 相比 `menu_bar` / `nav_panel`：这里是页码导航，不是命令或常驻导航

## 11. 参考设计系统与开源母本

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`PipsPager`
- 本次保留状态：
  - `standard`
  - `current page`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉了哪些效果或装饰

- 不做页面级 guide、状态栏、preview label 与额外说明控件
- 不做重阴影、Acrylic 和高装饰容器
- 不接入真实业务面板，只保留分页导航壳层
- 不做复杂动画编排，只保留必要的分页切换反馈

## 14. EGUI 适配时的简化点与约束

- 使用固定 snapshot 数据，优先保证 `480 x 480` 下的审阅效率
- compact 与 read-only 直接复用同一控件模式，避免多套壳层说明
- 通过程序化录制切换 snapshot，避免依赖隐藏标签触发
- 先完成 reference 版分页导航，再决定是否上升到框架层
