# info_label 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`InfoLabel`
- 本次保留状态：`label`、`info button`、`anchored bubble / popover`、`compact`、`read only`
- 本次删除内容：`TeachingTip` 式重型引导、复杂页面级讲解、额外故事化卡片与高噪音装饰
- EGUI 适配说明：当前仅在 `HelloCustomWidgets` 的 custom 层实现轻量 wrapper，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`InfoLabel` 用于在正文标签旁边提供一个低打扰的信息入口。用户可以在需要时展开一段简短说明，而不是被更重的提示卡片或整行反馈打断。它适合设置项、数据字段、只读说明和表单上下文提示。

当前仓库的 `reference` 主线里还没有一个对齐 `Fluent 2 / Fluent UI React InfoLabel` 语义的控件，因此需要补齐这一类轻量说明入口。

## 2. 为什么现有控件不够用
- `tool_tip` 更像短暂悬停提示，强调 target + hint，不承担标签旁边常驻信息入口的语义。
- `teaching_tip` 信息量更大，层级更高，适合教学和引导，不适合这里的轻量说明。
- `text_block` 只能静态显示文本，无法承担按需展开的说明气泡。
- `info_badge` 表达的是状态或数量提醒，不表达“标签解释”。

## 3. 目标场景与示例概览
- 主控件展示标准 `InfoLabel`，默认收起。
- 录制轨道依次覆盖 `closed accent`、`open accent`、`open warning`、`open neutral` 三组关键状态。
- 底部左侧 preview 展示 `compact` 的静态展开态。
- 底部右侧 preview 展示 `read only` 的静态展开态。
- 页面结构统一为：标题 -> 主 `InfoLabel` -> `Compact / Read only` 双 preview。

目标目录：`example/HelloCustomWidgets/display/info_label/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 228`
- 主控件尺寸：`196 x 96`
- 底部对照行尺寸：`216 x 88`
- 单个 preview 面板尺寸：`104 x 88`
- preview 控件尺寸：`84 x 54`
- 视觉约束：
  - 页面背景保持浅灰低噪音。
  - 主控件保持白底、柔和描边和轻量气泡阴影。
  - `accent / warning / neutral` 仅通过强调色和气泡语气变化，不引入额外大面积装饰。
  - `compact / read only` 通过控件自身样式表达，不依赖外部说明标签。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 228` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `InfoLabel` | 页面标题 |
| `primary_info_label` | `hcw_info_label_t` | `196 x 96` | closed | 主参考控件 |
| `compact_info_label` | `hcw_info_label_t` | `84 x 54` | compact + open | 紧凑静态 preview |
| `read_only_info_label` | `hcw_info_label_t` | `84 x 54` | read only + open | 只读静态 preview |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Project policy` | 默认 `accent`，初始关闭 |
| 主控件 | `Project policy open` | 点击信息按钮后展开 |
| 主控件 | `Export guidance` | `warning` 语气展开态 |
| 主控件 | `Reading help` | `neutral` 语气展开态 |
| `compact` preview | `Compact help` | 固定紧凑展开态 |
| `read only` preview | `Audit note` | 固定只读展开态，吞掉输入 |

## 7. 录制动作设计
1. 重置主控件和两个 preview 到默认状态。
2. 请求默认关闭态截图。
3. 点击主控件的信息按钮，展开 `accent` 气泡。
4. 请求展开后的第二张截图。
5. 切换到 `warning` 快照并保持展开。
6. 请求第三张截图。
7. 切换到 `neutral` 快照并保持展开。
8. 请求最终稳定截图。

## 8. 编译、单测、touch、runtime 与 web 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/info_label PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/info_label --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/info_label
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_info_label
```

## 9. 验收重点
- 主控件关闭态和展开态都必须完整可见，不能裁切。
- 气泡必须锚定在信息按钮附近，而不是漂浮成页面级卡片。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能切换开关。
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才允许提交。
- `Enter / Space` 必须切换开关，`Esc` 必须关闭已展开的气泡。
- 静态 preview 必须吞掉 `touch / key`，同时保持当前展开状态。

## 10. 与现有控件的边界
- 相比 `tool_tip`：这里强调 label 附近的解释入口，而不是独立 target 的悬停提示。
- 相比 `teaching_tip`：这里不承担教学卡片或高层级引导。
- 相比 `text_block`：这里保留按需展开的轻交互语义。
- 相比 `info_badge`：这里表达的是解释信息，不是状态角标。

## 11. 对应组件名与本次保留的核心状态
- 对应组件名：`InfoLabel`
- 本次保留核心状态：
  - `closed`
  - `open`
  - `accent / warning / neutral`
  - `compact`
  - `read only`

## 12. 删除的效果或装饰
- 删除额外的故事化页面文案和无关说明卡片。
- 删除大阴影、重浮层和过强动画。
- 删除 `TeachingTip` 式引导动作与复杂按钮区。

## 13. EGUI 适配时的简化点与限制
- 气泡采用控件内自绘的 anchored bubble，而不是系统级 popup。
- 只保留单按钮展开/收起，不做复杂定位避让。
- `compact` 与 `read only` 直接复用同一个控件 API，通过模式切换完成 reference 对照。
- 当前不下沉到 `src/widget/`，先保持在 `HelloCustomWidgets` 的 reference 维护范围内。
