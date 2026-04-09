# scroll_bar 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI ScrollBar`
- 对应组件名：`ScrollBar`
- 本次保留状态：`standard`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态回显、`section divider`、外部 `preview` 标签、标签点击切换、场景化 demo 壳、复杂 hover 动效、auto-hide 滚动条、容器级惯性滚动
- EGUI 适配说明：保留“独立滚动条 + viewport 比例 + decrease / track / thumb / increase”核心语义，在 `480 x 480` 画布中优先保证主控件层级、底部 `compact / read only` 双预览和键盘 / 触摸闭环稳定

## 1. 为什么需要这个控件？
`scroll_bar` 用来表达“长内容窗口在整段内容中的位置与可视比例”。它适合文档浏览、日志查看、时间线、属性面板和列表侧边轨道等场景，强调 thumb 尺寸与位置都受 `content_length / viewport_length / offset` 驱动，而不是单纯数值选择。

## 2. 为什么现有控件不够用
- `slider` 更偏数值选择，thumb 大小固定，不表达 viewport 尺寸
- `scroll` 是容器行为，不是独立标准控件
- `progress_bar` 只有展示语义，没有按钮、track page 和 thumb drag
- `number_box` / `stepper` 偏离散数值编辑，不适合连续内容浏览位置

因此这里继续保留 `scroll_bar`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `scroll_bar`，覆盖 `Document rail`、`Timeline rail`、`Audit rail` 三组 snapshot
- 左下 `compact` 预览展示紧凑比例摘要
- 右下 `read only` 预览展示可见但不可交互的锁定对照
- 主控件保留真实键盘 `Tab / Up / Down / Home / End / +/- / Enter / Space / Esc` 闭环
- 主控件底层仍支持真实 touch：减按钮、轨道分页、thumb 拖拽
- 页面只保留标题、主 `scroll_bar` 和底部 `compact / read only` 双预览，不再保留旧版 `guide`、状态文本、分隔线和 label-click 场景切换

目录：
- `example/HelloCustomWidgets/input/scroll_bar/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 238`
- 页面结构：标题 -> 主 `scroll_bar` -> `compact / read only` 双预览
- 主控件：`196 x 146`
- 底部双预览容器：`216 x 52`
- `compact` 预览：`104 x 52`
- `read only` 预览：`104 x 52`
- 视觉规则：
  - 使用浅灰白 `page panel` 和低噪音白色滚动条卡片，避免旧版 demo chrome
  - 主控件保留 `label + helper + viewport preview + scroll rail` 四段层级
  - `compact` 预览收敛为静态比例摘要，不再承担标签切换职责
  - `read only` 预览通过只读调色板和输入屏蔽表达锁定态，而不是额外装饰

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 238` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Scroll Bar` | 页面标题 |
| `scroll_bar_primary` | `egui_view_scroll_bar_t` | `196 x 146` | `Document rail` | 标准主控件 |
| `scroll_bar_compact` | `egui_view_scroll_bar_t` | `104 x 52` | compact | 紧凑静态预览 |
| `scroll_bar_read_only` | `egui_view_scroll_bar_t` | `104 x 52` | compact + read only | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 `Document rail` | 是 | 是 | 否 |
| 切换到 `Timeline rail` | 是 | 否 | 否 |
| 切换到 `Audit rail` | 是 | 否 | 否 |
| 切换到第二组 `compact` 对照 | 否 | 是 | 否 |
| 键盘 `Down / + / End` | 是 | 否 | 否 |
| 触摸减按钮 / track / thumb | 是 | 否 | 否 |
| 紧凑摘要 | 否 | 是 | 是 |
| 只读锁定 | 否 | 否 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主控件和 `compact` 预览状态
2. 抓取首帧 `Document rail` reference
3. 通过 `Down` 展示 line step
4. 抓取第二帧步进结果
5. 通过 `+` 展示 page step
6. 抓取第三帧分页结果
7. 通过 `End` 跳到尾部
8. 抓取第四帧收尾位置
9. 程序化把主控件切到 `Timeline rail`
10. 抓取第五帧第二组主状态
11. 程序化把 `compact` 预览切到第二组比例摘要
12. 抓取最终收尾截图并保留静态 `read only` 对照

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/scroll_bar PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/scroll_bar --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部双预览必须完整可见，不能被裁切
- 主滚动条必须清楚表达 viewport 大小与位置，不能退化成普通 `slider`
- 键盘 `Down / + / End` 结果必须清晰可辨
- 页面中不再出现旧版 `guide`、状态回显、分隔线和外部 `preview` 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 风格的低噪音浅色 reference

## 9. 已知限制与后续方向
- 当前先做纵向 `ScrollBar` reference，不覆盖横向方向
- 当前不实现 auto-hide、overlay scrollbar 和 hover-only 扩宽
- 当前不直接驱动真实容器内容滚动，只表达标准滚动条语义
- 若后续下沉到框架层，再评估横向支持、容器绑定接口和更细的 accessibility 语义

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `slider`：这里的 thumb 尺寸由 viewport 决定，目标是定位内容窗口，不是选择抽象数值
- 相比 `scroll`：这里是独立标准控件，不负责 child layout、惯性或内容实际滚动
- 相比 `progress_bar`：这里有按钮、track page、thumb drag 与 focus part 语义
- 相比 `number_box` / `stepper`：这里强调连续内容浏览，不是离散数值编辑

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`WinUI ScrollBar`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`ScrollBar`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `decrease`
  - `track`
  - `thumb`
  - `increase`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做页面级 `guide`、状态文本、分隔线和外部 `preview` 标签
- 不做 auto-hide、overlay scrollbar、hover-only 扩宽和复杂阴影
- 不做容器级惯性滚动、跨轴联动和 overscroll 回弹
- 不做系统级 tooltip、辅助提示浮层和额外场景化演示文案

## 14. EGUI 适配时的简化点与约束
- 使用固定 snapshot 驱动 reference，优先保证 `480 x 480` 下的稳定审阅
- 主控件保留 `label + helper + viewport preview + rail` 四段结构
- `compact` 预览通过 `compact_mode + touch/key override` 固定为静态对照
- `read only` 预览通过 `read_only_mode + compact_mode` 固定为静态锁定态
