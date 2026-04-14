# `list` 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`List`
- 当前保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 当前保留能力：单列条目、单选高亮、同目标释放提交、键盘导航、静态 preview
- 本轮删除内容：页面级 snapshot header/footer、双栏摘要、复杂业务卡片、额外 showcase 文案和无关装饰
- EGUI 适配说明：目录和 demo 仍然使用 `layout/list`，公开 C API 改为 `egui_view_reference_list_*`，避免与 SDK 自带 `egui_view_list_*` 符号冲突

## 1. 为什么需要这个控件
`list` 用来表达“同一组轻量条目按单列顺序排列，当前只有一个焦点项，用户可以快速浏览并切换选择”的 Fluent 2 `List` 语义。它适合消息队列、审核清单、归档入口、资源面板侧栏和小规模任务摘要这类场景。

当前仓库已有更重的列表类 reference 控件，但还缺一个贴近 Fluent 2 `List` 的轻量单列选择组件。这个控件补的是“低噪音、单列、单选、可嵌入”的空位。

## 2. 为什么现有控件不够用
- `data_list_panel` 更接近页面级 `ListView` 摘要面板，保留了 snapshot、标题层级和 footer，不适合当前这种轻量单列列表。
- `items_repeater` 更强调重复布局宿主，不承担单选高亮、键盘焦点和列表语义。
- SDK 自带 `List` 更偏基础控件能力，不是这个仓库当前要维护的 Fluent 2 reference 表达。
- `settings_panel`、`master_detail` 这类控件承担的是设置页或主从布局职责，不适合充当小型列表本体。

## 3. 目标场景与示例概览
- 页面只保留标题、一个可交互主 `list`，以及底部两个静态 preview。
- 主控件录制三组状态：
  - `Inbox`：accent 选中项
  - `Review`：warning 选中项
  - `Archive`：warning/neutral 混合条目
- 底部保留两个静态 reference：
  - `compact`：压缩条目高度，保留最小行语义
  - `read only`：弱化视觉并吞掉输入

目录：`example/HelloCustomWidgets/layout/list/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 224`
- 主控件：`196 x 112`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`

视觉原则：
- 使用浅灰页面底板、白色列表卡片和低噪音分隔线。
- 用单列行高、左侧 tone 指示和右侧 badge 表达 Fluent 2 `List` 的轻量层次。
- 标准态保留 `title + meta + badge` 三层信息；`compact` 只保留最核心行语义。
- `read only` 通过色彩弱化而不是外部标签来表达静态参考态。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 (W x H) | 用途 |
| --- | --- | ---: | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 224` | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_list` | `egui_view_reference_list_t` | `196 x 112` | 主交互列表 |
| `compact_list` | `egui_view_reference_list_t` | `104 x 72` | 紧凑静态 preview |
| `read_only_list` | `egui_view_reference_list_t` | `104 x 72` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 单列条目 | 是 | 是 | 是 |
| 单选高亮 | 是 | 是 | 是 |
| `compact_mode` | 关闭 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| `touch / key` 交互 | 是 | 否 | 否 |
| 静态 preview 吞输入 | 否 | 是 | 是 |
| tone badge | 是 | 是 | 是 |

## 7. 交互语义
- 触摸遵循 same-target release：只有 `DOWN(A) -> ... -> UP(A)` 才提交选择。
- `ACTION_MOVE` 只更新 pressed 呈现，不改提交目标。
- 键盘支持：
  - `Up / Left`：上一项
  - `Down / Right`：下一项
  - `Home / End`：首项 / 末项
  - `Tab`：循环到下一项
  - `Enter / Space`：仅消费，不重复通知当前项
- `read_only` 或 `!enable` 时必须清掉残留 pressed，并忽略输入。
- 静态 preview 通过 `egui_view_reference_list_override_static_preview_api()` 吞掉 `touch / key`，只保留视觉参考。

## 8. 本轮收口内容
- 新增 `egui_view_list.h/.c`
- 提供：
  - 数据与选择：`egui_view_reference_list_set_items()`、`get_item_count()`、`set_current_index()`、`get_current_index()`
  - 监听：`set_on_selection_changed_listener()`
  - 外观：`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`
  - helper：`get_item_region()`、`override_static_preview_api()`
- demo 页面接入主列表、`compact` preview 和 `read only` preview
- 单测覆盖：
  - setter 清 pressed
  - 选择与 helper
  - same-target touch
  - 键盘导航
  - 静态 preview 吞输入

## 9. 录制动作设计
1. 还原 `Inbox` 默认状态
2. 抓取第一帧
3. 切换到 `Review`
4. 抓取第二帧
5. 切换到 `Archive`
6. 抓取第三帧
7. 回到 `Inbox`，把选中项移到第 3 行
8. 抓取收尾帧

## 10. 编译、单测与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/list PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/list --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/list
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_list
```

验收重点：
- 主列表和两个 preview 都必须完整可见，不黑屏、不白屏、不裁切。
- 选中行、badge、tone 指示和分隔层次要能直接区分。
- `compact` preview 需要保持单列列表语义，而不是退化成纯文本块。
- 静态 preview 不参与真实输入，主控件保留 same-target release 和键盘切换。

## 11. 已知限制
- 当前是固定尺寸 reference 实现，不承担长列表滚动、虚拟化和数据源绑定。
- 当前每项只保留 `title / meta / badge / tone` 四类表达，不扩展复杂 trailing widgets。
- 当前没有引入多选、拖拽排序、分组 header 和嵌套层级。
- 如需进一步沉到框架层，应单独评估与 SDK 原生 `List` 的边界和命名关系。
