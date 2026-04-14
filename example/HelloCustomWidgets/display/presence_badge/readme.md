# PresenceBadge 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`PresenceBadge`
- 本次保留语义：`available / busy / away / do_not_disturb / offline` 五种在线状态、独立小尺寸状态点、`compact / read_only` 静态 preview
- 本次删除内容：联系人系统绑定、头像宿主布局、复杂通知组合、场景化故事外壳和额外交互
- EGUI 适配说明：在 `custom` 层新增轻量 `egui_view_presence_badge`，只负责独立状态点的几何和配色，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`PresenceBadge` 用来表达“某个对象当前是否可达、忙碌或离线”的极小面积状态语义。它和 `InfoBadge` 的提醒语义不同，不承载数量或一般告警；它也不是 `PersonPicture` 的附属功能，而是可以单独挂在列表项、卡片角落、消息线程或工作区入口旁边的状态点。

当前仓库虽然已经有 `person_picture` 内部的 presence 点和 `info_badge`，但还缺少一个对齐 Fluent 2 `PresenceBadge` 语义的独立控件，因此需要补齐。

## 2. 为什么现有控件不够用？
- `person_picture` 的 presence 只是头像内部的附属绘制，不提供独立控件 API，也不适合作为任意宿主的通用状态点。
- `info_badge` 更偏数量、图标和 attention dot，不区分在线状态、阻止打扰和离线 ring。
- `badge` 承载的是文本状态标签，不适合替代极小面积的 presence 信号。

## 3. 目标场景与示例概览
- 主面板演示三种核心 snapshot：
  - `Available`
  - `Do not disturb`
  - `Offline`
- 底部保留两个静态 preview：
  - `compact`：更小尺寸的 `away`
  - `read_only`：弱化后的 `busy`
- 页面结构统一收口为：标题 -> 主参考面板 -> 两个静态 preview

目标目录：`example/HelloCustomWidgets/display/presence_badge/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 206`
- 主面板：`196 x 96`
- 主控件：`28 x 28`
- 底部容器：`216 x 68`
- 单个 preview 面板：`104 x 68`
- preview 控件：`18 x 18`

视觉原则：
- 继续保持浅灰页面、白色承载面和低噪音边框。
- `PresenceBadge` 只承担状态点语义，不额外承担列表、头像或命令容器职责。
- `Offline` 采用 ring-only 语义，`Do not disturb` 用减号区分于普通 `busy` 红点。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 96` | 主展示面板 |
| `primary_badge` | `egui_view_presence_badge_t` | `28 x 28` | 主状态点 |
| `compact_badge` | `egui_view_presence_badge_t` | `18 x 18` | 紧凑静态 preview |
| `read_only_badge` | `egui_view_presence_badge_t` | `18 x 18` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `available` | 是 | 否 | 否 |
| `busy` | API 保留 | 否 | 是 |
| `away` | API 保留 | 是 | 否 |
| `do_not_disturb` | 是 | 否 | 否 |
| `offline` | 是 | 否 | 否 |
| `compact_mode` | 关闭 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| 接收焦点 / 交互 | 否 | 否 | 否 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `PresenceBadge` 本轮定义为纯 display 控件，不承担点击、导航或键盘激活职责。
- 主面板的状态切换通过程序化 snapshot 完成，不依赖用户输入。
- 静态 preview 通过 `override_static_preview_api()` 吞掉 `touch / key`，避免 reference 页面里误接管输入。

## 8. 本轮收口内容
- 新增 `egui_view_presence_badge.h/.c`
- 提供：
  - `set_status()` / `get_status()`
  - `set_palette()`
  - `set_compact_mode()` / `get_compact_mode()`
  - `set_read_only_mode()` / `get_read_only_mode()`
  - `get_indicator_region()`
  - `override_static_preview_api()`
- demo 页面接入主状态轮播、compact preview 与 read-only preview
- 单测覆盖默认初始化、状态/配色 setter、区域 helper 和静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Available`
2. 抓取第一帧
3. 切到 `Do not disturb`
4. 抓取第二帧
5. 切到 `Offline`
6. 抓取第三帧
7. 恢复初始状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/presence_badge PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/presence_badge --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/presence_badge
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_presence_badge
```

验收重点：
- 主控件和两个 preview 都必须完整可见，不黑屏、不白屏、不裁切。
- `Available`、`Do not disturb` 和 `Offline` 三个 snapshot 必须有清晰差异。
- `Offline` ring 在浅色背景上不能丢失边界。
- `read_only` preview 需要明显弱化，但仍能辨认出状态色和几何。

## 11. 已知限制
- 当前只覆盖独立状态点，不负责和头像、列表项或命令入口做宿主绑定。
- `away` 先收口为纯色 amber dot，不继续补更复杂的月牙或动画语义。
- 本轮不下沉到 `src/widget/`，先保持在 `HelloCustomWidgets` 的 reference 维护范围内。
