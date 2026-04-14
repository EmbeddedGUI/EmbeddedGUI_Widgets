# drawer 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Drawer`
- 本次保留语义：`inline / overlay`、`start / end anchor`、`open / closed`、`compact`、`read only`
- 本次删除内容：路由壳层、复杂表单插槽、业务化审批流和与抽屉面板无关的故事化场景
- EGUI 适配说明：继续复用仓库内 `drawer` 基础实现，本轮只收口 `reference` 页面结构、主控件录制轨道和静态 preview 语义，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`drawer` 用来承载和当前页面强相关、但又不适合直接塞进主内容区的附加信息或次级操作。它既可以作为 inline side rail 和页面并排存在，也可以在 overlay 模式下覆盖当前内容，适合过滤器、检查备注、属性编辑和上下文补充。

## 2. 为什么现有控件不够用
- `split_view` 更偏持久双栏布局，不强调抽屉的临时开合语义。
- `dialog_sheet` 更像确认或提交用的模态容器，交互重心在 action button，不是边缘侧抽屉。
- `flyout` 关注的是锚点气泡，不承担大尺寸 side rail 的空间占用和 overlay 语义。

## 3. 目标场景与示例概览
- 主控件：保留真实 `Drawer` 语义，展示 `Filters`、`Review`、`Archive` 三组 `inline / overlay / anchor / open` 状态。
- `compact` 预览：压缩为小尺寸 overlay 抽屉，只作为静态 reference 对照。
- `read only` 预览：保留冻结态 inline 抽屉和弱化 palette，只作为静态 reference 对照。
- 页面只保留标题、主 `drawer` 和底部 `compact / read only` 双 preview，不再保留额外 section label、外部说明壳和 preview 交互桥接。
- 底部两个 preview 统一通过 `egui_view_drawer_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只负责清理残留 `pressed`
  - 不修改 `anchor / presentation_mode / open / compact_mode / read_only_mode`
  - 不触发 `on_open_changed`

目标目录：`example/HelloCustomWidgets/layout/drawer/`

## 4. 视觉与布局规格
- 根布局：`224 x 216`
- 主控件：`196 x 112`
- 底部对照行：`216 x 72`
- `compact` 预览：`104 x 72`
- `read only` 预览：`104 x 72`
- 页面结构：标题 -> 主 `drawer` -> `compact / read only`
- 样式约束：
  - 继续保持浅灰页面、白色抽屉面和低噪音边框。
  - 用 body 区域的静态 host shell 表达页面上下文，不扩展真实业务表单。
  - 通过 host body 是否缩窄、是否覆盖 veil、以及 toggle 所在边缘来突出 `inline / overlay / anchor` 差异。
  - 底部两个 preview 固定为静态 reference 对照，不再承担点击桥接、焦点收尾或额外轨道切换职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_drawer` | `egui_view_drawer_t` | `196 x 112` | `Filters` | 主 `Drawer` |
| `compact_drawer` | `egui_view_drawer_t` | `104 x 72` | `Quick` | 紧凑静态对照 |
| `read_only_drawer` | `egui_view_drawer_t` | `104 x 72` | `Read` | 只读静态对照 |
| `primary_snapshots` | `drawer_demo_snapshot_t[3]` | - | `Filters / Review / Archive` | 主控件录制轨道 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Filters` | `inline + start + open`，默认状态 |
| 主控件 | `Review` | `overlay + end + open`，验证 veil 与 end anchor |
| 主控件 | `Archive` | `inline + end + closed`，验证收口后只保留 toggle |
| `compact` | `Quick` | 固定静态对照，验证紧凑 overlay 抽屉 |
| `read only` | `Read` | 固定静态对照，验证只读渲染与输入屏蔽 |

## 7. 交互语义与 preview 收口
- 主控件通过 edge `toggle` 负责开关 drawer。
- 打开状态下保留 header `close` 按钮，并继续遵循 same-target release 语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- 键盘语义：
  - `Enter / Space`：切换开合
  - `Escape`：关闭抽屉
- `set_title()`、`set_anchor()`、`set_presentation_mode()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()`、`set_open()`、`read only`、`!enable` 都必须先清理残留 `pressed`。
- 底部 `compact / read only` preview 固定为静态 reference 对照，不再承担焦点桥接或额外页面交互职责。

## 8. 录制动作设计
`egui_port_get_recording_action()` 的录制顺序如下：
1. 重置主控件与底部 `compact / read only` preview，输出默认 `Filters`。
2. 切到 `Review`，输出 `overlay + end + open`。
3. 切到 `Archive`，输出 `inline + end + closed`。
4. 恢复主控件默认状态，输出最终稳定帧。

录制只导出主控件的状态变化。底部 `compact / read only` preview 在整条 reference 轨道中保持静态，不承担点击桥接、轨道切换或收尾职责。

## 9. 编译、运行时、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/drawer PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/drawer --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category layout --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/drawer
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_drawer
```

验收重点：
- inline 和 overlay 两种主状态都必须能从布局上直接区分。
- `closed` 状态下只剩 toggle，不允许主抽屉残影。
- `same-target release / read_only / !enable / static preview` 必须全部通过单测。
- 两个 preview 必须完整可见，不能裁切、黑屏或白屏，并且在所有 runtime 帧里保持静态一致。

## 10. 已知限制与后续方向
- 当前只收口抽屉 shell，不继续承载真实表单控件树或可滚动列表。
- overlay 先表现为低噪音 veil，不做动画和拖拽手势。
- 当前不下沉到 `src/widget/`，先维持在 `HelloCustomWidgets` 的 reference 维护范围内。

## 11. 与现有控件的边界
- 相比 `split_view`：这里强调临时开合抽屉，而不是持久双栏布局。
- 相比 `dialog_sheet`：这里是边缘侧抽屉，不是动作确认容器。
- 相比 `flyout`：这里承载的是 side rail 面板，而不是锚点气泡。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `inline / overlay`
  - `start / end anchor`
  - `open / closed`
  - `compact`
  - `read only`
- 保留的交互：
  - 触摸 `toggle / close`
  - 键盘 `Enter / Space / Escape`
- 删除的装饰或桥接：
  - 外部路由壳层、复杂表单插槽和业务化审批流
  - preview 点击桥接与额外页面说明壳
  - 与抽屉面板无关的故事化场景
