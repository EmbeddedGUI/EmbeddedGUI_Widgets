# CounterBadge 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`CounterBadge`
- 本次保留语义：`count`、`overflow`、`dot`，以及 `compact / read_only` 静态 preview
- 本次删除内容：宿主头像绑定、命令面板联动、故事化场景壳层和与计数提醒无关的额外交互
- EGUI 适配说明：复用 SDK `notification_badge` 的计数格式化与基础背景绘制，在 custom 层补齐 `CounterBadge` 的数字提醒语义、outline 和静态 preview API，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`CounterBadge` 用来表达“某个入口上还挂着多少条未处理信息”这一类极小面积数字提醒。它常附着在通知入口、收件箱标签、任务卡片角落或工作区入口旁边，强调的是“数量仍在增长”或“仍需注意”，而不是块级告警。

当前仓库虽然已经有 `badge`、`info_badge` 和 `presence_badge`，但还缺少一个对齐 Fluent 2 `CounterBadge` 语义的独立数字提醒控件，因此需要补齐。

## 2. 为什么现有控件不够用？
- `badge` 更偏文本状态标签，承担的是内容分类或状态描述，不是附着式的数字提醒。
- `info_badge` 关注 `count / icon / attention dot` 的信息提示语义，噪音更高，也不强调 `CounterBadge` 的宿主角标定位和 overflow 形态。
- `presence_badge` 表达的是在线状态，不承载数量，也不会进入 `99+` 这类计数溢出语义。

## 3. 目标场景与示例概览
- 主面板录制三组 snapshot：
  - `Inbox queue`：单数字 `7`
  - `Escalation queue`：溢出 `99+`
  - `Quiet watch`：dot mode
- 底部保留两个静态 preview：
  - `compact`：更小尺寸的 `4`
  - `read_only`：弱化后的 `12`
- 页面结构统一收口为：标题 -> 主参考面板 -> 两个静态 preview

目标目录：`example/HelloCustomWidgets/display/counter_badge/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主面板：`196 x 96`
- 主控件：`42 x 24`
- 底部容器：`216 x 68`
- 单个 preview 面板：`104 x 68`

视觉原则：
- 继续保持浅灰页面、白色承载面和低噪音边框。
- `CounterBadge` 只承担数字提醒语义，不额外承担宿主布局、命令交互或头像语义。
- 单数字和 `99+` 都保持紧凑角标外观，dot mode 继续保留最小提醒信号。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 96` | 主展示面板 |
| `primary_badge` | `egui_view_counter_badge_t` | `42 x 24` | 主计数角标 |
| `compact_badge` | `egui_view_counter_badge_t` | `18 x 16` | 紧凑静态 preview |
| `read_only_badge` | `egui_view_counter_badge_t` | `22 x 16` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `count` | 是 | 是 | 是 |
| `overflow` | 是 | API 保留 | API 保留 |
| `dot` | 是 | API 保留 | API 保留 |
| `compact_mode` | 关闭 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| 接收焦点 / 交互 | 否 | 否 | 否 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `CounterBadge` 本轮定义为纯 display 控件，不承担点击、导航或键盘激活职责。
- 主面板状态切换通过程序化 snapshot 完成，不依赖用户输入。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，避免 reference 页面里误接管输入。

## 8. 本轮收口内容
- 新增 `egui_view_counter_badge.h/.c`
- 提供：
  - `set_count()` / `get_count()`
  - `set_max_display()` / `get_max_display()`
  - `set_dot_mode()` / `get_dot_mode()`
  - `set_palette()`
  - `set_compact_mode()` / `get_compact_mode()`
  - `set_read_only_mode()` / `get_read_only_mode()`
  - `get_badge_region()`
  - `override_static_preview_api()`
- demo 页面接入主状态轮播、compact preview 和 read_only preview
- 单测覆盖默认初始化、setter、overflow / dot 几何 helper 和静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Inbox queue`
2. 抓取第一帧
3. 切到 `Escalation queue`
4. 抓取第二帧
5. 切到 `Quiet watch`
6. 抓取第三帧
7. 恢复初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/counter_badge PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/counter_badge --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/counter_badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_counter_badge
```

验收重点：
- 主控件和两个 preview 都必须完整可见，不黑屏、不白屏、不裁切。
- 单数字、`99+` 和 dot 三个关键 snapshot 必须有清晰差异。
- `99+` 不允许因为溢出被裁剪。
- `read_only` preview 需要明显弱化，但仍能辨认数字提醒语义。

## 11. 已知限制
- 当前只覆盖独立 `CounterBadge`，不负责和头像、卡片或命令入口做宿主绑定。
- dot mode 先收口为单色实心点，不继续扩展动画或宿主吸附逻辑。
- 本轮不下沉到 `src/widget/`，先保持在 `HelloCustomWidgets` 的 `reference` 维护范围内。
