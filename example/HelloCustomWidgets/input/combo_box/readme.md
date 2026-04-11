# combo_box 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`WinUI`
- 对应组件名：`ComboBox`
- 本次保留状态：`standard`、`compact`、`read only`、`expanded`、`focused`
- 删除效果：页面级 `guide`、状态文案、外部 preview 标签、重装饰阴影、额外说明壳层
- EGUI 适配说明：直接复用 SDK 里的 `combobox` 基础交互，本轮只在 custom 层补齐 Fluent reference 样式、静态 preview 输入吞掉和 reference 页面录制

## 1. 为什么需要这个控件
`combo_box` 用来表达“在固定候选项里选择一个当前值”的标准表单语义，适合环境切换、筛选模式、时间范围、布局密度、默认模板这类离散单选场景。它比命令按钮更像输入控件，也比建议输入更强调“当前已选值”。

## 2. 为什么现有控件不够用
- `auto_suggest_box` 更强调建议列表与搜索/匹配语义，不适合作为纯粹的当前值选择器。
- `drop_down_button` 与 `split_button` 更接近命令入口，不承担表单字段语义。
- SDK 自带 `combobox` 示例偏基础 API 验证，缺少统一的 Fluent `reference` 页面。

因此这里补上一版 `combo_box` reference，把标准 `ComboBox` 放回 `HelloCustomWidgets/input` 主线。

## 3. 目标场景与示例概览
- 主区域展示一个标准 `combo_box`，覆盖工作区、视图模式和时间范围这类固定候选选择。
- 左下 `compact` preview 展示紧凑版本的静态对照。
- 右下 `read only` preview 展示只读版本的静态对照。
- 页面只保留标题、主控件和底部 `compact / read only` 双 preview，不再保留额外说明 chrome。

目录：
- `example/HelloCustomWidgets/input/combo_box/`

## 4. 视觉与布局规格
- 页面尺寸：`480 x 480`
- 根布局：`224 x 206`
- 页面结构：标题 -> 主 `combo_box` -> `compact / read only` 双 preview
- 主控件尺寸：`196 x 34`
- 底部双 preview 行：`216 x 28`
- 单个 preview：`104 x 28`

视觉约束：
- 保持浅色 page panel、白色字段面和轻边框。
- 展开列表只保留单层白底与低噪音强调，不做 showcase 式重装饰。
- focus 维持轻量边框强调，不做 glow。
- `compact` 只收紧尺寸和可见项数量，不改变语义。
- `read only` 保留当前值展示，但不再承担读写交互职责。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 206` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Combo Box` | 页面标题 |
| `control_primary` | `egui_view_combobox_t` | `196 x 34` | `Work` | 主选择框 |
| `control_compact` | `egui_view_combobox_t` | `104 x 28` | `Auto` | 紧凑静态预览 |
| `control_read_only` | `egui_view_combobox_t` | `104 x 28` | `Tablet` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | 是 | 是 | 是 |
| 展开态 | 是 | 否 | 否 |
| 键盘选择 | 是 | 否 | 否 |
| 静态 preview 对照 | 否 | 是 | 是 |
| focus ring | 是 | 否 | 否 |

## 7. 交互语义
- `Down`：未展开时展开；展开后移动到下一项。
- `Up`：展开后移动到上一项。
- `Home / End`：跳到第一项 / 最后一项。
- `Enter / Space`：未展开时展开；展开时提交当前项并收起。
- `Escape`：收起下拉列表。
- `DOWN(A) -> UP(A)` 才允许展开、收起或提交当前项。
- `DOWN(A) -> UP(B)` 不允许误提交。
- 底部 `compact / read only` preview 通过 `hcw_combo_box_override_static_preview_api()` 统一吞掉 `touch / key`，输入到达时立即清理残留 pressed 与展开态。

## 8. 本轮收口内容
- 新增 `egui_view_combo_box.h/.c`，作为 SDK `combobox` 的 Fluent reference 样式包装层。
- 在包装层里统一补齐：
  - `set_items()`
  - `set_current_index()`
  - `standard / compact / read only` 样式 helper
  - `static preview` 输入吞掉与状态清理
- 新增 `test.c` reference 页面，保留主控件真实键盘交互和底部双 preview 静态对照。
- 新增 `HelloUnitTest` 中对应包装层测试，覆盖样式 helper、same-target release、键盘提交和静态 preview 清理。

## 9. 录制动作设计
`egui_port_get_recording_action()` 的录制链路：
1. 还原主控件、`compact` 和 `read only` 默认快照，并给主控件 request focus
2. 截默认态
3. 发送 `Down` 展开
4. 截展开态
5. 再次发送 `Down` 切到下一项
6. 截键盘导航态
7. 发送 `Space` 提交并收起
8. 截提交结果
9. 切换到第二组主快照并重新 request focus
10. 发送 `Enter` 展开，再发送 `End + Space` 选中最后一项
11. 截第二组结果
12. 切换 `compact` preview 快照并截最终对照帧

## 10. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/combo_box PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/combo_box --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件展开列表、当前值和底部双 preview 必须完整可见。
- 展开列表不能黑屏、白屏或被裁切。
- `Space / Enter / Escape / Home / End / Up / Down` 的键盘闭环必须稳定。
- `compact / read only` preview 必须吞掉输入并立即清理残留 pressed / expanded 状态。

## 11. 已知限制
- 当前版本只做固定候选项选择，不做自由输入和过滤。
- 不做多列 item 模板、分组标题或异步数据源。
- 不做额外 placeholder、leading icon 或搜索联想。

## 12. 与现有控件的差异边界
- 相比 `auto_suggest_box`：这里强调当前值与固定候选，不强调搜索建议。
- 相比 `drop_down_button` / `split_button`：这里是表单字段，不是命令入口。
- 相比 SDK `combobox` 示例：这里强调 Fluent reference 页面和静态 preview 收口，而不是基础 API 演示。

## 13. EGUI 适配时的简化点与约束
- 直接复用 SDK `combobox` 的基础绘制和交互，避免重复实现底层列表展开逻辑。
- 通过 custom wrapper 统一收口样式、尺寸和静态 preview 语义。
- 先完成 reference 级 `ComboBox`，再决定是否补更复杂的 item 模板或只读字段 chrome。
