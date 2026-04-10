# password_box 自定义控件设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`PasswordBox`
- 本次保留状态：`masked`、`revealed`、`compact`、`read only`、`focused`
- 删除效果：页面级 guide、状态文案、standard label、section divider、preview label、标签点击切换、复杂校验提示、Acrylic 与桌面级 hover 装饰
- EGUI 适配说明：保留标准密码字段、右侧 reveal 切换和 `compact / read only` 对照，在 `480 x 480` 里优先保证遮罩文本、光标和图标都清晰可辨

## 1. 为什么需要这个控件
`password_box` 用于表达标准密码输入语义，例如 Wi-Fi 密码、部署密钥、管理员口令等。它比通用 `textinput` 更接近 Fluent / WPF UI 中的安全输入场景，也是当前 `input` 主线里需要保留的一类标准表单控件。

## 2. 为什么现有控件不够用
- `textinput` 只有通用文本编辑，没有密码遮罩和 reveal 入口
- `token_input` 面向多值编辑，不适合单条秘密字段
- `auto_suggest_box` 偏建议输入，不适合安全信息录入
- 当前主线需要一版明确对齐 `Fluent 2 / WPF UI PasswordBox` 的标准 password field reference

## 3. 目标场景与示例概览
- 主区域展示标准 `password_box`，包含 `label`、`helper`、遮罩文本和 reveal 按钮
- 左下 `compact` 预览展示紧凑密码字段
- 右下 `read only` 预览展示只读遮罩字段
- 示例页只保留标题、主 `password_box` 和底部 `compact / read only` 双预览，不再保留 guide、外部状态回显和标签点击
- 录制动作只保留主控件 snapshot 切换、键盘编辑和 reveal 切换，不再点按页面 chrome

目录：
- `example/HelloCustomWidgets/input/password_box/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 154`
- 页面结构：标题 -> 主 `password_box` -> `compact / read only` 双预览
- 主密码框：`196 x 70`
- 底部双预览容器：`216 x 44`
- `compact` 预览：`106 x 44`
- `read only` 预览：`106 x 44`
- 视觉规则：
  - 使用浅灰白 page panel + 白底轻边框 palette
  - 主密码框保留标准表单卡语义，不再叠加页面级说明和状态桥接
  - `compact` 与 `read only` 作为静态对照，不承担额外交互职责
  - accent、边框和文本色统一向 Fluent / WPF UI 的低噪音浅色体系收口

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 154` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Password Box` | 页面标题 |
| `box_primary` | `egui_view_password_box_t` | `196 x 70` | `studio-24` | 标准密码框 |
| `box_compact` | `egui_view_password_box_t` | `106 x 44` | `7429` | 紧凑静态预览 |
| `box_read_only` | `egui_view_password_box_t` | `106 x 44` | `fleet-admin` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主密码框 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `studio-24` 遮罩显示 | `7429` 遮罩显示 | `fleet-admin` 遮罩显示 |
| 键盘编辑 | `Backspace` + `2` | 不响应 | 不响应 |
| reveal 切换 | 明文 / 遮罩切换 | 仅作静态对照 | 不适用 |
| `Tab` / `Space` | field / reveal 闭环切换 | 不响应 | 不响应 |
| snapshot 切换 | `Wi-Fi` -> `Deploy secret` | `7429` -> `A-1709` | 固定 |
| 只读弱化 | 不适用 | 不适用 | 仅保留只读遮罩预览 |

本轮交互收口补充：
- `set_text / clear / current_part / compact / read only / revealed / palette` 切换后，必须同步清掉残留 `pressed_part / is_pressed`
- 底部 `compact / read only` 预览统一通过 static preview API 吞掉 `touch / key` 输入，并立即清理残留 pressed，不再保留旧的只吞 touch 逻辑
- `disabled` 分支继续保留原有 `touch` 返回语义，但同样必须清理残留 `pressed_part / is_pressed`

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 应用默认主 snapshot、`compact` snapshot 和只读预览
2. 请求第一页默认遮罩态截图
3. 点击主密码框聚焦
4. 发送 `Backspace` 和 `2`，验证字段编辑
5. 请求第二页编辑后截图
6. 程序化切换到 `revealed`
7. 请求第三页明文截图
8. 发送 `Tab` 和 `Space`，回到遮罩态
9. 请求第四页截图
10. 程序化切换主 snapshot 到 `Deploy secret`
11. 请求第五页截图
12. 程序化切换 `compact` 预览到第二个静态值
13. 请求最终对照截图

## 8. 编译、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=input/password_box PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/password_box --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主密码框和底部双预览必须完整可见，不能被裁切
- 主卡必须看起来像标准密码输入，而不是自造状态卡片
- 遮罩文本、光标和 reveal 图标要可辨识，不能挤压
- `compact` 和 `read only` 必须是静态对照，不再承担标签切换职责
- setter、guard 和 static preview 触发后，不能残留 `pressed_part / is_pressed` 高亮
- `compact / read only` 预览必须同时吞掉 `touch / key`，交互后也不能误切换 `revealed`、文本或当前部位
- 页面中不再出现 guide、状态文案、standard label、section divider 和外部 preview label

## 9. 已知限制与后续方向
- 当前只覆盖简化键盘闭环，不做完整桌面输入法行为
- 当前不做 caps lock、strength、validation 和错误提示
- 当前 `compact` 与 `read only` 仅作为静态对照，不承载真实交互
- 若后续要沉入框架层，再单独评估与表单校验、凭据管理和 submit 流程的联动

## 10. 与现有控件的重叠分析与差异化边界
- 相比 `textinput`：核心差异是密码遮罩和 reveal 按钮，而不是通用文本编辑
- 相比 `token_input`：核心差异是单值秘密字段，而不是多 token 管理
- 相比 `auto_suggest_box`：核心差异是安全输入，不涉及建议列表或下拉面板
- 相比 `number_box`：本控件表达秘密文本输入，不承担数值范围和步进语义

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态
- 对应组件名：`PasswordBox`
- 本次保留状态：
  - `masked`
  - `revealed`
  - `compact`
  - `read only`
  - `focused`

## 13. 相比参考原型删掉了哪些效果或装饰
- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做标签点击切换和外部状态桥接
- 不做复杂 validation、caps lock、密码强度和系统凭据入口
- 不做 Acrylic、hover 光效和复杂焦点动画

## 14. EGUI 适配时的简化点与约束
- 使用固定 snapshot 与轻量键盘事件，优先保证 `480 x 480` 页面里的可审阅性
- 只保留单一 reveal 入口，不引入浮层或额外辅助提示
- `compact` 与 `read only` 固定放底部双列，便于和主卡直接对照，并统一走 static preview API
- 先完成示例级 password field，再决定是否上升到框架公共控件
