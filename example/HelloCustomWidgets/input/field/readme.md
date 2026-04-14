# field 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件名：`Field`
- 本次保留状态：`label`、`required marker`、`helper text`、`validation message`、`info button / bubble`、`compact`、`read only`
- 本次删除内容：任意 child slot、复杂表单布局编排、页面级引导卡片、额外业务动作区
- EGUI 适配说明：当前仅在 `HelloCustomWidgets` 的 custom 层实现轻量 reference shell，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`Field` 是 Fluent 表单体系里最基础的壳层语义之一。它负责把 `label`、必填标记、辅助说明、校验信息和轻量解释入口组织到同一个字段上下文里，让用户能在不跳出当前表单流程的情况下理解字段含义和状态。

当前仓库虽然已经有 `text_box`、`info_label`、`message_bar` 等单点能力，但还没有一颗明确对齐 `Fluent 2 / Fluent UI React Field` 的 reference 控件，因此需要补齐。

## 2. 为什么现有控件不够用
- `text_box` 只覆盖输入框本体，不承载字段级 `label / helper / validation / info`
- `info_label` 更像标签旁的解释入口，不承担 field shell 的表单组织语义
- `message_bar` 是整条反馈条，不适合贴在字段底部做低噪音校验信息
- SDK 现有基础控件可以复用绘制和交互能力，但仓库里缺少一颗面向 reference 主线的字段级封装

## 3. 本次实现的语义边界
- 当前实现是一个自绘 `field shell`
- 它内部只承载静态 `field box` 预览，而不是通用 child slot 容器
- 交互只保留 `info button` 的展开/收起，不扩展成完整表单框架
- 如果后续需要承载任意输入控件，应在确认复用价值后再讨论是否上升到框架层

## 4. 目标场景与示例概览
- 主区域展示一颗标准 `Field`
- 录制轨道依次覆盖：
  - `default helper`
  - `required + info bubble`
  - `warning`
  - `error`
  - `success`
- 底部左侧 preview 展示 `compact`
- 底部右侧 preview 展示 `read only`
- 页面结构统一为：标题 -> 主 `Field` -> `Compact / Read only` 双 preview

目录：
- `example/HelloCustomWidgets/input/field/`

## 5. 视觉与布局规格
- 根容器尺寸：`224 x 248`
- 主控件尺寸：`196 x 126`
- 底部对照行尺寸：`216 x 96`
- 单个 preview 面板尺寸：`104 x 96`
- 单个 preview 控件尺寸：`84 x 66`

视觉约束：
- 主体保持 Fluent 风格的浅色字段壳层
- `warning / error / success` 只通过边框和说明文字的 tone 表达，不引入高噪音色块
- `info bubble` 保持贴近 label 的轻量 anchored 提示，不升级为 `TeachingTip`
- `compact / read only` 通过控件自身状态表达，不依赖额外页面文案

## 6. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 248` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Field` | 页面标题 |
| `primary_field` | `hcw_field_t` | `196 x 126` | default | 主参考控件 |
| `compact_field` | `hcw_field_t` | `84 x 66` | compact + static preview | 紧凑对照 |
| `read_only_field` | `hcw_field_t` | `84 x 66` | read only + static preview | 只读对照 |

## 7. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `default helper` | 占位文本 + 辅助说明 |
| 主控件 | `required + info` | 必填标记 + info bubble + warning message |
| 主控件 | `warning` | 字段保留值，底部展示 warning |
| 主控件 | `error` | 空值 + placeholder + error message |
| 主控件 | `success` | 已填值 + success message |
| `compact` preview | `compact` | 压缩 label 与 field shell |
| `read only` preview | `read only` | 禁止交互，只保留静态 reference |

## 8. 交互语义
- 主控件只保留 `info button` 的交互
- `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
- `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `Enter / Space` 切换 bubble 开关
- `Esc` 关闭已展开的 bubble
- `read only` 与 `disabled` 状态下，info 入口不响应，但必须清理 pressed 状态
- `compact / read only` preview 通过 `hcw_field_override_static_preview_api()` 吞掉 `touch / key`

## 9. 本轮收口内容
- 新增 `egui_view_field.h/.c`
- 在 custom 层实现：
  - `label`
  - `required marker`
  - `helper text`
  - `validation message`
  - `info button + anchored bubble`
  - `compact / read only`
  - `static preview` 输入吞掉
- 新增 `test.c` 参考页面
- 新增 `HelloUnitTest` 单测覆盖交互闭环与静态预览保护

## 10. 录制动作设计
`egui_port_get_recording_action()` 轨道：
1. 重置主控件与双 preview 到默认状态
2. 截取 `default helper`
3. 切换到 `required + info` 快照
4. 点击 info 入口展开 bubble
5. 截取展开后的字段状态
6. 切换到 `warning`
7. 截取 warning
8. 切换到 `error`
9. 截取 error
10. 切换到 `success`
11. 截取最终稳定帧

## 11. 编译、检查与验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/field PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/field --track reference --timeout 10 --keep-screenshots
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/field
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_field
```

## 12. 验收重点
- 主控件和底部双 preview 必须完整可见，不黑屏、不白屏、不裁切
- `info button` 的展开气泡必须锚定在 label 区域附近
- `warning / error / success` 的 tone 必须可辨认，但不能变成 message bar
- `read only` 与 `static preview` 不允许误触发 open 状态
- 触摸释放语义必须通过同目标提交检查

## 13. 与现有控件的边界
- 相比 `text_box`：这里是字段壳层，不承担真实文本编辑
- 相比 `info_label`：这里把解释入口放回字段上下文，而不是独立标签组件
- 相比 `message_bar`：这里只保留字段级低噪音校验提示，不做整条反馈条
- 相比 `settings_card`：这里不承载复合布局和附加 action

## 14. 对应组件名与保留状态
- 对应组件名：`Field`
- 本次保留核心状态：
  - `default`
  - `required`
  - `helper`
  - `warning`
  - `error`
  - `success`
  - `info bubble`
  - `compact`
  - `read only`

## 15. EGUI 适配时的简化点与限制
- 当前字段框体是静态 `field box`，不承载任意 child slot
- 当前 bubble 为控件内自绘 anchored bubble，不引入系统级 popup
- 当前交互只围绕 info 入口，不承担真实表单提交逻辑
- 先在 `HelloCustomWidgets` 内完成 reference 闭环，再决定是否需要上升到更通用的框架能力
