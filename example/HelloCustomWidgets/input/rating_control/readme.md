# rating_control 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`RatingControl`
- 本次保留状态：`standard`、`compact`、`read only`
- 删减效果：页面级 `guide`、状态回显、`section divider`、外部 `preview` 标签、标签点击切换、Acrylic、hover 动画、半星评分、真实 glyph 资源、桌面级 tooltip
- EGUI 适配说明：保留标准星级评分、页内 `Clear` 入口和键盘步进语义；在 `480 x 480` 下优先保证星形排布、标题留白和底部 `compact / read only` 双预览稳定

## 1. 为什么需要这个控件？
`rating_control` 用来表达标准页内评分语义，比如服务评价、交付速度、安装体验和反馈表单里的满意度选择。它比 `radio_button` 更接近用户熟悉的星级评分模型，也比 `slider` 更适合离散的 ordinal rating。

## 2. 为什么现有控件不够用
- `radio_button` 只能表达互斥选项，不具备星级评分的视觉语义
- `slider` 偏连续拖动，不适合 `1..5` 档位的离散评分
- `segmented_control` 更适合页内切换，不适合表达满意度或质量等级
- `number_box` 偏数值编辑，不是面向评价语义的标准评分控件

因此这里继续保留 `rating_control`，但示例页面必须收口到统一的 `Fluent 2 / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主区域展示标准 `rating_control`，覆盖 `Service quality`、`Delivery speed`、`Setup experience` 三组主线 snapshot
- 左下 `compact` 预览展示紧凑评分条的静态对照
- 右下 `read only` 预览展示只读弱化评分条
- 主控件保留真实触摸点击星级和 `Clear`
- 主控件保留 `Left / Right / Up / Down / Home / End / Tab / Enter / Space / Esc` 键盘闭环
- 页面只保留标题、主 `rating_control` 和底部 `compact / read only` 双预览，不再保留旧版 `guide`、状态回显、分隔线和 label-click 场景切换

目录：
- `example/HelloCustomWidgets/input/rating_control/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 174`
- 页面结构：标题 -> 主 `rating_control` -> `compact / read only` 双预览
- 主控件：`196 x 92`
- 底部双预览容器：`216 x 42`
- `compact` 预览：`104 x 42`
- `read only` 预览：`104 x 42`
- 视觉规则：
  - 采用浅灰 `page panel` + 白色评分卡，避免回到 showcase / HMI 风格
  - 星形继续使用暖金色 accent，保持标准 rating 语义
  - `Clear` 保留轻量页内入口，不再借助外部状态文本解释当前值
  - `compact` 与 `read only` 都收敛为静态低噪音 reference 对照

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 174` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Rating Control` | 页面标题 |
| `control_primary` | `egui_view_rating_control_t` | `196 x 92` | `Service quality` | 标准评分卡 |
| `control_compact` | `egui_view_rating_control_t` | `104 x 42` | `3 / 5` | 紧凑静态预览 |
| `control_read_only` | `egui_view_rating_control_t` | `104 x 42` | `4 / 5` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主评分卡 | Compact | Read only |
| --- | --- | --- | --- |
| 默认 `Service quality = 4/5` | 是 | 是 | 否 |
| 点击第 5 颗星 | 是 | 否 | 否 |
| 点击 `Clear` | 是 | 否 | 否 |
| 切换到 `Delivery speed` | 是 | 否 | 否 |
| 键盘 `End` 跳到最高评分 | 是 | 否 | 否 |
| 切换到第二组 `compact` 对照 | 否 | 是 | 否 |
| 静态对照 | 否 | 是 | 是 |
| 只读锁定 | 否 | 否 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主控件和 `compact` 预览状态
2. 抓取首帧 `Service quality` reference
3. 点击主卡第 5 颗星，展示触摸评分提交
4. 抓取第二帧触摸评分结果
5. 点击 `Clear`，展示真实 `part hit-testing`
6. 抓取第三帧清空结果
7. 切到 `Delivery speed` snapshot 并执行 `End`
8. 抓取第四帧键盘跳转结果
9. 程序化切到第二组 `compact` 预览
10. 抓取最终收尾截图并保留静态 `read only` 对照

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rating_control --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主卡和底部双预览必须完整可见，不能被裁切
- 主卡标题、星形、caption、低高标签与 `Clear` 之间留白平衡
- 星形焦点 ring 需要可辨识，但不能变成高噪音装饰
- 页面中不再出现旧版 `guide`、状态回显、分隔线和外部 `preview` 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 风格的低噪音浅色 reference

## 9. 已知限制与后续方向
- 当前只支持整星评分，不做半星或浮点评分
- 当前 `Clear` 是轻量页内入口，不弹出二次确认
- 当前 `compact` 与 `read only` 预览只做静态对照，不承担交互职责
- 若后续下沉到框架层，再评估与通用 focus / form 体系的对接

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `radio_button`：这里是标准星级评分，不是单纯表单互斥项
- 相比 `slider`：这里表达离散等级，不是连续区间拖动
- 相比 `segmented_control`：这里是评价语义，不承担页内切换
- 相比 `number_box`：这里表达质量 / 满意度档位，而不是数值输入

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`RatingControl`
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `clear action`
  - `keyboard rating`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做页面级 `guide`、状态回显、分隔线和外部 `preview` 标签
- 不做 hover glow、桌面级 pointer over 动画和系统级 tooltip
- 不做半星、caption 图标、Acrylic 和复杂阴影扩散
- 不接入真实 emoji / glyph 资源，星形继续用轻量绘制

## 14. EGUI 适配时的简化点与约束
- 用固定 `1..5` 星级 snapshot 驱动 reference，优先保证 `480 x 480` 下排布稳定
- 标准态保留 `title + stars + caption + low/high + clear` 五段结构
- `compact` 预览通过 `compact_mode + touch/key override` 固定为静态对照
- `read only` 预览通过 `read_only_mode + compact_mode` 固定为静态锁定态
