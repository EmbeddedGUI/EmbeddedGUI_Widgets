# swipe_control 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI SwipeControl`
- 对应组件名：`SwipeControl`
- 本次保留状态：`standard`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态回显、`section divider`、外部 `preview` 标签、标签点击切换、场景化 demo 壳、连续 easing 动画、多 action stack、真实列表容器滚动
- EGUI 适配说明：保留“单行内容默认收起，仅在 reveal 后暴露左右操作 rail”的核心语义，在 `480 x 480` 画布中优先保证主行层级、底部 `compact / read only` 双预览和键盘 / touch 闭环稳定

## 1. 为什么需要这个控件？
`swipe_control` 用来表达“列表行内容默认保持整洁，只在用户明确 reveal 时暴露上下文操作”的标准交互语义。它适合消息收件箱、任务队列、审核列表这类以单行内容为主、但又需要快速操作的场景。

## 2. 为什么现有控件不够用
- `settings_panel` 和 `data_list_panel` 更偏静态信息行，不承担 reveal action 语义
- `split_button` / `toggle_split_button` 强调按钮级复合入口，不是整行内容卡的侧滑暴露
- `flip_view` 强调整卡翻页，不是单行列表项上的局部 action reveal
- 普通 `card` 或 `button` 无法表达 `surface / start action / end action` 三段状态切换

因此这里继续保留 `swipe_control`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `swipe_control`，覆盖 `Inbox`、`Planner`、`Review` 三组主线 row snapshot
- 左下 `compact` 预览展示小尺寸比例压缩后的静态对照
- 右下 `read only` 预览展示可见但不可交互的锁定态
- 主控件保留真实 `Right / Left / Tab / Enter / Space / Escape` reveal 闭环
- 主控件底层仍支持真实 touch：surface swipe reveal、surface tap close、action 点击
- 页面只保留标题、主 `swipe_control` 和底部 `compact / read only` 双预览，不再保留旧版 `guide`、状态栏、分隔线和 label-click 场景切换

目录：
- `example/HelloCustomWidgets/input/swipe_control/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 222`
- 页面结构：标题 -> 主 `swipe_control` -> `compact / read only` 双预览
- 主控件：`196 x 118`
- 底部双预览容器：`216 x 64`
- `compact` 预览：`104 x 64`
- `read only` 预览：`104 x 64`
- 视觉规则：
  - 使用浅灰白 `page panel` 和低噪音白色 row 容器，避免旧版 demo chrome
  - 主控件保留 `title + helper + surface row + reveal action rail` 四段层级
  - `compact` 预览收敛为静态小尺寸对照，不再承担标签切换职责
  - `read only` 预览通过锁定调色板和输入屏蔽表达冻结态，而不是额外装饰

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 222` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Swipe Control` | 页面标题 |
| `swipe_control_primary` | `egui_view_swipe_control_t` | `196 x 118` | `Inbox` | 标准主控件 |
| `swipe_control_compact` | `egui_view_swipe_control_t` | `104 x 64` | compact | 紧凑静态预览 |
| `swipe_control_read_only` | `egui_view_swipe_control_t` | `104 x 64` | compact + read only | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 `Inbox` | 是 | 是 | 否 |
| `Right` reveal start action | 是 | 否 | 否 |
| `Left` reveal end action | 是 | 否 | 否 |
| 切换到 `Planner` row | 是 | 否 | 否 |
| 切换到第二组 `compact` 对照 | 否 | 是 | 否 |
| surface tap close | 是 | 否 | 否 |
| 紧凑静态对照 | 否 | 是 | 是 |
| 只读锁定 | 否 | 否 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主控件和 `compact` 预览状态
2. 抓取首帧 `Inbox` surface reference
3. 通过 `Right` 键 reveal 起始侧 action
4. 抓取第二帧 start action 结果
5. 通过 `Left` 键切到末端 action
6. 抓取第三帧 end action 结果
7. 程序化把主控件切到 `Planner`
8. 抓取第四帧第二组主状态
9. 程序化把 `compact` 预览切到第二组静态对照
10. 抓取最终收尾截图并保留静态 `read only` 对照

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/swipe_control PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/swipe_control --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部双预览必须完整可见，不能被裁切
- 主行 surface 与左右 action rail 必须层级清晰，不能退化成普通卡片或按钮组
- 键盘 `Right / Left` reveal 结果必须清晰可辨
- 页面中不再出现旧版 `guide`、状态回显、分隔线和外部 `preview` 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 风格的低噪音浅色 reference

## 9. 已知限制与后续方向
- 当前先做单行 `SwipeControl` reference，不接入真实列表容器
- 当前不实现连续像素级 reveal 动画和多 action stack
- 当前 action rail 用文本而不是图标资源表达操作
- 若后续下沉到框架层，再评估与列表容器、拖动阈值和批量操作的复用边界

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `settings_panel` / `data_list_panel`：这里的核心是 reveal action，而不是静态信息行
- 相比 `split_button` / `toggle_split_button`：这里的入口是整行 surface，不是按钮本体
- 相比 `flip_view`：这里不做整卡翻页，只做单行局部 action expose
- 相比普通 `card`：这里明确保留 `surface / start action / end action` 三段状态语义

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`WinUI SwipeControl`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`SwipeControl`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `surface`
  - `start action`
  - `end action`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做页面级 `guide`、状态栏、分隔线和外部 `preview` 标签
- 不做连续 easing 动画、多 action stack 和真实列表滚动容器
- 不做真实图标资源、阴影层叠和大面积场景化叙事
- 不做批量操作工具栏和跨行联动

## 14. EGUI 适配时的简化点与约束
- 使用固定 row snapshot 驱动 reference，优先保证 `480 x 480` 下的稳定审阅
- 主控件保留 `title + helper + surface row + action rail` 四段结构
- `compact` 预览通过 `compact_mode + touch/key override` 固定为静态对照
- `read only` 预览通过 `read_only_mode + compact_mode` 固定为静态锁定态
