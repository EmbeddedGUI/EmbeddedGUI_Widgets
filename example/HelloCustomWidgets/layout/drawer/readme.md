# Drawer 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Drawer`
- 本次保留语义：`inline / overlay`、`start / end` 锚点、`open / closed` 状态，以及 `compact / read_only` 静态 preview
- 本次删除内容：路由壳层、复杂表单插槽、业务化审批流和与抽屉面板无关的故事化场景
- EGUI 适配说明：在 custom 层绘制轻量 drawer shell、toggle 和 close 语义，用单个 `egui_view_drawer` 收口侧边抽屉的参考展示，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件？
`Drawer` 用来承载和当前页面强相关、但又不适合直接塞进主内容区的附加信息或次级操作。它既可以作为 inline side rail 和页面并排存在，也可以在 overlay 模式下覆盖当前内容，适合过滤器、检查备注、属性编辑和上下文补充。

当前仓库虽然已经有 `split_view`、`dialog_sheet` 和 `flyout`，但还没有一个对齐 Fluent UI React `Drawer` 语义的独立抽屉面板控件，因此需要补齐。

## 2. 为什么现有控件不够用？
- `split_view` 更偏持久双栏布局，不强调抽屉的临时开合语义。
- `dialog_sheet` 更像确认或提交用的模态容器，交互重心在 action button，不是边缘侧抽屉。
- `flyout` 关注的是锚点气泡，不承担大尺寸 side rail 的空间占用和 overlay 语义。

## 3. 目标场景与示例概览
- 主面板录制三组 snapshot：
  - `Filters`：`inline + start + open`
  - `Review`：`overlay + end + open`
  - `Archive`：`inline + end + closed`
- 底部保留两个静态 preview：
  - `compact`：压缩后的 overlay 抽屉
  - `read_only`：弱化后的 inline 抽屉
- 页面结构统一收口为：标题 -> 主 drawer -> 两个静态 preview

目标目录：`example/HelloCustomWidgets/layout/drawer/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 216`
- 主控件：`196 x 112`
- 底部容器：`216 x 72`
- 单个 preview：`104 x 72`

视觉原则：
- 继续保持浅灰页面、白色抽屉面和低噪音边框。
- 用 body 区域的静态 host shell 表达页面上下文，不再扩展真实业务表单。
- 通过 host body 是否缩窄、是否覆盖 veil、以及 toggle 所在边缘来突出 `inline / overlay / anchor` 差异。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 216` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_drawer` | `egui_view_drawer_t` | `196 x 112` | 主抽屉示例 |
| `compact_drawer` | `egui_view_drawer_t` | `104 x 72` | 紧凑静态 preview |
| `read_only_drawer` | `egui_view_drawer_t` | `104 x 72` | 只读静态 preview |

## 6. 状态矩阵
| 状态 / 能力 | 主控件 | Compact preview | Read-only preview |
| --- | --- | --- | --- |
| `inline / overlay` | 是 | 是 | 是 |
| `start / end anchor` | 是 | 是 | 是 |
| `open / closed` | 是 | 是 | 是 |
| `compact_mode` | 关闭 | 开启 | 开启 |
| `read_only_mode` | 关闭 | 关闭 | 开启 |
| `touch / key` 开合 | 是 | 否 | 否 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- 主控件通过 edge `toggle` 负责开关 drawer。
- 打开状态下保留 header `close` 按钮，继续遵循 same-target release 语义。
- 键盘语义：
  - `Enter / Space`：切换开合
  - `Escape`：关闭抽屉
- `compact` 与 `read_only` preview 通过 `override_static_preview_api()` 吞掉输入，不参与真实交互。

## 8. 本轮收口内容
- 新增 `egui_view_drawer.h/.c`
- 提供：
  - 文本 setter：`set_eyebrow()`、`set_title()`、`set_body_primary()`、`set_body_secondary()`、`set_footer()`、`set_tag()`
  - 状态 setter：`set_anchor()`、`set_presentation_mode()`、`set_open()`
  - 外观与模式：`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`
  - helper：`get_part_region()`、`override_static_preview_api()`
- demo 页面接入三组主状态与两个静态 preview
- 单测覆盖默认初始化、setter 清 pressed、same-target touch、键盘开关和静态 preview 输入抑制

## 9. 录制动作设计
1. 还原 `Filters` inline start open
2. 抓取第一帧
3. 切到 `Review` overlay end open
4. 抓取第二帧
5. 切到 `Archive` inline end closed
6. 抓取第三帧
7. 恢复第一组状态并收尾

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/drawer --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/drawer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_drawer
```

验收重点：
- inline 和 overlay 两种主状态都必须能从布局上直接区分。
- `closed` 状态下只剩 toggle，不允许主抽屉残影。
- 两个 preview 必须完整可见，不能裁切、黑屏或白屏。

## 11. 已知限制
- 当前只收口抽屉 shell，不继续承载真实表单控件树或可滚动列表。
- overlay 先表现为低噪音 veil，不做动画和拖拽手势。
- 本轮不下沉到 `src/widget/`，先维持在 `HelloCustomWidgets` 的 reference 维护范围内。
